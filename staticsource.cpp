#include <QHttpServer>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

static void addCorsHeaders(QHttpServerResponse& res)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    // Qt 6.10 及以上版本
    QHttpHeaders corsHeaders;
    corsHeaders.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowOrigin, "*");
    corsHeaders.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowMethods, "GET, POST, OPTIONS");
    corsHeaders.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowHeaders, "Content-Type, Authorization");
    res.setHeaders(corsHeaders);
    qDebug() << "[CORS] Headers added (Qt >= 6.10)";
#else
    // Qt 6.4 等旧版本
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    res.setHeader("Vary", "Origin");
    qDebug() << "[CORS] Headers added (legacy)";
#endif
}

static QHttpServerResponse makeJsonResponse(const QJsonObject& obj,
    QHttpServerResponse::StatusCode code)
{
    QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    QHttpServerResponse res(payload, "application/json", code);

    // 添加 CORS 头
    addCorsHeaders(res);

#if QT_VERSION < QT_VERSION_CHECK(6, 10, 0)
    // 手动设置长度等附加头
    res.setHeader("Content-Length", QByteArray::number(payload.size()));
    res.setHeader("Connection", "close");
#endif

    return res;
}
