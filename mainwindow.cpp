#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QSqlError"
#include "QSqlDatabase"
#include "QSqlIndex"
#include "QSqlQuery"
#include "QMessageBox"
#include "QDebug"
#include "QFile"
#include "QIcon"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(280, 175);
    setWindowIcon(QIcon(":/database.ico"));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    QString path("X:\\Library_PADS\\VBReport\\Output\\");
    QFile accessDB(path + "Library.mdl");
    if (!accessDB.exists())
    {
        QMessageBox::critical(this, "Ошибка!", "Не существует .mdl файла");
        return;
    }
    QFile oldMDB(path + "Library.mdb");
    if (oldMDB.exists())
    {
        if(!oldMDB.remove())
        {
            QMessageBox::critical(this, "Ошибка!", "Старый .mdb файл не может быть удален");
            return;
        }
    }
    if (!accessDB.rename(path + "Library.mdb"))
    {
        QMessageBox::critical(this, "Ошибка!", "Ошибка при переименовании .mdl в .mdb");
        return;
    }
    db.setDatabaseName(QString("Driver={Microsoft Access Driver (*.mdb, *.accdb)};DSN='';DBQ=%1Library.mdb").arg(path));
    db.open();
    if (!db.isOpen()) {
        this->hide();
        QMessageBox::critical(this, "Ошибка при открытии БД", db.lastError().text());
    }
    else {
        QSqlQuery query = db.exec("SELECT ps.ShapeID, ps.PartNumberID, ps.ShapeName, ps.ShapeType, pn.PartitionID, pn.PartNumber, pn.PartName, pn.PartLabel, pn.RefDesPre, pn.Description INTO TempTable \
                                  FROM PartNumbers AS pn RIGHT JOIN PartShapes AS ps ON pn.PartNumberID = ps.PartNumberID \
                                  GROUP BY ps.ShapeID, ps.PartNumberID, ps.ShapeName, ps.ShapeType, pn.PartitionID, pn.PartNumber, pn.PartName, pn.PartLabel, pn.RefDesPre, pn.Description;");
        query.finish();
        query = db.exec("DROP TABLE PartNumbers;");
        query = db.exec("SELECT * INTO PartNumbers FROM TempTable;");
        query = db.exec("DROP TABLE TempTable;");
        db.close();
        this->hide();
        QMessageBox msgOk;
        msgOk.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        msgOk.setWindowTitle("Создание .mdb");
        msgOk.setText("Создана .mdb! Таблица PartNumbers обновлена!");
        msgOk.exec();
    }
}
