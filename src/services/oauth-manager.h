#pragma once

#include <QObject>
#include <QString>
#include <QTcpServer>
#include "../models/oauth-credentials.h"

class CredentialStore;

// Interface for OAuth management
class IOAuthManager {
public:
    virtual ~IOAuthManager() = default;

    virtual void startAuthFlow() = 0;
    virtual void refreshToken() = 0;
    virtual void logout() = 0;
    virtual bool isAuthenticated() const = 0;
    virtual const OAuthCredentials& credentials() const = 0;
};

// OAuth 2.0 Authorization Code Flow implementation for Twitch
class OAuthManager : public QObject, public IOAuthManager {
    Q_OBJECT

public:
    explicit OAuthManager(CredentialStore* credentialStore, QObject* parent = nullptr);
    ~OAuthManager() override;

    // IOAuthManager interface
    void startAuthFlow() override;
    void refreshToken() override;
    void logout() override;
    bool isAuthenticated() const override;
    const OAuthCredentials& credentials() const override;

    // Session restoration
    void restoreSession();

    // Token access for API calls
    QString accessToken() const;

signals:
    void authenticationStarted();
    void authenticationSuccess(const QString& userName);
    void authenticationFailed(const QString& error);
    void tokenRefreshed();
    void tokenExpired();
    void loggedOut();

private slots:
    void onNewConnection();
    void onClientReadyRead();

private:
    // OAuth configuration
    static constexpr const char* CLIENT_ID = "YOUR_TWITCH_CLIENT_ID"; // TODO: Replace with actual client ID
    static constexpr int REDIRECT_PORT = 17563;
    static constexpr const char* REDIRECT_URI = "http://localhost:17563/callback";

    // Auth flow helpers
    QString buildAuthUrl() const;
    QString generateState() const;
    void handleCallback(const QString& code, const QString& state);
    void exchangeCodeForToken(const QString& code);
    void validateToken();
    void fetchUserInfo();

    // HTTP response helpers
    void sendSuccessResponse(QTcpSocket* socket);
    void sendErrorResponse(QTcpSocket* socket, const QString& error);

    // State
    CredentialStore* m_credentialStore;
    OAuthCredentials m_credentials;
    QTcpServer* m_callbackServer = nullptr;
    QString m_pendingState;
    bool m_isAuthenticating = false;
};
