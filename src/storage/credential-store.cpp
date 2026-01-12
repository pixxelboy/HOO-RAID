#include "credential-store.h"
#include <obs-module.h>
#include <qt6keychain/keychain.h>
#include <QEventLoop>

CredentialStore::CredentialStore(QObject* parent)
    : QObject(parent)
{
}

void CredentialStore::storeAccessToken(const QString& token)
{
    storeKey(ACCESS_TOKEN_KEY, token);
    m_cachedAccessToken = token;
    blog(LOG_DEBUG, "[HOO-Raid] Access token stored");
}

void CredentialStore::storeRefreshToken(const QString& token)
{
    storeKey(REFRESH_TOKEN_KEY, token);
    m_cachedRefreshToken = token;
    blog(LOG_DEBUG, "[HOO-Raid] Refresh token stored");
    emit credentialsStored();
}

QString CredentialStore::retrieveAccessToken() const
{
    loadCache();
    return m_cachedAccessToken;
}

QString CredentialStore::retrieveRefreshToken() const
{
    loadCache();
    return m_cachedRefreshToken;
}

void CredentialStore::clearCredentials()
{
    deleteKey(ACCESS_TOKEN_KEY);
    deleteKey(REFRESH_TOKEN_KEY);
    m_cachedAccessToken.clear();
    m_cachedRefreshToken.clear();
    blog(LOG_INFO, "[HOO-Raid] Credentials cleared");
    emit credentialsCleared();
}

bool CredentialStore::hasCredentials() const
{
    loadCache();
    return !m_cachedAccessToken.isEmpty();
}

void CredentialStore::storeKey(const QString& key, const QString& value)
{
    QKeychain::WritePasswordJob job(SERVICE_NAME);
    job.setAutoDelete(false);
    job.setKey(key);
    job.setTextData(value);

    QEventLoop loop;
    connect(&job, &QKeychain::WritePasswordJob::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error() != QKeychain::NoError) {
        blog(LOG_ERROR, "[HOO-Raid] Failed to store credential '%s': %s",
             key.toUtf8().constData(), job.errorString().toUtf8().constData());
        emit error(job.errorString());
    }
}

QString CredentialStore::retrieveKey(const QString& key) const
{
    QKeychain::ReadPasswordJob job(SERVICE_NAME);
    job.setAutoDelete(false);
    job.setKey(key);

    QEventLoop loop;
    connect(&job, &QKeychain::ReadPasswordJob::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error() != QKeychain::NoError) {
        if (job.error() != QKeychain::EntryNotFound) {
            blog(LOG_ERROR, "[HOO-Raid] Failed to retrieve credential '%s': %s",
                 key.toUtf8().constData(), job.errorString().toUtf8().constData());
        }
        return QString();
    }

    return job.textData();
}

void CredentialStore::deleteKey(const QString& key)
{
    QKeychain::DeletePasswordJob job(SERVICE_NAME);
    job.setAutoDelete(false);
    job.setKey(key);

    QEventLoop loop;
    connect(&job, &QKeychain::DeletePasswordJob::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error() != QKeychain::NoError && job.error() != QKeychain::EntryNotFound) {
        blog(LOG_ERROR, "[HOO-Raid] Failed to delete credential '%s': %s",
             key.toUtf8().constData(), job.errorString().toUtf8().constData());
    }
}

void CredentialStore::loadCache() const
{
    if (m_cacheLoaded) {
        return;
    }

    m_cachedAccessToken = retrieveKey(ACCESS_TOKEN_KEY);
    m_cachedRefreshToken = retrieveKey(REFRESH_TOKEN_KEY);
    m_cacheLoaded = true;
}
