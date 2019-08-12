#include "zchxoptionsdlg.h"
#include "ui_zchxoptionsdlg.h"
#include "zchxchecklistwidget.h"
#include "zchxopengloptiondlg.h"
#include "OCPNPlatform.h"
#include "_def.h"
#include "s52plib.h"
#include "s52utils.h"
#include "cm93.h"

extern  bool            g_bShowStatusBar;
extern  bool            g_bShowMenuBar;
extern  bool            g_bShowCompassWin;
extern  bool            g_bShowChartBar;
extern  double          g_display_size_mm;
extern  double          g_config_display_size_mm;
extern  bool            g_config_display_size_manual;
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
extern  bool            g_bopengl;
extern  int             g_cm93_zoom_factor;
extern  int             g_nDepthUnitDisplay;
extern OCPNPlatform     *g_Platform;
extern s52plib          *ps52plib;

zchxOptionsDlg::zchxOptionsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zchxOptionsDlg),
    m_pWorkDirList(0)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    ui->pPointStyle->setItemData(0, PAPER_CHART);
    ui->pPointStyle->setItemData(1, SIMPLIFIED);
}

zchxOptionsDlg::~zchxOptionsDlg()
{
    delete ui;
}



void zchxOptionsDlg::on_bOpenGL_clicked()
{

}

void zchxOptionsDlg::on_OK_clicked()
{
    on_APPLY_clicked();
    accept();
}

void zchxOptionsDlg::on_CANCEL_clicked()
{
    reject();
}

void zchxOptionsDlg::on_APPLY_clicked()
{
    setCursor(QCursor(Qt::BusyCursor));

    // Handle Chart Tab
    UpdateWorkArrayFromTextCtl();
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
    g_bShowCompassWin = ui->pShowCompassWin->isChecked();
    g_bShowChartBar = ui->pShowChartBar->isChecked();

    QString screenmm = ui->pScreenMM->text().trimmed();
    long mm = screenmm.toLongLong();
    g_config_display_size_mm = mm > 0 ? mm : -1;
    g_config_display_size_manual = ui->pRBSizeManual->isChecked();
    ;

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
    g_ChartScaleFactorExp = g_Platform->getChartScaleFactorExp(g_ChartScaleFactor);
    g_ShipScaleFactor = ui->m_pSlider_Ship_Factor->value();
    g_ShipScaleFactorExp = g_Platform->getChartScaleFactorExp(g_ShipScaleFactor);
    if (g_bopengl != ui->pOpenGL->isChecked()) m_returnChanges |= GL_CHANGED;
    g_bopengl = ui->pOpenGL->isChecked();

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
        OBJLElement* pOLE = (OBJLElement*)(ps52plib->pOBJLArray->Item(itemIndex));
        if(pOLE->nViz != ps57CtlListBox->IsChecked(iPtr))bUserStdChange = true;
        pOLE->nViz = ps57CtlListBox->IsChecked(iPtr);
    }

    if (ps52plib) {

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

        if(pDispCat) {
            enum _DisCat nset = OTHER;
            switch (pDispCat->GetSelection()) {
            case 0:
                nset = DISPLAYBASE;
                break;
            case 1:
                nset = STANDARD;
                break;
            case 2:
                nset = OTHER;
                break;
            case 3:
                nset = MARINERS_STANDARD;
                break;
            }
            ps52plib->SetDisplayCategory(nset);
        }

        ps52plib->m_bShowSoundg = ui->pCheck_SOUNDG->isChecked();
        ps52plib->m_bShowAtonText = ui->pCheck_ATONTEXT->isChecked();
        ps52plib->m_bShowLdisText = ui->pCheck_LDISTEXT->GetValue();
        ps52plib->m_bExtendLightSectors = ui->pCheck_XLSECTTEXT->GetValue();

        ps52plib->m_bShowMeta = ui->pCheck_META->GetValue();
        ps52plib->m_bDeClutterText = ui->pCheck_DECLTEXT->GetValue();
        ps52plib->m_bShowNationalTexts = ui->pCheck_NATIONALTEXT->GetValue();
        ps52plib->m_bShowS57ImportantTextOnly = ui->pCheck_SHOWIMPTEXT->GetValue();
        ps52plib->m_bUseSCAMIN = ui->pCheck_SCAMIN->GetValue();

        ps52plib->m_nSymbolStyle = ui->pPointStyle->GetSelection() == 0 ? PAPER_CHART : SIMPLIFIED;

        ps52plib->m_nBoundaryStyle = pBoundStyle->GetSelection() == 0
                                         ? PLAIN_BOUNDARIES
                                         : SYMBOLIZED_BOUNDARIES;

        S52_setMarinerParam(S52_MAR_TWO_SHADES, (p24Color->GetSelection() == 0) ? 1.0 : 0.0);

        // Depths
        double dval;
        float conv = 1;

        if (depthUnit == 0)  // feet
          conv = 0.3048f;    // international definiton of 1 foot is 0.3048 metres
        else if (depthUnit == 2)  // fathoms
          conv = 0.3048f * 6;     // 1 fathom is 6 feet

        if (m_SafetyCtl->GetValue().ToDouble(&dval)) {
          S52_setMarinerParam(S52_MAR_SAFETY_DEPTH, dval * conv);  // controls sounding display
          S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR, dval * conv);  // controls colour
        }

        if (m_ShallowCtl->GetValue().ToDouble(&dval))
          S52_setMarinerParam(S52_MAR_SHALLOW_CONTOUR, dval * conv);

        if (m_DeepCtl->GetValue().ToDouble(&dval))
          S52_setMarinerParam(S52_MAR_DEEP_CONTOUR, dval * conv);

        ps52plib->UpdateMarinerParams();
        ps52plib->m_nDepthUnitDisplay = depthUnit;

        ps52plib->GenerateStateHash();

        // Detect a change to S52 library config
        if( (stateHash != ps52plib->GetStateHash()) || bUserStdChange )
            m_returnChanges |= S52_CHANGED;

      }


        if ( g_bInlandEcdis != pInlandEcdis->GetValue() ){ // InlandEcdis changed
            g_bInlandEcdis = pInlandEcdis->GetValue();
            SwitchInlandEcdisMode( g_bInlandEcdis );
            m_returnChanges |= TOOLBAR_CHANGED;
        }
    #ifdef __WXOSX__
        if ( g_bDarkDecorations != pDarkDecorations->GetValue() ) {
            g_bDarkDecorations = pDarkDecorations->GetValue();

            OCPNMessageBox(this,
                           _("The changes to the window decorations will take full effect the next time you start OpenCPN."),
                           _("OpenCPN"));
        }
    #endif
      // PlugIn Manager Panel

      // Pick up any changes to selections
      if (g_pi_manager->UpdatePlugIns())
          m_returnChanges |= TOOLBAR_CHANGED;

      // And keep config in sync
      if (m_pPlugInCtrl) m_pPlugInCtrl->UpdatePluginsOrder();
      g_pi_manager->UpdateConfig();

      // PlugIns may have added panels
      if (g_pi_manager) g_pi_manager->CloseAllPlugInPanels((int)wxOK);

      m_returnChanges |= GENERIC_CHANGED | k_vectorcharts | k_charts |
                         m_groups_changed | k_plugins | k_tides;

      // Pick up all the entries in the DataSelected control
      // and update the global static array
      TideCurrentDataSet.Clear();
      int nEntry = tcDataSelected->GetCount();

      for (int i = 0; i < nEntry; i++)
        TideCurrentDataSet.Add(tcDataSelected->GetString(i));

      // Canvas configuration
      if (event.GetId() != ID_APPLY)                // only on ID_OK
        g_canvasConfig = m_screenConfig;

      if (event.GetId() == ID_APPLY) {
        gFrame->ProcessOptionsDialog(m_returnChanges, m_pWorkDirList);
        m_CurrentDirList = *m_pWorkDirList; // Perform a deep copy back to main database.

        //  We can clear a few flag bits on "Apply", so they won't be recognised at the "OK" click.
        //  Their actions have already been accomplished once...
        m_returnChanges &= ~( CHANGE_CHARTS | FORCE_UPDATE | SCAN_UPDATE );
        k_charts = 0;

        gFrame->RefreshAllCanvas();
      }


      //  Record notice of any changes to last applied template
      UpdateTemplateTitleText();

      ::wxEndBusyCursor();
    }
}


