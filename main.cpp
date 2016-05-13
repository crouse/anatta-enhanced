#include "mainwindow.h"
#include <QApplication>

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;

    case QtWarningMsg:
        text = QString("Warning:");
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        break;

    case QtFatalMsg:
        text = QString("Fatal:");
        break;
    case QtInfoMsg:
        text = QString("Info:");
        break;
    }

    QString logPath = QString("%1/anatta.log").arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ");
    QString current_date = QString("%1 ").arg(current_date_time);
    QString message = QString("%3 %1 %2").arg(text).arg(msg).arg(current_date);

    QFile file(logPath);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();

    mutex.unlock();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qInstallMessageHandler(outputMessage);
    MainWindow w;
    w.show();
    qDebug() << "Hello world";
    qWarning() << "ceshi";
    qInfo() << "Exxx";

    return a.exec();
}
