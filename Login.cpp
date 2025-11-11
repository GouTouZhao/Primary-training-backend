#include "Login.h"
#include<QJsonDocument>
#include<QJsonObject>
#include <QSqlError>
#include "staticsource.cpp"


void UserRegister::setupRoute(QHttpServer& server) {

	server.route("/Register", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "请求进入 /Register ";
			qDebug() << "Request body: /Register" << request.body();
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(),&parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject resObj;
				resObj["success"] = false;
				resObj["errors"] = "Json格式非法";
				QHttpServerResponse res (QJsonDocument(resObj).toJson(), 
					"application/json", 
					QHttpServerResponse::StatusCode::BadRequest);
				addCorsHeaders(res);
				return res;
			}
			QJsonObject obj = doc.object();


			QString email = obj.value("email").toString();
			QString username = obj.value("username").toString().trimmed();
			QString password = obj.value("password").toString();

			if (email.isEmpty() || username.isEmpty() || password.isEmpty()) {
				QJsonObject resObj;
				resObj["success"] = false;
				resObj["errors"] = "必填项为空";
				QHttpServerResponse res (QJsonDocument(resObj).toJson(),
					"application/json", 
					QHttpServerResponse::StatusCode::BadRequest);
				addCorsHeaders(res);
				return res;
			}
			if (username.length() > 30) {
				QJsonObject resObj;
				resObj["success"] = false;
				resObj["errors"] = "username 过长（max:30）";
				QHttpServerResponse res (QJsonDocument(resObj).toJson(),
					"application/json", 
					QHttpServerResponse::StatusCode::BadRequest);
				addCorsHeaders(res);
				return res;
			}
			if (password.length() < 6) {
				QJsonObject resObj;
				resObj["success"] = false;
				resObj["errors"] = "password过短（min:6）";
				QHttpServerResponse res (QJsonDocument(resObj).toJson(),
					"application/json", 
					QHttpServerResponse::StatusCode::BadRequest);
				addCorsHeaders(res);
				return res;
			}


			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject resObj;
				resObj["success"] = false;
				resObj["errors"] = "后端数据库未连接";
				QHttpServerResponse res (QJsonDocument(resObj).toJson(),
					"application/json", 
					QHttpServerResponse::StatusCode::InternalServerError);
				addCorsHeaders(res);
				return res;
			}

			QSqlQuery db(mysql);

			db.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
			db.addBindValue(username);
			if (!db.exec() || !db.next()) {
				QJsonObject resObj;
				resObj["success"] = false;
				resObj["errors"] = "数据库查询失败";
				QHttpServerResponse res (QJsonDocument(resObj).toJson(),
					"application/json", 
					QHttpServerResponse::StatusCode::InternalServerError);
				addCorsHeaders(res);
				return res;
			}
			if (db.value(0).toInt() > 0) {
				QJsonObject resObj;
				resObj["success"] = false;
				resObj["errors"] = "用户名已被使用";
				QHttpServerResponse res (QJsonDocument(resObj).toJson(),
					"application/json",
					QHttpServerResponse::StatusCode::BadRequest);
				addCorsHeaders(res);
				return res;
			}

			db.prepare("SELECT COUNT(*) FROM users WHERE email = ?");
			db.addBindValue(email);
			if (!db.exec() || !db.next()) {
				QJsonObject resObj;
				resObj["success"] = false;
				resObj["errors"] = "数据库查询失败";
				QHttpServerResponse res (QJsonDocument(resObj).toJson(),
					"application/json",
					QHttpServerResponse::StatusCode::InternalServerError);
				addCorsHeaders(res);
				return res;
			}
			if (db.value(0).toInt() > 0) {
				QJsonObject resObj;
				resObj["success"] = false;
				resObj["errors"] = "邮箱已被注册";
				QHttpServerResponse res (QJsonDocument(resObj).toJson(),
					"application/json",
					QHttpServerResponse::StatusCode::BadRequest);
				addCorsHeaders(res);
				return res;
			}

			QByteArray passwordhash = QCryptographicHash::hash(password.toUtf8(),
				QCryptographicHash::Sha256).toHex();

			db.prepare("INSERT INTO users (email,username,password) VALUES (?,?,?)");
			db.addBindValue(email);
			db.addBindValue(username);
			db.addBindValue(QString::fromUtf8(passwordhash));
			if (!db.exec()) {
				QJsonObject resObj;
				resObj["success"] = false;
				resObj["errors"] = "数据库插入新用户失败" + db.lastError().text();
				QHttpServerResponse res (QJsonDocument(resObj).toJson(),
					"application/json",
					QHttpServerResponse::StatusCode::InternalServerError);
				addCorsHeaders(res);
				return res;
			}

			QJsonObject resObj;
			resObj["success"] = true;
			resObj["message"] = "用户创建成功";
			QHttpServerResponse res (QJsonDocument(resObj).toJson(),
				"application/json",
				QHttpServerResponse::StatusCode::Ok);
			addCorsHeaders(res);
			return res;

		});
}