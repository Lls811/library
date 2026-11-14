# 图书馆管理系统：第一阶段架构设计

> 项目：Library / Qt Library Manager  
> 技术基线：C++17 + Qt 6（Widgets 为主，QML/Qt Quick 用于动效）+ MySQL 8  备选：Supabase PostgreSQL

## 1. 目标与边界

本项目分为两个可切换端：

- **User 端**：检索图书、查看详情、借阅/归还、续借、预约、个人借阅记录。
- **Manager 端**：图书与分类管理、借阅审核、逾期处理、读者管理、馆员管理、统计看板、系统配置。

系统启动后先进行身份认证，再依据角色进入对应工作台；Manager 可在权限允许时预览 User 端。端切换必须经过权限校验，不能只靠 UI 隐藏按钮实现。

## 2. 总体框架图

```text
┌────────────────────────────────────────────────────────────────┐
│                         Qt Desktop App                         │
│  AppShell / Login / RoleSwitcher / Theme / Toast / Dialog     │
├───────────────────────┬────────────────────────────────────────┤
│       User 端          │               Manager 端              │
│ Search & Detail        │ Dashboard / Statistics                │
│ Borrow / Return        │ Books / Kinds / Inventory              │
│ Reservation            │ Borrow Approval / Overdue             │
│ My Records             │ Readers / Workers / Permissions        │
└──────────────┬────────┴───────────────────────┬────────────────┘
               │                                │
               └──────────────┬─────────────────┘
                              ▼
┌────────────────────────────────────────────────────────────────┐
│                         Application Layer                      │
│ AuthService │ BookService │ BorrowService │ StatsService       │
│ KindService │ WorkerService │ InformationService              │
├────────────────────────────────────────────────────────────────┤
│                         Domain / Model                         │
│ User │ Worker │ Book │ BookCopy │ Kind │ Borrow │ Reservation  │
│ Notice │ AuditLog │ Permission                                  │
├────────────────────────────────────────────────────────────────┤
│                       Infrastructure Layer                     │
│ Repositories │ MySQL Connection Pool │ Transaction │ Logger    │
│ Config │ Migration │ DTO/Mapper │ Export (CSV/PDF optional)    │
└──────────────────────────────┬─────────────────────────────────┘
                               ▼
                    MySQL 8 / Supabase PostgreSQL
```

## 3. 模块拆分

### P0：四周内必须完成

1. **数目统计 / Dashboard**
   - 藏书总量、可借数量、借出数量、逾期数量、读者数。
   - 近 7/30 天借阅趋势、热门分类、逾期排行。
   - Manager 端只读取聚合接口，避免页面直接拼 SQL。

2. **栏目管理 / Kinds**
   - 分类树或扁平分类：编码、名称、描述、状态、创建/更新时间。
   - 分类下图书数量、启用/停用、重复名称校验。

3. **日期信息管理 / Information Module**
   - 开馆/闭馆时间、借阅期限、续借次数、逾期规则、节假日配置。
   - 系统公告与重要通知。
   - 所有规则带生效时间，避免修改配置后历史记录被重算。

4. **图书馆人员管理 / Worker**
   - 馆员账号、姓名、工号、岗位、状态、联系方式。
   - 角色与权限：管理员、馆员、只读统计员。
   - 登录失败次数、最后登录时间、操作审计日志。

5. **图书与库存管理 / Books & Inventory**
   - 书目与实体副本分离：Book 保存 ISBN/书名/作者，BookCopy 保存条码/位置/状态。
   - 入库、下架、盘点、遗失/损坏、批量导入。

6. **借阅流转 / Circulation**
   - 借出、归还、续借、预约、逾期计算。
   - 借还操作使用数据库事务；同一副本禁止并发借出。

### P1：可作为第四周加分项

- 读者管理与黑名单。
- CSV 导入导出与打印借阅单。
- 操作审计查询。
- 深色/浅色主题、动效开关、无障碍字号设置。

## 4. 建议数据模型

```text
users ──< borrow_records >── book_copies >── books >── kinds
  │             │                              │
  └──< reservations                              └── authors（可选）

workers ──< audit_logs
system_settings
notices
```

核心表建议：`users`、`workers`、`roles`、`permissions`、`books`、`book_copies`、`kinds`、`borrow_records`、`reservations`、`system_settings`、`notices`、`audit_logs`。

