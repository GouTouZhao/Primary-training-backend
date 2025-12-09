#include "currency.h"
#include<QJsonDocument>
#include<QJsonObject>
#include <QSqlError>
#include "staticsource.cpp"

void GetCurrency::setupRoute(QHttpServer& server) {
	server.route("/GetCurrency", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /GetCurrency";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();

			QString email = obj.value("email").toString();
			int userid = obj.value("id").toInt();
			qDebug() << "    request --email:" << email;

			if (email.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少email";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少id";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (email.length() > 50) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "输入邮箱似乎过长";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db1(mysql);
			db1.prepare("SELECT id FROM users WHERE email = ?");
			db1.addBindValue(email);
			if (!db1.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询用户失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db1.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Email未注册";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			int interid = db1.value("id").toInt();
			if (userid != interid) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "ID不匹配Email";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db2(mysql);
			db2.prepare("SELECT currency FROM users WHERE email = ?");
			db2.addBindValue(email);
			if (!db2.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询货币失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db2.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "用户不存在";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			uint currency = db2.value("currency").toUInt();

			QJsonObject res;
			res["success"] = true;
			res["message"] = "查询货币成功";
			res["currency"] = QString::number(currency);
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);
		});
}

void AddCurrency::setupRoute(QHttpServer& server) {
	server.route("/AddCurrency", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /AddCurrency";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();

			QString email = obj.value("email").toString();
			int userid = obj.value("id").toInt();
			uint amount = obj.value("amount").toInt();
			qDebug() << "    request --email:" << email;

			if (email.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少email";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少id";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (amount <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "充值金额必须大于0";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (email.length() > 50) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "输入邮箱似乎过长";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db1(mysql);
			db1.prepare("SELECT id FROM users WHERE email = ?");
			db1.addBindValue(email);
			if (!db1.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询用户失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db1.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Email未注册";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			int interid = db1.value("id").toInt();
			if (userid != interid) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "ID不匹配Email";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery query(mysql);
			query.prepare("UPDATE users SET currency = currency + ? WHERE email = ?");
			query.addBindValue(amount);
			query.addBindValue(email);
			if (!query.exec()) {
				QSqlError err = query.lastError();
				qDebug() << query.lastQuery() << "\n" << err.text() << "\n" << err.driverText()
					<< "\n" << err.databaseText() << "\n" << err.isValid();
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库更新货币失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "充值成功";
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);
		});
}

void SubtractCurrency::setupRoute(QHttpServer& server) {
	server.route("/SubtractCurrency", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /SubtractCurrency";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();

			QString email = obj.value("email").toString();
			int userid = obj.value("id").toInt();
			uint amount = obj.value("amount").toInt();
			qDebug() << "    request --email:" << email;

			if (email.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少email";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少id";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (amount <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "提现金额必须大于0";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (email.length() > 50) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "输入邮箱似乎过长";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db1(mysql);
			db1.prepare("SELECT id FROM users WHERE email = ?");
			db1.addBindValue(email);
			if (!db1.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询用户失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db1.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Email未注册";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			int interid = db1.value("id").toInt();
			if (userid != interid) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "ID不匹配Email";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db2(mysql);
			db2.prepare("SELECT currency FROM users WHERE email = ?");
			db2.addBindValue(email);
			if (!db2.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询货币失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db2.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "用户不存在";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			uint current_currency = db2.value("currency").toUInt();

			if (current_currency < amount) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "余额不足";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery query(mysql);
			query.prepare("UPDATE users SET currency = currency - ? WHERE email = ?");
			query.addBindValue(amount);
			query.addBindValue(email);
			if (!query.exec()) {
				QSqlError err = query.lastError();
				qDebug() << query.lastQuery() << "\n" << err.text() << "\n" << err.driverText()
					<< "\n" << err.databaseText() << "\n" << err.isValid();
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库更新货币失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "提现成功";
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);
		});
}
