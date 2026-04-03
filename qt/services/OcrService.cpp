#include "OcrService.h"
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QFileInfo>
#include <QRegularExpression>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QImage>
#include <QBuffer>
#include <algorithm>
#include "PaddleInferOcr.h"

static QString baiduApiKey(){ return qEnvironmentVariable("BAIDU_OCR_API_KEY"); }
static QString baiduSecretKey(){ return qEnvironmentVariable("BAIDU_OCR_SECRET_KEY"); }
static bool hasBaiduCreds(){ return !baiduApiKey().trimmed().isEmpty() && !baiduSecretKey().trimmed().isEmpty(); }

static QString configuredPython(){
#ifdef OCR_DEFAULT_PYTHON_EXE
    QString p = QDir::fromNativeSeparators(QString::fromUtf8(OCR_DEFAULT_PYTHON_EXE));
    if(QFile::exists(p)) return p;
#endif
    return QString();
}

static QString configuredSitePackages(){
#ifdef OCR_DEFAULT_PYTHON_SITE_PACKAGES
    QString p = QDir::fromNativeSeparators(QString::fromUtf8(OCR_DEFAULT_PYTHON_SITE_PACKAGES));
    if(QDir(p).exists()) return p;
#endif
    return QString();
}

static QString configuredScript(){
#ifdef OCR_DEFAULT_PADDLE_SCRIPT
    QString p = QDir::fromNativeSeparators(QString::fromUtf8(OCR_DEFAULT_PADDLE_SCRIPT));
    if(QFile::exists(p)) return p;
#endif
    return QString();
}

static QString guessExe(){
    QString base = QDir::fromNativeSeparators(QDir::cleanPath(QCoreApplication::applicationDirPath()+"/../include/bin"));
    QString exe = base + "/tesseract.exe";
    if(QFile::exists(exe)) return exe;
    return QString();
}

static QString guessPaddleScript(){
    QString appDir = QDir::fromNativeSeparators(QCoreApplication::applicationDirPath());
    QString rel = QDir::cleanPath(appDir + "/../include/bin/ocr_paddle.py");
    if(QFile::exists(rel)) return QDir::fromNativeSeparators(rel);
    return QString();
}

static QString resolvePythonExe(){
    QString fromEnv = qEnvironmentVariable("TOMATO_OCR_PYTHON");
    if(!fromEnv.isEmpty() && QFile::exists(fromEnv)) return QDir::fromNativeSeparators(fromEnv);
    QString cfg = configuredPython();
    if(!cfg.isEmpty()) return cfg;
    QString found = QStandardPaths::findExecutable("python");
    if(!found.isEmpty()) return found;
    QString pyLauncher = QStandardPaths::findExecutable("py");
    if(!pyLauncher.isEmpty()) return pyLauncher;
    return QString();
}

static QString resolvePaddleScript(){
    QString fromEnv = qEnvironmentVariable("TOMATO_OCR_SCRIPT");
    if(!fromEnv.isEmpty() && QFile::exists(fromEnv)) return QDir::fromNativeSeparators(fromEnv);
    QString cfg = configuredScript();
    if(!cfg.isEmpty()) return cfg;
    QString guessed = guessPaddleScript();
    if(!guessed.isEmpty()) return guessed;
    return QString();
}

static QStringList appendUniquePath(QStringList list, const QString& path){
    QString p = QDir::fromNativeSeparators(path.trimmed());
    if(p.isEmpty()) return list;
    if(!QDir(p).exists()) return list;
    if(!list.contains(p)) list.append(p);
    return list;
}

static QStringList discoverSitePackages(const QString& pyExe){
    QStringList paths;
    QString envSite = qEnvironmentVariable("TOMATO_OCR_SITE_PACKAGES");
    if(!envSite.isEmpty()) paths = appendUniquePath(paths, envSite);
    QString cfgSite = configuredSitePackages();
    if(!cfgSite.isEmpty()) paths = appendUniquePath(paths, cfgSite);

    QFileInfo pyInfo(pyExe);
    QString pyDir = pyInfo.absolutePath();
    paths = appendUniquePath(paths, QDir(pyDir).filePath("Lib/site-packages"));

    QRegularExpression re("Python(\\d+)$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch m = re.match(pyDir);
    if(m.hasMatch()){
        QString ver = m.captured(1);
        QString appData = qEnvironmentVariable("APPDATA");
        if(!appData.isEmpty()){
            paths = appendUniquePath(paths, QDir(appData).filePath("Python/Python" + ver + "/site-packages"));
        }
    }

    QProcess probe;
    QStringList args;
    QString exeName = pyInfo.fileName().toLower();
    if(exeName == "py.exe" || exeName == "py") args << "-3";
    args << "-c" << "import site\n"
                    "print(site.getusersitepackages())\n"
                    "x=getattr(site,'getsitepackages',lambda:[])()\n"
                    "print('\\n'.join(x))";
    probe.start(pyExe, args);
    if(probe.waitForFinished(10000)){
        const QString output = QString::fromUtf8(probe.readAllStandardOutput());
        for(const QString& line : output.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts)){
            paths = appendUniquePath(paths, line);
        }
    }
    return paths;
}

