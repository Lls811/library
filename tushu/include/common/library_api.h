#ifndef LIBRARY_COMMON_LIBRARY_API_H
#define LIBRARY_COMMON_LIBRARY_API_H

// 公共模块接口：由队长统一维护。
// 组员之间只能通过这里的模型、枚举和 Service 接口通信，减少头文件冲突。
/*
公共接口头文件，主要定义：
- 用户端和管理端类型
- 工作人员角色
- 登录会话
- 图书分类、工作人员、系统配置、统计数据
- 操作结果
- 图书、分类、借阅、工作人员等服务接口
*/

#include "domain.h"
#include <QDate> //日期类型
#include <QList> //引入Qt 列表类型 QList，用于返回多个对象
#include <QString> //字符串类型

namespace Library {

enum class ClientType { User, Manager }; //当前使用的客户端类型。
enum class WorkerRole { Reader, Librarian, Administrator, Statistician };//定义工作人员角色枚举。
                    //读者，馆员（管理图书），管理员（管理工作人员等系统内容），统计员（可以查看系统信息）。

//保存当前用户信息
struct Session {
    bool authenticated = false;
    int userId = 0; //当前用户编号。
    QString username; //登录用户名。
    QString displayName; //用户显示名称。
    //比如：username = "zhangsan"， displayName = "张三"

    ClientType client = ClientType::User;//当前使用的客户端类型，默认是用户端。
    WorkerRole role = WorkerRole::Reader;//当前用户角色，默认是普通读者。

    bool canManageBooks() const //定义一个权限判断函数，判断当前角色是否可以管理图书。
    {
        return role == WorkerRole::Librarian || role == WorkerRole::Administrator; //判断角色是馆员或管理员
    }
    bool canManageWorkers() const { return role == WorkerRole::Administrator; } //判断是否可以管理工作人员，只有管理员返回 true。
    bool canViewStatistics() const { return role != WorkerRole::Reader; } //判断是否可以查看统计数据，只要角色不是普通读者，就返回 true。
};

//图书分类
struct Kind {
    int id = 0;
    QString code;
    QString name;
    QString description;
    bool enabled = true; //分类是否启用
};
//如："文学"，"CS"，"HISTORY"

//工作人员
struct Worker {
    int id = 0;
    QString employeeNo; //工号，如L2026001
    QString name;
    QString phone;
    WorkerRole role = WorkerRole::Librarian; //角色，默认是馆员
    bool enabled = true; //是否有效工作人员
    QDate lastLoginDate; //最后登录日期
};

//系统信息
struct SystemInformation {
    QString libraryName = QStringLiteral("智慧图书馆");
    QString openingTime = QStringLiteral("08:00");
    QString closingTime = QStringLiteral("22:00");
    int loanDays = 30; //默认借阅期限30天
    int maxRenewals = 1; //最多续借次数为1次
    int overdueFineCentsPerDay = 0; //每天逾期罚金
};

//统计信息
struct Statistics {
    int totalTitles = 0; //图书种类总数
    int totalCopies = 0; //图书册数总数
    int availableCopies = 0; //当前可借出的图书册数
    int borrowedCopies = 0; //当前已借出的图书册数
    int overdueLoans = 0; //逾期借阅记录数量
    int registeredReaders = 0; //已注册读者数量
    int activeWorkers = 0; //当前启用的工作人员数量
};

//操作结果函数，表示操作是否成功
struct OperationResult {
    bool ok = false;
    QString message;
    static OperationResult success(const QString &message = {}) { return {true, message}; }
    static OperationResult failure(const QString &message) { return {false, message}; }
};

//图书服务接口
class IBookService {
public:
    virtual ~IBookService() = default;
    virtual QList<Book> searchBooks(const QString &keyword = {}) const = 0; //搜索图书的纯虚函数
    virtual OperationResult addBook(const Book &book) = 0; //添加图书
    virtual OperationResult updateBook(const Book &book) = 0; //修改图书信息
    virtual OperationResult removeBook(int bookId) = 0; //根据图书编号删除图书
};

//分类服务接口
class IKindService {
public:
    virtual ~IKindService() = default;
    virtual QList<Kind> listKinds(bool enabledOnly = false) const = 0; //获取分类列表
    virtual OperationResult saveKind(const Kind &kind) = 0; //保存分类
    virtual OperationResult disableKind(int kindId) = 0; //停用指定分类
};

//借阅服务接口
class IBorrowService {
public:
    virtual ~IBorrowService() = default;
    virtual OperationResult borrowBook(int readerId, int bookId) = 0; //借书操作
    virtual OperationResult returnBook(int readerId, int bookId) = 0; //还书操作
    virtual OperationResult renewBook(int readerId, int bookId) = 0; //续借操作
    virtual QList<Loan> currentLoans(int readerId = 0) const = 0; //查询当前借阅记录
};

//工作人员服务接口
class IWorkerService {
public:
    virtual ~IWorkerService() = default;
    virtual QList<Worker> listWorkers() const = 0; //获取工作人员列表
    virtual OperationResult saveWorker(const Worker &worker) = 0; //保存工作人员信息
    virtual OperationResult disableWorker(int workerId) = 0; //停用指定工作人员
};

//系统信息服务接口
class IInformationService {
public:
    virtual ~IInformationService() = default;
    virtual SystemInformation information() const = 0; //获取当前系统配置信息
    virtual OperationResult updateInformation(const SystemInformation &information) = 0; //更新系统配置信息
};

//统计服务接口
class IStatisticsService {
public:
    virtual ~IStatisticsService() = default;
    virtual Statistics summary() const = 0;
};
/*
返回一个 Statistics 对象，包括：图书种类数，图书册数，
可借数量，已借数量，逾期数量，读者数量，工作人员数量。
*/

} // namespace Library

#endif // LIBRARY_COMMON_LIBRARY_API_H
