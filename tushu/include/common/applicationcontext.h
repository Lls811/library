#ifndef LIBRARY_COMMON_APPLICATIONCONTEXT_H
#define LIBRARY_COMMON_APPLICATIONCONTEXT_H

// 应用会话：由队长维护。
// 统一保存当前用户、User/Manager 端和权限，页面不再各自维护登录状态。

#include "library_api.h"

class ApplicationContext
{
public:
    const Library::Session &session() const { return currentSession; }
    bool isLoggedIn() const { return currentSession.authenticated; }

    void setSession(const Library::Session &session)
    {
        currentSession = session;
        currentSession.authenticated = true;
    }

    void logout() { currentSession = {}; }

    bool switchClient(Library::ClientType client)
    {
        if (!isLoggedIn()) return false;
        if (client == Library::ClientType::Manager &&
            currentSession.role == Library::WorkerRole::Reader) return false;
        currentSession.client = client;
        return true;
    }

private:
    Library::Session currentSession;
};

#endif // LIBRARY_COMMON_APPLICATIONCONTEXT_H