static QString runPaddleScript(const QString& script, const QString& imagePath){
    QString pyExe = resolvePythonExe();
    if(pyExe.isEmpty()) return QString();
    QProcess p;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList sitePackages = discoverSitePackages(pyExe);
    if(!sitePackages.isEmpty()){
        QString oldPath = env.value("PYTHONPATH");
        QString joined = sitePackages.join(";");
        if(oldPath.isEmpty()) env.insert("PYTHONPATH", joined);
        else env.insert("PYTHONPATH", joined + ";" + oldPath);
    }
    p.setProcessEnvironment(env);
    p.setWorkingDirectory(QFileInfo(script).absolutePath());
    QStringList args;
    QString exeName = QFileInfo(pyExe).fileName().toLower();
    if(exeName == "py.exe" || exeName == "py") args << "-3";
    args << script << imagePath;
    p.start(pyExe, args);
    if(!p.waitForFinished(120000)){
        p.kill();
        p.waitForFinished();
        return QString::fromUtf8("OCR 识别超时，请检查 Paddle 环境或图片大小。");
    }
    QByteArray out = p.readAllStandardOutput().trimmed();
    QByteArray err = p.readAllStandardError().trimmed();
    if(p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0){
        if(!out.isEmpty()) return QString::fromUtf8(out);
        if(!err.isEmpty()) return QString::fromUtf8(err);
        return QString();
    }
    if(!err.isEmpty()){
        QString errText = QString::fromUtf8(err);
        if(errText.contains("No module named 'paddleocr'") || errText.contains("No module named paddleocr")){
            return QString::fromUtf8("未找到 paddleocr 模块。请在当前 Python 执行：python -m pip install -U paddleocr");
        }
        return errText;
    }
    if(!out.isEmpty()) return QString::fromUtf8(out);
    return QString::fromUtf8("OCR 执行失败，请确认已安装 paddleocr 与 paddlepaddle。");
}

namespace {
class BaiduOcrClient : public QObject {
public:
    explicit BaiduOcrClient(QObject* parent=nullptr): QObject(parent){}

