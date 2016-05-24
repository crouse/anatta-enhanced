#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDesktopServices>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QHostAddress>
#include <QNetworkInterface>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serverIp = "127.0.0.1";
    fee = ui->lineEditFee->text().trimmed().toInt();

    {
        lineEditSearch = new QLineEdit;
        lineEditSearch->setFixedSize(350, 20);
        lineEditSearch->setStyleSheet("border-radius: 5px;");
        lineEditSearch->setPlaceholderText(" 查询<ENTER>");
        ui->mainToolBar->addWidget(lineEditSearch);
        ui->mainToolBar->addSeparator();
    }


    /* config lineEdit */
    {
        lineEditConfig = new QLineEdit;
        lineEditConfig->setFixedSize(100, 20);
        lineEditConfig->setStyleSheet("border-radius: 5px;");
        lineEditConfig->setPlaceholderText("服务器地址");
        lineEditConfig->setText(serverIp);
        ui->mainToolBar->addWidget(lineEditConfig);
        ui->mainToolBar->addSeparator();
    }


    /* editor lineEdit */
    {
        lineEditEditor = new QLineEdit;
        lineEditEditor->setFixedSize(100, 20);
        lineEditEditor->setStyleSheet("border-radius: 5px; background: yellow");
        lineEditEditor->setPlaceholderText(" 管理员必填");
        ui->mainToolBar->addWidget(lineEditEditor);
        connect(lineEditEditor, SIGNAL(returnPressed()), this, SLOT(afterQueryPresshed()));
    }

    currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    setPaths();
    ui->spinBoxFrom->setRange(0, 2000);
    ui->spinBoxTo->setRange(1, 2000);

    maleStart = 0;
    femaleStart = 0;

    showHelp();

    modelAdmin = new QSqlQueryModel;
    modelCitta = new QSqlQueryModel;
    modelBrowser = new QSqlQueryModel;
    modelInfo = new QSqlQueryModel;
    modelImages = new QSqlQueryModel;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initAdminPage()
{
    modelAdmin->setQuery(QString("select name, ipaddr, signtime from zen_admin where signtime > '%1' ").arg(currentDate));
    modelAdmin->setHeaderData(0, Qt::Horizontal, tr("管理员"));
    modelAdmin->setHeaderData(1, Qt::Horizontal, tr("IP"));
    modelAdmin->setHeaderData(2, Qt::Horizontal, tr("登陆时间"));
    ui->tableViewAdmin->setModel(modelAdmin);
    ui->tableViewAdmin->show();

    modelCitta->setQuery(
                QString(
                    " SELECT  editor, ipaddress, COUNT(id) AS cnt, COUNT(id) * %1 AS money FROM "
                    " ((SELECT  id, editor, ipaddress FROM zen_male) UNION ALL (SELECT  id, editor, ipaddress FROM zen_female)) a "
                    " GROUP BY editor , ipaddress ORDER BY editor "
                    ).arg(fee)
                    );
    modelCitta->setHeaderData(0, Qt::Horizontal, tr("收费义工"));
    modelCitta->setHeaderData(1, Qt::Horizontal, tr("IP"));
    modelCitta->setHeaderData(2, Qt::Horizontal, tr("收费个数"));
    modelCitta->setHeaderData(3, Qt::Horizontal, tr("收费元"));
    ui->tableViewCitta->setModel(modelCitta);
    ui->tableViewCitta->show();

    modelBrowser->setQuery(
                QString(" SELECT  others, notes as ip, COUNT(id) AS cnt FROM "
                        " ((SELECT  id, others, notes FROM zen_male) UNION ALL ( SELECT  id, others, notes FROM zen_female)) a "
                        " GROUP BY others , notes  ORDER BY others "));
    modelBrowser->setHeaderData(0, Qt::Horizontal, tr("录入义工"));
    modelBrowser->setHeaderData(1, Qt::Horizontal, tr("IP"));
    modelBrowser->setHeaderData(2, Qt::Horizontal, tr("录入个数"));
    ui->tableViewBrowser->setModel(modelBrowser);
    ui->tableViewBrowser->show();
}

/*
  CREATE TABLE IF NOT EXISTS `zen_print_images` (
  `name` varchar(45) NOT NULL COMMENT '管理员姓名',
  `array` text NOT NULL COMMENT '打印的内容',
  `signtime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '记录写入时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
*/


void MainWindow::initPrintPage()
{
    modelInfo->setQuery("select name, gender, start, end, signtime from zen_print_info order by signtime");
    modelInfo->setHeaderData(0, Qt::Horizontal, tr("管理员"));
    modelInfo->setHeaderData(1, Qt::Horizontal, tr("导出信息的性别"));
    modelInfo->setHeaderData(2, Qt::Horizontal, tr("打印起始"));
    modelInfo->setHeaderData(3, Qt::Horizontal, tr("打印结束"));
    modelInfo->setHeaderData(4, Qt::Horizontal, tr("打印时间"));
    ui->tableViewPrintInfo->setModel(modelInfo);
    ui->tableViewPrintInfo->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewPrintInfo->show();

    modelImages->setQuery("select name, signtime, array from zen_print_images order by signtime");
    modelImages->setHeaderData(0, Qt::Horizontal, tr("管理员"));
    modelImages->setHeaderData(1, Qt::Horizontal, tr("打印时间"));
    modelImages->setHeaderData(2, Qt::Horizontal, tr("已打印照片的收据号序列"));
    ui->tableViewPrintImages->setModel(modelImages);
    ui->tableViewPrintImages->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewPrintImages->show();
}

void MainWindow::on_actionSetting_triggered()
{
    ;
}

bool MainWindow::databaseTest()
{
    serverIp = lineEditConfig->text().trimmed();
    qDebug() << "databaseTest" << serverIp;
    bool ret;
    QTcpSocket tsock;
    tsock.connectToHost(serverIp, 3306);
    ret = tsock.waitForConnected(1000);
    if (ret) tsock.close();
    getLocalAddr();
    return ret;
}

bool MainWindow::connectDatabase()
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(serverIp);
    db.setDatabaseName(DB_NAME);
    db.setUserName(DB_USER);
    db.setPassword(DB_PASS);

    if(!db.open()) {
        QMessageBox::critical(this, "数据库错误", db.lastError().text());
        return false;
    }

    setFahuiInfo();

    fee = ui->lineEditFee->text().trimmed().toInt();
    initAdminPage();

    return true;
}

void MainWindow::setModel(QSqlTableModel *mod, QString tableName, QTableView *view)
{
    mod = new QSqlTableModel(this);

    if (tableName == "zen_male") {
        model = mod;
    } else {
        modelFemale = mod;
    }

    mod->setTable(tableName);
    mod->setEditStrategy(QSqlTableModel::OnFieldChange);
    mod->setSort(0, Qt::AscendingOrder);
    mod->setHeaderData(0, Qt::Horizontal, "序号");
    mod->setHeaderData(1, Qt::Horizontal, "收据号");
    mod->setHeaderData(2, Qt::Horizontal, "姓名");
    mod->setHeaderData(3, Qt::Horizontal, "手机号");
    mod->setHeaderData(4, Qt::Horizontal, "性别");
    mod->setHeaderData(5, Qt::Horizontal, "法名");
    mod->setHeaderData(6, Qt::Horizontal, "民族");
    mod->setHeaderData(7, Qt::Horizontal, "生日");
    mod->setHeaderData(8, Qt::Horizontal, "学历");
    mod->setHeaderData(9, Qt::Horizontal, "现居省");
    mod->setHeaderData(10, Qt::Horizontal, "现居市");
    mod->setHeaderData(11, Qt::Horizontal, "现居区县");
    mod->setHeaderData(12, Qt::Horizontal, "详细街道地址");
    mod->setHeaderData(13, Qt::Horizontal, "皈依证号");
    mod->setHeaderData(14, Qt::Horizontal, "工作");
    mod->setHeaderData(15, Qt::Horizontal, "爱好");
    mod->setHeaderData(16, Qt::Horizontal, "身份证号码");
    mod->setHeaderData(17, Qt::Horizontal, "健康状况");
    mod->setHeaderData(18, Qt::Horizontal, "电话号码");
    mod->setHeaderData(19, Qt::Horizontal, "编辑时间");
    mod->setHeaderData(20, Qt::Horizontal, "工作单位");
    mod->setHeaderData(21, Qt::Horizontal, "邮编号码");
    mod->setHeaderData(22, Qt::Horizontal, "毕业时间");
    mod->setHeaderData(23, Qt::Horizontal, "毕业学校");
    mod->setHeaderData(24, Qt::Horizontal, "第一份工作入职时间");
    mod->setHeaderData(25, Qt::Horizontal, "第一份工作入职单位");
    mod->setHeaderData(26, Qt::Horizontal, "第二份工作入职时间");
    mod->setHeaderData(27, Qt::Horizontal, "第二份工作入职单位");
    mod->setHeaderData(28, Qt::Horizontal, "退休时间");
    mod->setHeaderData(29, Qt::Horizontal, "退休工作单位");
    mod->setHeaderData(30, Qt::Horizontal, "学佛起始时间");
    mod->setHeaderData(31, Qt::Horizontal, "学佛时间（年）");
    mod->setHeaderData(32, Qt::Horizontal, "是否深刻理解佛法层次");
    mod->setHeaderData(33, Qt::Horizontal, "学佛因缘");
    mod->setHeaderData(34, Qt::Horizontal, "读经数量");
    mod->setHeaderData(35, Qt::Horizontal, "认为易学的经典");
    mod->setHeaderData(36, Qt::Horizontal, "认为难读懂的经典");
    mod->setHeaderData(37, Qt::Horizontal, "格言");
    mod->setHeaderData(38, Qt::Horizontal, "家庭三宝弟子");
    mod->setHeaderData(39, Qt::Horizontal, "编辑人姓名");
    mod->setHeaderData(40, Qt::Horizontal, "修改人姓名");
    mod->setHeaderData(41, Qt::Horizontal, "学佛小组种类");
    mod->setHeaderData(42, Qt::Horizontal, "学佛小组地址");
    mod->setHeaderData(43, Qt::Horizontal, "是否提供学佛场地");
    mod->setHeaderData(44, Qt::Horizontal, "其他信息");
    mod->setHeaderData(45, Qt::Horizontal, "照片标记");
    mod->setHeaderData(46, Qt::Horizontal, "IP地址");
    mod->setHeaderData(47, Qt::Horizontal, "数据来源");
    mod->setHeaderData(48, Qt::Horizontal, "记录写入时间");
    mod->setHeaderData(49, Qt::Horizontal, "打印状态"); // default 0, success 1
    mod->setHeaderData(50, Qt::Horizontal, "数据准备情况");
    mod->setHeaderData(51, Qt::Horizontal, "法会名称");
    mod->select();

    //ui->tableView->setModel(model);
    view->setModel(mod);
}

