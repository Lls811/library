#include "mainwindow.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVariant>

namespace {

// 统一创建按钮和表格，保证三个业务页面的交互风格一致。
QPushButton *createButton(const QString &text)
{
    auto *button = new QPushButton(text);
    button->setMinimumHeight(32);
    return button;
}

QTableWidget *createTable(const QStringList &headers)
{
    auto *table = new QTableWidget;
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    return table;
}

void setRow(QTableWidget *table, int row, const QStringList &values)
{
    for (int column = 0; column < values.size(); ++column) {
        table->setItem(row, column, new QTableWidgetItem(values.at(column)));
    }
}

} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // 构造阶段只完成数据准备和登录页创建，主页面在登录成功后按需创建。
    resize(1100, 700);
    setWindowTitle("图书馆管理系统 MVP");
    seedData();
    loadData();
    buildLoginPage();
}

void MainWindow::seedData() {
    // 演示数据也是数据库连接失败时的兜底数据，保证 MVP 仍然可以启动。
    books={{1001,"软件工程导论","张海藩","计算机",10,8},{1002,"C++ Primer","Stanley Lippman","计算机",6,4},{1003,"百年孤独","加西亚·马尔克斯","文学",5,5}};
    readers={{1,"张三","本科生","13800000001"},{2,"李四","研究生","13800000002"},{3,"王老师","教师","13800000003"}};
    loans={{1,1001,QDate::currentDate().addDays(-5),QDate::currentDate().addDays(25),{},false},{2,1002,QDate::currentDate().addDays(-40),QDate::currentDate().addDays(-10),{},false}};
}

void MainWindow::loadData()
{
    // 先准备演示数据，再尝试从 MySQL 覆盖加载；这样数据库暂时不可用时仍可演示。
    if (!database.open()) {
        // 连接失败时保留 seedData() 的演示数据，不阻塞界面启动。
        return;
    }
    if (!database.load(books, readers, loans, nextBookId, nextReaderId)) return;
    if (books.isEmpty() && readers.isEmpty() && loans.isEmpty()) saveData();
}

void MainWindow::saveData()
{
    // 数据库层内部会忽略未连接状态，因此界面操作不需要重复判断连接状态。
    // save() 是非破坏式写入，不会删除数据库中已有的历史数据。
    database.save(books, readers, loans);
}

void MainWindow::buildLoginPage() {
    pages = new QStackedWidget(this);
    setCentralWidget(pages);
    auto *p = new QWidget;
    auto *v = new QVBoxLayout(p);
    v->setContentsMargins(260, 100, 260, 100);
    auto *title = new QLabel("图书馆管理系统");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #1f4e79;");
    v->addWidget(title);

    auto *box = new QGroupBox("登录");
    auto *form = new QFormLayout(box);
    username = new QLineEdit;
    password = new QLineEdit;
    password->setEchoMode(QLineEdit::Password);
    username->setPlaceholderText("管理员：admin；读者：reader");
    password->setPlaceholderText("管理员：admin；读者：123456");
    form->addRow("用户名", username);
    form->addRow("密码", password);

    auto *loginButton = createButton("登录");
    form->addRow(loginButton);
    v->addWidget(box);

    auto *hint = new QLabel("演示账号：admin / admin，reader / 123456");
    hint->setAlignment(Qt::AlignCenter);
    v->addWidget(hint);
    v->addStretch();
    pages->addWidget(p);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::login);
    connect(password, &QLineEdit::returnPressed, this, &MainWindow::login);
}

void MainWindow::buildMainPage() {
    // 主界面只创建一次，重复登录时复用已有页面和数据表。
    if (pages->count() > 1) return;

    auto *p = new QWidget;
    auto *v = new QVBoxLayout(p);
    auto *top = new QHBoxLayout;
    userLabel = new QLabel;
    auto *logoutButton = createButton("退出登录");

    top->addWidget(new QLabel("图书馆管理系统 MVP"));
    top->addStretch();
    top->addWidget(userLabel);
    top->addWidget(logoutButton);
    v->addLayout(top);

    auto *tabs = new QTabWidget;
    tabs->addTab(booksPage(), "图书管理");
    tabs->addTab(readersPage(), "读者管理");
    tabs->addTab(loansPage(), "借还管理");
    v->addWidget(tabs);

    pages->addWidget(p);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::logout);
}

