#include "oauth-manager.h"
#include "../storage/credential-store.h"
#include <obs-module.h>
#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>
#include <QDesktopServices>
#include <QRandomGenerator>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

OAuthManager::OAuthManager(CredentialStore* credentialStore, QObject* parent)
    : QObject(parent)
    , m_credentialStore(credentialStore)
{
}

OAuthManager::~OAuthManager()
{
    if (m_callbackServer) {
        m_callbackServer->close();
        delete m_callbackServer;
    }
}

void OAuthManager::startAuthFlow()
{
    if (m_isAuthenticating) {
        blog(LOG_WARNING, "[HOO-Raid] Auth flow already in progress");
        return;
    }

    m_isAuthenticating = true;
    emit authenticationStarted();

    // Start local callback server
    if (!m_callbackServer) {
        m_callbackServer = new QTcpServer(this);
        connect(m_callbackServer, &QTcpServer::newConnection,
                this, &OAuthManager::onNewConnection);
    }

    if (!m_callbackServer->listen(QHostAddress::LocalHost, REDIRECT_PORT)) {
        blog(LOG_ERROR, "[HOO-Raid] Failed to start callback server: %s",
             m_callbackServer->errorString().toUtf8().constData());
        m_isAuthenticating = false;
        emit authenticationFailed("Failed to start authentication server");
        return;
    }

    // Generate state for CSRF protection
    m_pendingState = generateState();

    // Open browser to Twitch auth page
    QString authUrl = buildAuthUrl();
    blog(LOG_INFO, "[HOO-Raid] Opening browser for authentication");
    QDesktopServices::openUrl(QUrl(authUrl));
}

void OAuthManager::refreshToken()
{
    QString refreshToken = m_credentialStore->retrieveRefreshToken();
    if (refreshToken.isEmpty()) {
        blog(LOG_WARNING, "[HOO-Raid] No refresh token available");
        emit tokenExpired();
        return;
    }

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QUrl url("https://id.twitch.tv/oauth2/token");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("grant_type", "refresh_token");
    params.addQueryItem("refresh_token", refreshToken);
    params.addQueryItem("client_id", CLIENT_ID);

    QNetworkReply* reply = manager->post(request, params.query(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
        reply->deleteLater();
        manager->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            blog(LOG_ERROR, "[HOO-Raid] Token refresh failed: %s",
                 reply->errorString().toUtf8().constData());
            emit tokenExpired();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();

        QString accessToken = obj["access_token"].toString();
        QString newRefreshToken = obj["refresh_token"].toString();
        int expiresIn = obj["expires_in"].toInt();

        if (accessToken.isEmpty()) {
            blog(LOG_ERROR, "[HOO-Raid] Token refresh returned empty access token");
            emit tokenExpired();
            return;
        }

        m_credentials.accessToken = accessToken;
        m_credentials.refreshToken = newRefreshToken;
        m_credentials.expiresAt = QDateTime::currentDateTimeUtc().addSecs(expiresIn);

        m_credentialStore->storeAccessToken(accessToken);
        if (!newRefreshToken.isEmpty()) {
            m_credentialStore->storeRefreshToken(newRefreshToken);
        }

        blog(LOG_INFO, "[HOO-Raid] Token refreshed successfully");
        emit tokenRefreshed();
    });
}

void OAuthManager::logout()
{
    m_credentials = OAuthCredentials();
    m_credentialStore->clearCredentials();
    blog(LOG_INFO, "[HOO-Raid] User logged out");
    emit loggedOut();
}

bool OAuthManager::isAuthenticated() const
{
    return m_credentials.isValid() && !m_credentials.isExpired();
}

const OAuthCredentials& OAuthManager::credentials() const
{
    return m_credentials;
}

void OAuthManager::restoreSession()
{
    QString accessToken = m_credentialStore->retrieveAccessToken();
    if (accessToken.isEmpty()) {
        blog(LOG_DEBUG, "[HOO-Raid] No stored credentials found");
        return;
    }

    m_credentials.accessToken = accessToken;
    m_credentials.refreshToken = m_credentialStore->retrieveRefreshToken();

    // Validate the token
    validateToken();
}

