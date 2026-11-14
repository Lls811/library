#ifndef LIBRARY_TEAM4_STATISTICSSERVICE_H
#define LIBRARY_TEAM4_STATISTICSSERVICE_H

// 队员 4：统计与系统信息。
// 负责 Dashboard、借阅趋势、热门图书、逾期统计、公告和系统规则。
// 统计模块只读业务服务，不直接修改库存和借阅记录。

#include "../common/library_api.h"

namespace Library::Team4 {
using StatisticsService = IStatisticsService;
using InformationService = IInformationService;
}

#endif // LIBRARY_TEAM4_STATISTICSSERVICE_H