    void recognize(const QString& imagePath, QObject* context, std::function<void(const QString&)> onDone){
        if(!context){
            onDone(QString::fromUtf8("识别失败：无效上下文。"));
            return;
        }
        if(!hasBaiduCreds()){
            onDone(QString::fromUtf8("未配置百度 OCR WebAPI：请设置环境变量 BAIDU_OCR_API_KEY / BAIDU_OCR_SECRET_KEY。"));
            return;
        }
        QFile f(imagePath);
        if(!f.open(QIODevice::ReadOnly)){
            onDone(QString::fromUtf8("识别失败：无法读取图片文件。"));
            return;
        }
        QByteArray imgBytes = f.readAll();
        QImage decoded(imagePath);
        if(!decoded.isNull()){
            QByteArray jpegData;
            QBuffer buffer(&jpegData);
            buffer.open(QIODevice::WriteOnly);
            decoded.convertToFormat(QImage::Format_RGB888).save(&buffer, "JPG", 90);
            if(!jpegData.isEmpty()) imgBytes = jpegData;
        }
        if(imgBytes.isEmpty()){
            onDone(QString::fromUtf8("识别失败：图片内容为空。"));
            return;
        }
        const QByteArray imgB64 = imgBytes.toBase64();
        ensureToken(context, [this, context, onDone, imgB64]{
            if(token_.isEmpty()){
                onDone(QString::fromUtf8("获取 access_token 失败，请检查 API Key/Secret Key。"));
                return;
            }
            QUrl url(QStringLiteral("https://aip.baidubce.com/rest/2.0/ocr/v1/general_basic"));
            QUrlQuery query;
            query.addQueryItem("access_token", token_);
            url.setQuery(query);

            QNetworkRequest req(url);
            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            QByteArray payload = "image=" + QUrl::toPercentEncoding(QString::fromLatin1(imgB64));
            payload += "&detect_direction=true&paragraph=true";

            QPointer<QObject> ctx(context);
            QNetworkReply* reply = nam_.post(req, payload);
            QObject::connect(reply, &QNetworkReply::finished, context, [reply, ctx, onDone]{
                const QByteArray raw = reply->readAll();
                reply->deleteLater();
                if(!ctx) return;
                const QJsonDocument doc = QJsonDocument::fromJson(raw);
                if(!doc.isObject()){
                    onDone(QString::fromUtf8("OCR 返回非 JSON：") + QString::fromUtf8(raw.left(500)));
                    return;
                }
                const QJsonObject obj = doc.object();
                if(obj.contains("error_code")){
                    const int code = obj.value("error_code").toInt();
                    const QString msg = obj.value("error_msg").toString();
                    if(code == 216201){
                        onDone(QString::fromUtf8("OCR 错误：216201 image format error。已自动转 JPEG 并进行 urlencode，但服务端仍判定格式异常；请尝试更换图片（PNG/JPG/BMP）或减小尺寸后重试。"));
                        return;
                    }
                    onDone(QString("OCR 错误：%1 %2").arg(code).arg(msg));
                    return;
                }
                const QJsonArray arr = obj.value("words_result").toArray();
                QStringList lines;
                lines.reserve(arr.size());
                for(const auto& v : arr){
                    const QJsonObject w = v.toObject();
                    const QString text = w.value("words").toString();
                    if(!text.trimmed().isEmpty()) lines.append(text.trimmed());
                }
                onDone(lines.join("\n"));
            });
        });
    }

private:
    void ensureToken(QObject* context, std::function<void()> cont){
        const qint64 now = QDateTime::currentSecsSinceEpoch();
        if(!token_.isEmpty() && tokenExpireAt_ > now + 30){
            cont();
            return;
        }
        QUrl url(QStringLiteral("https://aip.baidubce.com/oauth/2.0/token"));
        QUrlQuery query;
        query.addQueryItem("grant_type", "client_credentials");
        query.addQueryItem("client_id", baiduApiKey());
        query.addQueryItem("client_secret", baiduSecretKey());
        url.setQuery(query);

        QNetworkRequest req(url);
        QPointer<QObject> ctx(context);
        QNetworkReply* reply = nam_.get(req);
        QObject::connect(reply, &QNetworkReply::finished, context, [this, reply, ctx, cont]{
            const QByteArray raw = reply->readAll();
            reply->deleteLater();
            if(!ctx) return;
            const QJsonDocument doc = QJsonDocument::fromJson(raw);
            if(!doc.isObject()){
                token_.clear();
                tokenExpireAt_ = 0;
                cont();
                return;
            }
            const QJsonObject obj = doc.object();
            token_ = obj.value("access_token").toString();
            const int expiresIn = obj.value("expires_in").toInt();
            tokenExpireAt_ = QDateTime::currentSecsSinceEpoch() + std::max(0, expiresIn);
            cont();
        });
    }

    QNetworkAccessManager nam_{};
    QString token_{};
    qint64 tokenExpireAt_{0};
};

static BaiduOcrClient* baiduClient(){
    static BaiduOcrClient* inst = new BaiduOcrClient();
    return inst;
}
}

bool OcrService::available(){
    return hasBaiduCreds() || PaddleInferOcr::available() || (!resolvePaddleScript().isEmpty() && !resolvePythonExe().isEmpty()) || !guessExe().isEmpty();
}

bool OcrService::hasPaddle(){
    return PaddleInferOcr::available() || (!resolvePaddleScript().isEmpty() && !resolvePythonExe().isEmpty());
}

bool OcrService::hasBaiduWebApi(){
    return hasBaiduCreds();
}

void OcrService::recognizeAsync(const QString& imagePath, QObject* context, std::function<void(const QString&)> onDone){
    if(!context){
        onDone(QString::fromUtf8("识别失败：无效上下文。"));
        return;
    }
    if(hasBaiduCreds()){
        baiduClient()->recognize(imagePath, context, std::move(onDone));
        return;
    }
    QString text = recognize(imagePath);
    onDone(text);
}

QString OcrService::recognize(const QString& imagePath){
    if(!QFile::exists(imagePath)) return QString::fromUtf8("识别失败：图片不存在。");
    if(PaddleInferOcr::available()){
        QString text = PaddleInferOcr::recognize(imagePath);
        if(!text.trimmed().isEmpty()) return text;
    }
    QString script = resolvePaddleScript();
    if(!script.isEmpty()){
        QString text = runPaddleScript(script, imagePath);
        if(!text.trimmed().isEmpty()) return text;
    }
    QString exe = guessExe();
    if(exe.isEmpty()) return QString::fromUtf8("未检测到可用 OCR 引擎，请配置 Python 与 PaddleOCR，或放置 tesseract.exe。");
    QProcess p;
    QStringList args;
    args << imagePath << "stdout";
    p.start(exe, args);
    p.waitForFinished(30000);
    QByteArray out = p.readAllStandardOutput().trimmed();
    if(!out.isEmpty()) return QString::fromUtf8(out);
    QByteArray err = p.readAllStandardError().trimmed();
    if(!err.isEmpty()) return QString::fromUtf8(err);
    return QString::fromUtf8("未获取到识别结果");
}
