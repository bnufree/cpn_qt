#ifndef ZCHXOPTIONSDLG_H
#define ZCHXOPTIONSDLG_H

#include <QDialog>

namespace Ui {
class zchxOptionsDlg;
}

class zchxOptionsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit zchxOptionsDlg(QWidget *parent = 0);
    ~zchxOptionsDlg();

private:
    Ui::zchxOptionsDlg *ui;
};

#endif // ZCHXOPTIONSDLG_H
