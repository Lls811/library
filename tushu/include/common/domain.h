#ifndef LIBRARY_COMMON_DOMAIN_H
#define LIBRARY_COMMON_DOMAIN_H

// 公共领域模型：由队长维护。
// 所有组员必须复用这些类型，不得在自己的模块中重复定义 Book、Reader、Loan。

#include <QDate> //Qt 的 QDate 类型，用于表示日期。
#include <QString> //Qt 的 QString 类型，用于保存字符串，例如书名、作者、读者姓名等。

//图书
struct Book {
    int id = 0;
    QString title;
    QString author;
    QString category; //图书的编号，标题，作者，分类。
    int total = 0; //数量
    int available = 0; //还没有被借出去的数量
};

//读者
struct Reader {
    int id = 0;
    QString name; //读者编号，姓名。
    QString type; //读者类型，如学生，教师，校外人员等。
    QString phone; //联系方式。
};

//借阅记录
struct Loan {
    int readerId = 0; //借阅记录属于哪位读者。
    int bookId = 0; //借的是哪本书。
    QDate borrowDate; //借阅日期
    QDate dueDate; //应还日期。
    QDate returnDate; //实际归还日期。
    bool returned = false; //尚未归还
};

#endif // LIBRARY_COMMON_DOMAIN_H

/*
Book 保存图书信息
Reader 保存读者信息
Loan 通过 readerId 和 bookId 把读者与图书关联起来
*/