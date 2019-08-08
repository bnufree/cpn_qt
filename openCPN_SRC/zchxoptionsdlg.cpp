#include "zchxoptionsdlg.h"
#include "ui_zchxoptionsdlg.h"
#include "zchxchecklistwidget.h"

zchxOptionsDlg::zchxOptionsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zchxOptionsDlg)
{
    ui->setupUi(this);
}

zchxOptionsDlg::~zchxOptionsDlg()
{
    delete ui;
}



void zchxOptionsDlg::on_bOpenGL_clicked()
{

}