void zchxOptionsDlg::UpdateWorkArrayFromTextCtl(void)
{
    if(!m_pWorkDirList) return;
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
        }
    }
}

void zchxOptionsDlg::resetMarStdList(bool bsetConfig, bool bsetStd)
{
    ui->ps57CtlListBox->clear();
    marinersStdXref.clear();

    for (unsigned int i = 0; i < ps52plib->pOBJLArray->count(); i++)
    {
        OBJLElement* pOLE = (OBJLElement*)(ps52plib->pOBJLArray[i]);
        if(!pOLE) continue;
        if( !strncmp(pOLE->OBJLName, "CBLARE", 6))
            int yyp = 3;

        QString item = QString::fromUtf8(pOLE->OBJLName);
        if (i < ps52plib->OBJLDescriptions.size())
        {
            item = ps52plib->OBJLDescriptions[];
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
        int newpos = ps57CtlListBox->Append(item, benable);
        marinersStdXref.push_back(newpos);
        for (size_t i = 0; i < iPtr; i++) {
            if (marinersStdXref[i] >= newpos) marinersStdXref[i]++;
        }

        bool bviz = 0;
        if(bsetConfig)
            bviz = !(pOLE->nViz == 0);

        if(cat == DISPLAYBASE)
            bviz = true;

        if(bsetStd){
            if(cat == STANDARD){
                bviz = true;
            }
        }

        ps57CtlListBox->Check(newpos, bviz);
    }
    //  Force the wxScrolledWindow to recalculate its scroll bars
    wxSize s = ps57CtlListBox->GetSize();
    ps57CtlListBox->SetSize(s.x, s.y-1);
}

