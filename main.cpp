#include<QCoreApplication>
#include<QHttpServer>
#include<QTcpServer>
#include <QDebug>

int main(int argc, char* argv[]) {
	QCoreApplication app(argc, argv);
	QHttpServer server;

	auto tcpServer = new QTcpServer(&app);

	if (!tcpServer->listen(QHostAddress::Any, 8080)) {
		qCritical() << "8080";
		return -1;
	}

	if (!server.bind(tcpServer)) {
		qCritical() << "tcp to http";
		return -1;
	}

	qDebug() << "HTTP server running on port" << tcpServer->serverPort();

	return app.exec();
}