QString OAuthManager::accessToken() const
{
    return m_credentials.accessToken;
}

void OAuthManager::onNewConnection()
{
    QTcpSocket* socket = m_callbackServer->nextPendingConnection();
    if (socket) {
        connect(socket, &QTcpSocket::readyRead,
                this, &OAuthManager::onClientReadyRead);
    }
}

void OAuthManager::onClientReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray data = socket->readAll();
    QString request = QString::fromUtf8(data);

    // Parse the callback URL
    QRegularExpression regex("GET /callback\\?([^ ]+) HTTP");
    QRegularExpressionMatch match = regex.match(request);

    if (!match.hasMatch()) {
        sendErrorResponse(socket, "Invalid callback");
        return;
    }

    QUrlQuery query(match.captured(1));
    QString code = query.queryItemValue("code");
    QString state = query.queryItemValue("state");
    QString error = query.queryItemValue("error");

    if (!error.isEmpty()) {
        sendErrorResponse(socket, error);
        m_isAuthenticating = false;
        emit authenticationFailed(error);
        return;
    }

    if (state != m_pendingState) {
        sendErrorResponse(socket, "Invalid state parameter");
        m_isAuthenticating = false;
        emit authenticationFailed("Security validation failed");
        return;
    }

    sendSuccessResponse(socket);
    m_callbackServer->close();

    // Exchange code for token
    exchangeCodeForToken(code);
}

QString OAuthManager::buildAuthUrl() const
{
    QUrl url("https://id.twitch.tv/oauth2/authorize");
    QUrlQuery query;
    query.addQueryItem("client_id", CLIENT_ID);
    query.addQueryItem("redirect_uri", REDIRECT_URI);
    query.addQueryItem("response_type", "code");
    query.addQueryItem("scope", OAuthCredentials::requiredScopes().join(" "));
    query.addQueryItem("state", m_pendingState);
    url.setQuery(query);
    return url.toString();
}

QString OAuthManager::generateState() const
{
    const QString chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString state;
    for (int i = 0; i < 32; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        state.append(chars.at(index));
    }
    return state;
}

void OAuthManager::handleCallback(const QString& code, const QString& state)
{
    if (state != m_pendingState) {
        emit authenticationFailed("Invalid state parameter");
        return;
    }
    exchangeCodeForToken(code);
}

void OAuthManager::exchangeCodeForToken(const QString& code)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QUrl url("https://id.twitch.tv/oauth2/token");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("client_id", CLIENT_ID);
    params.addQueryItem("client_secret", "YOUR_CLIENT_SECRET"); // TODO: Secure handling
    params.addQueryItem("code", code);
    params.addQueryItem("grant_type", "authorization_code");
    params.addQueryItem("redirect_uri", REDIRECT_URI);

    QNetworkReply* reply = manager->post(request, params.query(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
        reply->deleteLater();
        manager->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            blog(LOG_ERROR, "[HOO-Raid] Token exchange failed: %s",
                 reply->errorString().toUtf8().constData());
            m_isAuthenticating = false;
            emit authenticationFailed(reply->errorString());
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();

        QString accessToken = obj["access_token"].toString();
        QString refreshToken = obj["refresh_token"].toString();
        int expiresIn = obj["expires_in"].toInt();

        if (accessToken.isEmpty()) {
            blog(LOG_ERROR, "[HOO-Raid] Token exchange returned empty access token");
            m_isAuthenticating = false;
            emit authenticationFailed("Failed to obtain access token");
            return;
        }

        m_credentials.accessToken = accessToken;
        m_credentials.refreshToken = refreshToken;
        m_credentials.expiresAt = QDateTime::currentDateTimeUtc().addSecs(expiresIn);

        // Parse scopes
        QJsonArray scopeArray = obj["scope"].toArray();
        m_credentials.scopes.clear();
        for (const QJsonValue& v : scopeArray) {
            m_credentials.scopes.append(v.toString());
        }

        // Store credentials
        m_credentialStore->storeAccessToken(accessToken);
        m_credentialStore->storeRefreshToken(refreshToken);

        // Fetch user info
        fetchUserInfo();
    });
}

