#include"initDB.h"

QSqlDatabase MysqlInitDB::db = QSqlDatabase();

bool MysqlInitDB::init() {
	qputenv("QT_DEBUG_PLUGINS", "1");
	if (QSqlDatabase::contains("qt_sql_default_connection")) {
		db = QSqlDatabase::database("qt_sql_default_connection");
	}
	else {
		db = QSqlDatabase::addDatabase("QMYSQL");
	}

	db.setHostName("gateway01.eu-central-1.prod.aws.tidbcloud.com");
	db.setUserName("b8SLzcNi5Bie2pX.root");
	db.setPassword("GRhkSSRuBNWXT2vG");
	db.setDatabaseName("sixonezero");
	db.setPort(4000);

	db.setConnectOptions("SSL_CA=./isrgrootx1.pem;SSL_VERIFY_SERVER_CERT=1");

	if (!db.open()) {
		qWarning() << "" << db.lastError().text();
		return false;
	}
	
	qDebug() << "Mysql数据库连接成功";
	return true;
}

QSqlDatabase MysqlInitDB::getMysql() {
	return db;
}