#include "zchxoptionsdlg.h"
#include "ui_zchxoptionsdlg.h"
#include "zchxchecklistwidget.h"
#include "zchxopengloptiondlg.h"
#include "OCPNPlatform.h"
#include "_def.h"
#include "s52plib.h"
#include "s52utils.h"
#include "cm93.h"
#include "zchxmapmainwindow.h"
#include "zchxconfig.h"
#include "zchxopengloptiondlg.h"
#include "glChartCanvas.h"
#include "chcanv.h"

extern  bool            g_bShowStatusBar;
extern  bool            g_bShowMenuBar;
extern  bool            g_bShowChartBar;
extern  double          g_display_size_mm;
extern  bool            g_bskew_comp;
extern  bool            g_bresponsive;
extern  bool            g_bAutoHideToolbar;
extern  int             g_nAutoHideToolbar;
extern  bool            g_bsmoothpanzoom;
extern  int             g_COGAvgSec;
extern  bool            g_bShowTrue;
extern  bool            g_bShowMag;
extern  int             g_iSDMMFormat;
extern  int             g_iDistanceFormat;
extern  int             g_iSpeedFormat;
extern  bool            g_bEnableZoomToCursor;
extern  int             g_chart_zoom_modifier;
extern  int             g_chart_zoom_modifier_vector;
extern  int             g_GUIScaleFactor;
extern  int             g_ChartScaleFactor;
extern  int             g_ShipScaleFactor;
extern  float           g_ChartScaleFactorExp;
extern  float           g_ShipScaleFactorExp;
extern  int             g_cm93_zoom_factor;
extern  int             g_nDepthUnitDisplay;
extern s52plib          *ps52plib;
extern  zchxMapMainWindow   *gFrame;
extern  zchxGLOptions    g_GLOptions;

extern void LoadS57();

zchxOptionsDlg::zchxOptionsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zchxOptionsDlg),
    m_pWorkDirList(new ArrayOfCDI)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    ui->pPointStyle->setItemData(0, PAPER_CHART);
    ui->pPointStyle->setItemData(1, SIMPLIFIED);
    ui->pBoundStyle->setItemData(0, PLAIN_BOUNDARIES);
    ui->pBoundStyle->setItemData(1, SYMBOLIZED_BOUNDARIES);

    //显示模式赋值
    ui->pDispCat->setItemData(0, DISPLAYBASE);
    ui->pDispCat->setItemData(1, STANDARD);
    ui->pDispCat->setItemData(2, OTHER);
    ui->pDispCat->setItemData(3, MARINERS_STANDARD);

    //获取当前指定的数据目录
    ui->pActiveChartsList->setColumnCount(1);
    ui->pActiveChartsList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ArrayOfCDI ChartDirArray;
    ZCHX_CFG_INS->LoadChartDirArray( ChartDirArray );
    ui->pActiveChartsList->setRowCount(ChartDirArray.size());
    for(int i=0; i<ChartDirArray.size(); i++)
    {
        m_CurrentDirList.append(ChartDirArray[i]);
        QString dir = ChartDirArray[i].fullpath;
        QTableWidgetItem* item = ui->pActiveChartsList->item(i, 0);
        if(!item)
        {
            item = new QTableWidgetItem(dir);
            ui->pActiveChartsList->setItem(i, 0, item);
        } else
        {
            item->setText(dir);
        }
        item->setData(Qt::UserRole, QVariant::fromValue(ChartDirArray[i]));
    }
    ui->pOpenGL->setChecked(true);

    //更新S52显示的obj对象
    if(!ps52plib) LoadS57();
    resetMarStdList(true, false);

}

zchxOptionsDlg::~zchxOptionsDlg()
{
    delete ui;
}

extern bool g_bGLexpert;
extern bool g_bShowFPS;
extern bool g_bSoftwareGL;

