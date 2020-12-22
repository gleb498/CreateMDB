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
    libPath = "";
    setFixedSize(509, 145);
    setWindowIcon(QIcon(":/database.ico"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

QString Dialog::getProjectFileWithPath()
{
    return ui->lineEdit_prjFile->text();
}

bool Dialog::getLibraryDir()
{
    libPath = "";
    QString prjPath = getProjectFileWithPath();
    if (prjPath == "")
    {
        return true;
    }
    QFile prjFile(prjPath);
    if (!prjFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, QString("Ошибка!"),
        QString("Неудалось открыть файл %1").arg(prjPath),
        QString("Ок"));
        return false;
    }

    QTextStream stream(&prjFile);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.contains("KEY CentralLibrary"))
        {
            libPath = line.split("KEY CentralLibrary")[1].simplified().remove(QChar('"'));
            QStringList dirList = libPath.split('/');
            if (dirList[0] != libPath)
            {
                libPath.remove(dirList.last());
            }
            else
            {
                dirList = libPath.split('\\');
                if (dirList[0] == libPath)
                {
                    prjFile.close();
                    QMessageBox::critical(this, QString("Ошибка!"),
                                          QString("Некорректный путь к библиотеке"),
                                          QString("Ок"));
                    return false;
                }
                libPath.remove(dirList.last());
            }
            break;
        }
    }
    if (stream.status() != QTextStream::Ok) {
        prjFile.close();
        QMessageBox::critical(this, QString("Ошибка!"),
                              QString("Ошибка чтения файла %1").arg(prjPath),
                              QString("Ок"));
        return false;
    }
    if (libPath == "")
    {
        prjFile.close();
        QMessageBox::critical(this, QString("Ошибка!"),
                              QString("Не найден путь к библиотеке"),
                              QString("Ок"));
        return false;
    }
    libPath += "VBReport\\Output\\";
    prjFile.close();
    return true;
}

void Dialog::on_pushButton_clicked()
{
    if(!getLibraryDir())
    {
        return;
    }
    if (libPath == "")
    {
//        libPath = "C:\\Users\\Gleb\\Desktop\\MDL\\";
        libPath = "X:\\Library_PADS\\VBReport\\Output\\";
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    QFile accessDB(libPath + "Library.mdl");
    if (!accessDB.exists())
    {
        QMessageBox::critical(this, "Ошибка!", QString("Не существует файла\n %1").arg(accessDB.fileName()));
        return;
    }
    QFile oldMDB(libPath + "Library.mdb");
    if (oldMDB.exists())
    {
        if(!oldMDB.remove())
        {
            QMessageBox::critical(this, "Ошибка!", "Старый .mdb файл не может быть удален");
            return;
        }
    }
    if (!accessDB.rename(libPath + "Library.mdb"))
    {
        QMessageBox::critical(this, "Ошибка!", "Ошибка при переименовании .mdl в .mdb");
        return;
    }
    db.setDatabaseName(QString("Driver={Microsoft Access Driver (*.mdb, *.accdb)};DSN='';DBQ=%1").arg(libPath + "Library.mdb"));
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
        msgOk.setWindowTitle("MDB");
        msgOk.setText("Создана .mdb! Таблица PartNumbers обновлена!");
        msgOk.exec();
    }
}

void Dialog::on_pushButton_browse_clicked()
{
    ui->lineEdit_prjFile->setText(QFileDialog::getOpenFileName(0, tr("Open Project Directory"), "C:\\", tr("*.prj")));
}
