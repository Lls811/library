#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "include/common/applicationcontext.h"
#include "include/common/domain.h"
#include "include/database/librarydatabase.h"

class QLineEdit;
class QStackedWidget;
class QTableWidget;
class QLabel;
// 主窗口只负责界面展示、用户输入校验和业务流程编排。
// 领域数据由 domain.h 定义，数据库操作由 LibraryDatabase 完成。
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    // 登录页和主界面之间的状态切换。
    void login(); void logout();
    // 图书管理：查询、展示以及基本维护。
    void refreshBooks(); void addBook(); void editBook(); void deleteBook();
    // 读者管理：维护借阅人基础资料。
    void refreshReaders(); void addReader(); void editReader(); void deleteReader();
    // 借还管理：变更借阅状态并同步库存。
    void refreshLoans(); void borrowBook(); void returnBook();

private:
    void seedData(); void loadData(); void saveData();
    void buildLoginPage(); void buildMainPage();
    QWidget *booksPage(); QWidget *readersPage(); QWidget *loansPage();
    void error(const QString &message);
    Book *selectedBook(); Reader *selectedReader(); Loan *selectedLoan();
    QString bookTitle(int id) const; QString readerName(int id) const;

    // pages[0] 是登录页，pages[1] 是登录成功后创建的主页面。
    QStackedWidget *pages = nullptr; QLineEdit *username = nullptr; QLineEdit *password = nullptr;
    QLabel *userLabel = nullptr; QTableWidget *bookTable = nullptr; QTableWidget *readerTable = nullptr; QTableWidget *loanTable = nullptr;
    QLineEdit *bookSearch = nullptr; QLineEdit *readerNameEdit = nullptr; QLineEdit *readerTypeEdit = nullptr; QLineEdit *readerPhoneEdit = nullptr;
    QLineEdit *borrowReader = nullptr; QLineEdit *borrowBookEdit = nullptr;
    // 这些列表是界面运行期间的工作数据；每次修改后由 saveData() 持久化。
    QList<Book> books; QList<Reader> readers; QList<Loan> loans;
    int nextBookId = 1004; int nextReaderId = 4;
    ApplicationContext appContext;
    LibraryDatabase database;
};

#endif