void zchxOptionsDlg::on_bOpenGL_clicked()
{
    zchxOpenGlOptionDlg *dlg = new zchxOpenGlOptionDlg;

    if (dlg->exec() == QDialog::Accepted) {
#if 0
        if(gFrame->GetPrimaryCanvas()->GetglCanvas()){
            g_GLOptions.m_bUseAcceleratedPanning =
                    g_bGLexpert ? dlg.GetAcceleratedPanning()
                                : gFrame->GetPrimaryCanvas()->GetglCanvas()->CanAcceleratePanning();
        }
#endif

        g_bShowFPS = dlg->GetShowFPS();
        g_bSoftwareGL = dlg->GetSoftwareGL();

        g_GLOptions.m_GLPolygonSmoothing = dlg->GetPolygonSmoothing();
        g_GLOptions.m_GLLineSmoothing = dlg->GetLineSmoothing();

        if (g_bGLexpert) {
            // user defined
            g_GLOptions.m_bTextureCompressionCaching =
                    dlg->GetTextureCompressionCaching();
            g_GLOptions.m_iTextureMemorySize = dlg->GetTextureMemorySize();
        } else {
            // caching is on if textures are compressed
            g_GLOptions.m_bTextureCompressionCaching = dlg->GetTextureCompression();
        }
#if 0
        if (g_glTextureManager && g_GLOptions.m_bTextureCompression != dlg.GetTextureCompression()) {
            // new g_GLoptions setting is needed in callees
            g_GLOptions.m_bTextureCompression = dlg.GetTextureCompression();

            if(gFrame->GetPrimaryCanvas()->GetglCanvas()){
                Qt::CursorShape old = cursor().shape();
                setCursor(Qt::BusyCursor);
                gFrame->GetPrimaryCanvas()->GetglCanvas()->SetupCompression();
                g_glTextureManager->ClearAllRasterTextures();
                setCursor(old);
            }
        }
        else
            g_GLOptions.m_bTextureCompression = dlg.GetTextureCompression();
#endif

    }

    if (dlg->GetRebuildCache()) {
        m_returnChanges = REBUILD_RASTER_CACHE;
    }

    delete dlg;
}

void zchxOptionsDlg::on_OK_clicked()
{
    hide();
    processApply(false);

    close();

}

void zchxOptionsDlg::on_CANCEL_clicked()
{
    close();
}

void zchxOptionsDlg::on_APPLY_clicked()
{
    processApply(true);
}

