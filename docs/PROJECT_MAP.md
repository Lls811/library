# 项目开发地图

> 状态：🟡 基础框架已搭建（2026-11-14）

## 团队状态

| 成员 | 角色 | 当前任务 | 状态 |
|---|---|---|---|
| 队长 | 架构/集成 | 确认 Qt、CMake、分支策略 | 🟡 待开始 |
| 队员 A | 数据库/后端基础设施 | 设计 schema 与 migration | 🟡 待开始 |
| 队员 B | User 端 | 设计检索与借阅流程 | 🟡 待开始 |
| 队员 C | Manager 端 | 设计图书、Kinds、Worker 页面 | 🟡 待开始 |
| 队员 D | 统计/UI | 设计 Dashboard 与 Liquid Glass 规范 | 🟡 待开始 |

## 模块状态

| 模块 | 优先级 | 负责人 | 分支 | 状态 | 下一步 |
|---|---:|---|---|---|---|
| 工程骨架与登录 | P0 | 队长 | `feature/app-shell` | 🟡 进行中 | 已加入统一 API 头文件与 ApplicationContext |
| 数据库与 Repository | P0 | 队员 A | `feature/database` | ⬜ 未开始 | 建立 migration |
| Kinds | P0 | 队员 C | `feature/kinds` | ⬜ 未开始 | 确认分类字段 |
| Books/Inventory | P0 | 队员 C | `feature/books` | ⬜ 未开始 | 设计 Book/Copy 分离 |
| User 端 | P0 | 队员 B | `feature/user-client` | ⬜ 未开始 | 绘制主流程 |
| Circulation | P0 | 队员 B | `feature/circulation` | ⬜ 未开始 | 定义借还事务 |
| Worker/Permission | P0 | 队员 C | `feature/workers` | ⬜ 未开始 | 定义角色矩阵 |
| Information | P0 | 队员 A | `feature/information` | ⬜ 未开始 | 定义规则版本化 |
| Dashboard | P0 | 队员 D | `feature/dashboard` | ⬜ 未开始 | 确认统计口径 |
| UI Theme/Motion | P1 | 队员 D | `feature/ui-theme` | ⬜ 未开始 | 建立 QSS token |
| CI/测试/发布 | P0 | 队长+A | `feature/ci` | ⬜ 未开始 | 配置构建矩阵 |

## 状态定义

- ⬜ 未开始
- 🟡 进行中
- 🔵 待联调
- ✅ 已完成
- 🔴 有风险/阻塞

每次合并 PR 后同步本文件：负责人、状态、提交/PR、验证结果、遗留风险。
