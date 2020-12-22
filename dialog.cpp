#include "dialog.h"
#include "ui_dialog.h"
#include "QSqlError"
#include "QSqlDatabase"
#include "QSqlIndex"
#include "QSqlQuery"
#include "QMessageBox"
#include "QDebug"
#include "QFile"
#include "QIcon"
#include "QFileDialog"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    mdbPath = "";
    setFixedSize(509, 145);
    setWindowIcon(QIcon(":/database.ico"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    ui->lineEdit_prjFile->setText("X:/Library_PADS/Library.lmc");
}

Dialog::~Dialog()
{
    delete ui;
}

QString Dialog::getLibPath()
{
    return ui->lineEdit_prjFile->text();
}

bool Dialog::getLibraryDir()
{
    mdbPath = "";
    QString libPath = getLibPath();
    if (libPath == "")
    {
        QMessageBox::critical(this, "Ошибка!", QString("Не указан файл библиотеки"));
        return false;
    }
    int pos = libPath.lastIndexOf("/");
    mdbPath = libPath.left(pos);
    mdbPath += "\\VBReport\\Output\\";
    QFileInfo info(mdbPath + "Library.mdl");
    if (!info.exists())
    {
        QMessageBox::critical(this, "Ошибка!", QString("Не существует файла\n %1").arg(mdbPath + "Library.mdl"));
        return false;
    }
    return true;
}

void Dialog::on_pushButton_clicked()
{
    if(!getLibraryDir())
    {
        return;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    QFile accessDB(mdbPath + "Library.mdl");
    if (!accessDB.exists())
    {
        QMessageBox::critical(this, "Ошибка!", QString("Не существует файла\n %1").arg(accessDB.fileName()));
        return;
    }
    QFile oldMDB(mdbPath + "Library.mdb");
    if (oldMDB.exists())
    {
        if(!oldMDB.remove())
        {
            QMessageBox::critical(this, "Ошибка!", "Старый .mdb файл не может быть удален");
            return;
        }
    }
    if (!accessDB.rename(mdbPath + "Library.mdb"))
    {
        QMessageBox::critical(this, "Ошибка!", "Ошибка при переименовании .mdl в .mdb");
        return;
    }
    db.setDatabaseName(QString("Driver={Microsoft Access Driver (*.mdb, *.accdb)};DSN='';DBQ=%1").arg(mdbPath + "Library.mdb"));
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
        msgOk.setWindowTitle("Create .mdb");
        msgOk.setText("Создана .mdb! Таблица PartNumbers обновлена!");
        msgOk.exec();
    }
}

void Dialog::on_pushButton_browse_clicked()
{
    ui->lineEdit_prjFile->setText(QFileDialog::getOpenFileName(0, tr("Open Project Directory"), "C:\\", tr("*.lmc")));
}