void zchxOptionsDlg::processApply(bool apply)
{
    m_returnChanges = 0;
    Qt::CursorShape cursor_type = cursor().shape();
    setCursor(QCursor(Qt::BusyCursor));

    // Handle Chart Tab
    UpdateWorkArrayFromTextCtl();
    //检测数据目录是否发生了变化
    ArrayOfCDI old_chart_list;
    ZCHX_CFG_INS->LoadChartDirArray( old_chart_list );
    if(m_pWorkDirList->size() == 0)
    {
        m_returnChanges |= CHANGE_CHARTS;
    } else
    {
        //如果两次的目录完全一样
        if(m_pWorkDirList->size() != old_chart_list.size())
        {
            m_returnChanges |= CHANGE_CHARTS;
        } else
        {
            bool found = true;
            for(int i=0; i<m_pWorkDirList->size(); i++)
            {
                ChartDirInfo info = m_pWorkDirList->at(i);
                if(old_chart_list.indexOf(info) < 0)
                {
                    found = false;
                    break;
                }
            }
            if(!found)
            {
                m_returnChanges |= CHANGE_CHARTS;
            }
        }
    }

    int k_force = FORCE_UPDATE;
    if (!ui->pUpdateCheckBox->isChecked()) k_force = 0;
    ui->pUpdateCheckBox->setEnabled(true);
    ui->pUpdateCheckBox->setChecked(false);
    m_returnChanges |= k_force;

    int k_scan = SCAN_UPDATE;
    if (!ui->pScanCheckBox->isChecked()) k_scan = 0;
    ui->pScanCheckBox->setEnabled(true);
    ui->pScanCheckBox->setChecked(false);
    m_returnChanges |= k_scan;

    g_bShowStatusBar = ui->pShowStatusBar->isChecked();
    g_bShowMenuBar = ui->pShowMenuBar->isChecked();
    g_bShowChartBar = ui->pShowChartBar->isChecked();


    g_bskew_comp = ui->pSkewComp->isChecked();
    g_bresponsive = ui->pResponsive->isChecked();

    g_bAutoHideToolbar = ui->pToolbarAutoHideCB->isChecked();
    int hide_val = ui->pToolbarHideSecs->text().toInt();
    g_nAutoHideToolbar = qMin(hide_val, 100);
    g_nAutoHideToolbar = qMax(g_nAutoHideToolbar, 2);
    g_bsmoothpanzoom = ui->pSmoothPanZoom->isChecked();
    int update_val = ui->pCOGUPUpdateSecs->text().toInt();
    g_COGAvgSec = qMin(update_val, MAX_COG_AVERAGE_SECONDS);

    g_bShowTrue = ui->pCBTrueShow->isChecked();
    g_bShowMag = ui->pCBMagShow->isChecked();

    g_iSDMMFormat = ui->pSDMMFormat->currentData().toInt();
    g_iDistanceFormat = ui->pDistanceFormat->currentData().toInt();
    g_iSpeedFormat = ui->pSpeedFormat->currentData().toInt();
    g_bEnableZoomToCursor = ui->pEnableZoomToCursor->isChecked();

    g_chart_zoom_modifier = ui->m_pSlider_Zoom->value();
    g_chart_zoom_modifier_vector = ui->m_pSlider_Zoom_Vector->value();
    g_GUIScaleFactor = ui->m_pSlider_GUI_Factor->value();
    g_ChartScaleFactor = ui->m_pSlider_Chart_Factor->value();
    g_ChartScaleFactorExp = zchxFuncUtil::getChartScaleFactorExp(g_ChartScaleFactor);
    g_ShipScaleFactor = ui->m_pSlider_Ship_Factor->value();
    g_ShipScaleFactorExp = zchxFuncUtil::getChartScaleFactorExp(g_ShipScaleFactor);

    g_cm93_zoom_factor = ui->m_pSlider_CM93_Zoom->value();
    g_nDepthUnitDisplay = ui->pDepthUnitSelect->currentData().toInt();

    //  Process the UserStandard display list, noting if any changes were made
    bool bUserStdChange = false;
    int nOBJL = ui->ps57CtlListBox->count();
    for (int iPtr = 0; iPtr < nOBJL; iPtr++) {
        int itemIndex = -1;
        for (size_t i = 0; i < marinersStdXref.size(); i++) {
            if (marinersStdXref[i] == iPtr) {
                itemIndex = i;
                break;
            }
        }
        Q_ASSERT(itemIndex >= 0);
        OBJLElement* pOLE = (OBJLElement*)(ps52plib->pOBJLArray->at(itemIndex));
        if(pOLE->nViz != ui->ps57CtlListBox->ischecked(iPtr))bUserStdChange = true;
        pOLE->nViz = ui->ps57CtlListBox->ischecked(iPtr);
    }

    if (ps52plib)
    {
        // Take a snapshot of the S52 config right now,
        // for later comparison
        ps52plib->GenerateStateHash();
        long stateHash = ps52plib->GetStateHash();

        if (m_returnChanges & GL_CHANGED) {
            // Do this now to handle the screen refresh that is automatically
            // generated on Windows at closure of the options dialog...
            ps52plib->FlushSymbolCaches();
            // some CNSY depends on renderer (e.g. CARC)
            ps52plib->ClearCNSYLUPArray();
            ps52plib->GenerateStateHash();
        }

        if(ui->pDispCat) {
            enum _DisCat nset = (_DisCat)(ui->pDispCat->currentData().toInt());
            ps52plib->SetDisplayCategory(nset);
        }

        ps52plib->m_bShowSoundg = ui->pCheck_SOUNDG->isChecked();
        ps52plib->m_bShowAtonText = ui->pCheck_ATONTEXT->isChecked();
        ps52plib->m_bShowLdisText = ui->pCheck_LDISTEXT->isChecked();
        ps52plib->m_bExtendLightSectors = ui->pCheck_XLSECTTEXT->isChecked();

        ps52plib->m_bShowMeta = ui->pCheck_META->isChecked();
        ps52plib->m_bDeClutterText = ui->pCheck_DECLTEXT->isChecked();
        ps52plib->m_bShowNationalTexts = ui->pCheck_NATIONALTEXT->isChecked();
        ps52plib->m_bShowS57ImportantTextOnly = ui->pCheck_SHOWIMPTEXT->isChecked();
        ps52plib->m_bUseSCAMIN = ui->pCheck_SCAMIN->isChecked();

        ps52plib->m_nSymbolStyle = (LUPname)(ui->pPointStyle->currentData().toInt());
        ps52plib->m_nBoundaryStyle = (LUPname)(ui->pBoundStyle->currentData().toInt());
        S52_setMarinerParam(S52_MAR_TWO_SHADES, (ui->p24Color->currentIndex() == 0) ? 1.0 : 0.0);

        // Depths
        float conv = 1;

        if (g_nDepthUnitDisplay == 0)  // feet
        {
            conv = 0.3048f;    // international definiton of 1 foot is 0.3048 metres
        } else if (g_nDepthUnitDisplay == 2)  // fathoms
        {
            conv = 0.3048f * 6;     // 1 fathom is 6 feet
        }

        double dval = ui->m_SafetyCtl->text().toDouble();
        S52_setMarinerParam(S52_MAR_SAFETY_DEPTH, dval * conv);  // controls sounding display
        S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR, dval * conv);  // controls colour

        dval = ui->m_ShallowCtl->text().toDouble();
        S52_setMarinerParam(S52_MAR_SHALLOW_CONTOUR, dval * conv);

        dval = ui->m_DeepCtl->text().toDouble();
        S52_setMarinerParam(S52_MAR_DEEP_CONTOUR, dval * conv);

        ps52plib->UpdateMarinerParams();
        ps52plib->m_nDepthUnitDisplay = g_nDepthUnitDisplay;

        ps52plib->GenerateStateHash();

        // Detect a change to S52 library config
        if( (stateHash != ps52plib->GetStateHash()) || bUserStdChange )
        {
            m_returnChanges |= S52_CHANGED;
        }
    }


    m_returnChanges |= GENERIC_CHANGED | VISIT_CHARTS/* | k_vectorcharts | k_charts | m_groups_changed */;

    if (/*apply &&*/ gFrame) {
        gFrame->ProcessOptionsDialog(m_returnChanges, m_pWorkDirList);
        m_CurrentDirList = *m_pWorkDirList; // Perform a deep copy back to main database.

        //  We can clear a few flag bits on "Apply", so they won't be recognised at the "OK" click.
        //  Their actions have already been accomplished once...
        m_returnChanges &= ~( CHANGE_CHARTS | FORCE_UPDATE | SCAN_UPDATE );
        k_charts = 0;

        gFrame->RefreshAllCanvas();
    }
    setCursor(QCursor(cursor_type));
}

