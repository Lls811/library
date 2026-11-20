#ifndef LIBRARY_TEAM1_BOOKSERVICE_H
#define LIBRARY_TEAM1_BOOKSERVICE_H

// 队员 1：图书与分类管理。
// 负责 Book、BookCopy、Kind 的增删改查和分类筛选。
// 不负责借还书，不直接修改 MainWindow。

#include "../common/library_api.h"

namespace Library::Team1 {
using BookService = IBookService;
using KindService = IKindService;
}

#endif // LIBRARY_TEAM1_BOOKSERVICE_H
/*
把公共接口 IBookService 和 IKindService 暴露给 Library::Team1 命名空间使用。
*/