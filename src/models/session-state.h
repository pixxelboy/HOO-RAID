#pragma once

#include <QString>

// Authentication state
enum class AuthState {
    NotAuthenticated,
    Authenticating,
    Authenticated,
    TokenExpired,
    Error
};

// UI operation state
enum class OperationState {
    Idle,
    Loading,
    Success,
    Error
};

struct SessionState {
    // Authentication
    AuthState authState = AuthState::NotAuthenticated;
    QString authError;

    // Current user (when authenticated)
    QString userId;
    QString userLogin;
    QString userName;

    // UI state
    OperationState operationState = OperationState::Idle;
    QString operationError;

    // Rate limiting
    bool isRateLimited = false;
    int rateLimitResetSeconds = 0;

    // Helpers
    bool isAuthenticated() const { return authState == AuthState::Authenticated; }
    bool isLoading() const { return operationState == OperationState::Loading; }
    bool hasError() const { return operationState == OperationState::Error || authState == AuthState::Error; }

    // Reset methods
    void clearAuth();
    void clearOperation();
    void clearRateLimit();
};
