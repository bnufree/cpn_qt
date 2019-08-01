#ifndef SENDTOGPSDLG_H
#define SENDTOGPSDLG_H

#include <QDialog>

#include <QComboBox>
#include <QProgressBar>
#include <QPushButton>

//    Constants for SendToGps... Dialog
#define ID_STGDIALOG 10005
#define SYMBOL_STG_STYLE    Qt::Dialog|Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint|Qt::WindowSystemMenuHint
#define SYMBOL_STG_TITLE ("Send to GPS")
#define SYMBOL_STG_IDNAME ID_STGDIALOG
#define SYMBOL_STG_SIZE_WIDTH   500
#define SYMBOL_STG_SIZE_HEIGHT  500

enum {
      ID_STG_CANCEL =            10000,
      ID_STG_OK,
      ID_STG_CHOICE_COMM
};

class Route;
class RoutePoint;

namespace Ui {
class SendToGpsDlg;
}

class SendToGpsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SendToGpsDlg(QWidget *parent = 0);
    ~SendToGpsDlg();
    SendToGpsDlg(const QString& caption, const QString& hint, const QSize& size, long style, QWidget* parent = 0);
    bool Create(const QString& caption = SYMBOL_STG_TITLE, const QString& hint = SYMBOL_STG_TITLE, const QSize& size = QSize(SYMBOL_STG_SIZE_WIDTH, SYMBOL_STG_SIZE_HEIGHT), long style = SYMBOL_STG_STYLE);
    void SetRoute(Route *pRoute){m_pRoute = pRoute;}
    void SetWaypoint(RoutePoint *pRoutePoint){m_pRoutePoint = pRoutePoint;}
private slots:
    void OnCancelClick();
    void OnSendClick();

private:
    Ui::SendToGpsDlg *ui;
    Route               *m_pRoute;
    RoutePoint          *m_pRoutePoint;
    QComboBox           *m_itemCommListBox;
    QProgressBar        *m_pgauge;
    QPushButton         *m_CancelButton;
    QPushButton         *m_SendButton;
};

#endif // SENDTOGPSDLG_H
