#include "LocalApiServer.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QStringList>

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
