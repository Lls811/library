#ifndef LIBRARY_COMMON_DOMAIN_H
#define LIBRARY_COMMON_DOMAIN_H

// 公共领域模型：由队长维护。
// 所有组员必须复用这些类型，不得在自己的模块中重复定义 Book、Reader、Loan。

#include <QDate>
#include <QString>

struct Book {
    int id = 0;
    QString title;
    QString author;
    QString category;
    int total = 0;
    int available = 0;
};

struct Reader {
    int id = 0;
    QString name;
    QString type;
    QString phone;
};

struct Loan {
    int readerId = 0;
    int bookId = 0;
    QDate borrowDate;
    QDate dueDate;
    QDate returnDate;
    bool returned = false;
};

#endif // LIBRARY_COMMON_DOMAIN_H