void MainWindow::on_actionDb_triggered()
{
    if(!testIfAdmin()) {
        qDebug() << "testIfAdmin test not pass, maybe not input admin name";
        return;
    }

    bool conStatus = databaseTest();
    if (conStatus == false) {
        QMessageBox::critical(this, "", "请设置正确的数据库地址以及端口.");
        return;
    }
    bool stat = connectDatabase();
    if (!stat) {
        qDebug() << "Not connected.";
        return;
    }
    qDebug() << "set Model";
    ui->actionDb->setDisabled(true);
    setModel(model, "zen_male", ui->tableView);
    setModel(modelFemale, "zen_female", ui->tableViewFemale);
    insertAdminInfo();
    lineEditEditor->setDisabled(true);
    lineEditConfig->setDisabled(true);
}

bool MainWindow::searchInfo(QString search)
{
    // 支持手环编号、手机号以及姓名的查询
    if (search.startsWith("A")) {
        model->setFilter(QString(" receipt = '%1'").arg(search));
        model->select();
        ui->tableView->reset();
    } else if (search.startsWith("B")) {
        modelFemale->setFilter(QString(" receipt = '%1'").arg(search));
        modelFemale->select();
        ui->tableViewFemale->reset();
    } else if (search.startsWith("1")) {
        QString sql = QString(" phone_num = '%1'").arg(search);
        model->setFilter(sql);
        model->select();
        ui->tableView->reset();

        modelFemale->setFilter(sql);
        modelFemale->select();
        ui->tableViewFemale->reset();
    } else if (search.startsWith("sql:")) {
        QString sql = search.section(":", 1);
        qDebug() << "searchInfo:" << sql;
        model->setFilter(sql);
        model->select();
        ui->tableView->reset();

        modelFemale->setFilter(sql);
        modelFemale->select();
        ui->tableViewFemale->reset();

    } else {
        QString sql = QString(" name = '%1'").arg(search);
        model->setFilter(sql);
        model->select();
        ui->tableView->reset();

        modelFemale->setFilter(sql);
        modelFemale->select();
        ui->tableViewFemale->reset();
    }

    return true;
}

void MainWindow::on_actionRefresh_triggered()
{
    model->setFilter("");
    modelFemale->setFilter("");

    QString search = lineEditSearch->text().trimmed();
    qDebug() << "search [" << search << "]";
    if (!search.isEmpty()) {
        qInfo() << "searching for: " << search;
        searchInfo(search);
        return;
    }

    model->select();
    ui->tableView->reset();

    modelFemale->select();
    ui->tableViewFemale->reset();
}

void MainWindow::on_tableView_customContextMenuRequested(const QPoint &pos) // male
{
    int rowNum = ui->tableView->verticalHeader()->logicalIndexAt(pos);
    int colNum = ui->tableView->horizontalHeader()->logicalIndexAt(pos);

    int db_id = model->index(rowNum, 0).data().toInt();
    QString receipt = model->index(rowNum, 1).data().toString();
    QString sth = model->index(rowNum, colNum).data().toString();
    qDebug() << db_id << receipt;
    qDebug() << sth;

    pReceipt = receipt;
    QMenu *popMenu = new QMenu(this);
    popMenu->addAction(ui->actionExportPersonalInfo);
    popMenu->exec(QCursor::pos());

    pReceipt = "";
    delete popMenu;
}

void MainWindow::on_tableViewFemale_customContextMenuRequested(const QPoint &pos) // female
{
    int rowNum = ui->tableViewFemale->verticalHeader()->logicalIndexAt(pos);
    int colNum = ui->tableViewFemale->horizontalHeader()->logicalIndexAt(pos);

    int db_id = modelFemale->index(rowNum, 0).data().toInt();
    QString receipt = modelFemale->index(rowNum, 1).data().toString();
    QString sth = modelFemale->index(rowNum, colNum).data().toString();
    qDebug() << "customContexMenuRequest" << db_id << receipt;
    qDebug() << sth;

    pReceipt = receipt;
    QMenu *popMenu = new QMenu(this);
    popMenu->addAction(ui->actionExportPersonalInfo);
    popMenu->exec(QCursor::pos());

    pReceipt = "";
    delete popMenu;
}

void MainWindow::on_actionExportPersonalInfo_triggered()
{
    // export excel format personnel info
    QString tableName;
    QChar gender = pReceipt[0];
    if (gender == QChar('A'))
        tableName = "zen_male";
    else
        tableName = "zen_female";
    qDebug() << tableName;
}

void MainWindow::on_actionPrintPersonnelCredentials_triggered()
{
    // export pdf format personnel info
}

