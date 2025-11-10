#include<QHttpServer>

static void addCorsHeaders(QHttpServerResponse& res)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    // Qt 6.10 及以上版本：直接使用 QHttpHeaders API
    QHttpHeaders corsHeaders;
    corsHeaders.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowOrigin, "*");
    corsHeaders.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowMethods, "GET, POST, OPTIONS");
    corsHeaders.replaceOrAppend(QHttpHeaders::WellKnownHeader::AccessControlAllowHeaders, "Content-Type, Authorization");
    res.setHeaders(corsHeaders);
#else
    // Qt 6.4 等旧版本：使用基本字符串接口
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
#endif
}