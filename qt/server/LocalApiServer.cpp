#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>
#include <QVariant>

namespace {

QString nowIso(){
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
}

QString passwordHash(const QString& password){
    return QString::fromLatin1(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString safeSegment(const QString& raw){
    const QString decoded = QUrl::fromPercentEncoding(raw.toUtf8());
    if(decoded.isEmpty() || decoded.contains('/') || decoded.contains('\\') || decoded == "." || decoded == ".."){
        return {};
    }
    return decoded;
}

QByteArray jsonBytes(const QJsonObject& object){
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

QByteArray response(int status, const QByteArray& contentType, const QByteArray& body){
    QByteArray statusText = "OK";
    if(status == 400) statusText = "Bad Request";
    if(status == 404) statusText = "Not Found";
    if(status == 409) statusText = "Conflict";
    if(status == 500) statusText = "Internal Server Error";

    return "HTTP/1.1 " + QByteArray::number(status) + " " + statusText +
           "\r\nContent-Type: " + contentType +
           "\r\nContent-Length: " + QByteArray::number(body.size()) +
           "\r\nConnection: close\r\n\r\n" + body;
}

QByteArray jsonResponse(const QJsonObject& object, int status=200){
    return response(status, "application/json; charset=utf-8", jsonBytes(object));
}

QByteArray jsonResponse(const QJsonArray& array, int status=200){
    const QByteArray body = QJsonDocument(array).toJson(QJsonDocument::Compact);
    return response(status, "application/json; charset=utf-8", body);
}

class LocalApiServer : public QTcpServer {
    Q_OBJECT

public:
    explicit LocalApiServer(QObject* parent=nullptr): QTcpServer(parent) {}

    bool initializeDatabase(const QString& path){
        QDir().mkpath(QFileInfo(path).absolutePath());
        db_ = QSqlDatabase::addDatabase("QSQLITE", "local_api_server");
        db_.setDatabaseName(path);
        if(!db_.open()){
            qWarning("Failed to open SQLite database");
            return false;
        }

        QSqlQuery query(db_);
        const bool usersOk = query.exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT NOT NULL UNIQUE,"
            "password_hash TEXT NOT NULL,"
            "real_name TEXT,"
            "gender TEXT,"
            "age INTEGER,"
            "phone TEXT,"
            "email TEXT,"
            "school TEXT,"
            "major TEXT,"
            "created_at TEXT NOT NULL,"
            "updated_at TEXT NOT NULL"
            ")"
        );
        const bool filesOk = query.exec(
            "CREATE TABLE IF NOT EXISTS cloud_files ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "user_id INTEGER NOT NULL,"
            "filename TEXT NOT NULL,"
            "content BLOB NOT NULL,"
            "size INTEGER NOT NULL,"
            "updated_at TEXT NOT NULL,"
            "UNIQUE(user_id, filename),"
            "FOREIGN KEY(user_id) REFERENCES users(id)"
            ")"
        );
        return usersOk && filesOk;
    }

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        auto* socket = new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);
        connect(socket, &QTcpSocket::readyRead, this, [this, socket](){
            buffers_[socket] += socket->readAll();
            const QByteArray data = buffers_.value(socket);
            const int headerEnd = data.indexOf("\r\n\r\n");
            if(headerEnd < 0) return;

            const QByteArray headers = data.left(headerEnd);
            int contentLength = 0;
            for(const QByteArray& line : headers.split('\n')){
                const QByteArray trimmed = line.trimmed();
                if(trimmed.toLower().startsWith("content-length:")){
                    contentLength = trimmed.mid(15).trimmed().toInt();
                }
            }
            if(data.size() < headerEnd + 4 + contentLength) return;

            const QByteArray body = data.mid(headerEnd + 4, contentLength);
            const QList<QByteArray> requestLine = headers.split('\n').first().trimmed().split(' ');
            const QString method = QString::fromLatin1(requestLine.value(0));
            const QString path = QString::fromLatin1(requestLine.value(1));
            socket->write(handleRequest(method, path, body));
            socket->disconnectFromHost();
            buffers_.remove(socket);
        });
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }

private:
    QByteArray handleRequest(const QString& method, const QString& rawPath, const QByteArray& body){
        const QString path = rawPath.section('?', 0, 0);
        const QStringList parts = path.split('/', Qt::SkipEmptyParts);

        if(method == "GET" && path == "/health"){
            return jsonResponse(QJsonObject{{"status", "ok"}});
        }
        if(method == "POST" && path == "/api/register"){
            return registerUser(body);
        }
        if(method == "POST" && path == "/api/login"){
            return loginUser(body);
        }
        if(parts.size() == 5 && method == "POST" && parts[0] == "api" && parts[1] == "users" &&
           parts[3] == "files" && parts[4] == "upload"){
            return uploadFile(parts[2].toInt(), body);
        }
        if(parts.size() == 4 && method == "GET" && parts[0] == "api" && parts[1] == "users" &&
           parts[3] == "files"){
            return listFiles(parts[2].toInt());
        }
        if(parts.size() == 5 && method == "GET" && parts[0] == "api" && parts[1] == "users" &&
           parts[3] == "files"){
            return downloadFile(parts[2].toInt(), safeSegment(parts[4]));
        }
        return jsonResponse(QJsonObject{{"ok", false}, {"error", "接口不存在"}}, 404);
    }

    QByteArray registerUser(const QByteArray& body){
        const QJsonObject root = QJsonDocument::fromJson(body).object();
        const QString username = root.value("username").toString().trimmed();
        const QString password = root.value("password").toString();
        if(username.isEmpty() || password.isEmpty()){
            return jsonResponse(QJsonObject{{"ok", false}, {"error", "用户名和密码为必填项"}}, 400);
        }

        QSqlQuery query(db_);
        query.prepare(
            "INSERT INTO users (username, password_hash, real_name, gender, age, phone, email, school, major, created_at, updated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );
        query.addBindValue(username);
        query.addBindValue(passwordHash(password));
        query.addBindValue(root.value("realName").toString());
        query.addBindValue(root.value("gender").toString());
        query.addBindValue(root.value("age").toInt());
        query.addBindValue(root.value("phone").toString());
        query.addBindValue(root.value("email").toString());
        query.addBindValue(root.value("school").toString());
        query.addBindValue(root.value("major").toString());
        query.addBindValue(nowIso());
        query.addBindValue(nowIso());
        if(!query.exec()){
            return jsonResponse(QJsonObject{{"ok", false}, {"error", "用户名已存在或数据库写入失败"}}, 409);
        }
        return jsonResponse(QJsonObject{{"ok", true}, {"id", query.lastInsertId().toInt()}});
    }

    QByteArray loginUser(const QByteArray& body){
        const QJsonObject root = QJsonDocument::fromJson(body).object();
        const QString username = root.value("username").toString().trimmed();
        const QString password = root.value("password").toString();

        QSqlQuery query(db_);
        query.prepare("SELECT id, username, real_name, school, major FROM users WHERE username = ? AND password_hash = ?");
        query.addBindValue(username);
        query.addBindValue(passwordHash(password));
        if(!query.exec() || !query.next()){
            return jsonResponse(QJsonObject{{"ok", false}, {"error", "用户名或密码错误"}}, 400);
        }

        QJsonObject user;
        user.insert("id", query.value(0).toInt());
        user.insert("username", query.value(1).toString());
        user.insert("realName", query.value(2).toString());
        user.insert("school", query.value(3).toString());
        user.insert("major", query.value(4).toString());
        return jsonResponse(QJsonObject{{"ok", true}, {"user", user}});
    }

    QByteArray uploadFile(int userId, const QByteArray& body){
        const QJsonObject root = QJsonDocument::fromJson(body).object();
        const QString filename = safeSegment(root.value("filename").toString());
        const QByteArray content = QByteArray::fromBase64(root.value("contentBase64").toString().toLatin1());
        if(userId <= 0 || filename.isEmpty()){
            return jsonResponse(QJsonObject{{"ok", false}, {"error", "上传参数无效"}}, 400);
        }

        QSqlQuery query(db_);
        query.prepare(
            "INSERT INTO cloud_files (user_id, filename, content, size, updated_at) VALUES (?, ?, ?, ?, ?) "
            "ON CONFLICT(user_id, filename) DO UPDATE SET content = excluded.content, size = excluded.size, updated_at = excluded.updated_at"
        );
        query.addBindValue(userId);
        query.addBindValue(filename);
        query.addBindValue(content);
        query.addBindValue(content.size());
        query.addBindValue(nowIso());
        if(!query.exec()){
            return jsonResponse(QJsonObject{{"ok", false}, {"error", "文件保存失败"}}, 500);
        }
        return jsonResponse(QJsonObject{{"ok", true}, {"filename", filename}, {"size", content.size()}});
    }

    QByteArray listFiles(int userId){
        QSqlQuery query(db_);
        query.prepare("SELECT filename, size, updated_at FROM cloud_files WHERE user_id = ? ORDER BY filename");
        query.addBindValue(userId);
        if(!query.exec()){
            return jsonResponse(QJsonObject{{"ok", false}, {"error", "文件列表读取失败"}}, 500);
        }
        QJsonArray files;
        while(query.next()){
            files.append(QJsonObject{
                {"filename", query.value(0).toString()},
                {"size", query.value(1).toInt()},
                {"modified", query.value(2).toString()}
            });
        }
        return jsonResponse(files);
    }

    QByteArray downloadFile(int userId, const QString& filename){
        if(filename.isEmpty()){
            return jsonResponse(QJsonObject{{"ok", false}, {"error", "文件名无效"}}, 400);
        }
        QSqlQuery query(db_);
        query.prepare("SELECT content FROM cloud_files WHERE user_id = ? AND filename = ?");
        query.addBindValue(userId);
        query.addBindValue(filename);
        if(!query.exec() || !query.next()){
            return jsonResponse(QJsonObject{{"ok", false}, {"error", "文件不存在"}}, 404);
        }
        const QByteArray body = query.value(0).toByteArray();
        return response(200, "application/octet-stream", body);
    }

    QSqlDatabase db_;
    QHash<QTcpSocket*, QByteArray> buffers_;
};

} // namespace

int main(int argc, char* argv[]){
    QCoreApplication app(argc, argv);
    QString dbPath = QCoreApplication::applicationDirPath() + "/local_cloud.db";
    int port = 18080;

    const QStringList args = app.arguments();
    for(int i = 1; i < args.size(); ++i){
        if(args[i] == "--db" && i + 1 < args.size()) dbPath = args[++i];
        else if(args[i] == "--port" && i + 1 < args.size()) port = args[++i].toInt();
    }

    LocalApiServer server;
    if(!server.initializeDatabase(dbPath)){
        return 1;
    }
    if(!server.listen(QHostAddress::LocalHost, quint16(port))){
        return 2;
    }

    qInfo("Local API server listening on http://127.0.0.1:%d", port);
    qInfo("SQLite database: %s", qPrintable(dbPath));
    return app.exec();
}

#include "LocalApiServer.moc"
