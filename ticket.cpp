#include"ticket.h"
#include<QJsonDocument>
#include<QJsonObject>
#include <QSqlError>
#include<random>
#include<vector>
#include<QJsonArray>
#include "staticsource.cpp"

void UserGetTicketsNum::setupRoute(QHttpServer& server) {
	server.route("/GetTicketsNum", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /GetTickets";
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
			QString departureairport = obj.value("departureairport").toString();
			QString arrivalairport = obj.value("arrivalairport").toString();
			QString time = obj.value("time").toString();
			int userid = obj.value("id").toInt();
			qDebug() << "    request --email:" << email;

			if (email.isEmpty() || departureairport.isEmpty() ||
				arrivalairport.isEmpty() || time.isEmpty()||userid<=0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少必须数据";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (departureairport.length() > 6||arrivalairport.length() > 6) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "站名名称过长";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "服务器连接失败";
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

			QDate data = QDate::fromString(time, "yyyy-MM-dd");
			QDateTime start(data, QTime(0, 0, 0));
			QDateTime end = start.addDays(1);
			QString starttime = start.toString("yyyy-MM-dd HH:mm:ss");
			QString endtime = end.toString("yyyy-MM-dd HH:mm:ss");

			QSqlQuery db2(mysql);
			db2.prepare(R"(SELECT COUNT(*) FROM flights
				WHERE departure_airport = ?
				AND arrival_airport = ?
				AND departure_time >= ?
				AND arrival_time < ?)");
			db2.addBindValue(departureairport);
			db2.addBindValue(arrivalairport);
			db2.addBindValue(starttime);
			db2.addBindValue(endtime);
			if (!db2.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询机票失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			QJsonObject res;
			res["success"] = true;
			res["message"] = "查询数量成功";
			res["ticketsnum"] = db2.value(0).toInt();
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

QString RootPushTickets::getFlightNumber() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist1(0,20);
	std::vector<QString> airlines{ "CA","MU","CZ","HU","ZH","3U","MF","HO",
	"9C","KN","PN","GS","JD","SC","OQ","KY","GT","GJ","A6","NS","QW" };
	std::uniform_int_distribution<> dist2(1000, 9999);
	QString airline = airlines[dist1(gen)];
	QString num = QString::number(dist2(gen));
	return airline + num;
}

int RootPushTickets::getPrice() {
	std::vector<int> prices{ 199,299,399,499,599,699,799,899,999 };
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, 8);
	return prices[dist(gen)];
}

QString RootPushTickets::getAirport() {
	std::vector<QString> airports{"北京","广州","上海","成都","武汉","香港"};
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, 5);
	return airports[dist(gen)];
}

int RootPushTickets::getFlyTime() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, 12);
	return 3600 + 1800 + 300 * dist(gen);
}

void RootPushTickets::setupRoute(QHttpServer& server) {
	server.route("/PushTickets", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /PushTickets";
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

			QString starttime = obj.value("starttime").toString();
			int lasttime = obj.value("lasttime").toInt();
			int number = obj.value("number").toInt();
			qDebug() << "    request --email: root";

			if (lasttime < 1) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "持续时间至少为1";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库已关闭";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dist(0, 11);
			QDate startdate = QDate::fromString(starttime, "yyyy-MM-dd");
			for (int i = 0; i < lasttime; ++i) {
				for (int j = 0; j < number; ++j) {
					QDateTime date(startdate, QTime(dist(gen) * 2, dist(gen) * 5, 0));
					QDateTime inserttime = date.addDays(i);
					QDateTime arrtime = inserttime.addSecs(getFlyTime());
					QString time = inserttime.toString("yyyy-MM-dd HH:mm:ss");
					QString arrivaltime = arrtime.toString("yyyy-MM-dd HH:mm:ss");
					QString flightnumber = getFlightNumber();
					int price = getPrice();
					QString departureairport = getAirport();
					QString arrivalairport;
					while(true) {
						arrivalairport = getAirport();
						if (departureairport != arrivalairport) {
							break;
						}
					};
					QSqlQuery db(mysql);
					db.prepare("INSERT INTO flights (flight_number,departure_airport,arrival_airport,departure_time,arrival_time,price) VALUES (?,?,?,?,?,?)");
					db.addBindValue(flightnumber);
					db.addBindValue(departureairport);
					db.addBindValue(arrivalairport);
					db.addBindValue(time);
					db.addBindValue(arrivaltime);
					db.addBindValue(price);
					if (!db.exec()) {
						QJsonObject res;
						res["success"] = false;
						res["errors"] = "数据库插入数据失败";
						return makeJsonResponse(res,
							QHttpServerResponse::StatusCode::InternalServerError);
					}
				}
			}
			QJsonObject res;
			res["success"] = true;
			res["message"] = "新建机票成功";
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void UserGetTickets::setupRoute(QHttpServer& server) {
	server.route("/GetTickets", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /GetTickets";
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
			QString sort = obj.value("sort").toString();
			QString time = obj.value("time").toString();
			QString departureairport = obj.value("departureairport").toString();
			QString arrivalairport = obj.value("arrivalairport").toString();
			int offset = obj.value("offset").toInt();
			int limit = obj.value("limit").toInt();
			qDebug() << "    request --email:"<<email;

			if (email.isEmpty() || sort.isEmpty() || time.isEmpty() || userid < 0 ||
				departureairport.isEmpty() || arrivalairport.isEmpty()||offset<0||limit<0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少必须项";
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

			QDate data = QDate::fromString(time, "yyyy-MM-dd");
			QDateTime start(data, QTime(0, 0, 0));
			QDateTime end = start.addDays(1);
			QString starttime = start.toString("yyyy-MM-dd HH:mm:ss");
			QString endtime = end.toString("yyyy-MM-dd HH:mm:ss");

			QSqlQuery db2(mysql);
			if (sort == "starttime") {
				db2.prepare(
					"SELECT id,flight_number,departure_time,arrival_time,price FROM flights "
					"WHERE departure_airport = ? AND arrival_airport = ? "
					"AND departure_time BETWEEN ? AND ? "
					"ORDER BY departure_time "
					"LIMIT ?,?"
				);
			}
			else if (sort == "price") {
				db2.prepare(
					"SELECT id,flight_number,departure_time,arrival_time,price FROM flights "
					"WHERE departure_airport = ? AND arrival_airport = ? "
					"AND departure_time BETWEEN ? AND ? "
					"ORDER BY arrival_time "
					"LIMIT ?,?"
				);
			}
			else if (sort == "endtime") {
				db2.prepare(
					"SELECT id,flight_number,departure_time,arrival_time,price FROM flights "
					"WHERE departure_airport = ? AND arrival_airport = ? "
					"AND departure_time BETWEEN ? AND ? "
					"ORDER BY price "
					"LIMIT ?,? "
				);
			}
			else {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "sort类型不正确";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			db2.addBindValue(departureairport);
			db2.addBindValue(arrivalairport);
			db2.addBindValue(starttime);
			db2.addBindValue(endtime);
			db2.addBindValue(offset);
			db2.addBindValue(limit);
			
			if (!db2.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询票务失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			
			QJsonArray resdata;
			while (db2.next()) {
				QJsonObject ticket;
				ticket["id"] = db2.value("id").toInt();
				ticket["flightnumber"] = db2.value("flight_number").toString();
				ticket["departuretime"] = db2.value("departure_time").toDateTime().toString();
				ticket["arrivaltime"] = db2.value("arrival_time").toDateTime().toString();
				ticket["price"] = db2.value("price").toString();
				resdata.append(ticket);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "请求成功";
			res["date"] = resdata;
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}