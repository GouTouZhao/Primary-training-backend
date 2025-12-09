#include "Login.h"
#include<QJsonDocument>
#include<QJsonObject>
#include <QSqlError>
#include "staticsource.cpp"


void UserRegister::setupRoute(QHttpServer& server) {
	server.route("/Register", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /Register";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(),&parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();


			QString email = obj.value("email").toString();
			QString username = obj.value("username").toString().trimmed();
			QString password = obj.value("password").toString();
			qDebug() << "    request--email:" << email;

			if (email.isEmpty() || username.isEmpty() || password.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "必填项为空";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			if (username.length() > 10) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "username 过长（max:30）";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			if (password.length() < 6) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "password过短（min:6）";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}


			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "后端数据库未连接";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db(mysql);

			db.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
			db.addBindValue(username);
			if (!db.exec() || !db.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询失败";
				return makeJsonResponse(res,QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (db.value(0).toInt() > 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "用户名已被使用";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			db.prepare("SELECT COUNT(*) FROM users WHERE email = ?");
			db.addBindValue(email);
			if (!db.exec() || !db.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (db.value(0).toInt() > 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "邮箱已被注册";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			QByteArray passwordhash = QCryptographicHash::hash(password.toUtf8(),
				QCryptographicHash::Sha256).toHex();

			QSqlQuery query(mysql);
			query.prepare("INSERT INTO users (email,username,password_hash,profile_color) VALUES (?,?,?,?)");
			query.addBindValue(email);
			query.addBindValue(username);
			query.addBindValue(QString::fromUtf8(passwordhash));
			query.addBindValue("FF0078D4");
			if (!query.exec()) {
				QSqlError err = query.lastError();
				qDebug() << query.lastQuery() << "\n" << err.text() << "\n" << err.driverText()
					<< "\n" << err.databaseText() << "\n" << err.isValid();
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库插入新用户失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "用户创建成功";
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);

		});
}

void UserLogin::setupRoute(QHttpServer& server) {
	server.route("/Login", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /Login";
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
			QString password = obj.value("password").toString();
			qDebug() << "    request --email:" << email;
			if (email.isEmpty() || password.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "必填项为空";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (password.length() < 6) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "密码过短（min：6）";
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
			db1.prepare("SELECT id,username,password_hash FROM users WHERE email = ?");
			db1.addBindValue(email);
			if (!db1.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询邮箱失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db1.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "邮箱未注册";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			int userid = db1.value("id").toInt();
			QString username = db1.value("username").toString();
			QString passwordHash = db1.value("password_hash").toString();

			QByteArray respassword_hash = QCryptographicHash::hash(password.toUtf8(),
				QCryptographicHash::Sha256).toHex();
			if (passwordHash != QString(respassword_hash)) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "密码错误";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "登录成功";
			res["userid"] = userid;
			res["email"] = email;
			res["username"] = username;
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);
		});
}

void UserInfo::setupRoute(QHttpServer& server) {
	server.route("/Info", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /Info";
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
			qDebug() << "    request --email:" << email;

			if (email.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求邮箱为空";
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
			db1.prepare("SELECT id,username FROM users WHERE email = ?");
			db1.addBindValue(email);
			if (!db1.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db1.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "用户不存在";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			int userid = db1.value("id").toInt();
			QString username = db1.value("username").toString();

			QJsonObject res;
			res["success"] = true;
			res["message"] = "查询信息成功";
			res["id"] = userid;
			res["username"] = username;
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);
		});
}

void UpdateProfileColor::setupRoute(QHttpServer& server) {
	server.route("/UpdateProfileColor", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /UpdateProfileColor";
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
			QString profile_color = obj.value("profile_color").toString();
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

			if (profile_color.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少profile_color";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (email.length() > 50) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "输入邮箱似乎过长";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (profile_color.length() != 8) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "头像颜色格式错误（应为8位十六进制）";
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
			query.prepare("UPDATE users SET profile_color = ? WHERE email = ?");
			query.addBindValue(profile_color);
			query.addBindValue(email);
			if (!query.exec()) {
				QSqlError err = query.lastError();
				qDebug() << query.lastQuery() << "\n" << err.text() << "\n" << err.driverText()
					<< "\n" << err.databaseText() << "\n" << err.isValid();
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库更新头像颜色失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "头像颜色更新成功";
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);
		});
}

void UpdateUsername::setupRoute(QHttpServer& server) {
	server.route("/UpdateUsername", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /UpdateUsername";
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
			QString new_username = obj.value("new_username").toString().trimmed();
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

			if (new_username.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少new_username";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (email.length() > 50) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "输入邮箱似乎过长";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (new_username.length() > 10) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "username 过长（max:30）";
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
			db2.prepare("SELECT COUNT(*) FROM users WHERE username = ? AND id != ?");
			db2.addBindValue(new_username);
			db2.addBindValue(userid);
			if (!db2.exec() || !db2.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (db2.value(0).toInt() > 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "用户名已被使用";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery query(mysql);
			query.prepare("UPDATE users SET username = ? WHERE email = ?");
			query.addBindValue(new_username);
			query.addBindValue(email);
			if (!query.exec()) {
				QSqlError err = query.lastError();
				qDebug() << query.lastQuery() << "\n" << err.text() << "\n" << err.driverText()
					<< "\n" << err.databaseText() << "\n" << err.isValid();
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库更新用户名失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "用户名更新成功";
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);
		});
}