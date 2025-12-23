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

			if (email.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (departureairport.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少depare";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (arrivalairport.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少arr";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (time.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少time";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "userid可能为零";
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
				AND departure_time < ?)");
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
			int count = 0;
			if (db2.next()) {     // ← 必须加！
				count = db2.value(0).toInt();
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "查询数量成功";
			res["ticketsnum"] = count;
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

			if (email.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (sort.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少sort";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (time.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少sort";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少userid";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (departureairport.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少depa";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (arrivalairport.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少arrport";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (offset < 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少offset";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (limit <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "请求缺少limit";
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
				ticket["ticketid"] = db2.value("id").toInt();
				ticket["flightnumber"] = db2.value("flight_number").toString();
				ticket["departuretime"] = db2.value("departure_time").toDateTime().toString();
				ticket["arrivaltime"] = db2.value("arrival_time").toDateTime().toString();
				ticket["price"] = db2.value("price").toString();
				resdata.append(ticket);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "请求成功";
			res["data"] = resdata;
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void UserGetTicketDetails::setupRoute(QHttpServer& server) {
	server.route("/GetTicketDetails", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /GetTicketDetails";
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
			db2.prepare("SELECT * FROM flights WHERE id = ?");
			db2.addBindValue(ticketid);
			if (!db2.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询票务失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db2.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询的票不存在";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			QString departureairport = db2.value("departure_airport").toString();
			QString arrivalairport = db2.value("arrival_airport").toString();
			QString flightnumber = db2.value("flight_number").toString();
			QString departuretime = db2.value("departure_time").toDateTime().toString();
			QString arrivaltime = db2.value("arrival_time").toDateTime().toString();
			int price = db2.value("price").toInt();

			QJsonObject res;
			res["success"] = true;
			res["errors"] = "查询成功";
			res["ticketid"] = ticketid;
			res["departureairport"] = departureairport;
			res["arrivalairport"] = arrivalairport;
			res["flightnumber"] = flightnumber;
			res["departuretime"] = departuretime;
			res["arrivaltime"] = arrivaltime;
			res["price"] = price;
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void UserGetRemainingTicketsNum::setupRoute(QHttpServer& server) {
	server.route("/GetRemainingTicketsNum", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /GetRemainingTicketsNum";
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
			db3.prepare("SELECT COUNT(*) FROM tickets WHERE ticket_id = ? AND is_refund = false");
			db3.addBindValue(ticketid);
			if (!db3.exec() || !db3.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询票数失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			int number = db3.value(0).toInt();
			if (number > 4) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "票数有误，请联系管理员";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "查票成功";
			res["remainingnumber"] = (4 - number);
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void UserBuyTicket::setupRoute(QHttpServer& server) {
	server.route("/BuyTicket", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /BuyTicket";
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
			QString passengerName = obj.value("passengername").toString();
			QString passengerPhone = obj.value("passengerphone").toString();
			QString passengerIDNumber = obj.value("passengeridnumber").toString();
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

			if(passengerName.isEmpty()){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少passengerName";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if(passengerPhone.isEmpty()){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少passengerPhone";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if(passengerIDNumber.isEmpty()){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少passengerIDNumber";
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
			db3.prepare("SELECT COUNT(*) FROM tickets WHERE ticket_id = ? AND is_refund = false");
			db3.addBindValue(ticketid);
			if (!db3.exec() || !db3.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询票数失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			int number = db3.value(0).toInt();
			if (number >= 4) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "目前剩余票数为0";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db_price(mysql);
			db_price.prepare("SELECT price FROM flights WHERE id = ?");
			db_price.addBindValue(ticketid);
			if (!db_price.exec() || !db_price.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询票价失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			uint ticket_price = db_price.value("price").toUInt();

			QSqlQuery db_currency(mysql);
			db_currency.prepare("SELECT currency FROM users WHERE email = ?");
			db_currency.addBindValue(email);
			if (!db_currency.exec() || !db_currency.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询用户货币失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			uint user_currency = db_currency.value("currency").toUInt();

			if (user_currency < ticket_price) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "余额不足";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db4(mysql);
			db4.prepare("INSERT INTO tickets (ticket_id,user_id,is_refund,passenger_name,passenger_phone,passenger_id_number) VALUES (?,?,0,?,?,?)");
			db4.addBindValue(ticketid);
			db4.addBindValue(userid);
			db4.addBindValue(passengerName);
			db4.addBindValue(passengerPhone);
			db4.addBindValue(passengerIDNumber);
			if (!db4.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库插入票务失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db_update_currency(mysql);
			db_update_currency.prepare("UPDATE users SET currency = currency - ? WHERE email = ?");
			db_update_currency.addBindValue(ticket_price);
			db_update_currency.addBindValue(email);
			if (!db_update_currency.exec()) {
				QSqlError err = db_update_currency.lastError();
				qDebug() << db_update_currency.lastQuery() << "\n" << err.text() << "\n" << err.driverText()
					<< "\n" << err.databaseText() << "\n" << err.isValid();
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库更新货币失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "购票成功";
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void UserGetOwnTicketsNum::setupRoute(QHttpServer& server) {
	server.route("/GetOwnTicketsNum", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /GetOwnTicketsNum";
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
				res["errors"] = "缺少email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少id";
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
			db2.prepare("SELECT COUNT(*) FROM tickets WHERE user_id = ? AND is_refund = 0");
			db2.addBindValue(userid);
			if (!db2.exec()||!db2.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询票数失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			int number = db2.value(0).toInt();
			QJsonObject res;
			res["success"] = true;
			res["message"] = "请求成功";
			res["number"] = number;
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void UserGetOwnTickets::setupRoute(QHttpServer& server) {
	server.route("/GetOwnTickets", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /GetOwnTickets";
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
				res["errors"] = "缺少email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少id";
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
			db2.prepare("SELECT * FROM tickets WHERE user_id = ? AND is_refund = 0 LIMIT ?,?");
			db2.addBindValue(userid);
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
				int ticketId = db2.value("ticket_id").toInt();
				bool isRefund = db2.value("is_refund").toBool();
				
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
				ticket["orderid"] = db2.value("id").toInt();
				ticket["ticketid"] = ticketId;
				ticket["flightnumber"] = db3.value("flight_number").toString();
				ticket["departureairport"] = db3.value("departure_airport").toString();
				ticket["arrivalairport"] = db3.value("arrival_airport").toString();
				ticket["departuretime"] = db3.value("departure_time").toString();
				ticket["arrivaltime"] = db3.value("arrival_time").toString();
				ticket["price"] = db3.value("price").toInt();
				ticket["isrefund"] = isRefund;
				resdata.append(ticket);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "请求成功";
			res["data"] = resdata;
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void UserRefundTicket::setupRoute(QHttpServer& server) {
	server.route("/RefundTicket", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /RefundTicket";
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
			int orderid = obj.value("orderid").toInt();
			qDebug() << "    request --email:" << email;

			if (email.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (userid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少id";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if (orderid <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少orderid";
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
			db2.prepare("SELECT user_id, is_refund, ticket_id FROM tickets WHERE id = ?");
			db2.addBindValue(orderid);
			if (!db2.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询订单失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!db2.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "订单不存在";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			int orderUserId = db2.value("user_id").toInt();
			bool isRefund = db2.value("is_refund").toBool();
			int ticketId = db2.value("ticket_id").toInt();
			if (orderUserId != userid) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "该订单不属于当前用户";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			if (isRefund) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "该订单已退票";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db_price(mysql);
			db_price.prepare("SELECT price FROM flights WHERE id = ?");
			db_price.addBindValue(ticketId);
			if (!db_price.exec() || !db_price.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询票价失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			uint ticket_price = db_price.value("price").toUInt();

			QSqlQuery db3(mysql);
			db3.prepare("UPDATE tickets SET is_refund = true WHERE id = ?");
			db3.addBindValue(orderid);
			if (!db3.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "退票失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db_update_currency(mysql);
			db_update_currency.prepare("UPDATE users SET currency = currency + ? WHERE email = ?");
			db_update_currency.addBindValue(ticket_price);
			db_update_currency.addBindValue(email);
			if (!db_update_currency.exec()) {
				QSqlError err = db_update_currency.lastError();
				qDebug() << db_update_currency.lastQuery() << "\n" << err.text() << "\n" << err.driverText()
					<< "\n" << err.databaseText() << "\n" << err.isValid();
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库更新货币失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "退票成功";
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void UserGetOrderDetails::setupRoute(QHttpServer& server){
	server.route("/GetOrderDetails",QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request){
			qDebug() << "post to /GetOrderDetails";
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
			int orderid = obj.value("orderid").toInt();
			qDebug() << "    request --email:" << email;

			if(email.isEmpty()){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "email为空";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if(userid<=0){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "id为空";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			if(orderid<=0){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "orderid为空";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if(!mysql.isOpen()){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}

			QSqlQuery db1(mysql);
			db1.prepare("SELECT id FROM users WHERE email = ?");
			db1.addBindValue(email);
			if(!db1.exec()){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询用户失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if(!db1.next()){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Email未注册";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			int interid = db1.value("id").toInt();
			if(userid != interid){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "ID不匹配Email";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db2(mysql);
			db2.prepare("SELECT ticket_id, user_id, is_refund, passenger_name, passenger_phone, passenger_id_number FROM tickets WHERE id = ?");
			db2.addBindValue(orderid);
			if(!db2.exec()){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询订单失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if(!db2.next()){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "订单不存在";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}
			int orderUserId = db2.value("user_id").toInt();
			int ticketId = db2.value("ticket_id").toInt();
			bool isRefund = db2.value("is_refund").toBool();
			QString passengerName = db2.value("passenger_name").toString();
			QString passengerPhone = db2.value("passenger_phone").toString();
			QString passengerIDNumber = db2.value("passenger_id_number").toString();

			if(orderUserId != userid){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "该订单不属于当前用户";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QSqlQuery db3(mysql);
			db3.prepare("SELECT * FROM flights WHERE id = ?");
			db3.addBindValue(ticketId);
			if(!db3.exec()){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库查询航班信息失败";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::InternalServerError);
			}
			if(!db3.next()){
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "关联航班不存在";
				return makeJsonResponse(res,
					QHttpServerResponse::StatusCode::BadRequest);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "查询订单详情成功";
			res["orderid"] = orderid;
			res["ticketid"] = ticketId;
			res["flightnumber"] = db3.value("flight_number").toString();
			res["departureairport"] = db3.value("departure_airport").toString();
			res["arrivalairport"] = db3.value("arrival_airport").toString();
			res["departuretime"] = db3.value("departure_time").toString();
			res["arrivaltime"] = db3.value("arrival_time").toString();
			res["price"] = db3.value("price").toInt();
			res["isrefund"] = isRefund;
			res["passengername"] = passengerName;
			res["passengerphone"] = passengerPhone;
			res["passengeridnumber"] = passengerIDNumber;
			return makeJsonResponse(res,
				QHttpServerResponse::StatusCode::Ok);
		});
}

void AdminAddTicket::setupRoute(QHttpServer& server) {
	server.route("/AdminAddTicket", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /AdminAddTicket";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();

			QString departureTime = obj.value("departuretime").toString();
			QString arrivalTime = obj.value("arrivaltime").toString();
			QString departureAirport = obj.value("departureairport").toString();
			QString arrivalAirport = obj.value("arrivalairport").toString();
			int price = obj.value("price").toInt();
			QString flightNumber = obj.value("flightnumber").toString();

			qDebug() << "    request --AdminAddTicket";

			// 验证必需参数
			if (departureTime.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少起飞时间";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (arrivalTime.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少到达时间";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (departureAirport.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少起飞机场";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (arrivalAirport.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少到达机场";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (flightNumber.isEmpty()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少飞机编号";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (price <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "票价必须大于0";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			// 验证机场是否在六个固定机场中
			QStringList validAirports = {"北京", "广州", "上海", "成都", "武汉", "香港"};
			if (!validAirports.contains(departureAirport)) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "起飞机场不在可选范围内";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (!validAirports.contains(arrivalAirport)) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "到达机场不在可选范围内";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (departureAirport == arrivalAirport) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "起飞机场和到达机场不能相同";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			// 验证时间格式 "2025-12-20-23-50"
			QDateTime departureDateTime = QDateTime::fromString(departureTime, "yyyy-MM-dd-HH-mm");
			if (!departureDateTime.isValid()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "起飞时间格式不正确，应为：2025-12-20-23-50";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			QDateTime arrivalDateTime = QDateTime::fromString(arrivalTime, "yyyy-MM-dd-HH-mm");
			if (!arrivalDateTime.isValid()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "到达时间格式不正确，应为：2025-12-20-23-50";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (arrivalDateTime <= departureDateTime) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "到达时间必须晚于起飞时间";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			// 连接数据库
			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			// 插入航班数据
			QString dbDepartureTime = departureDateTime.toString("yyyy-MM-dd HH:mm:ss");
			QString dbArrivalTime = arrivalDateTime.toString("yyyy-MM-dd HH:mm:ss");

			QSqlQuery db(mysql);
			db.prepare("INSERT INTO flights (flight_number, departure_airport, arrival_airport, departure_time, arrival_time, price) VALUES (?,?,?,?,?,?)");
			db.addBindValue(flightNumber);
			db.addBindValue(departureAirport);
			db.addBindValue(arrivalAirport);
			db.addBindValue(dbDepartureTime);
			db.addBindValue(dbArrivalTime);
			db.addBindValue(price);

			if (!db.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库插入航班失败: " + db.lastError().text();
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "管理员加票成功";
			res["flightnumber"] = flightNumber;
			res["departureairport"] = departureAirport;
			res["arrivalairport"] = arrivalAirport;
			res["departuretime"] = dbDepartureTime;
			res["arrivaltime"] = dbArrivalTime;
			res["price"] = price;
			
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);
		});
}

void AdminDeleteFlight::setupRoute(QHttpServer& server) {
	server.route("/AdminDeleteFlight", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /AdminDeleteFlight";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();

			int flightId = obj.value("flightid").toInt();

			qDebug() << "    request --AdminDeleteFlight flightid:" << flightId;

			// 验证必需参数
			if (flightId <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "缺少flightid或flightid无效";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			// 连接数据库
			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			// 检查航班是否存在
			QSqlQuery checkFlight(mysql);
			checkFlight.prepare("SELECT * FROM flights WHERE id = ?");
			checkFlight.addBindValue(flightId);
			if (!checkFlight.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询航班失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}
			if (!checkFlight.next()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "航班不存在";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			// 查询所有购买了此航班但未退票的用户
			QSqlQuery findTickets(mysql);
			findTickets.prepare("SELECT t.id, t.user_id, u.email, f.price FROM tickets t "
				"JOIN users u ON t.user_id = u.id "
				"JOIN flights f ON t.ticket_id = f.id "
				"WHERE t.ticket_id = ? AND t.is_refund = false");
			findTickets.addBindValue(flightId);
			if (!findTickets.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "查询购票记录失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			// 为所有未退票的用户办理退票退钱
			int refundCount = 0;
			while (findTickets.next()) {
				int ticketId = findTickets.value("id").toInt();
				int userId = findTickets.value("user_id").toInt();
				QString userEmail = findTickets.value("email").toString();
				int ticketPrice = findTickets.value("price").toInt();

				// 标记为已退票
				QSqlQuery refundTicket(mysql);
				refundTicket.prepare("UPDATE tickets SET is_refund = true WHERE id = ?");
				refundTicket.addBindValue(ticketId);
				if (!refundTicket.exec()) {
					QJsonObject res;
					res["success"] = false;
					res["errors"] = "退票处理失败";
					return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
				}

				// 退还货币
				QSqlQuery refundCurrency(mysql);
				refundCurrency.prepare("UPDATE users SET currency = currency + ? WHERE email = ?");
				refundCurrency.addBindValue(ticketPrice);
				refundCurrency.addBindValue(userEmail);
				if (!refundCurrency.exec()) {
					QSqlError err = refundCurrency.lastError();
					qDebug() << "Refund currency error:" << err.text();
					QJsonObject res;
					res["success"] = false;
					res["errors"] = "退钱处理失败";
					return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
				}

				refundCount++;
			}

			// 删除航班
			QSqlQuery deleteFlight(mysql);
			deleteFlight.prepare("DELETE FROM flights WHERE id = ?");
			deleteFlight.addBindValue(flightId);
			if (!deleteFlight.exec()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "删除航班失败: " + deleteFlight.lastError().text();
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QJsonObject res;
			res["success"] = true;
			res["message"] = "删除航班成功";
			res["flightid"] = flightId;
			res["refundcount"] = refundCount;
			res["refundinfo"] = QString("已为%1位用户办理退票退钱").arg(refundCount);
			
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);
		});
}

void AdminSearchTickets::setupRoute(QHttpServer& server) {
	server.route("/AdminSearchTickets", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /AdminSearchTickets";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();

			QString departureAirport = obj.value("departureairport").toString();
			QString arrivalAirport = obj.value("arrivalairport").toString();
			QString startDate = obj.value("startdate").toString();
			QString endDate = obj.value("enddate").toString();
			int ticketId = obj.value("ticketid").toInt();
			QString flightNumber = obj.value("flightnumber").toString();
			int offset = obj.value("offset").toInt();
			int limit = obj.value("limit").toInt();


			// 验证分页参数
			if (offset < 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "offset不能小于0";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			if (limit <= 0) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "limit必须大于0";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}

			// 连接数据库
			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QString sqlQuery;
			QSqlQuery db(mysql);
			QJsonArray resdata;

			// 分情况处理：锁定查询 vs 筛选查询
			if (ticketId > 0 || !flightNumber.isEmpty()) {
				// 锁定查询：根据ticketId或flightNumber精确查询
				qDebug() << "执行锁定查询";
				
				if (ticketId > 0) {
					// 根据票ID查询
					sqlQuery = "SELECT id, flight_number, departure_airport, arrival_airport, departure_time, arrival_time, price FROM flights WHERE id = ?";
					db.prepare(sqlQuery);
					db.addBindValue(ticketId);
				} else {
					// 根据飞机编号查询
					sqlQuery = "SELECT id, flight_number, departure_airport, arrival_airport, departure_time, arrival_time, price FROM flights WHERE flight_number = ?";
					db.prepare(sqlQuery);
					db.addBindValue(flightNumber);
				}

				if (!db.exec()) {
					QJsonObject res;
					res["success"] = false;
					res["errors"] = "数据库查询失败: " + db.lastError().text();
					return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
				}

				while (db.next()) {
					QJsonObject ticket;
					ticket["ticketid"] = db.value("id").toInt();
					ticket["flightnumber"] = db.value("flight_number").toString();
					ticket["departureairport"] = db.value("departure_airport").toString();
					ticket["arrivalairport"] = db.value("arrival_airport").toString();
					ticket["departuretime"] = db.value("departure_time").toString();
					ticket["arrivaltime"] = db.value("arrival_time").toString();
					ticket["price"] = db.value("price").toInt();
					resdata.append(ticket);
				}
			} else {
				// 筛选查询：根据机场和时间范围筛选
				qDebug() << "执行筛选查询";
				
				// 构建筛选条件描述
				QString filterDescription = "筛选条件：";
				QStringList conditions;
				QStringList bindValues;

				if (!departureAirport.isEmpty() && departureAirport != "all") {
					conditions << "departure_airport = ?";
					bindValues << departureAirport;
					filterDescription += QString("起飞机场：%1 ").arg(departureAirport);
				}

				if (!arrivalAirport.isEmpty() && arrivalAirport != "all") {
					conditions << "arrival_airport = ?";
					bindValues << arrivalAirport;
					filterDescription += QString("到达机场：%1 ").arg(arrivalAirport);
				}

				if (!startDate.isEmpty() && startDate != "all") {
					conditions << "departure_time >= ?";
					QDateTime startDateTime = QDateTime::fromString(startDate, "yyyy-MM-dd");
					if (!startDateTime.isValid()) {
						QJsonObject res;
						res["success"] = false;
						res["errors"] = "开始日期格式不正确，应为：yyyy-MM-dd";
						return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
					}
					QString startDateTimeStr = startDateTime.toString("yyyy-MM-dd HH:mm:ss");
					bindValues << startDateTimeStr;
					filterDescription += QString("开始时间：%1 ").arg(startDate);
				}

				if (!endDate.isEmpty() && endDate != "all") {
					conditions << "departure_time < ?";
					QDateTime endDateTime = QDateTime::fromString(endDate, "yyyy-MM-dd");
					if (!endDateTime.isValid()) {
						QJsonObject res;
						res["success"] = false;
						res["errors"] = "结束日期格式不正确，应为：yyyy-MM-dd";
						return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
					}
					// 将结束日期设置为第二天的开始时间，以包含当天的所有航班
					endDateTime = endDateTime.addDays(1);
					QString endDateTimeStr = endDateTime.toString("yyyy-MM-dd HH:mm:ss");
					bindValues << endDateTimeStr;
					filterDescription += QString("结束时间：%1 ").arg(endDate);
				}

				if (conditions.isEmpty()) {
					// 如果没有筛选条件，查询所有航班
					sqlQuery = "SELECT id, flight_number, departure_airport, arrival_airport, departure_time, arrival_time, price FROM flights ORDER BY id LIMIT ?,?";
					db.prepare(sqlQuery);
					db.addBindValue(offset);
					db.addBindValue(limit);
					filterDescription = "查询所有航班";
				} else {
					// 有筛选条件，构建WHERE子句
					QString whereClause = conditions.join(" AND ");
					sqlQuery = QString("SELECT id, flight_number, departure_airport, arrival_airport, departure_time, arrival_time, price FROM flights WHERE %1 ORDER BY id LIMIT ?,?").arg(whereClause);
					db.prepare(sqlQuery);
					
					// 绑定筛选条件的值
					for (const QString& value : bindValues) {
						db.addBindValue(value);
					}
					
					// 绑定分页参数
					db.addBindValue(offset);
					db.addBindValue(limit);
				}

				qDebug() << "SQL查询语句:" << sqlQuery;
				qDebug() << filterDescription;

				if (!db.exec()) {
					QJsonObject res;
					res["success"] = false;
					res["errors"] = "数据库查询失败: " + db.lastError().text();
					return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
				}

				while (db.next()) {
					QJsonObject ticket;
					ticket["ticketid"] = db.value("id").toInt();
					ticket["flightnumber"] = db.value("flight_number").toString();
					ticket["departureairport"] = db.value("departure_airport").toString();
					ticket["arrivalairport"] = db.value("arrival_airport").toString();
					ticket["departuretime"] = db.value("departure_time").toString();
					ticket["arrivaltime"] = db.value("arrival_time").toString();
					ticket["price"] = db.value("price").toInt();
					resdata.append(ticket);
				}
			}

			// 构建响应
			QJsonObject res;
			res["success"] = true;
			res["message"] = "管理员查询票务成功";
			res["data"] = resdata;
			res["count"] = resdata.size();
			
			// 如果是筛选查询，添加筛选描述
			if (ticketId <= 0 && flightNumber.isEmpty()) {
				res["filter"] = "已按条件筛选航班信息";
			} else {
				res["filter"] = "已按精确条件锁定航班";
			}
			
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);
		});
}

void AdminSearchTicketsCount::setupRoute(QHttpServer& server) {
	server.route("/AdminSearchTicketsCount", QHttpServerRequest::Method::Post,
		[](const QHttpServerRequest& request) {
			qDebug() << "post to /AdminSearchTicketsCount";
			QJsonParseError parseError;
			QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "Json格式非法";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
			}
			QJsonObject obj = doc.object();

			QString departureAirport = obj.value("departureairport").toString();
			QString arrivalAirport = obj.value("arrivalairport").toString();
			QString startDate = obj.value("startdate").toString();
			QString endDate = obj.value("enddate").toString();
			int ticketId = obj.value("ticketid").toInt();
			QString flightNumber = obj.value("flightnumber").toString();


			// 连接数据库
			QSqlDatabase mysql = MysqlInitDB::getMysql();
			if (!mysql.isOpen()) {
				QJsonObject res;
				res["success"] = false;
				res["errors"] = "数据库连接失败";
				return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
			}

			QString sqlQuery;
			QSqlQuery db(mysql);
			int totalCount = 0;

			// 分情况处理：锁定查询 vs 筛选查询
			if (ticketId > 0 || !flightNumber.isEmpty()) {
				// 锁定查询：根据ticketId或flightNumber精确查询
				qDebug() << "执行锁定查询统计";
				
				if (ticketId > 0) {
					// 根据票ID查询
					sqlQuery = "SELECT COUNT(*) FROM flights WHERE id = ?";
					db.prepare(sqlQuery);
					db.addBindValue(ticketId);
				} else {
					// 根据飞机编号查询
					sqlQuery = "SELECT COUNT(*) FROM flights WHERE flight_number = ?";
					db.prepare(sqlQuery);
					db.addBindValue(flightNumber);
				}

				if (!db.exec()) {
					QJsonObject res;
					res["success"] = false;
					res["errors"] = "数据库查询失败: " + db.lastError().text();
					return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
				}

				if (db.next()) {
					totalCount = db.value(0).toInt();
				}
			} else {
				// 筛选查询：根据机场和时间范围筛选
				qDebug() << "执行筛选查询统计";
				
				// 构建筛选条件描述
				QString filterDescription = "筛选条件：";
				QStringList conditions;
				QStringList bindValues;

				if (!departureAirport.isEmpty() && departureAirport != "all") {
					conditions << "departure_airport = ?";
					bindValues << departureAirport;
					filterDescription += QString("起飞机场：%1 ").arg(departureAirport);
				}

				if (!arrivalAirport.isEmpty() && arrivalAirport != "all") {
					conditions << "arrival_airport = ?";
					bindValues << arrivalAirport;
					filterDescription += QString("到达机场：%1 ").arg(arrivalAirport);
				}

				if (!startDate.isEmpty() && startDate != "all") {
					conditions << "departure_time >= ?";
					QDateTime startDateTime = QDateTime::fromString(startDate, "yyyy-MM-dd");
					if (!startDateTime.isValid()) {
						QJsonObject res;
						res["success"] = false;
						res["errors"] = "开始日期格式不正确，应为：yyyy-MM-dd";
						return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
					}
					QString startDateTimeStr = startDateTime.toString("yyyy-MM-dd HH:mm:ss");
					bindValues << startDateTimeStr;
					filterDescription += QString("开始时间：%1 ").arg(startDate);
				}

				if (!endDate.isEmpty() && endDate != "all") {
					conditions << "departure_time < ?";
					QDateTime endDateTime = QDateTime::fromString(endDate, "yyyy-MM-dd");
					if (!endDateTime.isValid()) {
						QJsonObject res;
						res["success"] = false;
						res["errors"] = "结束日期格式不正确，应为：yyyy-MM-dd";
						return makeJsonResponse(res, QHttpServerResponse::StatusCode::BadRequest);
					}
					// 将结束日期设置为第二天的开始时间，以包含当天的所有航班
					endDateTime = endDateTime.addDays(1);
					QString endDateTimeStr = endDateTime.toString("yyyy-MM-dd HH:mm:ss");
					bindValues << endDateTimeStr;
					filterDescription += QString("结束时间：%1 ").arg(endDate);
				}

				if (conditions.isEmpty()) {
					// 如果没有筛选条件，查询所有航班总数
					sqlQuery = "SELECT COUNT(*) FROM flights";
					db.prepare(sqlQuery);
					filterDescription = "查询所有航班总数";
				} else {
					// 有筛选条件，构建WHERE子句
					QString whereClause = conditions.join(" AND ");
					sqlQuery = QString("SELECT COUNT(*) FROM flights WHERE %1").arg(whereClause);
					db.prepare(sqlQuery);
					
					// 绑定筛选条件的值
					for (const QString& value : bindValues) {
						db.addBindValue(value);
					}
				}

				qDebug() << "SQL查询语句:" << sqlQuery;
				qDebug() << filterDescription;

				if (!db.exec()) {
					QJsonObject res;
					res["success"] = false;
					res["errors"] = "数据库查询失败: " + db.lastError().text();
					return makeJsonResponse(res, QHttpServerResponse::StatusCode::InternalServerError);
				}

				if (db.next()) {
					totalCount = db.value(0).toInt();
				}
			}

			// 构建响应
			QJsonObject res;
			res["success"] = true;
			res["message"] = "管理员查询票务总数成功";
			res["count"] = totalCount;
			
			// 如果是筛选查询，添加筛选描述
			if (ticketId <= 0 && flightNumber.isEmpty()) {
				res["filter"] = "已按条件统计航班总数";
			} else {
				res["filter"] = "已按精确条件统计航班总数";
			}
			
			return makeJsonResponse(res, QHttpServerResponse::StatusCode::Ok);
		});
}
