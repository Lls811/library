#ifndef LIBRARY_COMMON_LIBRARY_API_H
#define LIBRARY_COMMON_LIBRARY_API_H

// 公共模块接口：由队长统一维护。
// 组员之间只能通过这里的模型、枚举和 Service 接口通信，减少头文件冲突。

#include "domain.h"
#include <QDate>
#include <QList>
#include <QString>

namespace Library {

enum class ClientType { User, Manager };
enum class WorkerRole { Reader, Librarian, Administrator, Statistician };

struct Session {
    bool authenticated = false;
    int userId = 0;
    QString username;
    QString displayName;
    ClientType client = ClientType::User;
    WorkerRole role = WorkerRole::Reader;

    bool canManageBooks() const
    {
        return role == WorkerRole::Librarian || role == WorkerRole::Administrator;
    }
    bool canManageWorkers() const { return role == WorkerRole::Administrator; }
    bool canViewStatistics() const { return role != WorkerRole::Reader; }
};

struct Kind {
    int id = 0;
    QString code;
    QString name;
    QString description;
    bool enabled = true;
};

struct Worker {
    int id = 0;
    QString employeeNo;
    QString name;
    QString phone;
    WorkerRole role = WorkerRole::Librarian;
    bool enabled = true;
    QDate lastLoginDate;
};

struct SystemInformation {
    QString libraryName = QStringLiteral("智慧图书馆");
    QString openingTime = QStringLiteral("08:00");
    QString closingTime = QStringLiteral("22:00");
    int loanDays = 30;
    int maxRenewals = 1;
    int overdueFineCentsPerDay = 0;
};

struct Statistics {
    int totalTitles = 0;
    int totalCopies = 0;
    int availableCopies = 0;
    int borrowedCopies = 0;
    int overdueLoans = 0;
    int registeredReaders = 0;
    int activeWorkers = 0;
};

struct OperationResult {
    bool ok = false;
    QString message;
    static OperationResult success(const QString &message = {}) { return {true, message}; }
    static OperationResult failure(const QString &message) { return {false, message}; }
};

class IBookService {
public:
    virtual ~IBookService() = default;
    virtual QList<Book> searchBooks(const QString &keyword = {}) const = 0;
    virtual OperationResult addBook(const Book &book) = 0;
    virtual OperationResult updateBook(const Book &book) = 0;
    virtual OperationResult removeBook(int bookId) = 0;
};

class IKindService {
public:
    virtual ~IKindService() = default;
    virtual QList<Kind> listKinds(bool enabledOnly = false) const = 0;
    virtual OperationResult saveKind(const Kind &kind) = 0;
    virtual OperationResult disableKind(int kindId) = 0;
};

class IBorrowService {
public:
    virtual ~IBorrowService() = default;
    virtual OperationResult borrowBook(int readerId, int bookId) = 0;
    virtual OperationResult returnBook(int readerId, int bookId) = 0;
    virtual OperationResult renewBook(int readerId, int bookId) = 0;
    virtual QList<Loan> currentLoans(int readerId = 0) const = 0;
};

class IWorkerService {
public:
    virtual ~IWorkerService() = default;
    virtual QList<Worker> listWorkers() const = 0;
    virtual OperationResult saveWorker(const Worker &worker) = 0;
    virtual OperationResult disableWorker(int workerId) = 0;
};

class IInformationService {
public:
    virtual ~IInformationService() = default;
    virtual SystemInformation information() const = 0;
    virtual OperationResult updateInformation(const SystemInformation &information) = 0;
};

class IStatisticsService {
public:
    virtual ~IStatisticsService() = default;
    virtual Statistics summary() const = 0;
};

} // namespace Library

#endif // LIBRARY_COMMON_LIBRARY_API_H
