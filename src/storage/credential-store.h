#pragma once

#include <QObject>
#include <QString>
#include <functional>

// Interface for credential storage
class ICredentialStore {
public:
    virtual ~ICredentialStore() = default;

    virtual void storeAccessToken(const QString& token) = 0;
    virtual void storeRefreshToken(const QString& token) = 0;
    virtual QString retrieveAccessToken() const = 0;
    virtual QString retrieveRefreshToken() const = 0;
    virtual void clearCredentials() = 0;
    virtual bool hasCredentials() const = 0;
};

// QtKeychain-based implementation
class CredentialStore : public QObject, public ICredentialStore {
    Q_OBJECT

public:
    explicit CredentialStore(QObject* parent = nullptr);
    ~CredentialStore() override = default;

    // ICredentialStore interface
    void storeAccessToken(const QString& token) override;
    void storeRefreshToken(const QString& token) override;
    QString retrieveAccessToken() const override;
    QString retrieveRefreshToken() const override;
    void clearCredentials() override;
    bool hasCredentials() const override;

signals:
    void credentialsStored();
    void credentialsCleared();
    void error(const QString& message);

private:
    static constexpr const char* SERVICE_NAME = "HOO-Raid";
    static constexpr const char* ACCESS_TOKEN_KEY = "access_token";
    static constexpr const char* REFRESH_TOKEN_KEY = "refresh_token";

    void storeKey(const QString& key, const QString& value);
    QString retrieveKey(const QString& key) const;
    void deleteKey(const QString& key);

    // Cached values for synchronous access
    mutable QString m_cachedAccessToken;
    mutable QString m_cachedRefreshToken;
    mutable bool m_cacheLoaded = false;

    void loadCache() const;
};
