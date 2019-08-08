#ifndef ZCHXFRAME_H
#define ZCHXFRAME_H

#include <QMainWindow>

class zchxFrame : public QMainWindow
{
    Q_OBJECT
public:
    zchxFrame(const QString& title, quint64 processID, QWidget* parent = 0);
    ~zchxFrame();

    void setApplicationName(const QString& name);
    QString applicationName() const {return mApplicationName;}

    quint64 proceeId() const {return mProcessedID;}
    void    setProcessID(quint64 id) {mProcessedID = id;}
    quint64 getProcessIDFromSystem();
    int     GetApplicationMemoryUse(void);
    void    getMemoryStatus();
private:
    bool    prepareClose();
protected:
    void closeEvent(QCloseEvent& event);
public:
#if 0
    void OnExit(QCommandEvent& event);
    void OnSize(QResizeEvent& event);
    void OnMove(QMoveEvent& event);
    void OnInitTimer(QTimerEvent& event);
    void OnFrameTimer1(QTimerEvent& event);
    bool DoChartUpdate(void);
    void OnEvtTHREADMSG(OCPN_ThreadMessageEvent& event);
    void OnEvtOCPN_NMEA(OCPN_DataStreamEvent & event);
    void OnEvtPlugInMessage( OCPN_MsgEvent & event );
    void OnMemFootTimer(QTimerEvent& event);
    void OnRecaptureTimer(QTimerEvent& event);
    void OnSENCEvtThread( OCPN_BUILDSENC_ThreadEvent & event);
    void OnIconize(QIconizeEvent& event);
    void OnBellsFinished(QCommandEvent& event);

#ifdef wxHAS_POWER_EVENTS
    void OnSuspending(wxPowerEvent &event);
    void OnSuspended(wxPowerEvent &event);
    void OnSuspendCancel(wxPowerEvent &event);
    void OnResume(wxPowerEvent &event);
#endif // wxHAS_POWER_EVENTS

    void RefreshCanvasOther( ChartCanvas *ccThis );
    void UpdateAllFonts(void);
    void PositionConsole(void);
    void OnToolLeftClick(QCommandEvent& event);
    void ClearRouteTool();
    void DoStackUp(ChartCanvas *cc);
    void DoStackDown(ChartCanvas *cc);
    void selectChartDisplay( int type, int family);
    void applySettingsString( QString settings);
    void setStringVP(QString VPS);
    void InvalidateAllGL();
    void RefreshAllCanvas( bool bErase = true);
    void CancelAllMouseRoute();

    QMenuBar *GetMainMenuBar(){ return m_pMenuBar; }

    ChartCanvas *GetPrimaryCanvas();
    ChartCanvas *GetFocusCanvas();

    void DoStackDelta( ChartCanvas *cc, int direction );
    void DoSettings( void );
    void SwitchKBFocus( ChartCanvas *pCanvas );
    ChartCanvas *GetCanvasUnderMouse();
    int GetCanvasIndexUnderMouse();

    bool DropMarker( bool atOwnShip = true );

    void TriggerResize(QSize sz);
    void OnResizeTimer(QTimerEvent &event);

    void TriggerRecaptureTimer();
    bool SetGlobalToolbarViz( bool viz );

    void MouseEvent(QMouseEvent& event);
//     void SelectChartFromStack(int index,  bool bDir = false,  ChartTypeEnum New_Type = CHART_TYPE_DONTCARE, ChartFamilyEnum New_Family = CHART_FAMILY_DONTCARE);
//     void SelectdbChart(int dbindex);
//     void SelectQuiltRefChart(int selected_index);
//     void SelectQuiltRefdbChart(int db_index, bool b_autoscale = true);
    void CenterView(ChartCanvas *cc, const LLBBox& bbox);

    void JumpToPosition( ChartCanvas *cc, double lat, double lon, double scale );

    void ProcessCanvasResize(void);

