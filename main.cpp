#include <QCoreApplication>
#include <QHttpServer>
#include <QTcpServer>
#include <QHostAddress>
#include <QDebug>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QTcpServer tcpServer;
    if (!tcpServer.listen(QHostAddress::Any, 8080)) {
        qWarning() << "yes" << tcpServer.errorString();
        return -1;
    }

    QHttpServer server;
    server.route("/", []() {
        return "Hello, Qt HttpServer!";
        });

    server.bind(&tcpServer);

    qInfo() << "123213" << tcpServer.serverPort();

    return app.exec();
}
