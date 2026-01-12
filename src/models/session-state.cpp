#include "session-state.h"

void SessionState::clearAuth()
{
    authState = AuthState::NotAuthenticated;
    authError.clear();
    userId.clear();
    userLogin.clear();
    userName.clear();
}

void SessionState::clearOperation()
{
    operationState = OperationState::Idle;
    operationError.clear();
}

void SessionState::clearRateLimit()
{
    isRateLimited = false;
    rateLimitResetSeconds = 0;
}
