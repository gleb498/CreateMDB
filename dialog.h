#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_browse_clicked();

private:
    Ui::Dialog *ui;
    QString mdbPath;
    QString getLibPath();
    bool getLibraryDir();
};

#endif // DIALOG_H
