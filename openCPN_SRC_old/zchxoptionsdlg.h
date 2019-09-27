#ifndef ZCHXOPTIONSDLG_H
#define ZCHXOPTIONSDLG_H

#include <QDialog>
#include "_def.h"


namespace Ui {
class zchxOptionsDlg;
}

/* Define an int bit field for dialog return value
 * to indicate which types of settings have changed */

enum OptionChgValue{
    GENERIC_CHANGED  = 1,
    S52_CHANGED = 2,
    FONT_CHANGED = 4,
    FORCE_UPDATE = 8,
    VISIT_CHARTS = 16,
    LOCALE_CHANGED = 32,
    TOOLBAR_CHANGED = 64,
    CHANGE_CHARTS = 128,
    SCAN_UPDATE = 256,
    GROUPS_CHANGED = 512,
    STYLE_CHANGED = 1024,
    TIDES_CHANGED = 2048,
    GL_CHANGED = 4096,
    REBUILD_RASTER_CACHE = 8192,
    CONFIG_CHANGED = 8192 * 2,
};

class zchxOptionsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit zchxOptionsDlg(QWidget *parent = 0);
    ~zchxOptionsDlg();
    void resetMarStdList(bool bsetConfig, bool bsetStd);

private:
    void processApply(bool apply);
    bool UpdateWorkArrayFromTextCtl(void);  //更新地图文件列表
    void resizeEvent(QResizeEvent* e);

private slots:
    void on_bOpenGL_clicked();

    void on_OK_clicked();

    void on_CANCEL_clicked();

    void on_APPLY_clicked();

    void on_addBtn_clicked();

    void on_m_removeBtn_clicked();

public:
    Ui::zchxOptionsDlg          *ui;
    ArrayOfCDI                  m_CurrentDirList;
    ArrayOfCDI                  *m_pWorkDirList;
    int                         m_returnChanges;
    std::vector<int>            marinersStdXref;

    int                         k_vectorcharts;
    int                         k_charts;
    int                         m_groups_changed;
};

#endif // ZCHXOPTIONSDLG_H