    void BuildMenuBar( void );
    void ApplyGlobalSettings(bool bnewtoolbar);
    void RegisterGlobalMenuItems();
    void UpdateGlobalMenuItems();
    void UpdateGlobalMenuItems( ChartCanvas *cc);
    void SetChartThumbnail(int index);
    int  DoOptionsDialog();
    bool  ProcessOptionsDialog(int resultFlags, ArrayOfCDI *pNewDirArray );
    void DoPrint(void);
    void StopSockets(void);
    void ResumeSockets(void);
    void ToggleDataQuality( ChartCanvas *cc );
    void TogglebFollow(ChartCanvas *cc);
    void ToggleFullScreen();
    void ToggleChartBar(ChartCanvas *cc);
    void SetbFollow(ChartCanvas *cc);
    void ClearbFollow(ChartCanvas *cc);
    void ToggleChartOutlines(ChartCanvas *cc);
    void ToggleENCText(ChartCanvas *cc);
    void ToggleSoundings(ChartCanvas *cc);
    void ToggleRocks(void);
    bool ToggleLights( ChartCanvas *cc );
    void ToggleAnchor( ChartCanvas *cc );
    void ToggleAISDisplay( ChartCanvas *cc );
    void ToggleAISMinimizeTargets( ChartCanvas *cc );

    void ToggleTestPause(void);
    void TrackOn(void);
    void SetENCDisplayCategory( ChartCanvas *cc, enum _DisCat nset );
    void ToggleNavobjects( ChartCanvas *cc );

    Track *TrackOff(bool do_add_point = false);
    void TrackDailyRestart(void);
    bool ShouldRestartTrack();
    void ToggleColorScheme();
    void SetMenubarItemState ( int item_id, bool state );
    void SetMasterToolbarItemState( int tool_id, bool state );

    void SetToolbarItemBitmaps ( int tool_id, QBitmap *bitmap, QBitmap *bmpDisabled );
    void SetToolbarItemSVG( int tool_id, QString normalSVGfile,
                            QString rolloverSVGfile,
                            QString toggledSVGfile );
    void ToggleQuiltMode(ChartCanvas *cc);
    void ToggleCourseUp(ChartCanvas *cc);
    void UpdateControlBar(ChartCanvas *cc);

    void ShowTides(bool bShow);
    void ShowCurrents(bool bShow);

    void SubmergeAllCanvasToolbars(void);
    void SurfaceAllCanvasToolbars(void);
    void ToggleAllToolbars( bool b_smooth = false );
    void SetAllToolbarScale(void);
    void SetGPSCompassScale(void);
    void InvalidateAllCanvasUndo();

    void RefreshGroupIndices(void);

    double GetBestVPScale(ChartBase *pchart);

    ColorScheme GetColorScheme();
    void SetAndApplyColorScheme(ColorScheme cs);

    void OnFrameTCTimer(QTimerEvent& event);
    void OnFrameCOGTimer(QTimerEvent& event);

    void ChartsRefresh();

    bool CheckGroup(int igroup);
    double GetMag(double a);
    double GetMag(double a, double lat, double lon);
    bool SendJSON_WMM_Var_Request(double lat, double lon, QDateTime date);

    void DestroyPersistentDialogs();
    void TouchAISActive(void);
    void UpdateAISTool(void);

    void ActivateAISMOBRoute( AIS_Target_Data *ptarget );
    void UpdateAISMOBRoute( AIS_Target_Data *ptarget );

    QStatusBar         *m_pStatusBar;
    QMenuBar           *m_pMenuBar;
    int                 nBlinkerTick;
    bool                m_bTimeIsSet;

    QTimer             *InitTimer;
    int                 m_iInitCount;
    bool                m_initializing;

    QTimer             *FrameTCTimer;
    QTimer             *FrameTimer1;
    QTimer             *FrameCOGTimer;
    QTimer             *MemFootTimer;
    QTimer             *m_resizeTimer;

    int                 m_BellsToPlay;
    QTimer             *BellsTimer;

