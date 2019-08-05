#ifndef ZCHXOPENGLOPTIONDLG_H
#define ZCHXOPENGLOPTIONDLG_H

#include <QDialog>

namespace Ui {
class zchxOpenGlOptionDlg;
}

class zchxOpenGlOptionDlg : public QDialog
{
    Q_OBJECT

public:
    explicit zchxOpenGlOptionDlg(QWidget *parent = 0);
    ~zchxOpenGlOptionDlg();

private:
    Ui::zchxOpenGlOptionDlg *ui;
};

#endif // ZCHXOPENGLOPTIONDLG_H
