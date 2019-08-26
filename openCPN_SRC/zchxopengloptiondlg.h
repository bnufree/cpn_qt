#ifndef ZCHXOPENGLOPTIONDLG_H
#define ZCHXOPENGLOPTIONDLG_H

#include <QDialog>

namespace Ui {
class zchxOpenGlOptionDlg;
}

class zchxMapMainWindow;
class glTextureManager;


class zchxOpenGlOptionDlg : public QDialog
{
    Q_OBJECT

public:
    explicit zchxOpenGlOptionDlg(zchxMapMainWindow* frame,  QWidget *parent = 0);
    ~zchxOpenGlOptionDlg();
    bool GetAcceleratedPanning(void) const;
    bool GetTextureCompression(void) const;
    bool GetPolygonSmoothing(void) const;
    bool GetLineSmoothing(void) const;
    bool GetShowFPS(void) const;
    bool GetSoftwareGL(void) const;
    bool GetTextureCompressionCaching(void) const;
    bool GetRebuildCache(void) const;
    int GetTextureMemorySize(void) const;
    QString GetTextureCacheSize(void) const ;
    void Populate(void) ;
    void setGLTextureManager(glTextureManager* mgr) {mGLTextureMgr = mgr;}
private:
    QString getCachePath() const;

private slots:
    void    OnButtonClear();
    void    OnButtonRebuild();

    void on_OK_clicked();

    void on_Cancel_clicked();

private:
    Ui::zchxOpenGlOptionDlg *ui;
    bool m_brebuild_cache;
    zchxMapMainWindow*          mMainWindow;
    glTextureManager*           mGLTextureMgr;
};

#endif // ZCHXOPENGLOPTIONDLG_H