void MainWindow::createCard(QString fileName, QSqlTableModel *mod, QString filter, QString pixmapPath)
{
    qDebug() << pixmapPath << filter;
    if (!mod) return;
    QFile pdfFile(fileName);
    pdfFile.open(QIODevice::WriteOnly);
    QPdfWriter *pdfWriter = new QPdfWriter(&pdfFile);
    pdfWriter->setCreator("quqinglei");
    pdfWriter->setPageMargins(QMarginsF(0, 0, 0, 0));
    pdfWriter->setPageSize(QPagedPaintDevice::A4);
    QPainter *pdfPainter = new QPainter(pdfWriter);

    QFont font;
    font.setFamily("华文楷体");
    font.setPointSize(10);
    pdfPainter->setFont(font);
    pdfPainter->setPen(Qt::red);
    pdfPainter->drawRect(0, 0, 9600, 13700); // 大矩形
    pdfPainter->drawLine(4191, 0, 4191, 13600);
    pdfPainter->drawLine(4800, 0, 4800, 13600);
    pdfPainter->drawLine(8991, 0, 8991, 13600);
    pdfPainter->drawLine(0, 5858, 9600, 5858);
    pdfPainter->drawLine(0, 6363, 9600, 6363);
    pdfPainter->drawLine(0, 12221, 9600, 12221);
    // 内框
    pdfPainter->setPen(Qt::black);
    pdfPainter->drawLine(QPointF(0, 252.5), QPointF(9600, 252.5));
    pdfPainter->drawLine(QPointF(252.5, 0), QPointF(252.5, 13600));
    pdfPainter->drawLine(QPointF(8738.5, 0), QPointF(8738.5, 13600));
    pdfPainter->drawLine(QPointF(0, 11968.5), QPointF(9600, 11968.5));
    pdfPainter->drawLine(QPointF(0, 5605.5), QPointF(9600, 5605.5));
    pdfPainter->drawLine(QPointF(3938, 0), QPointF(3938, 13600));
    pdfPainter->drawLine(QPointF(5052.5, 0), QPointF(5052.5, 13600));
    pdfPainter->drawLine(QPointF(0, 6615.5), QPointF(9600, 6615.5));
    pdfPainter->drawLine(QPointF(252.5, 1262.5), QPointF(8738.5, 1262.5));
    pdfPainter->drawLine(QPointF(252.5, 7625.5), QPointF(8738.5, 7625.5));
    pdfPainter->drawLine(QPointF(252.5, 1696.8), QPointF(8738.5, 1696.5));
    pdfPainter->drawLine(QPointF(252.5, 8059.8), QPointF(8738.5, 8059.8));
    pdfPainter->drawLine(QPointF(2697.2, 1696.5), QPointF(2697.2, 3433.7));
    pdfPainter->drawLine(QPointF(252.5, 3433.7), QPointF(3938, 3433.7));
    pdfPainter->drawLine(QPointF(252.5, 2130.8), QPointF(2697.2, 2130.8));
    pdfPainter->drawLine(QPointF(252.5, 2565.1), QPointF(2697.2, 2565.1));
    pdfPainter->drawLine(QPointF(252.5, 2999.4), QPointF(2697.2, 2999.4));
    pdfPainter->drawLine(QPointF(252.5, 3433.7), QPointF(2697.2, 3433.7));
    pdfPainter->drawLine(QPointF(252.5, 3868), QPointF(8738.5, 3868));
    pdfPainter->drawLine(QPointF(252.5, 4302.3), QPointF(8738.5, 4302.3));
    pdfPainter->drawLine(QPointF(252.5, 4736.6), QPointF(8738.5, 4736.6));
    pdfPainter->drawLine(QPointF(252.5, 5170.9), QPointF(8738.5, 5170.9));
    pdfPainter->drawLine(QPointF(252.5, 3868), QPointF(8738.5, 3868));
    pdfPainter->drawLine(QPointF(1252.5, 1696.5), QPointF(1252.5, 5605.5));

    // mark
    /*
    {
        pdfPainter->setPen(Qt::blue);
        pdfPainter->drawText(QPointF(252.5, 252.5), "(252.5, 252.5)");
        pdfPainter->drawText(QPointF(252.5, 6615.5), "(252.5, 6615.5)");
        pdfPainter->drawText(QPointF(3938, 6615.5), "(3938, 6615.5)");
        pdfPainter->drawText(QPointF(5052.5, 6615.5), "(5052.5, 6615.5)");
        pdfPainter->drawText(QPointF(8738.5, 6615.5), "(8738.5, 6615.5)");
        pdfPainter->drawText(QPointF(3938, 252.5), "(3938, 252.5)");
        pdfPainter->drawText(QPointF(5052.5, 252.5), "(5052.5, 252.5)");
        pdfPainter->drawText(QPointF(252.5, 5605.5), "(252.5, 5605.5)");
        pdfPainter->drawText(QPointF(3938, 5605), "(3938, 5605)");
        pdfPainter->drawText(QPointF(5052.5, 5605), "(5052.5, 5605)");
        pdfPainter->drawText(QPointF(8738.5, 5605), "(8738.5, 5605)");
        pdfPainter->drawText(QPointF(8738.5, 252.5), "(8738.5, 252.5)");
        pdfPainter->drawText(QPointF(252.5, 11968.5), "(252.5, 11968.5)");
        pdfPainter->drawText(QPointF(3938, 11968.5), "(3938, 11968.5)");
        pdfPainter->drawText(QPointF(5052.5, 11968.5), "(5052.5, 11968.5)");
        pdfPainter->drawText(QPointF(8738.5, 11968.5), "(8738.5, 11968.5)");
        pdfPainter->drawText(QPointF(252.5, 1262.5), "(252.5, 1262.5)");
        pdfPainter->drawText(QPointF(3938, 1262.5), "(3938, 1262.5)");
        pdfPainter->drawText(QPointF(5052.5, 1262.5), "(5052.5, 1262.5)");
        pdfPainter->drawText(QPointF(8738.5, 1262.5), "(8738.5, 1262.5)");
        pdfPainter->drawText(QPointF(252.5, 7625.5), "(252.5, 7625.5)");
        pdfPainter->drawText(QPointF(3938, 7625.5), "(3938, 7625.5)");
        pdfPainter->drawText(QPointF(8738.5, 7625.5), "(8738.5, 7625.5)");
        pdfPainter->drawText(QPointF(252.5, 1696.5), "(252.5, 1696.5)");
        pdfPainter->drawText(QPointF(3938, 1696.5), "(3938, 1696.5)");
        pdfPainter->drawText(QPointF(5052.5, 1696.5), "(5052.5, 1696.5)");
        pdfPainter->drawText(QPointF(8738.5, 1696.5), "(8738.5, 1696.5)");
        pdfPainter->drawText(QPointF(252.5, 8059.5), "(252.5, 8059.5)");
        pdfPainter->drawText(QPointF(3938, 8059.5), "(3938, 8059.5)");
        pdfPainter->drawText(QPointF(5052.5, 7625.5), "(5052.5, 7625.5)");
        pdfPainter->drawText(QPointF(5052.5, 8059.5), "(5052.5, 8059.5)");
        pdfPainter->drawText(QPointF(8738.5, 8059.5), "(8738.5, 8059.5)");
        pdfPainter->drawText(QPointF(2697.2, 3433.7), "(2697.2, 3433.7)");
        pdfPainter->drawText(QPointF(2697.2, 1696.5), "(2697.2, 1696.5)");
        pdfPainter->drawText(QPointF(1252.5, 5605.5), "(1252.5, 5605.5)");
    }
    */


    // 默认文字
    // 皈依师签字
    QString guiyi_shifu = QString("皈依师（签字）");
    pdfPainter->setPen(Qt::black);
    pdfPainter->drawText(QPointF(505, 1009.5), guiyi_shifu);
    pdfPainter->drawText(QPointF(5305, 1009.5), guiyi_shifu);
    pdfPainter->drawText(QPointF(505, 7373), guiyi_shifu);
    pdfPainter->drawText(QPointF(5305, 7373), guiyi_shifu);

    // 皈依寺院
    QString siyuan = QString("皈依寺院：北京龙泉寺");
    pdfPainter->drawText(QPointF(505, 1596.5), siyuan);

    // 照片
    QPixmap pixmap("/Users/quqinglei//myself.jpg"); // [tbd just test]
    //pdfPainter->drawPixmap(2697.2, 1696.5, 1245.8, 1737.2, pixmap);
    pdfPainter->drawPixmap(2697, 1696, 1246, 1737, pixmap);

    // 姓名
    pdfPainter->drawText(QPointF(505,2030), "姓名");
    pdfPainter->drawText(QPointF(505 + 1000, 2030), "屈庆磊");

    // 性别
    pdfPainter->drawText(QPointF(505, 2464.3), "性别");
    pdfPainter->drawText(QPointF(505 + 1000, 2464.3), "男");

    // 法名
    pdfPainter->drawText(QPointF(505, 2898.6), "法名");
    pdfPainter->drawText(QPointF(505 + 1000, 2898.6), "贤磊");

    // 民族
    pdfPainter->drawText(QPointF(505, 3332.9), "民族");
    pdfPainter->drawText(QPointF(505 + 1000, 3332.9), "汉族");

    // 出生年月
    pdfPainter->drawText(QPointF(505, 3767.2), "出生年月");
    pdfPainter->drawText(QPointF(505 + 1000, 3767.2), "1987-04-04");

    // 文化程度
    pdfPainter->drawText(QPointF(505, 4201.5), "文化程度");
    pdfPainter->drawText(QPointF(505 + 1000, 4201.5), "大学本科");

    // 现居地址
    pdfPainter->drawText(QPointF(505, 4635.8), "现居地址");
    pdfPainter->drawText(QPointF(505 + 1000, 4635.8), "北京 北京 昌平 霍营");

    // 皈依编号
    pdfPainter->drawText(QPointF(505, 5070.1), "皈依编号");
    pdfPainter->drawText(QPointF(505 + 1000, 5070.1), "A0000012933");

    // 发证时间
    pdfPainter->drawText(QPointF(505, 5504.4), "发证时间");
    pdfPainter->drawText(QPointF(505 + 1000, 5504.4), "2014-11-11");

    pdfPainter->end();
    pdfFile.close();

    delete pdfPainter;
    delete pdfWriter;
}

