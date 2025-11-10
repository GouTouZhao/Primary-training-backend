#include <QCoreApplication>
#include <QHttpServer>
#include <QTcpServer>
#include <QHostAddress>
#include <QDebug>
#include"initDB.h"
#include"Login.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (!MysqlInitDB::init()) {
        return -1;
    }

    QTcpServer tcpServer;
    if (!tcpServer.listen(QHostAddress::Any, 8080)) {
        qWarning() << "yes" << tcpServer.errorString();
        return -1;
    }

    QHttpServer server;
    //接口目录
    server.route("/", []() {
        return "Hello, HttpServer 已运行";
        });

    UserRegister::setupRoute(server);

    server.bind(&tcpServer);

    qInfo() << "123213" << tcpServer.serverPort();

    return app.exec();
}