void OAuthManager::validateToken()
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QUrl url("https://id.twitch.tv/oauth2/validate");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("OAuth %1").arg(m_credentials.accessToken).toUtf8());

    QNetworkReply* reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
        reply->deleteLater();
        manager->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            blog(LOG_WARNING, "[HOO-Raid] Token validation failed, attempting refresh");
            refreshToken();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();

        m_credentials.userId = obj["user_id"].toString();
        m_credentials.userLogin = obj["login"].toString();

        int expiresIn = obj["expires_in"].toInt();
        m_credentials.expiresAt = QDateTime::currentDateTimeUtc().addSecs(expiresIn);

        QJsonArray scopeArray = obj["scopes"].toArray();
        m_credentials.scopes.clear();
        for (const QJsonValue& v : scopeArray) {
            m_credentials.scopes.append(v.toString());
        }

        blog(LOG_INFO, "[HOO-Raid] Session restored for user: %s",
             m_credentials.userLogin.toUtf8().constData());
        emit authenticationSuccess(m_credentials.userLogin);
    });
}

void OAuthManager::fetchUserInfo()
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QUrl url("https://api.twitch.tv/helix/users");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_credentials.accessToken).toUtf8());
    request.setRawHeader("Client-Id", CLIENT_ID);

    QNetworkReply* reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
        reply->deleteLater();
        manager->deleteLater();

        m_isAuthenticating = false;

        if (reply->error() != QNetworkReply::NoError) {
            blog(LOG_ERROR, "[HOO-Raid] Failed to fetch user info: %s",
                 reply->errorString().toUtf8().constData());
            emit authenticationFailed("Failed to fetch user information");
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray dataArray = doc.object()["data"].toArray();

        if (dataArray.isEmpty()) {
            emit authenticationFailed("No user data returned");
            return;
        }

        QJsonObject user = dataArray.first().toObject();
        m_credentials.userId = user["id"].toString();
        m_credentials.userLogin = user["login"].toString();
        m_credentials.userName = user["display_name"].toString();

        blog(LOG_INFO, "[HOO-Raid] Authentication successful for user: %s",
             m_credentials.userName.toUtf8().constData());
        emit authenticationSuccess(m_credentials.userName);
    });
}

void OAuthManager::sendSuccessResponse(QTcpSocket* socket)
{
    QString html = R"(<!DOCTYPE html>
<html>
<head><title>HOO-Raid - Authentication Successful</title></head>
<body style="font-family: Arial, sans-serif; text-align: center; padding-top: 50px;">
<h1>Authentication Successful!</h1>
<p>You can close this window and return to OBS.</p>
</body>
</html>)";

    QString response = QString("HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: %1\r\n"
                               "Connection: close\r\n"
                               "\r\n%2")
                           .arg(html.length())
                           .arg(html);

    socket->write(response.toUtf8());
    socket->flush();
    socket->close();
    socket->deleteLater();
}

void OAuthManager::sendErrorResponse(QTcpSocket* socket, const QString& error)
{
    QString html = QString(R"(<!DOCTYPE html>
<html>
<head><title>HOO-Raid - Authentication Failed</title></head>
<body style="font-family: Arial, sans-serif; text-align: center; padding-top: 50px;">
<h1>Authentication Failed</h1>
<p>Error: %1</p>
<p>Please close this window and try again.</p>
</body>
</html>)").arg(error.toHtmlEscaped());

    QString response = QString("HTTP/1.1 400 Bad Request\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: %1\r\n"
                               "Connection: close\r\n"
                               "\r\n%2")
                           .arg(html.length())
                           .arg(html);

    socket->write(response.toUtf8());
    socket->flush();
    socket->close();
    socket->deleteLater();
}
