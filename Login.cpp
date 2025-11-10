#include "Login.h"
#include<QJsonDocument>
#include<QJsonObject>
#include <QSqlError>


void UserRegister::setupRoute(QHttpServer& server) {

	server.route("/Register", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(),&parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return QHttpServerResponse(QJsonDocument(res).toJson(), 
					"application/json", 
					QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();


			QString email = obj.value("email").toString();
			QString username = obj.value("username").toString().trimmed();
			QString password = obj.value("password").toString();

			if (email.isEmpty() || username.isEmpty() || password.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "必填项为空";
				return QHttpServerResponse(QJsonDocument(res).toJson(), 
					"application/json", 
					QHttpServerResponse::StatusCode::BadRequest);
			}
			if (username.length() > 30) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "username 过长（max:30）";
				return QHttpServerResponse(QJsonDocument(res).toJson(), 
					"application/json", 
					QHttpServerResponse::StatusCode::BadRequest);
			}
			if (password.length() < 6) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "password过短（min:6）";
				return QHttpServerResponse(QJsonDocument(res).toJson(), 
					"application/json", 
					QHttpServerResponse::StatusCode::BadRequest);
			}


			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "后端数据库未连接";
				return QHttpServerResponse(QJsonDocument(res).toJson(), 
					"application/json", 
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db(mysql);

			db.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
			db.addBindValue(username);
			if (!db.exec() || !db.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询失败";
				return QHttpServerResponse(QJsonDocument(res).toJson(),
					"application/json", 
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (db.value(0).toInt() > 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "用户名已被使用";
				return QHttpServerResponse(QJsonDocument(res).toJson(),
					"application/json",
					QHttpServerResponse::StatusCode::BadRequest);
			}

			db.prepare("SELECT COUNT(*) FROM users WHERE email = ?");
			db.addBindValue(email);
			if (!db.exec() || !db.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询失败";
				return QHttpServerResponse(QJsonDocument(res).toJson(),
					"application/json",
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (db.value(0).toInt() > 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "邮箱已被注册";
				return QHttpServerResponse(QJsonDocument(res).toJson(),
					"application/json",
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QByteArray passwordhash = QCryptographicHash::hash(password.toUtf8(),
				QCryptographicHash::Sha256).toHex();

			db.prepare("INSERT INTO (email,username,password) VALUES (?,?,?)");
			db.addBindValue(email);
			db.addBindValue(username);
			db.addBindValue(QString::fromUtf8(passwordhash));
			if (!db.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库插入新用户失败" + db.lastError().text();
				return QHttpServerResponse(QJsonDocument(res).toJson(),
					"application/json",
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "用户创建成功";
			return QHttpServerResponse(QJsonDocument(res).toJson(),
				"application/json",
				QHttpServerResponse::StatusCode::Ok);

		});
}