#ifndef LIBRARY_TEAM3_WORKERSERVICE_H
#define LIBRARY_TEAM3_WORKERSERVICE_H

// 队员 3：用户与馆员管理。
// 负责读者、馆员、角色、账号状态、密码和黑名单。
// 不负责主窗口导航和全局登录会话。

#include "../common/library_api.h"

namespace Library::Team3 {
using WorkerService = IWorkerService;
}

#endif // LIBRARY_TEAM3_WORKERSERVICE_H
