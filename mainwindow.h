#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QMenu>
#include <QDebug>
#include <QtSql>
#include <QTcpSocket>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QTableView>
#include <QSqlTableModel>
#include <QPdfWriter>
#include <QPainter>
#include <QFileDialog>
#include <QPageLayout>
#include <QMarginsF>
#include <QFile>
#include <QFont>
#include <QPixmap>
#include <QPen>
#include <QDate>
#include <QStringList>
#include <QSqlQueryModel>
#include "xlsxdocument.h"
#define DB_NAME "citta"
#define DB_PASS "attic"
#define DB_USER "citta"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool databaseTest();
    void getLocalAddr();
    void insertAdminInfo();
    bool searchInfo(QString search);
    bool connectDatabase();
    void setFahuiInfo();
    void setPaths();
    void setModel(QSqlTableModel *mod, QString tableName, QTableView *view);
    void makePhotos(QString imagePath, QString savePath, int imageWidth, int imageHeight);
    void savePdfs(QString fileName, QSqlTableModel *mod, QString filter);
    void makePrintedPhotos(QString imagePath, int imageWidth, int imageHeight);
    void savePrintPdfs(int gender, int from, int to); // 0: male, 1: female
    void exportExcel(QString fileName, QSqlTableModel *mod);
    void createCard(QString fileName, QSqlTableModel *mod, QString filter, QString pixmapPath);

    bool testIfAdmin();
    int getImages();
    void showHelp();
    void refreshStat();

    void exportAllExcels();
    void exportAllPdfs();
    void exportAllPics();
    void initAdminPage();

    QLineEdit *lineEditSearch;
    QLineEdit *lineEditConfig;
    QLineEdit *lineEditEditor;

    QString saveFilePath;
    QString desktopPath;
    QString homePath;
    QString imageFilePath;
    QString currentDate;

    QStringList canPrinted;
    QString localAddr;

    int maleStart;
    int femaleStart;

private slots:
    void afterQueryPresshed();
    void on_actionSetting_triggered();

    void on_actionDb_triggered();
    void on_actionRefresh_triggered();

    void on_tableView_customContextMenuRequested(const QPoint &pos);

    void on_tableViewFemale_customContextMenuRequested(const QPoint &pos);

    void on_actionExportPersonalInfo_triggered();

    void on_actionPrintPersonnelCredentials_triggered();

    void on_actionExportPdf_triggered();

    void on_toolButtonImagePath_clicked();

    void on_toolButtonBackPath_clicked();

    void on_actionCard_triggered();

    void on_actionExcel_triggered();

    void on_actionPdf_triggered();

    void on_toolButton_clicked();

    void on_pushButton_clicked();

    void on_pushButtonTruncateTable_clicked();



    void on_pushButton_2_clicked();

    void on_toolButton_2_clicked();

    void on_pushButtonInitDb_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_refreshView_clicked();

    void on_toolButtonExcel_clicked();

    void on_toolButtonPdf_clicked();

    void on_toolButtonPics_clicked();

    void on_tabWidget_tabBarDoubleClicked(int index);

    void on_pushButtonFee_clicked();

private:
    Ui::MainWindow *ui;
    QString serverIp;
    QSqlDatabase db;
    QSqlDatabase dbinit;
    QSqlTableModel *model;
    QSqlTableModel *modelFemale;
    QSqlQueryModel *modelAdmin;
    QSqlQueryModel *modelCitta;
    QSqlQueryModel *modelBrowser;
    int fee;
};

#endif // MAINWINDOW_H