void zchxOptionsDlg::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
}


bool zchxOptionsDlg::UpdateWorkArrayFromTextCtl(void)
{
    if(!m_pWorkDirList) return false;
    bool chart_change = false;
    int n = ui->pActiveChartsList->rowCount();
    m_pWorkDirList->clear();
    for (int i = 0; i < n; i++)
    {
        QTableWidgetItem* item = ui->pActiveChartsList->item(i, 0);
        if(!item) continue;
        QString dirname = item->text().trimmed();
        if(dirname.isEmpty()) continue;
        while (dirname.right(1) == "\n" || dirname.right(1) == "\r") dirname.remove(dirname.size()-1, 1);

        //    scan the current array to find a match
        //    if found, add the info to the work list, preserving the magic
        //    number
        //    If not found, make a new ChartDirInfo, and add it
        int index = m_CurrentDirList.indexOf(dirname);
        if(index >= 0)
        {
            m_pWorkDirList->append(m_CurrentDirList[index]);
        } else
        {
            m_pWorkDirList->append(ChartDirInfo(dirname));
            chart_change = true;
        }
    }
    return chart_change;
}

void zchxOptionsDlg::resetMarStdList(bool bsetConfig, bool bsetStd)
{
    ui->ps57CtlListBox->clear();
    marinersStdXref.clear();
    for (unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->count(); iPtr++)
    {
        OBJLElement* pOLE = (OBJLElement*)(ps52plib->pOBJLArray->at(iPtr));
        if(!pOLE) continue;
        if( !strncmp(pOLE->OBJLName, "CBLARE", 6))
            int yyp = 3;

        QString item = QString::fromUtf8(pOLE->OBJLName);
        if (iPtr < ps52plib->OBJLDescriptions.size())
        {
            item = ps52plib->OBJLDescriptions[iPtr];
        }

        // Find the most conservative Category, among Point, Area, and Line LUPs
        DisCat cat = OTHER;
        DisCat catp = ps52plib->findLUPDisCat(pOLE->OBJLName, SIMPLIFIED);
        DisCat cata = ps52plib->findLUPDisCat(pOLE->OBJLName, PLAIN_BOUNDARIES);
        DisCat catl = ps52plib->findLUPDisCat(pOLE->OBJLName, LINES);

        if((catp == DISPLAYBASE) || (cata == DISPLAYBASE) || (catl== DISPLAYBASE) )
            cat = DISPLAYBASE;
        else if((catp == STANDARD) || (cata == STANDARD) || (catl== STANDARD) )
            cat = STANDARD;

        bool benable = true;
        if(cat > 0) benable = (cat != DISPLAYBASE);

        // The ListBox control will insert entries in sorted order, which means
        // we need to
        // keep track of already inserted items that gets pushed down the line.
        int newpos = ui->ps57CtlListBox->append(item, benable);
        marinersStdXref.push_back(newpos);
        for (size_t i = 0; i < iPtr; i++) {
            if (marinersStdXref[i] >= newpos) marinersStdXref[i]++;
        }

        bool bviz = 0;
        if(bsetConfig) bviz = !(pOLE->nViz == 0);

        if(cat == DISPLAYBASE) bviz = true;

        if(bsetStd && cat == STANDARD) bviz = true;
        ui->ps57CtlListBox->setChecked(newpos, bviz);
    }
}


void zchxOptionsDlg::on_addBtn_clicked()
{
    QString path = QFileDialog::getExistingDirectory(0, "Select a chart directory");
    if(path.size() > 0)
    {
        ui->pActiveChartsList->insertRow(ui->pActiveChartsList->rowCount());
        QTableWidgetItem *item = new QTableWidgetItem(path);
        ChartDirInfo info;
        info.fullpath = path;
        item->setData(Qt::UserRole, QVariant::fromValue(info));
        ui->pActiveChartsList->setItem(ui->pActiveChartsList->rowCount()-1, 0, item);
    }
}

void zchxOptionsDlg::on_m_removeBtn_clicked()
{
    if(ui->pActiveChartsList->selectedItems().size() == 0) return;
    QTableWidgetItem *item = ui->pActiveChartsList->selectedItems().first();
    m_CurrentDirList.removeOne(item->data(Qt::UserRole).value<ChartDirInfo>());
    int row = item->row();
    ui->pActiveChartsList->removeRow(row);
}
