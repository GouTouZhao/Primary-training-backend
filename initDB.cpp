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

	// 远程数据库连接配置（已注释）
	/*
	db.setHostName("******");
	db.setUserName("******");
	db.setPassword("******");
	db.setDatabaseName("******");
	db.setPort(******);
	db.setConnectOptions("SSL_CA=./isrgrootx1.pem;SSL_VERIFY_SERVER_CERT=1");
	*/

	// 从环境变量读取数据库连接配置
	QString hostName = qgetenv("DB_HOST");
	QString userName = qgetenv("DB_USER");
	QString password = qgetenv("DB_PASSWORD");
	QString databaseName = qgetenv("DB_NAME");
	QString portStr = qgetenv("DB_PORT");
	
	// 设置默认值（如果环境变量未设置）
	if (hostName.isEmpty()) hostName = "host.docker.internal";
	if (userName.isEmpty()) userName = "root";
	if (password.isEmpty()) password = "******";
	if (databaseName.isEmpty()) databaseName = "******";
	if (portStr.isEmpty()) portStr = "******";
	
	db.setHostName(hostName);
	db.setUserName(userName);
	db.setPassword(password);
	db.setDatabaseName(databaseName);
	db.setPort(portStr.toInt());

	if (!db.open()) {
		qWarning() << "" << db.lastError().text();
		return false;
	}
	
	qDebug() << "Mysql数据库连接成功";
	return true;
}

QSqlDatabase MysqlInitDB::getMysql() {
	if (!db.isOpen()||!db.isValid()) {
		qDebug() << "数据库重新连接";
		db.close();
		init();
		return db;
	}
	
	QSqlQuery testdb(db);
	if (!testdb.exec("SELECT 1"))
	{
		qDebug() << "数据库重新连接";
		db.close();
		init();
	}
	return db;
}