QWidget *MainWindow::booksPage() {
    auto *p = new QWidget;
    auto *v = new QVBoxLayout(p);
    auto *h = new QHBoxLayout;
    bookSearch = new QLineEdit;
    bookSearch->setPlaceholderText("按书名、作者或分类搜索");

    auto *query = createButton("查询");
    auto *add = createButton("新增");
    auto *edit = createButton("编辑");
    auto *remove = createButton("删除");
    h->addWidget(bookSearch, 1);
    h->addWidget(query); h->addWidget(add); h->addWidget(edit); h->addWidget(remove);
    v->addLayout(h);

    bookTable = createTable({"编号", "书名", "作者", "分类", "总量", "可借"});
    v->addWidget(bookTable);
    connect(query, &QPushButton::clicked, this, &MainWindow::refreshBooks);
    connect(bookSearch, &QLineEdit::returnPressed, this, &MainWindow::refreshBooks);
    connect(add, &QPushButton::clicked, this, &MainWindow::addBook);
    connect(edit, &QPushButton::clicked, this, &MainWindow::editBook);
    connect(remove, &QPushButton::clicked, this, &MainWindow::deleteBook);
    refreshBooks();
    return p;
}

QWidget *MainWindow::readersPage() {
    auto *p = new QWidget;
    auto *v = new QVBoxLayout(p);
    auto *h = new QHBoxLayout;
    readerNameEdit = new QLineEdit;
    readerTypeEdit = new QLineEdit;
    readerPhoneEdit = new QLineEdit;
    readerNameEdit->setPlaceholderText("姓名");
    readerTypeEdit->setPlaceholderText("类别");
    readerPhoneEdit->setPlaceholderText("电话");

    auto *add = createButton("新增");
    auto *edit = createButton("编辑");
    auto *remove = createButton("删除");
    h->addWidget(readerNameEdit); h->addWidget(readerTypeEdit);
    h->addWidget(readerPhoneEdit, 1);
    h->addWidget(add); h->addWidget(edit); h->addWidget(remove);
    v->addLayout(h);

    readerTable = createTable({"编号", "姓名", "类别", "电话"});
    v->addWidget(readerTable);
    connect(add, &QPushButton::clicked, this, &MainWindow::addReader);
    connect(edit, &QPushButton::clicked, this, &MainWindow::editReader);
    connect(remove, &QPushButton::clicked, this, &MainWindow::deleteReader);
    refreshReaders();
    return p;
}

QWidget *MainWindow::loansPage() {
    auto *p = new QWidget;
    auto *v = new QVBoxLayout(p);
    auto *h = new QHBoxLayout;
    borrowReader = new QLineEdit;
    borrowBookEdit = new QLineEdit;
    borrowReader->setPlaceholderText("读者编号");
    borrowBookEdit->setPlaceholderText("图书编号");

    auto *borrow = createButton("借书");
    auto *returnButton = createButton("还书");
    auto *refresh = createButton("刷新");
    h->addWidget(borrowReader); h->addWidget(borrowBookEdit);
    h->addWidget(borrow); h->addWidget(returnButton); h->addWidget(refresh);
    h->addStretch();
    v->addLayout(h);

    loanTable = createTable({"读者", "图书", "借书日期", "应还日期", "状态"});
    v->addWidget(loanTable);
    connect(borrow, &QPushButton::clicked, this, &MainWindow::borrowBook);
    connect(returnButton, &QPushButton::clicked, this, &MainWindow::returnBook);
    connect(refresh, &QPushButton::clicked, this, &MainWindow::refreshLoans);
    refreshLoans();
    return p;
}

void MainWindow::login()
{
    const auto user = username->text().trimmed();
    const auto pass = password->text();
    const bool valid = (user == "admin" && pass == "admin")
                       || (user == "reader" && pass == "123456");

    if (!valid) {
        error("用户名或密码错误。请使用 admin/admin 或 reader/123456。");
        return;
    }

    Library::Session session;
    session.userId = user == "admin" ? 1 : 2;
    session.username = user;
    session.displayName = user == "admin" ? "系统管理员" : "演示读者";
    session.client = user == "admin" ? Library::ClientType::Manager
                                      : Library::ClientType::User;
    session.role = user == "admin" ? Library::WorkerRole::Administrator
                                    : Library::WorkerRole::Reader;
    appContext.setSession(session);

    buildMainPage();
    userLabel->setText(QString("当前端：%1 | 用户：%2")
                           .arg(session.client == Library::ClientType::Manager
                                    ? "Manager"
                                    : "User",
                                session.displayName));
    pages->setCurrentIndex(1);
}

void MainWindow::logout()
{
    appContext.logout();
    username->clear();
    password->clear();
    pages->setCurrentIndex(0);
}

