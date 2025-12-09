#include "star.h"
#include<QJsonDocument>
#include<QJsonObject>
#include <QSqlError>
#include<QJsonArray>
#include "staticsource.cpp"

void AddStar::setupRoute(QHttpServer& server) {
	server.route("/AddStar", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /AddStar";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();

			QString email = obj.value("email").toString();
			int userid = obj.value("id").toInt();
			int ticketid = obj.value("ticketid").toInt();
			qDebug() << "    request --email:" << email;

			if (email.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少id";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (ticketid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少ticketid";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db1(mysql);
			db1.prepare("SELECT id FROM users WHERE email = ?");
			db1.addBindValue(email);
			if (!db1.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询用户失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db1.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Email未注册";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			int interid = db1.value("id").toInt();
			if (userid != interid) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "ID不匹配Email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db2(mysql);
			db2.prepare("SELECT COUNT(*) FROM flights WHERE id = ?");
			db2.addBindValue(ticketid);
			if (!db2.exec() || !db2.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询票务失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (db2.value(0).toInt() == 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "未查到此航班";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db3(mysql);
			db3.prepare("SELECT COUNT(*) FROM stars WHERE user_id = ? AND ticket_id = ? AND is_cancel = false");
			db3.addBindValue(userid);
			db3.addBindValue(ticketid);
			if (!db3.exec() || !db3.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询收藏失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (db3.value(0).toInt() > 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "该票已在收藏中";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db4(mysql);
			db4.prepare("SELECT COUNT(*) FROM stars WHERE user_id = ? AND ticket_id = ? AND is_cancel = true");
			db4.addBindValue(userid);
			db4.addBindValue(ticketid);
			if (!db4.exec() || !db4.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询收藏失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (db4.value(0).toInt() > 0) {
				QSqlQuery update_query(mysql);
				update_query.prepare("UPDATE stars SET is_cancel = false, created_at = NOW() WHERE user_id = ? AND ticket_id = ?");
				update_query.addBindValue(userid);
				update_query.addBindValue(ticketid);
				if (!update_query.exec()) {
					QSqlError err = update_query.lastError();
					qDebug() << update_query.lastQuery() << "\n" << err.text() << "\n" << err.driverText()
						<< "\n" << err.databaseText() << "\n" << err.isValid();
					QJsonObject res;
					res["success"] = false;
					res["errors"] = "数据库更新收藏失败";
					return makeJsonResponse(res,
						QHttpServerResponse::StatusCode::InternalServerError);
				}
			} else {
				QSqlQuery query(mysql);
				query.prepare("INSERT INTO stars (user_id, ticket_id, created_at, is_cancel) VALUES (?,?,NOW(),false)");
				query.addBindValue(userid);
				query.addBindValue(ticketid);
				if (!query.exec()) {
					QSqlError err = query.lastError();
					qDebug() << query.lastQuery() << "\n" << err.text() << "\n" << err.driverText()
						<< "\n" << err.databaseText() << "\n" << err.isValid();
					QJsonObject res;
					res["success"] = false;
					res["errors"] = "数据库添加收藏失败";
					return makeJsonResponse(res,
						QHttpServerResponse::StatusCode::InternalServerError);
				}
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "添加收藏成功";
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void RemoveStar::setupRoute(QHttpServer& server) {
	server.route("/RemoveStar", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /RemoveStar";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();

			QString email = obj.value("email").toString();
			int userid = obj.value("id").toInt();
			int ticketid = obj.value("ticketid").toInt();
			qDebug() << "    request --email:" << email;

			if (email.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少id";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (ticketid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少ticketid";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db1(mysql);
			db1.prepare("SELECT id FROM users WHERE email = ?");
			db1.addBindValue(email);
			if (!db1.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询用户失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db1.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Email未注册";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			int interid = db1.value("id").toInt();
			if (userid != interid) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "ID不匹配Email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db2(mysql);
			db2.prepare("SELECT COUNT(*) FROM stars WHERE user_id = ? AND ticket_id = ? AND is_cancel = false");
			db2.addBindValue(userid);
			db2.addBindValue(ticketid);
			if (!db2.exec() || !db2.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询收藏失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (db2.value(0).toInt() == 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "该票未在收藏中";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery query(mysql);
			query.prepare("UPDATE stars SET is_cancel = true WHERE user_id = ? AND ticket_id = ?");
			query.addBindValue(userid);
			query.addBindValue(ticketid);
			if (!query.exec()) {
				QSqlError err = query.lastError();
				qDebug() << query.lastQuery() << "\n" << err.text() << "\n" << err.driverText()
					<< "\n" << err.databaseText() << "\n" << err.isValid();
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库删除收藏失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "取消收藏成功";
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void GetStarCount::setupRoute(QHttpServer& server) {
	server.route("/GetStarCount", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /GetStarCount";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();

			QString email = obj.value("email").toString();
			int userid = obj.value("id").toInt();
			qDebug() << "    request --email:" << email;

			if (email.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少id";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db1(mysql);
			db1.prepare("SELECT id FROM users WHERE email = ?");
			db1.addBindValue(email);
			if (!db1.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询用户失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db1.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Email未注册";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			int interid = db1.value("id").toInt();
			if (userid != interid) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "ID不匹配Email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db2(mysql);
			db2.prepare("SELECT COUNT(*) FROM stars WHERE user_id = ? AND is_cancel = false");
			db2.addBindValue(userid);
			if (!db2.exec() || !db2.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询收藏数量失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			int count = db2.value(0).toInt();

			QJsonObject res;
			res["success"] = true;
			res["message"] = "查询收藏数量成功";
			res["count"] = count;
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void GetStarTickets::setupRoute(QHttpServer& server) {
	server.route("/GetStarTickets", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /GetStarTickets";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();

			QString email = obj.value("email").toString();
			int userid = obj.value("id").toInt();
			int offset = obj.value("offset").toInt();
			int limit = obj.value("limit").toInt();
			qDebug() << "    request --email:" << email;

			if (email.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少id";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (offset < 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少offset";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (limit <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少limit";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db1(mysql);
			db1.prepare("SELECT id FROM users WHERE email = ?");
			db1.addBindValue(email);
			if (!db1.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询用户失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db1.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Email未注册";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			int interid = db1.value("id").toInt();
			if (userid != interid) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "ID不匹配Email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db2(mysql);
			db2.prepare("SELECT * FROM stars WHERE user_id = ? AND is_cancel = false ORDER BY created_at DESC LIMIT ?,?");
			db2.addBindValue(userid);
			db2.addBindValue(offset);
			db2.addBindValue(limit);
			if (!db2.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询收藏票务失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			QJsonArray resdata;
			while (db2.next()) {
				int ticketId = db2.value("ticket_id").toInt();
				
				QSqlQuery db3(mysql);
				db3.prepare("SELECT * FROM flights WHERE id = ?");
				db3.addBindValue(ticketId);
				if (!db3.exec()) {
					continue;
				}
				if (!db3.next()) {
					continue;
				}
				
				QJsonObject ticket;
				ticket["starid"] = db2.value("id").toInt();
				ticket["flightnumber"] = db3.value("flight_number").toString();
				ticket["airline"] = db3.value("airline").toString();
				ticket["departureairport"] = db3.value("departure_airport").toString();
				ticket["arrivalairport"] = db3.value("arrival_airport").toString();
				ticket["departuretime"] = db3.value("departure_time").toString();
				ticket["arrivaltime"] = db3.value("arrival_time").toString();
				ticket["price"] = db3.value("price").toInt();
				ticket["availableseats"] = db3.value("available_seats").toInt();
				ticket["createdat"] = db2.value("created_at").toString();
				resdata.append(ticket);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "查询收藏票务成功";
			res["data"] = resdata;
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}
