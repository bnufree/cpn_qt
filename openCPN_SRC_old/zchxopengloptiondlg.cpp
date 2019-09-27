#include "zchxopengloptiondlg.h"
#include "ui_zchxopengloptiondlg.h"
#include "_def.h"
#include <QDir>
#include "zchxmapmainwindow.h"
#include "zchxconfig.h"
#include "glTextureManager.h"
#include "GL/gl.h"
#include "OCPNPlatform.h"
#include <QDebug>


extern      zchxGLOptions                       g_GLOptions;
extern      bool                                g_bGlExpert;
/*extern*/      GLuint                              g_raster_format = GL_RGB;
extern      bool                                g_bShowFPS;
extern      zchxMapMainWindow                   *gFrame;
extern      glTextureManager                    *g_glTextureManager;

zchxOpenGlOptionDlg::zchxOpenGlOptionDlg(QWidget *parent) :
    QDialog(parent),
    m_brebuild_cache(false),
    ui(new Ui::zchxOpenGlOptionDlg)
{
    ui->setupUi(this);
//    setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    qDebug()<<"frame:"<<gFrame;

    if(g_bGlExpert)
    {
        ui->m_cbTextureCompression->setText(tr("Texture Compression"));
    } else
    {
        ui->m_cbTextureCompression->setText(tr("Texture Compression with Caching"));
    }
    ui->btnRebuild->setEnabled(g_GLOptions.m_bTextureCompressionCaching);
    if (g_raster_format == GL_RGB)
    {
        ui->btnRebuild->setEnabled(false);
    }
    ui->btnClear->setEnabled(g_GLOptions.m_bTextureCompressionCaching);
    ui->m_cacheSize->setText(QString("Size:%1").arg(GetTextureCacheSize()));
    Populate();
}

bool zchxOpenGlOptionDlg::GetAcceleratedPanning(void) const
{
  return ui->m_cbUseAcceleratedPanning->isChecked();
}

bool zchxOpenGlOptionDlg::GetTextureCompression(void) const
{
  return ui->m_cbTextureCompression->isChecked();
}

bool zchxOpenGlOptionDlg::GetPolygonSmoothing(void) const
{
    return ui->m_cbPolygonSmoothing->isChecked();
}

bool zchxOpenGlOptionDlg::GetLineSmoothing(void) const
{
    return ui->m_cbLineSmoothing->isChecked();
}

bool zchxOpenGlOptionDlg::GetShowFPS(void) const
{
    return ui->m_cbShowFPS->isChecked();
}

bool zchxOpenGlOptionDlg::GetSoftwareGL(void) const
{
    return ui->m_cbSoftwareGL->isChecked();
}

bool zchxOpenGlOptionDlg::GetTextureCompressionCaching(void) const
{
    return ui->m_cbTextureCompressionCaching->isChecked();
}

bool zchxOpenGlOptionDlg::GetRebuildCache(void) const
{
    return m_brebuild_cache;
}

int zchxOpenGlOptionDlg::GetTextureMemorySize(void) const
{
    return ui->m_sTextureMemorySize->value();
}

void zchxOpenGlOptionDlg::Populate(void)
{
    extern PFNGLCOMPRESSEDTEXIMAGE2DPROC s_glCompressedTexImage2D;
    extern bool b_glEntryPointsSet;
    ui->m_cbTextureCompression->setChecked(g_GLOptions.m_bTextureCompression);
    /* disable caching if unsupported */
    if (b_glEntryPointsSet && !s_glCompressedTexImage2D) {
        g_GLOptions.m_bTextureCompressionCaching = false;
        ui->m_cbTextureCompression->setEnabled(false);
        ui->m_cbTextureCompression->setChecked(false);
    }

    ui->m_cbTextureCompressionCaching->setVisible(g_bGlExpert);
    ui->m_memorySize->setVisible(g_bGlExpert);
    ui->m_sTextureMemorySize->setVisible(g_bGlExpert);
    if (g_bGlExpert)
    {
        ui->m_cbTextureCompressionCaching->setChecked(g_GLOptions.m_bTextureCompressionCaching);
        ui->m_sTextureMemorySize->setValue(g_GLOptions.m_iTextureMemorySize);
    }
    ui->m_cbShowFPS->setChecked(g_bShowFPS);
    ui->m_cbPolygonSmoothing->setChecked(g_GLOptions.m_GLPolygonSmoothing);
    ui->m_cbLineSmoothing->setChecked(g_GLOptions.m_GLLineSmoothing);
    ui->m_cbSoftwareGL->hide();

    if (g_bGlExpert) {
        if (/*mFrame && mFrame->GetPrimaryCanvas()->GetglCanvas()*/true){
            if(/*mFrame->GetPrimaryCanvas()->GetglCanvas()->CanAcceleratePanning()*/true) {
                ui->m_cbUseAcceleratedPanning->setEnabled(true);
                ui->m_cbUseAcceleratedPanning->setChecked(g_GLOptions.m_bUseAcceleratedPanning);
            } else {
                ui->m_cbUseAcceleratedPanning->setChecked(false);
                ui->m_cbUseAcceleratedPanning->setEnabled(false);
            }
        } else {
            ui->m_cbUseAcceleratedPanning->setChecked(g_GLOptions.m_bUseAcceleratedPanning);
        }
    } else {
        ui->m_cbUseAcceleratedPanning->setChecked(g_GLOptions.m_bUseAcceleratedPanning);
        ui->m_cbUseAcceleratedPanning->setEnabled(false);
    }
}

void zchxOpenGlOptionDlg::OnButtonRebuild()
{
    if (g_GLOptions.m_bTextureCompressionCaching)
    {
        m_brebuild_cache = true;
        on_Cancel_clicked();
    }
}

void zchxOpenGlOptionDlg::OnButtonClear()
{
  if (g_glTextureManager)
  {
      Qt::CursorShape old_shape = cursor().shape();
      //busy
      QCursor busy(Qt::BusyCursor);
      setCursor(busy);

      g_glTextureManager->ClearAllRasterTextures();
      QString path = getCachePath();
      QDir dir(path);
      if (dir.exists()) {
          QFileInfoList files = dir.entryInfoList();
          for (unsigned int i = 0; i < files.size(); i++)
          {
              QFile file(files[i].filePath());
              file.remove();
          }
      }
      ui->m_cacheSize->setText(QString("Size: %1").arg(GetTextureCacheSize()));
      QCursor old_cursor(old_shape);
      setCursor(old_cursor);
  }
}

QString zchxOpenGlOptionDlg::GetTextureCacheSize(void) const
{
    QString path = getCachePath();
    long long total = 0;
    QDir dir(path);
    if (dir.exists()) {
        QFileInfoList files = dir.entryInfoList();
        for (unsigned int i = 0; i < files.size(); i++)
            total += files[i].size();
    }
    double mb = total / (1024.0 * 1024.0);
    if (mb < 10000.0) return QString("").sprintf( "%.1f MB" , mb);
    mb = mb / 1024.0;
    return QString("").sprintf("%.1f GB" , mb);
}

QString zchxOpenGlOptionDlg::getCachePath() const
{
    return QString("%1/raster_texture_cache").arg(zchxFuncUtil::getDataDir());
}




zchxOpenGlOptionDlg::~zchxOpenGlOptionDlg()
{
    delete ui;
    qDebug()<<"!!";
}

void zchxOpenGlOptionDlg::on_OK_clicked()
{
    accept();
}

void zchxOpenGlOptionDlg::on_Cancel_clicked()
{
    reject();
}