    //      PlugIn support
    int GetNextToolbarToolId(){return m_next_available_plugin_tool_id;}
    void RequestNewToolbarArgEvent(QCommandEvent&  event  ){Q_UNUSED (event); return RequestNewMasterToolbar(); }
    void RequestNewToolbars( bool bforcenew = false);

    void ActivateMOB(void);
    void UpdateGPSCompassStatusBoxes(bool b_force_new = false);
    void UpdateRotationState( double rotation );

    bool UpdateChartDatabaseInplace(ArrayOfCDI &DirArray,
                                    bool b_force, bool b_prog,
                                    const QString &ChartListFileName);

    bool                m_bdefer_resize;
    QSize              m_defer_size;
    QSize              m_newsize;
    double           COGTable[MAX_COG_AVERAGE_SECONDS];

    void FastClose();
    void SetChartUpdatePeriod();
    void CreateCanvasLayout( bool b_useStoredSize = false );
    void LoadHarmonics();
    void ReloadAllVP();
    void SetCanvasSizes( QSize frameSize );

    ocpnToolBarSimple *CreateMasterToolbar();
    void RequestNewMasterToolbar( bool bforcenew = true );
    bool CheckAndAddPlugInTool( );
    bool AddDefaultPositionPlugInTools( );

    void NotifyChildrenResize( void );
    void UpdateCanvasConfigDescriptors();
    void ScheduleSettingsDialog();
    static void RebuildChartDatabase();
    void PositionIENCToolbar();

  private:

    void CheckToolbarPosition();
    void ODoSetSize(void);
    void DoCOGSet(void);

    void UpdateAllToolbars( ColorScheme cs );

    void FilterCogSog(void);

    void ApplyGlobalColorSchemetoStatusBar(void);
    void PostProcessNMEA(bool pos_valid, bool cog_sog_valid, const QString &sfixtime);

    bool ScrubGroupArray();
    QString GetGroupName(int igroup);

    bool EvalPriority(const QString & message, DataStream *pDS );
    void SetAISDisplayStyle(ChartCanvas *cc, int StyleIndx);

    bool GetMasterToolItemShow( int toolid );
    void OnToolbarAnimateTimer( QTimerEvent& event );
    bool CollapseGlobalToolbar();

    int                 m_StatusBarFieldCount;

    NMEA0183        m_NMEA0183;                 // Used to parse messages from NMEA threads

    QDateTime       m_MMEAeventTime;
    unsigned long    m_ulLastNMEATicktime;

    QMutex          m_mutexNMEAEvent;         // Mutex to handle static data from NMEA threads

    QString         m_last_reported_chart_name;
    QString         m_last_reported_chart_pubdate;


    QString         m_lastAISiconName;

    //      Plugin Support
    int                 m_next_available_plugin_tool_id;

    double              COGFilterTable[MAX_COGSOG_FILTER_SECONDS];
    double              SOGFilterTable[MAX_COGSOG_FILTER_SECONDS];

    bool                m_bpersistent_quilt;
    int                 m_ChartUpdatePeriod;
    bool                m_last_bGPSValid;

    QString            prev_locale;
    bool                bPrevQuilt;
    bool                bPrevFullScreenQuilt;
    bool                bPrevOGL;

    MsgPriorityHash     NMEA_Msg_Hash;
    QString            m_VDO_accumulator;

    time_t              m_fixtime;
    QMenu              *piano_ctx_menu;
    bool                b_autofind;

    time_t              m_last_track_rotation_ts;
    QRect              m_mainlast_tb_rect;
    QTimer             *ToolbarAnimateTimer;
    int                 m_nMasterToolCountShown;
    QTimer             *m_recaptureTimer;
#endif

    //内存监控
    quint64               mProcessedID;
    QString               mApplicationName;
    quint64               mMemUsed;
    quint64               mMemTotal;

    //程序关闭检测
    bool                  mInCloseWindow;

};

#endif // ZCHXFRAME_H