数据库方案建议优先采用 **MySQL 8**：课程项目本地演示和 Qt 连接更直观；如果需要多人远程协作，再把 Repository 层保持 SQL 方言简单并迁移到 Supabase。不要让 UI 直接依赖 Supabase SDK，这样本地 MySQL 与远程数据库可以切换。

## 5. Qt 工程目录建议

```text
Library/
├─ CMakeLists.txt
├─ docs/                         # 设计、接口、进度地图
├─ resources/                    # qrc、图标、字体、主题
├─ sql/migrations/               # 建表与增量迁移
├─ src/
│  ├─ app/                       # main、AppShell、路由、主题
│  ├─ core/                      # Result、错误、配置、日志、权限
│  ├─ domain/models/             # 领域模型
│  ├─ application/services/      # 用例服务
│  ├─ infrastructure/db/        # 连接、事务、Repository
│  └─ presentation/
│     ├─ common/                 # 通用控件、表格、弹窗
│     ├─ user/                   # User 端页面
│     └─ manager/                # Manager 端页面
├─ tests/
└─ .github/workflows/            # build + test
```

## 6. UI 设计方向

- 基础色：深海军蓝背景 `#08111F`，青蓝强调 `#38BDF8`，紫色辅助 `#8B5CF6`，成功色 `#34D399`。
- “液态玻璃”只用于导航栏、卡片、弹窗和侧边栏：半透明背景、细边框、柔和阴影、轻微背景模糊；数据表格保持高对比度，避免玻璃效果降低可读性。
- 交互：侧边栏端切换、卡片 hover、页面进入淡入/上移、统计数字递增、加载骨架屏、成功/失败 Toast。
- Qt 实现优先级：先用 QSS + QWidget 保证稳定，再用 QPropertyAnimation/QGraphicsOpacityEffect 增加动效；只有复杂背景和图表动画才引入 Qt Quick。

## 7. 五人分工与交付物

| 角色 | 负责模块 | 主要交付 |
|---|---|---|
| 队长/架构 | 工程骨架、登录、角色切换、代码评审、发布 | CMake、AppShell、权限边界、集成版本 |
| 队员 A | 数据库与基础设施 | schema、migration、连接层、Repository、测试数据 |
| 队员 B | User 端 | 搜索、详情、借还、预约、个人记录 |
| 队员 C | Manager 端 | 图书、Kinds、库存、Worker 管理 |
| 队员 D | 统计与 UI 体验 | Dashboard、图表、主题、动效、交互规范 |

每个模块必须包含：页面截图、数据模型/接口说明、正常流程、异常流程、最小测试、提交记录。

## 8. 四周里程碑

| 周期 | 目标 | 验收标准 |
|---|---|---|
| 第 1 周 | 需求冻结、工程骨架、数据库、登录/角色 | 能编译、能登录、能切 User/Manager、迁移可重复执行 |
| 第 2 周 | 图书/Kinds/库存、User 搜索与详情 | 可新增图书和副本，User 能搜索并看到可借状态 |
| 第 3 周 | 借还/预约/逾期、Worker、Information | 借还事务正确，权限生效，规则可配置 |
| 第 4 周 | Dashboard、UI 动效、联调、测试、答辩材料 | 关键流程闭环，CI 构建通过，有演示数据和项目文档 |

## 9. Git 协作约定

```text
main                 可演示/可发布
develop              集成分支
feature/<module>     单模块开发
fix/<issue>          缺陷修复
```

建议流程：

```bash
git clone https://github.com/lls811/Library.git
git fetch origin
git checkout -b feature/kinds develop
git add .
git commit -m "feat(kinds): add category management skeleton"
git push -u origin feature/kinds
```

提交信息使用 `feat/fix/docs/test/refactor/chore` 前缀；每个 PR 只解决一个模块，至少一名成员 review 后合并到 `develop`，阶段验收后再合并 `main`。

## 10. 当前开发顺序

1. 确认 Qt 版本、编译器（建议 Qt 6.7+ / MinGW 或 MSVC）和数据库连接方式。
2. 建立 CMake 工程、目录结构、迁移脚本与 CI。
3. 实现登录与双端 AppShell。
4. 先做 `Kinds → Books → BookCopy`，再做借阅流转，最后接 Dashboard 聚合数据。
5. 每完成一个模块更新 `docs/PROJECT_MAP.md`，记录负责人、状态、分支、PR、风险和下一步。
