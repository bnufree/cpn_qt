#include "zchxoptionsdlg.h"
#include "ui_zchxoptionsdlg.h"
#include "zchxchecklistwidget.h"
#include "zchxopengloptiondlg.h"

extern  bool            g_bShowStatusBar;
extern  bool            g_bShowMenuBar;
extern  bool            g_bShowCompassWin;
extern  bool            g_bShowChartBar;
extern  double          g_display_size_mm;
extern  double          g_config_display_size_mm;
extern  bool            g_config_display_size_manual;

zchxOptionsDlg::zchxOptionsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zchxOptionsDlg),
    m_pWorkDirList(0)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
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

      // Connections page.
      g_bfilter_cogsog = m_cbFilterSogCog->GetValue();

      long filter_val = 1;
      m_tFilterSec->GetValue().ToLong(&filter_val);
      g_COGFilterSec = wxMin(static_cast<int>(filter_val), MAX_COGSOG_FILTER_SECONDS);
      g_COGFilterSec = wxMax(g_COGFilterSec, 1);
      g_SOGFilterSec = g_COGFilterSec;

      g_bMagneticAPB = m_cbAPBMagnetic->GetValue();
      g_NMEAAPBPrecision = m_choicePrecision->GetCurrentSelection();

      // NMEA Source
      //  If the stream selected exists, capture some of its existing parameters
      //  to facility identification and allow stop and restart of the stream
      wxString lastAddr;
      int lastPort = 0;
      NetworkProtocol lastNetProtocol = PROTO_UNDEFINED;

      if (mSelectedConnection) {
        ConnectionParams* cpo = mSelectedConnection;
        lastAddr = cpo->NetworkAddress;
        lastPort = cpo->NetworkPort;
        lastNetProtocol = cpo->NetProtocol;
      }

      if (!connectionsaved) {
        size_t nCurrentPanelCount = g_pConnectionParams->GetCount();
        ConnectionParams *cp = NULL;
        int old_priority = -1;
        {
          if (mSelectedConnection) {
            cp =  mSelectedConnection;
            old_priority = cp->Priority;
            UpdateConnectionParamsFromSelectedItem( cp );
            cp->b_IsSetup = false;

            //delete g_pConnectionParams->Item(itemIndex)->m_optionsPanel;
            //old_priority = g_pConnectionParams->Item(itemIndex)->Priority;
            //g_pConnectionParams->RemoveAt(itemIndex);
            //g_pConnectionParams->Insert(cp, itemIndex);
            //mSelectedConnection = cp;
            //cp->m_optionsPanel->SetSelected( true );
          } else {
            cp = CreateConnectionParamsFromSelectedItem();
            if(cp)
                g_pConnectionParams->Add(cp);
          }

          //  Record the previous parameters, if any
          if(cp){
            cp->LastNetProtocol = lastNetProtocol;
            cp->LastNetworkAddress = lastAddr;
            cp->LastNetworkPort = lastPort;
          }

          if(g_pConnectionParams->GetCount() != nCurrentPanelCount)
            FillSourceList();
          else if(old_priority >= 0){
              if(old_priority != cp->Priority)             // need resort
                 UpdateSourceList( true );
              else
                 UpdateSourceList( false );
          }




          connectionsaved = TRUE;
        }
    //     else {
    //       ::wxEndBusyCursor();
    //       if (m_bNMEAParams_shown) event.SetInt(wxID_STOP);
    //     }

        SetSelectedConnectionPanel( nullptr );

      }

      // Recreate datastreams that are new, or have been edited
      for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
        ConnectionParams* cp = g_pConnectionParams->Item(i);

        // Stream is new, or edited
        if (cp->b_IsSetup) continue;
        // Terminate and remove any existing stream with the same port name
        DataStream* pds_existing = g_pMUX->FindStream(cp->GetDSPort());
        if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);

        //  Try to stop any previous stream to avoid orphans
        pds_existing = g_pMUX->FindStream(cp->GetLastDSPort());
        if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);

        //  This for Bluetooth, which has strange parameters
        if(cp->Type == INTERNAL_BT){
            pds_existing = g_pMUX->FindStream(cp->GetPortStr());
            if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);
        }

        if (!cp->bEnabled) continue;
        dsPortType port_type = cp->IOSelect;
        DataStream* dstr = new DataStream(g_pMUX, cp->Type, cp->GetDSPort(),
                                          wxString::Format(wxT("%i"), cp->Baudrate),
                                          port_type, cp->Priority, cp->Garmin);
        dstr->SetInputFilter(cp->InputSentenceList);
        dstr->SetInputFilterType(cp->InputSentenceListType);
        dstr->SetOutputFilter(cp->OutputSentenceList);
        dstr->SetOutputFilterType(cp->OutputSentenceListType);
        dstr->SetChecksumCheck(cp->ChecksumCheck);
        g_pMUX->AddStream(dstr);

        cp->b_IsSetup = TRUE;
      }

      g_bGarminHostUpload = m_cbGarminUploadHost->GetValue();
      g_GPS_Ident = m_cbFurunoGP3X->GetValue() ? _T( "FurunoGP3X" ) : _T( "Generic" );

      // End of Connections page
      if(pCDOOutlines) g_bShowOutlines = pCDOOutlines->GetValue();
      if(pSDisplayGrid) g_bDisplayGrid = pSDisplayGrid->GetValue();

      if(pCDOQuilting){
        bool temp_bquilting = pCDOQuilting->GetValue();
    //   if (!g_bQuiltEnable && temp_bquilting)
    //     cc1->ReloadVP(); /* compose the quilt */
        g_bQuiltEnable = temp_bquilting;
      }
    //  g_bFullScreenQuilt = !pFullScreenQuilt->GetValue();

      if(pSDepthUnits) g_bShowDepthUnits = pSDepthUnits->GetValue();
      g_bskew_comp = pSkewComp->GetValue();
      g_btouch = pMobile->GetValue();
      g_bresponsive = pResponsive->GetValue();

      g_bAutoHideToolbar = pToolbarAutoHideCB->GetValue();

      long hide_val = 10;
      pToolbarHideSecs->GetValue().ToLong(&hide_val);
      g_nAutoHideToolbar = wxMin(static_cast<int>(hide_val), 100);
      g_nAutoHideToolbar = wxMax(g_nAutoHideToolbar, 2);

      //g_fog_overzoom = !pOverzoomEmphasis->GetValue();
      //g_oz_vector_scale = !pOZScaleVector->GetValue();

      if(pSmoothPanZoom) g_bsmoothpanzoom = pSmoothPanZoom->GetValue();

      long update_val = 1;
      pCOGUPUpdateSecs->GetValue().ToLong(&update_val);
      g_COGAvgSec = wxMin(static_cast<int>(update_val), MAX_COG_AVERAGE_SECONDS);

      //TODO if (g_bCourseUp != pCBCourseUp->GetValue()) gFrame->ToggleCourseUp();

      if(pCBLookAhead) g_bLookAhead = pCBLookAhead->GetValue();

      g_bShowTrue = pCBTrueShow->GetValue();
      g_bShowMag = pCBMagShow->GetValue();

      b_haveWMM = g_pi_manager && g_pi_manager->IsPlugInAvailable(_T("WMM"));
      if(!b_haveWMM  && !b_oldhaveWMM)
        pMagVar->GetValue().ToDouble(&g_UserVar);

      m_pText_OSCOG_Predictor->GetValue().ToDouble(&g_ownship_predictor_minutes);
      m_pText_OSHDT_Predictor->GetValue().ToDouble(&g_ownship_HDTpredictor_miles);

      double temp_dbl;
      g_iNavAidRadarRingsNumberVisible =
          pNavAidRadarRingsNumberVisible->GetSelection();
      if (pNavAidRadarRingsStep->GetValue().ToDouble(&temp_dbl))
        g_fNavAidRadarRingsStep = temp_dbl;
      g_pNavAidRadarRingsStepUnits = m_itemRadarRingsUnits->GetSelection();
      g_iWaypointRangeRingsNumber = pWaypointRangeRingsNumber->GetSelection();
      if (pWaypointRangeRingsStep->GetValue().ToDouble(&temp_dbl))
        g_fWaypointRangeRingsStep = temp_dbl;
      g_iWaypointRangeRingsStepUnits = m_itemWaypointRangeRingsUnits->GetSelection();
      g_colourWaypointRangeRingsColour =  m_colourWaypointRangeRingsColour->GetColour();
       g_colourWaypointRangeRingsColour =
           wxColour(g_colourWaypointRangeRingsColour.Red(), g_colourWaypointRangeRingsColour.Green(), g_colourWaypointRangeRingsColour.Blue());
      g_bWayPointPreventDragging = pWayPointPreventDragging->GetValue();
      g_own_ship_sog_cog_calc = pSogCogFromLLCheckBox->GetValue();
      g_own_ship_sog_cog_calc_damp_sec = pSogCogFromLLDampInterval->GetValue();

      g_bConfirmObjectDelete = pConfirmObjectDeletion->GetValue();

      if(pPreserveScale) g_bPreserveScaleOnX = pPreserveScale->GetValue();

      if ( g_bUIexpert && pCmdSoundString) {
          g_CmdSoundString = pCmdSoundString->GetValue( );
          if ( wxIsEmpty( g_CmdSoundString ) ) {
              g_CmdSoundString = wxString( SYSTEM_SOUND_CMD );
              pCmdSoundString->SetValue( g_CmdSoundString );
          }
      }

      g_bPlayShipsBells = pPlayShipsBells->GetValue();
      if (pSoundDeviceIndex)
          g_iSoundDeviceIndex = pSoundDeviceIndex->GetSelection();
      //g_bTransparentToolbar = pTransparentToolbar->GetValue();
      g_iSDMMFormat = pSDMMFormat->GetSelection();
      g_iDistanceFormat = pDistanceFormat->GetSelection();
      g_iSpeedFormat = pSpeedFormat->GetSelection();

      // LIVE ETA OPTION
      if(pSLiveETA) g_bShowLiveETA = pSLiveETA->GetValue();
      if(pSDefaultBoatSpeed) pSDefaultBoatSpeed->GetValue().ToDouble(&g_defaultBoatSpeedUserUnit);
      g_defaultBoatSpeed = fromUsrSpeed(g_defaultBoatSpeedUserUnit);

      g_bAdvanceRouteWaypointOnArrivalOnly = pAdvanceRouteWaypointOnArrivalOnly->GetValue();

      g_colourTrackLineColour =  m_colourTrackLineColour->GetColour();
      g_colourTrackLineColour =  wxColour(g_colourTrackLineColour.Red(), g_colourTrackLineColour.Green(), g_colourTrackLineColour.Blue());
      g_nTrackPrecision = pTrackPrecision->GetSelection();

      g_bTrackDaily = pTrackDaily->GetValue();

      g_track_rotate_time = 0;
    #if wxCHECK_VERSION(2, 9, 0)
      int h,m,s;
      if( pTrackRotateTime->GetTime(&h, &m, &s) )
          g_track_rotate_time = h*3600 + m*60 + s;
    #endif

        if( pTrackRotateUTC->GetValue() )
            g_track_rotate_time_type = TIME_TYPE_UTC;
        else if( pTrackRotateLMT->GetValue() )
            g_track_rotate_time_type = TIME_TYPE_LMT;
        else g_track_rotate_time_type = TIME_TYPE_COMPUTER;


      g_bHighliteTracks = pTrackHighlite->GetValue();

      if(pEnableZoomToCursor) g_bEnableZoomToCursor = pEnableZoomToCursor->GetValue();

      g_colourOwnshipRangeRingsColour =  m_colourOwnshipRangeRingColour->GetColour();
      g_colourOwnshipRangeRingsColour =  wxColour(g_colourOwnshipRangeRingsColour.Red(), g_colourOwnshipRangeRingsColour.Green(), g_colourOwnshipRangeRingsColour.Blue());

      // AIS Parameters
      //   CPA Box
      g_bCPAMax = m_pCheck_CPA_Max->GetValue();
      m_pText_CPA_Max->GetValue().ToDouble(&g_CPAMax_NM);
      g_bCPAWarn = m_pCheck_CPA_Warn->GetValue();
      m_pText_CPA_Warn->GetValue().ToDouble(&g_CPAWarn_NM);
      g_bTCPA_Max = m_pCheck_CPA_WarnT->GetValue();
      m_pText_CPA_WarnT->GetValue().ToDouble(&g_TCPA_Max);

      //   Lost Targets
      g_bMarkLost = m_pCheck_Mark_Lost->GetValue();
      m_pText_Mark_Lost->GetValue().ToDouble(&g_MarkLost_Mins);
      g_bRemoveLost = m_pCheck_Remove_Lost->GetValue();
      m_pText_Remove_Lost->GetValue().ToDouble(&g_RemoveLost_Mins);

      //   Display
      g_bShowCOG = m_pCheck_Show_COG->GetValue();
      m_pText_COG_Predictor->GetValue().ToDouble(&g_ShowCOG_Mins);

      g_bAISShowTracks = m_pCheck_Show_Tracks->GetValue();
      m_pText_Track_Length->GetValue().ToDouble(&g_AISShowTracks_Mins);

      //   Update all the current targets
      if (g_pAIS) {
        AIS_Target_Hash::iterator it;
        AIS_Target_Hash* current_targets = g_pAIS->GetTargetList();
        for (it = current_targets->begin(); it != current_targets->end(); ++it) {
          AIS_Target_Data* pAISTarget = it->second;
          if (NULL != pAISTarget) pAISTarget->b_show_track = g_bAISShowTracks;
        }
      }

      g_bHideMoored = m_pCheck_Hide_Moored->GetValue();
      m_pText_Moored_Speed->GetValue().ToDouble(&g_ShowMoored_Kts);

      g_bAllowShowScaled = m_pCheck_Scale_Priority->GetValue();
      long l;
      m_pText_Scale_Priority->GetValue().ToLong(&l);
      g_ShowScaled_Num = (int)l;

      g_bShowAreaNotices = m_pCheck_Show_Area_Notices->GetValue();
      g_bDrawAISSize = m_pCheck_Draw_Target_Size->GetValue();
      g_bShowAISName = m_pCheck_Show_Target_Name->GetValue();
      long ais_name_scale = 5000;
      m_pText_Show_Target_Name_Scale->GetValue().ToLong(&ais_name_scale);
      g_Show_Target_Name_Scale = (int)wxMax(5000, ais_name_scale);

      g_bWplIsAprsPosition = m_pCheck_Wpl_Aprs->GetValue();

      //   Alert
      g_bAIS_CPA_Alert = m_pCheck_AlertDialog->GetValue();
      g_bAIS_CPA_Alert_Audio = m_pCheck_AlertAudio->GetValue();
      g_bAIS_CPA_Alert_Suppress_Moored = m_pCheck_Alert_Moored->GetValue();

      g_bAIS_ACK_Timeout = m_pCheck_Ack_Timout->GetValue();
      m_pText_ACK_Timeout->GetValue().ToDouble(&g_AckTimeout_Mins);

      //   Rollover
      g_bAISRolloverShowClass = m_pCheck_Rollover_Class->GetValue();
      g_bAISRolloverShowCOG = m_pCheck_Rollover_COG->GetValue();
      g_bAISRolloverShowCPA = m_pCheck_Rollover_CPA->GetValue();

      g_chart_zoom_modifier = m_pSlider_Zoom->GetValue();
      g_chart_zoom_modifier_vector = m_pSlider_Zoom_Vector->GetValue();
      g_GUIScaleFactor = m_pSlider_GUI_Factor->GetValue();
      g_ChartScaleFactor = m_pSlider_Chart_Factor->GetValue();
      g_ChartScaleFactorExp = g_Platform->getChartScaleFactorExp(g_ChartScaleFactor);
      g_ShipScaleFactor = m_pSlider_Ship_Factor->GetValue();
      g_ShipScaleFactorExp = g_Platform->getChartScaleFactorExp(g_ShipScaleFactor);

      //  Only reload the icons if user has actually visted the UI page
      if(m_bVisitLang)
        pWayPointMan->ReloadAllIcons();

      g_NMEAAPBPrecision = m_choicePrecision->GetCurrentSelection();

      g_TalkerIdText = m_TalkerIdText->GetValue().MakeUpper();

      if (g_bopengl != pOpenGL->GetValue()) m_returnChanges |= GL_CHANGED;
      g_bopengl = pOpenGL->GetValue();

      //   Handle Vector Charts Tab
      g_cm93_zoom_factor = m_pSlider_CM93_Zoom->GetValue();

      int depthUnit = pDepthUnitSelect->GetSelection();
      g_nDepthUnitDisplay = depthUnit;

      //  Process the UserStandard display list, noting if any changes were made
      bool bUserStdChange = false;

      int nOBJL = ps57CtlListBox->GetCount();

      for (int iPtr = 0; iPtr < nOBJL; iPtr++) {
        int itemIndex = -1;
        for (size_t i = 0; i < marinersStdXref.size(); i++) {
          if (marinersStdXref[i] == iPtr) {
            itemIndex = i;
            break;
          }
        }
        assert(itemIndex >= 0);
        OBJLElement* pOLE = (OBJLElement*)(ps52plib->pOBJLArray->Item(itemIndex));
        if(pOLE->nViz != ps57CtlListBox->IsChecked(iPtr))
            bUserStdChange = true;
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

        if(pCheck_SOUNDG) ps52plib->m_bShowSoundg = pCheck_SOUNDG->GetValue();
        if(pCheck_ATONTEXT) ps52plib->m_bShowAtonText = pCheck_ATONTEXT->GetValue();
        if(pCheck_LDISTEXT) ps52plib->m_bShowLdisText = pCheck_LDISTEXT->GetValue();
        if(pCheck_XLSECTTEXT) ps52plib->m_bExtendLightSectors = pCheck_XLSECTTEXT->GetValue();

        ps52plib->m_bShowMeta = pCheck_META->GetValue();
        ps52plib->m_bDeClutterText = pCheck_DECLTEXT->GetValue();
        ps52plib->m_bShowNationalTexts = pCheck_NATIONALTEXT->GetValue();
        ps52plib->m_bShowS57ImportantTextOnly = pCheck_SHOWIMPTEXT->GetValue();
        ps52plib->m_bUseSCAMIN = pCheck_SCAMIN->GetValue();

        ps52plib->m_nSymbolStyle = pPointStyle->GetSelection() == 0 ? PAPER_CHART : SIMPLIFIED;

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

    // User Interface Panel
    #if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
      if (m_bVisitLang) {
        wxString new_canon = _T("en_US");
        wxString lang_sel = m_itemLangListBox->GetStringSelection();

        int nLang = sizeof(lang_list) / sizeof(int);
        for (int it = 0; it < nLang; it++) {
          const wxLanguageInfo* pli = wxLocale::GetLanguageInfo(lang_list[it]);
          if (pli) {
            wxString lang_canonical = pli->CanonicalName;
            wxString test_string = GetOCPNKnownLanguage(lang_canonical);
            if (lang_sel == test_string) {
              new_canon = lang_canonical;
              break;
            }
          }
        }

        wxString locale_old = g_locale;
        g_locale = new_canon;

        if (g_locale != locale_old) m_returnChanges |= LOCALE_CHANGED;

        wxString oldStyle = g_StyleManager->GetCurrentStyle()->name;
        //g_StyleManager->SetStyleNextInvocation( m_itemStyleListBox->GetStringSelection());
        if (g_StyleManager->GetStyleNextInvocation() != oldStyle) {
          m_returnChanges |= STYLE_CHANGED;
        }
        wxSizeEvent nullEvent;
        gFrame->OnSize(nullEvent);
      }
    #endif
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