void MainWindow::savePdfs(QString fileName, QSqlTableModel *mod, QString filter)
{
#ifndef ONE_PAGE_NUM
#define ONE_PAGE_NUM 8
#endif
    if (!mod) {
        QMessageBox::information(this, "warn", "数据库未连接或者没有导出的数据");
        return;
    }

    int pageNum = 0;
    int leftMargin;
    int leftWordMargin;
    int pixMapMagin;
    int topMargin = 400;
    int blockHeight = 1600;

    int headerPos = 0;
    int footer = 0;

    QString dt = QDate::currentDate().toString(Qt::ISODate);

    QFile pdf_file(fileName);
    pdf_file.open(QIODevice::WriteOnly);
    QPdfWriter *pdf_writer = new QPdfWriter(&pdf_file);
    QPainter *pdf_painter = new QPainter(pdf_writer);
    QFont font;
    font.setFamily("微软雅黑");
    //font.setPixelSize(150);
    font.setPointSize(8);

    QFont mfont;
    //mfont.setPixelSize(120);
    mfont.setPointSize(6);
    mfont.setFamily("苹果方");
    QPen vpen;
    vpen.setStyle(Qt::DashLine);
    pdf_painter->setPen(vpen);
    pdf_painter->setFont(mfont);
    pdf_painter->setRenderHint(QPainter::NonCosmeticDefaultPen, true);
    pdf_writer->setPageSize(QPagedPaintDevice::A4);

    qDebug() << filter;
    mod->setFilter(filter);
    mod->select();

    for (int i = 0; i < mod->rowCount(); i += 8) {
        /* handle page margin */
            pixMapMagin = 800;
            leftMargin = 1800;
            leftWordMargin = leftMargin + 1100;
            footer = 8500;

            /*
            pixMapMagin = 400;
            leftMargin = 1400;
            leftWordMargin = leftMargin + 1100;
            footer = 1500;
            */

        headerPos = 7500;

        QSqlRecord record[ONE_PAGE_NUM];

        pdf_painter->drawText(QRect(footer, 13400, 600, 300), QString("%1").arg(pageNum + 1));
        pdf_painter->drawLine(QPoint(pixMapMagin, 300), QPoint(9000, 300));
        for(int m = 0; m < 8; ++m) {
            qDebug() << m;
            record[m] = mod->record(i + m);
            /* make one page pdf */
            /* set image */

            /* If receipt is empty which means there is no page any more, so end this func */
            QString receipt = record[m].value("receipt").toString();
            if (receipt.isEmpty()) {
                qDebug() << "Receipt is empty" << "Page Num is " << pageNum;
                pdf_painter->end();
                delete pdf_painter;
                delete pdf_writer;
                pdf_file.close();
                return;
            }

            QString pixmapPath, pixmapAbsPath;
            if (ui->lineEditImagePath->text().isEmpty()) {
                pixmapAbsPath = QString("%1/%2.png").arg(imageFilePath).arg(receipt);
            } else {
                pixmapAbsPath = QString("%1/%2.png").arg(imageFilePath).arg(receipt);
            }

            qDebug() << pixmapAbsPath;
            QPixmap pixmap(pixmapAbsPath);
            int height = m * blockHeight;

            pdf_painter->drawPixmap(pixMapMagin, topMargin + height, 885, 1239, pixmap);
            pdf_painter->setFont(font);
            QString name = QString("姓名：%1").arg(record[m].value("name").toString());
            QString gender = QString("性别：%1").arg(record[m].value("gender").toString());
            QString fname = QString("法名：%1").arg(record[m].value("fname").toString());
            QString race = QString("民族：%1").arg(record[m].value("race").toString());
            QString birthday = QString("生日：%1").arg(record[m].value("birthday").toString());
            QString degree = QString("学历：%1").arg(record[m].value("degree").toString());
            QString addr = QString("住址：%1 %2 %3")
                    .arg(record[m].value("province").toString())
                    .arg(record[m].value("city").toString())
                    .arg(record[m].value("district").toString());
            QString health = QString("健康状况：%1").arg(record[m].value("health").toString());
            QString job = QString("工作：%1").arg(record[m].value("job").toString());
            QString level_time = QString("学佛时间/程度：%1/%2")
                    .arg(record[m].value("year2start_learning_buddhism").toString())
                    .arg(record[m].value("deep_understanding_of_dharma").toString());

            QString code = QString("编号：%1").arg(record[m].value("code").toString());
            QString phone = QString("手机：%1").arg(record[m].value("phone_num").toString());
            QString pid = QString("身份证：%1").arg(record[m].value("personnel_id").toString());

            pdf_painter->drawText(QRect(leftMargin + 50, topMargin + height, 2000, 300), name);
            pdf_painter->drawText(QRect(leftMargin + 2100, topMargin + height, 2000, 300), fname);
            pdf_painter->drawText(QRect(leftMargin + 4100, topMargin + height, 2000, 300), gender);

            QRectF rect(leftMargin + 6100, topMargin + height, 1100, 1000);
            pdf_painter->drawRect(rect);
            pdf_painter->drawLine(QPoint(leftMargin + 6100, topMargin + height + 300), QPoint(leftMargin + 7200, topMargin + height + 300));
            pdf_painter->drawText(QRect(leftMargin + 6200, topMargin + height + 20, 2000, 300), "签字栏");

            pdf_painter->drawText(QRect(leftMargin + 50, topMargin + height + 300, 2000, 300), race);
            pdf_painter->drawText(QRect(leftMargin + 2100, topMargin + height + 300, 2000, 300), birthday);
            pdf_painter->drawText(QRect(leftMargin + 4100, topMargin + height + 300, 2000, 300), degree);

            pdf_painter->drawText(QRect(leftMargin + 50, topMargin + height + 600, 2000, 300), health);
            pdf_painter->drawText(QRect(leftMargin + 2100, topMargin + height + 600, 2000, 300), job);
            pdf_painter->drawText(QRect(leftMargin + 4100, topMargin + height + 600, 2000, 300), level_time);

            pdf_painter->drawText(QRect(leftMargin + 50, topMargin + height + 900, 2000, 300), QString("%1/%2").arg(code).arg(receipt));
            pdf_painter->drawText(QRect(leftMargin + 2100, topMargin + height + 900, 2000, 300), phone);
            pdf_painter->drawText(QRect(leftMargin + 4100, topMargin + height + 900, 2000, 300), pid);

            pdf_painter->drawText(QRect(leftMargin + 50, topMargin + height + 1200, 9000, 300), QString("%1").arg(addr));

            pdf_painter->drawLine(QPoint(pixMapMagin, 1900 + height), QPoint(9000, 1900 + height));


        }

        pdf_writer->newPage();
        ++pageNum;
    }

    pdf_painter->end();
    delete pdf_writer;
    delete pdf_painter;
    pdf_file.close();
}

void MainWindow::on_actionExportPdf_triggered()
{
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this, "打开保存文件路径", "", "pdf (*.pdf)");
    if (fileName.isNull()) return;

    QString maleFileName, femaleFileName;
    maleFileName = fileName.section('0', 0, 0) + ".male.pdf";
    femaleFileName = fileName.section('0', 0, 0) + ".female.pdf";
    qDebug() << maleFileName << femaleFileName;

    savePdfs(maleFileName, model, "");
    savePdfs(femaleFileName, modelFemale, "");
}

void MainWindow::on_toolButtonImagePath_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("打开路径"),
                                                    "~/Desktop/images",
                                                    QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks
                                                    );
    ui->lineEditImagePath->setText(dir);
    imageFilePath = dir;
}

void MainWindow::on_toolButtonBackPath_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("打开路径"),
                                                    "~/Desktop/backup",
                                                    QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks
                                                    );
    ui->lineEditBackPath->setText(dir);
    saveFilePath = dir;
}

void MainWindow::on_actionCard_triggered()
{
    createCard("/Users/quqinglei/Desktop/hello.pdf", model, "", "");
}

