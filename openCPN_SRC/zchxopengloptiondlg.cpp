#include "zchxopengloptiondlg.h"
#include "ui_zchxopengloptiondlg.h"

zchxOpenGlOptionDlg::zchxOpenGlOptionDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zchxOpenGlOptionDlg)
{
    ui->setupUi(this);
}

zchxOpenGlOptionDlg::~zchxOpenGlOptionDlg()
{
    delete ui;
}
