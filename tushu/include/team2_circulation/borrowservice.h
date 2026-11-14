#ifndef LIBRARY_TEAM2_BORROWSERVICE_H
#define LIBRARY_TEAM2_BORROWSERVICE_H

// 队员 2：借阅业务。
// 负责借书、还书、续借、预约、逾期判断和库存同步。
// 只通过 IBookService 查询图书，不改动图书模块页面。

#include "../common/library_api.h"

namespace Library::Team2 {
using BorrowService = IBorrowService;
}

#endif // LIBRARY_TEAM2_BORROWSERVICE_H
