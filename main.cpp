#include <QCoreApplication>
#include <QHttpServer>
#include <QTcpServer>
#include <QHostAddress>
#include <QDebug>
#include<QHttpServerResponse>
#include <functional>
#include"initDB.h"
#include"Login.h"
#include"staticsource.cpp"

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





    /*server.addAfterRequestHandler(&server, [](const QHttpServerRequest&, QHttpServerResponse& res) {
        addCorsHeaders(res);
        });*/

    server.route(".*", QHttpServerRequest::Method::Options, []() {
        QHttpServerResponse res(QHttpServerResponse::StatusCode::Ok);
        addCorsHeaders(res);
        return res;
        });

    server.bind(&tcpServer);

    qInfo() << "运行在端口：" << tcpServer.serverPort();

    return app.exec();
}