void MainWindow::exportExcel(QString fileName, QSqlTableModel *mod)
{
    QXlsx::Document xlsx;
    QString names[] = {
        "姓名",
        "性别",
        "职业",
        "特长",
        "法名",
        "皈依法会名称",
        "皈依日期",
        "皈依年度",
        "皈依证号",
        "出生年月",
        "身份证号",
        "电话",
        "民族",
        "文化程度",
        "健康状况",
        "固定电话",
        "填表时间",
        "收据编号",
        "工作单位",
        "省(直辖市/自治区)",
        "市",
        "区/县",
        "通讯地址",
        "邮编",
        "毕业时间",
        "毕业学校",
        "工作入职时间1",
        "工作单位1",
        "工作入职时间2",
        "工作入职单位2",
        "退休时间",
        "退休单位",
        "学佛时间",
        "学佛时长",
        "是否深刻理解佛法",
        "以何因缘接触佛法",
        "所读经典部数",
        "认为易学经典",
        "认为不易学经典",
        "感悟最深的一句话",
        "家庭成员三宝弟子"
    };

    for(int i = 0; i < 41; i++) {
        xlsx.write(1, i+1, names[i]);
    }

    mod->select();
    for(int i = 0; i < mod->rowCount(); i++) {
        int j = i + 2;
        xlsx.write(j, 1, mod->record(i).value("name").toString());
        xlsx.write(j, 2, mod->record(i).value("gender").toString());
        xlsx.write(j, 3, mod->record(i).value("job").toString());
        xlsx.write(j, 4, mod->record(i).value("hobby").toString());
        xlsx.write(j, 5, mod->record(i).value("fname").toString());
        xlsx.write(j, 6, mod->record(i).value("fahui_name").toString());
        xlsx.write(j, 7, mod->record(i).value("mod_time").toString().left(10));
        xlsx.write(j, 8, mod->record(i).value("mod_time").toString().left(4));
        xlsx.write(j, 9, mod->record(i).value("code").toString());
        xlsx.write(j, 10, mod->record(i).value("birthday").toString());
        xlsx.write(j, 11, mod->record(i).value("personnel_id").toString());
        xlsx.write(j, 12, mod->record(i).value("phone_num").toString());
        xlsx.write(j, 13, mod->record(i).value("race").toString());
        xlsx.write(j, 14, mod->record(i).value("degree").toString());
        xlsx.write(j, 15, mod->record(i).value("health").toString());
        xlsx.write(j, 16, mod->record(i).value("telephone_num").toString());
        xlsx.write(j, 17, mod->record(i).value("mod_time").toString().left(10));
        xlsx.write(j, 18, mod->record(i).value("receipt").toString());
        xlsx.write(j, 19, mod->record(i).value("workplace").toString());
        xlsx.write(j, 20, mod->record(i).value("province").toString());
        xlsx.write(j, 21, mod->record(i).value("city").toString());
        xlsx.write(j, 22, mod->record(i).value("district").toString());
        xlsx.write(j, 23, mod->record(i).value("address").toString());
        xlsx.write(j, 24, mod->record(i).value("postcode").toString());
        xlsx.write(j, 25, mod->record(i).value("graduate_time").toString());
        xlsx.write(j, 26, mod->record(i).value("graduate_school").toString());
        xlsx.write(j, 27, mod->record(i).value("first_job_entry_time").toString());
        xlsx.write(j, 28, mod->record(i).value("first_job_workplace").toString());
        xlsx.write(j, 29, mod->record(i).value("second_job_entry_time").toString());
        xlsx.write(j, 30, mod->record(i).value("second_job_workplace").toString());
        xlsx.write(j, 31, mod->record(i).value("retirement_date").toString());
        xlsx.write(j, 32, mod->record(i).value("retirement_workplace").toString());
        xlsx.write(j, 33, mod->record(i).value("year2start_learning_buddhism").toString());
        xlsx.write(j, 34, mod->record(i).value("years_of_learning_buddhism").toString());
        xlsx.write(j, 35, mod->record(i).value("deep_understanding_of_dharma").toString());
        xlsx.write(j, 36, mod->record(i).value("reason2learning_dharma").toString());
        xlsx.write(j, 37, mod->record(i).value("nums_of_buddhism_book").toString());
        xlsx.write(j, 38, mod->record(i).value("easy2learn_buddhism_book").toString());
        xlsx.write(j, 39, mod->record(i).value("hard2read").toString());
        xlsx.write(j, 40, mod->record(i).value("maxim").toString());
        xlsx.write(j, 41, mod->record(i).value("buddhist_disciples_of_family").toString());
    }
    xlsx.saveAs(fileName);
}

void MainWindow::exportAllExcels()
{
    QDateTime time = QDateTime::currentDateTime();
    QString currentDate = time.toString("yyyy-MM-dd");
    QString savePath;
    if (ui->lineEditBackPath->text().trimmed().isEmpty()) {
        savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    } else {
        savePath = ui->lineEditBackPath->text();
    }

    exportExcel(QString("%1/男众-%2.xlsx").arg(savePath).arg(currentDate), model);
    exportExcel(QString("%1/女众-%2.xlsx").arg(savePath).arg(currentDate), modelFemale);
}

void MainWindow::exportAllPdfs()
{
    QDateTime time = QDateTime::currentDateTime();
    QString currentDate = time.toString("yyyy-MM-dd");
    QString savePath;
    if (ui->lineEditBackPath->text().trimmed().isEmpty()) {
        savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    } else {
        savePath = ui->lineEditBackPath->text();
    }

    savePdfs(QString("%1/男众-%2.pdf").arg(saveFilePath).arg(currentDate), model, "");
    savePdfs(QString("%1/女众-%2.pdf").arg(saveFilePath).arg(currentDate), modelFemale, "");
}

void MainWindow::on_actionExcel_triggered()
{
    exportAllExcels();
}

void MainWindow::on_actionPdf_triggered()
{
    exportAllPdfs();
}

void MainWindow::on_toolButton_clicked()
{
    QSqlQuery query;
    QString fahui_name, last_male_code, last_female_code, sql;
    fahui_name = ui->lineEditFahui_name->text();
    last_male_code = ui->lineEditLastMaleCode->text();
    last_female_code = ui->lineEditLastFemaleCode->text();

    maleStart = last_male_code.toInt();
    femaleStart = last_female_code.toInt();

    QString currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");

    sql = QString("replace into zen_config (`fahui_name`, `last_male_code`, `last_female_code`, `date`) values ('%1', '%2', '%3', '%4')")
            .arg(fahui_name, last_male_code, last_female_code, currentDate);
    query.exec(sql);
    query.clear();
}

void MainWindow::setFahuiInfo()
{ // read database and set info
    qDebug() << "set Fahui info";
    QString fahui_name, last_male_code, last_female_code, sql;
    QString currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");

    sql = QString("select `fahui_name`, `last_male_code`, `last_female_code` from zen_config where `date` = '%1'").arg(currentDate);
    qDebug() << sql;
    QSqlQuery q(sql);
    QSqlRecord rec = q.record();
    int fahui_name_col = rec.indexOf("fahui_name");
    int last_male_code_col = rec.indexOf("last_male_code");
    int last_female_code_col = rec.indexOf("last_female_code");

    qDebug() << "Numbers of columns: " << rec.count();

    while (q.next()) {
        qDebug() << "here";
        fahui_name = q.value(fahui_name_col).toString();
        last_male_code = q.value(last_male_code_col).toString();
        last_female_code = q.value(last_female_code_col).toString();
    }

    ui->lineEditFahui_name->setText(fahui_name);
    ui->lineEditLastMaleCode->setText(last_male_code);
    ui->lineEditLastFemaleCode->setText(last_female_code);

    maleStart = last_male_code.toInt();
    femaleStart = last_female_code.toInt();
}

void MainWindow::setPaths()
{
    desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    saveFilePath = QString("%1/Faihui/%3").arg(desktopPath).arg(currentDate);
    imageFilePath = QString("%1/RsyncShare/").arg(homePath);
    QDir *tmp = new QDir;
    if (!tmp->exists(saveFilePath)) {
        tmp->mkpath(saveFilePath);
    }
    ui->lineEditBackPath->setText(saveFilePath);
    ui->lineEditImagePath->setText(imageFilePath);
}