Book *MainWindow::selectedBook()
{
    if (!bookTable || bookTable->currentRow() < 0) {
        return nullptr;
    }

    const int id = bookTable->item(bookTable->currentRow(), 0)->text().toInt();
    for (auto &book : books) {
        if (book.id == id) {
            return &book;
        }
    }
    return nullptr;
}

Reader *MainWindow::selectedReader()
{
    if (!readerTable || readerTable->currentRow() < 0) {
        return nullptr;
    }

    const int id = readerTable->item(readerTable->currentRow(), 0)->text().toInt();
    for (auto &reader : readers) {
        if (reader.id == id) {
            return &reader;
        }
    }
    return nullptr;
}

Loan *MainWindow::selectedLoan()
{
    if (!loanTable || loanTable->currentRow() < 0) {
        return nullptr;
    }

    auto *readerItem = loanTable->item(loanTable->currentRow(), 0);
    auto *bookItem = loanTable->item(loanTable->currentRow(), 1);
    const int readerId = readerItem->data(Qt::UserRole).toInt();
    const int bookId = bookItem->data(Qt::UserRole).toInt();

    for (auto &loan : loans) {
        if (loan.readerId == readerId && loan.bookId == bookId && !loan.returned) {
            return &loan;
        }
    }
    return nullptr;
}

void MainWindow::refreshBooks()
{
    // 每次刷新先清空表格，再按当前搜索条件重新生成可见行。
    // 查询只过滤当前内存数据，修改后的数据会在操作完成时同步到数据库。
    bookTable->setRowCount(0);
    const auto query = bookSearch ? bookSearch->text().trimmed() : QString();

    for (const auto &book : books) {
        const auto searchable = QStringList{
            book.title, book.author, book.category
        }.join(" ");
        if (!query.isEmpty() && !searchable.contains(query, Qt::CaseInsensitive)) {
            continue;
        }

        const int row = bookTable->rowCount();
        bookTable->insertRow(row);
        setRow(bookTable, row, {
            QString::number(book.id), book.title, book.author, book.category,
            QString::number(book.total), QString::number(book.available)
        });
    }
}

void MainWindow::refreshReaders()
{
    // 读者输入框用于新增，表格当前行用于编辑或删除选择。
    readerTable->setRowCount(0);
    for (const auto &reader : readers) {
        const int row = readerTable->rowCount();
        readerTable->insertRow(row);
        setRow(readerTable, row, {
            QString::number(reader.id), reader.name, reader.type, reader.phone
        });
    }
}

void MainWindow::refreshLoans()
{
    // 表格中的 UserRole 保存真实 ID，避免从“编号 - 名称”显示文本反向解析。
    loanTable->setRowCount(0);
    for (const auto &loan : loans) {
        const int row = loanTable->rowCount();
        loanTable->insertRow(row);

        auto *readerItem = new QTableWidgetItem(readerName(loan.readerId));
        readerItem->setData(Qt::UserRole, loan.readerId);
        auto *bookItem = new QTableWidgetItem(bookTitle(loan.bookId));
        bookItem->setData(Qt::UserRole, loan.bookId);

        loanTable->setItem(row, 0, readerItem);
        loanTable->setItem(row, 1, bookItem);
        loanTable->setItem(row, 2, new QTableWidgetItem(
            loan.borrowDate.toString("yyyy-MM-dd")));
        loanTable->setItem(row, 3, new QTableWidgetItem(
            loan.dueDate.toString("yyyy-MM-dd")));

        // 逾期状态由应还日期实时计算，不额外存储，避免状态过期。
        const auto status = loan.returned
                                ? "已归还"
                                : (loan.dueDate < QDate::currentDate()
                                       ? "已超期"
                                       : "借阅中");
        loanTable->setItem(row, 4, new QTableWidgetItem(status));
    }
}

void MainWindow::addBook()
{
    bool accepted = false;
    const auto title = QInputDialog::getText(
        this, "新增图书", "书名：", QLineEdit::Normal, {}, &accepted);
    if (!accepted || title.trimmed().isEmpty()) return;

    const auto author = QInputDialog::getText(
        this, "新增图书", "作者：", QLineEdit::Normal, {}, &accepted);
    if (!accepted) return;

    const auto category = QInputDialog::getText(
        this, "新增图书", "分类：", QLineEdit::Normal, "其他", &accepted);
    if (!accepted) return;

    const int total = QInputDialog::getInt(
        this, "新增图书", "数量：", 1, 1, 9999, 1, &accepted);
    if (!accepted) return;

    books.append({nextBookId++, title.trimmed(), author.trimmed(),
                  category.trimmed(), total, total});
    saveData();
    refreshBooks();
}

