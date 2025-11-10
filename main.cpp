#include <QCoreApplication>
#include <QHttpServer>
#include <QTcpServer>
#include <QHostAddress>
#include <QDebug>
#include<QHttpServerResponse>
#include <functional>
#include"initDB.h"
#include"Login.h"

static void addCorsHeaders(QHttpServerResponse& res)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    // Qt 6.10 及以上版本：直接使用 QHttpHeaders API
    QHttpHeaders corsHeaders;
    corsHeaders.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowOrigin, "*");
    corsHeaders.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowMethods, "GET, POST, OPTIONS");
    corsHeaders.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowHeaders, "Content-Type, Authorization");
    res.setHeaders(corsHeaders);
#else
    // Qt 6.4 等旧版本：使用基本字符串接口
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
#endif
}

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





    server.addAfterRequestHandler(&server, [](const QHttpServerRequest&, QHttpServerResponse& res) {
        addCorsHeaders(res);
        });

    server.route(".*", QHttpServerRequest::Method::Options, []() {
        QHttpServerResponse res(QHttpServerResponse::StatusCode::Ok);
        addCorsHeaders(res);
        return res;
        });

    server.bind(&tcpServer);

    qInfo() << "运行在端口：" << tcpServer.serverPort();

    return app.exec();
}