bool MainWindow::savePrintPdfs(int gender, int from, int to) // gender 0 male, 1 female
{
    QString table;
    if (gender == 0) {
        table = "zen_male";
    } else {
        table = "zen_female";
    }

    QString filename = QString("%1/%2__%3-%4.pdf").arg(saveFilePath).arg(table).arg(from).arg(to);

    QString sql = QString("select `receipt`, `name`, `gender`, `fname`, `race`, `birthday`, `degree`, `province`, `city`, `district`, `address`, `code` from %1 where id >= '%2' and id <= '%3'")
            .arg(table)
            .arg(from)
            .arg(to);

    QSqlQuery q(sql);
    int cnt = 1;
    int rowCount = q.size();
    if (rowCount > (to - from + 1) || rowCount < (to - from + 1) || (to - from + 1) < 0) {
        qInfo() << "there is not so much info";
        QMessageBox::information(this, "", "请核对数量后再打印");
        return false;
    }

    QFile f(filename);
    f.open(QIODevice::WriteOnly);
    QPdfWriter *writer = new QPdfWriter(&f);
    QPainter *painter = new QPainter(writer);
    QFont font;
    font.setPointSize(8);
    painter->setFont(font);

    writer->setPageSize(QPagedPaintDevice::A4);

    int pixMapMagin = 0;
    int topMargin = 0;
    int height = 0;

    painter->drawText(QRect(500, 0, 10000, 300), QString("***师兄您好，请制证完成后把纸张原路送回，统一处理!!***"));
    painter->drawLine(QPoint(0, 300), QPoint(10000, 300));
    painter->drawLine(QPoint(0, 320), QPoint(10000, 320));
    painter->drawLine(QPoint(0, 320), QPoint(0, 13440));
    painter->drawLine(QPoint(445, 320), QPoint(445, 13440));
    painter->drawLine(QPoint(1000, 320), QPoint(1000, 13440)); // receipt line
    painter->drawLine(QPoint(1800, 320), QPoint(1800, 13440)); // name line
    painter->drawLine(QPoint(2100, 320), QPoint(2100, 13440)); // gender line
    painter->drawLine(QPoint(2600, 320), QPoint(2600, 13440)); // fname line
    painter->drawLine(QPoint(3400, 320), QPoint(3400, 13440)); // race line
    painter->drawLine(QPoint(4200, 320), QPoint(4200, 13440)); // birthday line
    painter->drawLine(QPoint(4700, 320), QPoint(4700, 13440)); // degree line
    painter->drawLine(QPoint(5400, 320), QPoint(5400, 13440)); // province line
    painter->drawLine(QPoint(6100, 320), QPoint(6100, 13440)); // city line
    painter->drawLine(QPoint(6900, 320), QPoint(6900, 13440)); // district line
    painter->drawLine(QPoint(8500, 320), QPoint(8500, 13440)); // address line
    // code with no line

    while (q.next()) {
        QString receipt = q.value(0).toString();
        QString pixmapAbsPath = QString("%1/%2.png").arg(imageFilePath).arg(receipt);
        QPixmap pixmap(pixmapAbsPath);
        height += 640;
        painter->drawPixmap(pixMapMagin, topMargin + height, 442, 619, pixmap);
        painter->drawLine(QPoint(0, height), QPoint(10000, height));
        QStringList strs;
        /*
         * for (int i = 0; i < 12; i++) strs.append(q.value(i).toString());
         * painter->drawText(QRect(500, height + 200, 10000, 300), strs.join(","));
        */
        painter->drawText(QRect(500, height + 200, 500, 300), q.value(0).toString()); // receipt
        painter->drawText(QRect(1050, height + 200, 800, 300), q.value(1).toString()); // name
        painter->drawText(QRect(1850, height + 200, 800, 300), q.value(2).toString()); // gender
        painter->drawText(QRect(2150, height + 200, 800, 300), q.value(3).toString()); // fname
        painter->drawText(QRect(2650, height + 200, 800, 620), q.value(4).toString()); // race
        painter->drawText(QRect(3450, height + 200, 800, 300), q.value(5).toString()); // birthday
        painter->drawText(QRect(4250, height + 200, 800, 300), q.value(6).toString()); // degree
        painter->drawText(QRect(4750, height + 200, 800, 620), q.value(7).toString()); // province
        painter->drawText(QRect(5450, height + 200, 800, 620), q.value(8).toString()); // city
        painter->drawText(QRect(6150, height + 200, 800, 620), q.value(9).toString()); // district
        painter->drawText(QRect(6950, height + 200, 1500, 620), q.value(10).toString()); // address
        painter->drawText(QRect(8550, height + 200, 1200, 300), q.value(11).toString()); // code

        if (cnt % 20 == 0 && (to - from + 1) != cnt) {
            painter->drawLine(QPoint(0, height + 640), QPoint(10000, height + 640));
            writer->newPage();

            painter->drawText(QRect(500, 0, 10000, 300), QString("***师兄您好，请制证完成后把纸张原路送回，统一处理!!***"));
            painter->drawLine(QPoint(0, 320), QPoint(0, 13440));
            painter->drawLine(QPoint(0, 300), QPoint(10000, 300));
            painter->drawLine(QPoint(0, 320), QPoint(10000, 320));
            painter->drawLine(QPoint(445, 320), QPoint(445, 13440));
            painter->drawLine(QPoint(1000, 320), QPoint(1000, 13440)); // receipt line
            painter->drawLine(QPoint(1800, 320), QPoint(1800, 13440)); // name line
            painter->drawLine(QPoint(2100, 320), QPoint(2100, 13440)); // gender line
            painter->drawLine(QPoint(2600, 320), QPoint(2600, 13440)); // fname line
            painter->drawLine(QPoint(3400, 320), QPoint(3400, 13440)); // race line
            painter->drawLine(QPoint(4200, 320), QPoint(4200, 13440)); // birthday line
            painter->drawLine(QPoint(4700, 320), QPoint(4700, 13440)); // degree line
            painter->drawLine(QPoint(5400, 320), QPoint(5400, 13440)); // province line
            painter->drawLine(QPoint(6100, 320), QPoint(6100, 13440)); // city line
            painter->drawLine(QPoint(6900, 320), QPoint(6900, 13440)); // district line
            painter->drawLine(QPoint(8500, 320), QPoint(8500, 13440)); // address line

            pixMapMagin = 0;
            topMargin = 0;
            height = 0;
        }
        cnt += 1;
    }

    delete painter;
    delete writer;
    f.close();

    return true;
}

void MainWindow::on_pushButton_clicked()
{
    // gender_index: 0 男 1 女
    QString gender = ui->comboBox->currentText();
    int gender_index = ui->comboBox->currentIndex();
    int from = ui->spinBoxFrom->text().toInt();
    int to = ui->spinBoxTo->text().toInt();
    qDebug() << gender << gender_index << from << to;
    bool stat = savePrintPdfs(gender_index, from, to);
    if (stat) {
        insertPrintInfo(lineEditEditor->text().trimmed(), gender, from, to);
        initPrintPage();
    }
}

void MainWindow::on_pushButtonTruncateTable_clicked()
{
   QString passcode = QString("truncate");
   QString inputPasscode = ui->lineEditTruncateTable->text();
   if(passcode == inputPasscode) {
       qDebug() << "clean table";
       QSqlQuery query;
       QString trunc = QString("truncate zen_print_info; truncate zen_print_images; truncate zen_config; "
                  "truncate zen_admin; " "truncate zen_male; " "truncate zen_female; ");
       query.exec(trunc);

       qDebug() << query.lastQuery() << query.lastError().text();


       // delete images
       QDir dir(imageFilePath);
       dir.setNameFilters(QStringList() << "*.png");
       foreach (QString dirFile, dir.entryList()) {
           dir.remove(dirFile);
       }
       ui->labelInitStat->setText("数据库在开始时已经清理，请勿在中途清理数据库！！！");
       QMessageBox::information(this, "Yes", "成功初始化");
   } else {
        QMessageBox::information(this, "No", "输入口令错误");
   }

   ui->lineEditTruncateTable->clear();
}

int MainWindow::getImages()
{
    canPrinted.clear();
    QSqlQuery query;
    QString sql = QString("select receipt from zen_male where mark = 1 union all select receipt from zen_female where mark = 1");
    query.exec(sql);
    int cnt = 0;
    while(query.next()) {
        QString receipt = query.value(0).toString();
        qDebug() << receipt;
        canPrinted.append(receipt);
        cnt++;
    }

    return cnt;
}

void MainWindow::makePhotos(QString imagePath, QString savePath, int imageWidth, int imageHeight)
{

    // Get images and test if sync to server
    QSqlQuery query;
    QString sql = QString("select receipt from zen_male where mark = 1 union all select receipt from zen_female where mark = 1");
    query.exec(sql);
    int cnt = 0;

    while(query.next()) {
        QString receipt = query.value(0).toString();
        cnt++;
    }

    QChar separator = QDir::separator();

    QFont font;
    font.setPointSize(10);
    QFile pdfFile(savePath + separator + QString("output.pdf"));
    pdfFile.open(QIODevice::WriteOnly);

    QPdfWriter *pdfWriter = new QPdfWriter(&pdfFile);
    pdfWriter->setPageMargins(QMarginsF(0, 0, 0, 0));
    pdfWriter->setPageSize(QPagedPaintDevice::A4);

    QPainter *pdfPainter = new QPainter(pdfWriter);
    pdfPainter->setFont(font);

    int height = pdfPainter->device()->height();
    int width = pdfPainter->device()->width();

    QRect rect(0, 0, pdfPainter->device()->width(), pdfPainter->device()->height());

    float hs = height / 7.0;
    float ws = width / 8.0;

    QPoint points[8][7];
    for (int i = 0; i < 8; ++i) {
        for(int j = 0; j < 7; ++j) {
            points[i][j].setX(i*ws);
            points[i][j].setY(j*hs);
        }
    }

    int imagesCnt;
    QDir dir(imagePath);
    if (!dir.exists()) {
        return;
    }
    dir.setFilter(QDir::Files| QDir::NoSymLinks);
    QStringList filters;
    filters << QString("*.jpeg") << QString("*.png") << QString("jpg");
    dir.setNameFilters(filters);
    imagesCnt = dir.count();
    if (imagesCnt <= 0) {
        return;
    }

    for (int m = 0; m < imagesCnt; m += 56) {
        pdfPainter->drawRect(rect);
        for (int j = 1; j < 7; j++) {
            pdfPainter->drawLine(QPointF(0, j*hs), QPointF(width, j*hs));
        }

        for(int j = 1; j < 8; j++) {
            pdfPainter->drawLine(QPointF(j*ws, 0), QPointF(j*ws, height));
        }

        for(int i = 0; i < 8; i++) {
            for(int j = 0; j < 7; j++) {
                if (cnt >= imagesCnt) break;
                QPixmap pixmap(imagePath + separator + dir[cnt]);
                pdfPainter->drawPixmap(points[i][j].rx(), points[i][j].ry(), imageWidth, imageHeight, pixmap);
                pdfPainter->drawText(QRect(points[i][j].rx(), points[i][j].ry() + imageHeight, 2000, 300), dir[cnt].section('.', 0, 0));
                qDebug() << dir[cnt];
                cnt++;
            }
        }
        if (imagesCnt - cnt > 0) {
            pdfWriter->newPage();
        }
    }

    pdfPainter->end();
    pdfFile.close();

    delete pdfPainter;
    delete pdfWriter;
}

