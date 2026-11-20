#ifndef LIBRARY_COMMON_APPLICATIONCONTEXT_H//头文件保护
#define LIBRARY_COMMON_APPLICATIONCONTEXT_H//宏定义

// 应用会话：由队长维护。
// 统一保存当前用户、User/Manager 端和权限，页面不再各自维护登录状态。

#include "library_api.h"

class ApplicationContext
{
public:
    const Library::Session &session() const { return currentSession; }
    bool isLoggedIn() const { return currentSession.authenticated; }//判断当前用户是否已经登录

    void setSession(const Library::Session &session)//当前会话函数
    {
        currentSession = session;//保存登录信息
        currentSession.authenticated = true;//已登录
    }

    void logout() { currentSession = {}; }//定义注销函数

    bool switchClient(Library::ClientType client)//定义切换客户端类型函数
    {
        if (!isLoggedIn()) return false;
        if (client == Library::ClientType::Manager &&
            currentSession.role == Library::WorkerRole::Reader) return false;
        currentSession.client = client;
        return true;
    }

private:
    Library::Session currentSession;//保存当前会话
};

#endif // LIBRARY_COMMON_APPLICATIONCONTEXT_H

/*
登录成功 → setSession()
检查登录 → isLoggedIn()
读取当前用户 → session()
切换用户端/管理端 → switchClient()
退出登录 → logout()
*/