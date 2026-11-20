#ifndef LIBRARY_DATABASE_LIBRARYDATABASE_H
#define LIBRARY_DATABASE_LIBRARYDATABASE_H

// 数据库访问层：由队长和数据库负责人共同维护。
// 页面和业务模块不能直接编写 SQL，统一通过本类或 Repository 访问数据库。

#include "../common/domain.h"
#include <QList> //Qt 的 QList 容器，用于保存多个对象。
#include <QString>

class QSqlDatabase; //前向声明，减少头文件依赖，加快编译速度。

class LibraryDatabase  //统一管理图书馆系统的数据访问
{
public:
    LibraryDatabase();   //声明构造函数，通常用于初始化数据库相关成员。
    ~LibraryDatabase();

    bool open();  //打开数据库
    bool isReady() const { return ready; }  //判断数据库是否准备好
    QString lastError() const { return errorMessage; } //获取最近一次错误信息
    bool load(QList<Book> &books, QList<Reader> &readers, QList<Loan> &loans,
              int &nextBookId, int &nextReaderId);  //声明加载数据的函数，如：图书列表，读者列表，借阅记录列表
    bool save(const QList<Book> &books, const QList<Reader> &readers,
              const QList<Loan> &loans);  //声明保存数据的函数

private:
    bool createSchema(); //声明创建数据库结构的函数
    bool execute(const QString &sql); //声明执行 SQL 语句的辅助函数
    void setError(const QString &message); //声明设置错误信息的辅助函数

    QSqlDatabase *database = nullptr; //定义一个指向 QSqlDatabase 的指针
    bool ready = false;
    QString errorMessage; //保存最近一次错误信息
};

#endif // LIBRARY_DATABASE_LIBRARYDATABASE_H