void MainWindow::on_pushButton_2_clicked()
{
    /* 生成打印的照片文件*/
    int readyToPrintedCnt = getImages();
    qInfo() << "photo can be printed cnt: " << readyToPrintedCnt;
    foreach(const QString &str, canPrinted) {
        qDebug() << "[" << str << "]";
    }
    qDebug() << imageFilePath;

    makePrintedPhotos(imageFilePath, 1050, 1470);
    initPrintPage();
}

void MainWindow::on_toolButton_2_clicked()
{
    QSqlQuery query;
    query.exec("truncate connect.zen_person");
    QString sql1 =  "insert into connect.zen_person (receipt, name, gender, birthday, phone_num, province, city, district, address, code) \
            select receipt, name, gender, birthday, phone_num, province, city, district, address, code from citta.zen_female; ";
    QString sql2 = "insert into connect.zen_person (receipt, name, gender, birthday, phone_num, province, city, district, address, code) \
            select receipt, name, gender, birthday, phone_num, province, city, district, address, code from citta.zen_male; ";
    query.exec(sql1);
    query.exec(sql2);
    qDebug() << query.lastQuery() << query.lastError();
}

bool MainWindow::makePrintedPhotos(QString imagePath, int imageWidth, int imageHeight)
{
    /* 1. 获取已经拍照的列表 */
    bool isChecked; // true: 全部打印 false: 只打印56张的倍数
    QStringList hasBeenTaken;
    QSqlQuery query("(select receipt, name from zen_male where mark = 1 order by id) "
                    " union all (select receipt, name from zen_female where mark = 1 order by id)");

    while(query.next()) {
        hasBeenTaken.append(query.value(0).toString());
    }

    /* 2. 获取已经通过 RSYNC 传递过来的图片文件 */
    int imagesCnt;
    QDir dir(imagePath);
    if (!dir.exists()) {
        qDebug() << "directory not exists";
        return false;
    }

    dir.setFilter(QDir::Files| QDir::NoSymLinks);
    QStringList filters;
    filters << QString("*.png");
    dir.setNameFilters(filters);
    imagesCnt = dir.count();
    qInfo() << "imagesCnt=" << imagesCnt;
    if (imagesCnt <= 0) {
        QMessageBox::information(this, "", QString("%1 路径未发现有照片，请确保RSYNC 服务开启可用").arg(imagePath));
        return false;
    }

    isChecked = ui->radioButtonIfPrintToEnd->isChecked();

    /* 3. 获取可以打印的照片列表 = 已传过来 且 mark = 1 的 */
    int index;
    QStringList canBePrint;
    for(int i = 0; i < imagesCnt; i++) {
        QString fileName = dir[i];
        fileName = fileName.section(".", 0, 0);
        index = hasBeenTaken.indexOf(fileName);
        if (index >= 0) {
            canBePrint.append(fileName);
            qDebug() << QString("canBePrinted: %1").arg(fileName);
        }
    }

    /* 4. 准备打印文件 */
    int canBePrintCnt = canBePrint.count();

    if (canBePrintCnt <= 0) {
        QString info = QString("现在没有可以打印的文件");
        qInfo() << info;
        QMessageBox::information(this, "", info);
        return false;
    }

    if (!isChecked) { // 如果不打印到最后，只是按最优的打印方式
        if (canBePrintCnt < 56) {
            QString info = QString("目前可打印的照片数量为: %1，目前打印模式为省纸模式。"
                                   " 当且仅当照片数量大于56时才会打印。").arg(canBePrintCnt);
            qInfo() << info;
            QMessageBox::information(this, "", info);
            return false;
        }
        canBePrintCnt = canBePrintCnt - canBePrintCnt % 56;
    }

    qInfo() << "目前可以打印的照片数量为:" << canBePrintCnt << "张。";

    /* 5. 准备打印成PDF文件 */
    QChar separator = QDir::separator();
    QString savePath = saveFilePath;
    QFont font;
    font.setPointSize(10);
    QString fname = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    QFile pdfFile(savePath + separator + "photo." + fname + ".pdf");
    pdfFile.open(QIODevice::WriteOnly);

    QPdfWriter *pdfWriter = new QPdfWriter(&pdfFile);
    pdfWriter->setPageMargins(QMarginsF(0, 0, 0, 0));
    pdfWriter->setPageSize(QPagedPaintDevice::A4);

    QPainter *pdfPainter = new QPainter(pdfWriter);
    pdfPainter->setFont(font);

    int height = pdfPainter->device()->height();
    int width = pdfPainter->device()->width();

    QRect rect(0, 0, pdfPainter->device()->width(), pdfPainter->device()->height());

    float hs = height / 7.0;
    float ws = width / 8.0;

    QPoint points[8][7];
    for (int i = 0; i < 8; ++i) {
        for(int j = 0; j < 7; ++j) {
            points[i][j].setX(i*ws);
            points[i][j].setY(j*hs);
        }
    }

#ifndef MAX_IMAGES_CNT
#define MAX_IMAGES_CNT 5000
#endif
    int i = 0;
    QString fileNameArray[MAX_IMAGES_CNT];
    foreach (QString str, canBePrint) {
        if (i > canBePrintCnt) break;
        fileNameArray[i] = str;
        i++;
    }

    int cnt = 0;
    for (int m = 0; m < canBePrintCnt; m += 56) {
        pdfPainter->drawRect(rect);
        for (int j = 1; j < 7; j++) {
            pdfPainter->drawLine(QPointF(0, j*hs), QPointF(width, j*hs));
        }

        for(int j = 1; j < 8; j++) {
            pdfPainter->drawLine(QPointF(j*ws, 0), QPointF(j*ws, height));
        }

        for(int i = 0; i < 8; i++) {
            for(int j = 0; j < 7; j++) {
                if (cnt >= canBePrintCnt) break;
                QPixmap pixmap(imagePath + separator + fileNameArray[cnt] + ".png");
                pdfPainter->drawPixmap(points[i][j].rx(), points[i][j].ry(), imageWidth, imageHeight, pixmap);
                pdfPainter->drawText(QRect(points[i][j].rx(), points[i][j].ry() + imageHeight, 2000, 300), fileNameArray[cnt]);
                cnt++;
            }
        }
        if (canBePrintCnt - cnt > 0) {
            pdfWriter->newPage();
        }
    }

    pdfPainter->end();
    pdfFile.close();
    delete pdfPainter;
    delete pdfWriter;

    QString arrayString = "";

    /* 6. 更新数据库，设置已经打印的标记为 2 */
    for(int i = 0; i < canBePrintCnt; i++) {
        QString sql;
        if (fileNameArray[i].startsWith("A")) {
            sql = QString("update zen_male set mark = 2 where receipt = '%1'").arg(fileNameArray[i]);
            arrayString += fileNameArray[i];
            arrayString += ",";
        } else {
            sql = QString("update zen_female set mark = 2 where receipt = '%1'").arg(fileNameArray[i]);
            arrayString += fileNameArray[i];
            arrayString += ",";
        }
        query.exec(sql);
        qDebug() << sql;
    }

    insertPrintImages(lineEditEditor->text().trimmed(), arrayString);

    return true;
}

static bool portTest(QString ip, int port)
{
    /* Func: 端口测试 */
    bool ret;
    QTcpSocket tsock;
    tsock.connectToHost(ip, port);
    ret = tsock.waitForConnected(1000);
    if (ret) tsock.close();
    return ret;
}

void MainWindow::on_pushButtonInitDb_clicked()
{
    /* Func: 数据库初始化 */
    qInfo() << "Init mysql database";
    QString ip = ui->lineEditInitIp->text().trimmed();
    QString user = ui->lineEditInitDbUser->text().trimmed();
    QString pass = ui->lineEditInitDbPassword->text().trimmed();

    if (user.isEmpty() || pass.isEmpty()) {
        qInfo() << "Username or password is empty";
        QMessageBox::critical(this, "", "数据库用户名或者密码没输入");
        return;
    }

    if (ip.isEmpty()) ip = "127.0.0.1";
    if (!portTest(ip, 3306)) {
        qFatal("Mysql server not installed or port is not open");
        QMessageBox::information(this, "", "数据库没有开放3306端口，或者压根就没安装啦!");
        return;
    }

    dbinit = QSqlDatabase::addDatabase("QMYSQL");
    dbinit.setHostName(ip);
    dbinit.setUserName(user);
    dbinit.setPassword(pass);

    if(!dbinit.open()) {
        QMessageBox::critical(this, "数据库错误", dbinit.lastError().text());
        qInfo() << "Can not open database" << dbinit.lastError().text();
        return;
    }

    QSqlQuery query(dbinit);
    QFile sqlFile(":/sql");
    sqlFile.open(QIODevice::ReadOnly);
    QTextStream in(&sqlFile);
    QString text = in.readAll();
    QStringList list = text.split(";");

    foreach (QString str, list) {
        QString sql = str.replace("\n", "");
        if (str.isEmpty()) continue;
        query.exec(sql);
        qDebug() << query.lastError().text();
    }

    sqlFile.close();

    qInfo() << "Clear username and password lineEdits";

    ui->lineEditInitDbUser->clear();
    ui->lineEditInitDbPassword->clear();
    ui->lineEditInitIp->clear();
}