void MainWindow::editBook()
{
    auto *book = selectedBook();
    if (!book) {
        error("请先选择图书。");
        return;
    }

    bool accepted = false;
    const auto title = QInputDialog::getText(
        this, "编辑图书", "书名：", QLineEdit::Normal, book->title, &accepted);
    if (!accepted || title.trimmed().isEmpty()) return;

    const int borrowed = book->total - book->available;
    const int total = QInputDialog::getInt(
        this, "编辑图书", "总数量：", book->total, borrowed, 9999, 1, &accepted);
    if (!accepted) return;

    book->title = title.trimmed();
    book->total = total;
    book->available = total - borrowed;
    saveData();
    refreshBooks();
}

void MainWindow::deleteBook()
{
    auto *book = selectedBook();
    if (!book) {
        error("请先选择图书。");
        return;
    }

    for (const auto &loan : loans) {
        if (loan.bookId == book->id && !loan.returned) {
            error("该图书仍有未归还借阅，不能删除。");
            return;
        }
    }

    // 当前数据库层不做物理删除，防止破坏已有借阅历史。
    error("MVP 为保护历史借阅数据暂不支持物理删除图书。");
}

void MainWindow::addReader()
{
    const auto name = readerNameEdit->text().trimmed();
    if (name.isEmpty()) {
        error("姓名不能为空。");
        return;
    }

    const auto inputType = readerTypeEdit->text().trimmed();
    const auto type = inputType.isEmpty() ? "本科生" : inputType;
    readers.append({nextReaderId++, name, type, readerPhoneEdit->text().trimmed()});
    saveData();

    readerNameEdit->clear();
    readerTypeEdit->clear();
    readerPhoneEdit->clear();
    refreshReaders();
}

void MainWindow::editReader()
{
    auto *reader = selectedReader();
    if (!reader) {
        error("请先选择读者。");
        return;
    }

    bool accepted = false;
    const auto name = QInputDialog::getText(
        this, "编辑读者", "姓名：", QLineEdit::Normal, reader->name, &accepted);
    if (!accepted || name.trimmed().isEmpty()) return;

    reader->name = name.trimmed();
    saveData();
    refreshReaders();
    refreshLoans();
}

void MainWindow::deleteReader()
{
    auto *reader = selectedReader();
    if (!reader) {
        error("请先选择读者。");
        return;
    }

    for (const auto &loan : loans) {
        if (loan.readerId == reader->id && !loan.returned) {
            error("该读者仍有未归还图书，不能删除。");
            return;
        }
    }

    error("MVP 为保护历史借阅数据暂不支持物理删除读者。");
}

void MainWindow::borrowBook()
{
    // 借书前依次校验读者、图书、库存和重复借阅，确保库存不会出现负数。
    const int readerId = borrowReader->text().toInt();
    const int bookId = borrowBookEdit->text().toInt();
    Reader *reader = nullptr;
    Book *book = nullptr;

    for (auto &item : readers) {
        if (item.id == readerId) {
            reader = &item;
            break;
        }
    }
    for (auto &item : books) {
        if (item.id == bookId) {
            book = &item;
            break;
        }
    }

    if (!reader) { error("读者不存在。"); return; }
    if (!book) { error("图书不存在。"); return; }
    if (book->available <= 0) { error("图书暂无库存。"); return; }

    for (const auto &loan : loans) {
        if (loan.readerId == readerId && loan.bookId == bookId && !loan.returned) {
            error("该读者已借阅此书。");
            return;
        }
    }

    loans.append({readerId, bookId, QDate::currentDate(),
                  QDate::currentDate().addDays(30), {}, false});
    --book->available;
    saveData();
    refreshBooks();
    refreshLoans();
}

void MainWindow::returnBook()
{
    // 归还通过标记记录完成，同时恢复对应图书的可借数量。
    auto *loan = selectedLoan();
    if (!loan) {
        error("请先选择借阅中的记录。");
        return;
    }

    loan->returned = true;
    loan->returnDate = QDate::currentDate();
    for (auto &book : books) {
        if (book.id == loan->bookId) {
            ++book.available;
            break;
        }
    }

    saveData();
    refreshBooks();
    refreshLoans();
}

QString MainWindow::bookTitle(int id) const
{
    for (const auto &book : books) {
        if (book.id == id) {
            return QString::number(id) + " - " + book.title;
        }
    }
    return "未知图书";
}

QString MainWindow::readerName(int id) const
{
    for (const auto &reader : readers) {
        if (reader.id == id) {
            return QString::number(id) + " - " + reader.name;
        }
    }
    return "未知读者";
}

void MainWindow::error(const QString &message)
{
    QMessageBox::warning(this, "操作提示", message);
}
