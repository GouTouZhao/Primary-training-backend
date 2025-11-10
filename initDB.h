#pragma once
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QSqlError>
#include<QDebug>

class MysqlInitDB {
public:
	static bool init();
	static QSqlDatabase getMysql();
private:
	static QSqlDatabase db;
};