void MainWindow::showHelp()
{
    QFile help(":/help.html");
    help.open(QIODevice::ReadOnly);
    QTextStream in(&help);
    QString text = in.readAll();
    help.close();
    ui->textBrowserHelp->setHtml(text);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    qDebug() << "Current tab index: " << index;
    switch(index) {
    case 0:
        qDebug() << "0 tab";
        break;
    case 1:
        qDebug() << "1 tab";
        break;
    case 2:
        initPrintPage();
        break;
    case 3:
        qDebug() << "update statics";
        refreshStat();
        break;
    case 4:
        qDebug() << "4 tab";
        break;
    case 5:
        // admin page
        initAdminPage();
        qDebug() << "5 tab";
        break;
    case 6:
        qDebug() << "6 tab";
        break;
    default:
        break;
    }
}

void MainWindow::refreshStat()
{
    QSqlQuery query;
    QString male_cnt;
    QString female_cnt;
    QString beijing_cnt;
    // male count
    query.exec("select count(id) as male_cnt from zen_male");
    while (query.next()) {
        male_cnt = query.value(0).toString();
        ui->man->setText(male_cnt);
    }

    // female count
    query.exec("select count(id) as female_cnt from zen_female");
    while (query.next()) {
        female_cnt = query.value(0).toString();
        ui->woman->setText(female_cnt);
    }

    QString sum = QString("%1").arg(male_cnt.toInt() + female_cnt.toInt());
    ui->sum->setText(sum);

    query.exec("select count(id) as beijing_cnt from "
               "((select * from zen_male) union all (select * from zen_female)) a "
               "where province like '%北京%'");

    qDebug() << query.lastQuery();
    while (query.next()) {
        beijing_cnt = query.value(0).toString();
        ui->beijing->setText(beijing_cnt);
    }
    QString jingwai_cnt = QString("%1").arg(sum.toInt() - beijing_cnt.toInt());
    ui->jingwai->setText(jingwai_cnt);
}

void MainWindow::on_refreshView_clicked()
{
    // 调用刷新统计信息函数
    refreshStat();
}

void MainWindow::afterQueryPresshed()
{
    qDebug() << "afterQueryPresshed";
}

/* export guiyi data */
void MainWindow::on_toolButtonExcel_clicked()
{
    exportAllExcels();
}

void MainWindow::on_toolButtonPdf_clicked()
{
    exportAllPdfs();
}

void MainWindow::exportAllPics()
{
    QString imagePath = ui->lineEditImagePath->text().trimmed(); // 原始照片存放路径

    // export images
    QString savePath;
    if (ui->lineEditBackPath->text().trimmed().isEmpty()) {
        savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    } else {
        savePath = ui->lineEditBackPath->text();
    }

    QString exportImageAbsPath = QString("%1/images.%2/").arg(savePath).arg(currentDate);
    qDebug() << exportImageAbsPath;
    QDir *tmp = new QDir;
    if (!tmp->exists(exportImageAbsPath)) {
        tmp->mkpath(exportImageAbsPath);
        qInfo() << "mkpath" << exportImageAbsPath;
    }

    int imagesCnt;
    QDir dir(imagePath);
    if (!dir.exists()) {
        QString info;
        info = QString("路径:  %1 没发现有照片").arg(savePath);
        QMessageBox::critical(this, "", info);
        return;
    }

    dir.setFilter(QDir::Files| QDir::NoSymLinks);
    QStringList filters;
    filters << QString("*.png");
    dir.setNameFilters(filters);
    imagesCnt = dir.count();
    qInfo() << "imagesCnt=" << imagesCnt;
    qInfo() << "src:" << imagePath << "dst:" << exportImageAbsPath;

    qDebug() << "male start:" << maleStart << "female start:" << femaleStart;

    for(int i = 0; i < imagesCnt; i++) {
        QString oriName = dir[i];
        QChar c = oriName.toUpper()[0];
        int currentNum = oriName.mid(1, 4).toInt();
        QString fileName;
        if (c == 'A') {
            fileName.sprintf("A%010d.png", maleStart + currentNum);
        } else if (c == 'B') {
            fileName.sprintf("B%010d.png", femaleStart + currentNum);
        } else {
            qInfo() << "file wrong: " << oriName;
            continue;
        }

        QString srcFile = QString("%1/%2").arg(imagePath).arg(dir[i]);
        QString dstFile = QString("%1/%2").arg(exportImageAbsPath).arg(fileName);
        QFile::copy(srcFile, dstFile);
    }
}

void MainWindow::on_toolButtonPics_clicked()
{
    exportAllPics();
}

bool MainWindow::testIfAdmin()
{
    QString adminName = lineEditEditor->text().trimmed();
    if (adminName.isEmpty()) {
        qInfo() << "管理员必填选项没有填写";
        QMessageBox::critical(this, "", "管理员必填处请输入自己的名字");
        return false;
    }

    QString IP = lineEditConfig->text().trimmed();
    /* IP ends num < 9 got all privileges */
    if (IP.section(".", -1).toInt() < 9) {
        return true;
    }

    ui->actionExportPdf->setDisabled(true);
    ui->actionPdf->setDisabled(true);
    ui->actionExcel->setDisabled(true);
    ui->tabConfig->setDisabled(true);
    ui->tab->setDisabled(true);
    ui->tab_4->setDisabled(true);
    ui->tabConfig->setHidden(true);
    return true;
}

void MainWindow::getLocalAddr()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();

    for(int i = 0; i < list.count(); i++) {
        if(!list[i].isLoopback())
            if (list[i].protocol() == QAbstractSocket::IPv4Protocol ) {
                QString ip = list[i].toString();
                qDebug() << ip;
                if (ip.startsWith("192.168")) {
                    localAddr = ip;
                }
            }
    }

    if (lineEditConfig->text().trimmed().startsWith("127")) {
        qDebug() << "getLocalAddr" << lineEditConfig->text().trimmed();
        localAddr = "127.0.0.1";
    }
}

/*
CREATE TABLE IF NOT EXISTS `zen_print_info` (
  `name` varchar(45) NOT NULL COMMENT '管理员姓名',
  `gender` varchar(10) DEFAULT NULL COMMENT '性别',
  `start` int(11) NOT NULL COMMENT '打印起始',
  `end` int(11) NOT NULL COMMENT '打印结束',
  `signtime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '记录写入时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `zen_print_images` (
  `name` varchar(45) NOT NULL COMMENT '管理员姓名',
  `array` text NOT NULL COMMENT '打印的内容',
  `signtime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '记录写入时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
*/

void MainWindow::insertPrintInfo(QString name, QString gender, int start, int end)
{
    QSqlQuery query;
    QString sql = QString("insert into zen_print_info(name, gender, start, end)  values ('%1', '%2', '%3', '%4')")
            .arg(name).arg(gender).arg(start).arg(end);
    query.exec(sql);
    qDebug() << sql << query.lastError().text();
}

void MainWindow::insertPrintImages(QString name, QString array)
{
    QSqlQuery query;
    QString sql = QString("insert into zen_print_images (name, array) values ('%1', '%2')").arg(name, array);
    query.exec(sql);
    qDebug() << sql << query.lastError().text();
}

void MainWindow::insertAdminInfo()
{
    QSqlQuery query;
    QString sql = QString("insert into zen_admin (name, ipaddr) values ('%1', '%2')").arg(lineEditEditor->text().trimmed(), localAddr);
    query.exec(sql);
    qDebug() << "insertAdminInfo: " << sql << query.lastError().text();
}

void MainWindow::on_tabWidget_tabBarDoubleClicked(int index)
{
    qDebug() << "tab:" << index;
    switch(index) {
    case 5:
        initAdminPage();
        qDebug() << "initAdminPage";
        break;
    default:
        break;
    }
}

void MainWindow::on_pushButtonFee_clicked()
{
    fee = ui->lineEditFee->text().trimmed().toInt();
    qDebug() << "Fee = " << fee;
    ui->lineEditFee->setDisabled(true);
}
