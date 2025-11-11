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
    QHttpServer server;
    
    if (!tcpServer.listen(QHostAddress::Any, 8080)) {
        qWarning() << "yes" << tcpServer.errorString();
        return -1;
    }
    server.bind(&tcpServer);
    server.route("/", []() {
        qDebug() << "请求进入 / ";
        QHttpServerResponse res("Hello, HttpServer 已运行");
        addCorsHeaders(res);
        return res;
        });

    UserRegister::setupRoute(server);





    /*server.addAfterRequestHandler(&server, [](const QHttpServerRequest&, QHttpServerResponse& res) {
        addCorsHeaders(res);
        });*/

    server.route(".*", QHttpServerRequest::Method::Options, []() {
        qDebug() << "请求进入 请求头函数 ";
        QHttpServerResponse res(QHttpServerResponse::StatusCode::Ok);
        addCorsHeaders(res);
        return res;
        });

    
    

    qInfo() << "运行在端口：" << tcpServer.serverPort();

    return app.exec();
}