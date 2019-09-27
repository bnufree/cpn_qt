/***************************************************************************
 *
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Manager Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef _PLUGINMGR_H_
#define _PLUGINMGR_H_




#include "config.h"

#include "ocpn_plugin.h"
#include "chart1.h"                 // for MyFrame
//#include "chcanv.h"                 // for ViewPort
#include "sound/OCPN_Sound.h"
#include "chartimg.h"

#include "s57chart.h"               // for Object list


#ifndef __OCPN__ANDROID__
#ifdef OCPN_USE_CURL
#endif
#endif

//    Include wxJSON headers
//    We undefine MIN/MAX so avoid warning of redefinition coming from
//    json_defs.h
//    Definitions checked manually, and are identical
#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif


//    Assorted static helper routines

PlugIn_AIS_Target *Create_PI_AIS_Target(AIS_Target_Data *ptarget);

class PluginListPanel;
class PluginPanel;
class QLibrary;

typedef struct {
    QString name;      // name of the plugin
    int version_major;  // major version
    int version_minor;  // minor version
    bool hard;          // hard blacklist - if true, don't load it at all, if false, load it and just warn the user
    bool all_lower;     // if true, blacklist also all the lower versions of the plugin
} BlackListedPlugin;

const BlackListedPlugin PluginBlacklist[] = {
    { ("aisradar_pi"), 0, 95, true, true },
    { ("radar_pi"), 0, 95, true, true },             // GCC alias for aisradar_pi
    { ("watchdog_pi"), 1, 00, true, true },
    { ("squiddio_pi"), 0, 2, true, true },
    { ("objsearch_pi"), 0, 3, true, true },
#ifdef __WXOSX__
    { ("s63_pi"), 0, 6, true, true },
#endif    
};

//----------------------------------------------------------------------------
// PlugIn Messaging scheme Event
//----------------------------------------------------------------------------

class OCPN_MsgEvent: public QEvent
{
public:
    OCPN_MsgEvent();

    OCPN_MsgEvent(const OCPN_MsgEvent & event)
    : QEvent(event),
    m_MessageID(event.m_MessageID),
    m_MessageText(event.m_MessageText)
    { }

    ~OCPN_MsgEvent( );

    // accessors
    QString GetID() { return m_MessageID; }
    QString GetJSONText() { return m_MessageText; }

    void SetID(const QString &string) { m_MessageID = string; }
    void SetJSONText(const QString &string) { m_MessageText = string; }


    // required for sending with wxPostEvent()
    QEvent *Clone() const;

private:
    QString    m_MessageID;
    QString    m_MessageText;


};

extern  const QEvent::Type wxEVT_OCPN_MSG;


//-----------------------------------------------------------------------------------------------------
//
//          The PlugIn Container Specification
//
//-----------------------------------------------------------------------------------------------------
class PlugInContainer
{
      public:
            PlugInContainer(){ m_pplugin = NULL;
                               m_bEnabled = false;
                               m_bInitState = false;
                               m_bToolboxPanel = false;
                               m_bitmap = NULL; }

            opencpn_plugin    *m_pplugin;
            bool              m_bEnabled;
            bool              m_bInitState;
            bool              m_bToolboxPanel;
            int               m_cap_flag;             // PlugIn Capabilities descriptor
            QString          m_plugin_file;          // The full file path
            QString          m_plugin_filename;      // The short file path
            QDateTime        m_plugin_modification;  // used to detect upgraded plugins
            destroy_t         *m_destroy_fn;
            QLibrary          *m_plibrary;
            QString          m_common_name;            // A common name string for the plugin
            QString          m_short_description;
            QString          m_long_description;
            int               m_api_version;
            int               m_version_major;
            int               m_version_minor;
            QBitmap         *m_bitmap;

};

//    Declare an array of PlugIn Containers
typedef QList<PlugInContainer *> ArrayOfPlugIns;

class PlugInMenuItemContainer
{
      public:
            QAction        *pmenu_item;
            opencpn_plugin    *m_pplugin;
            bool              b_viz;
            bool              b_grey;
            int               id;
            QString          m_in_menu;
};

//    Define an array of PlugIn MenuItem Containers
typedef QList<PlugInMenuItemContainer*> ArrayOfPlugInMenuItems;


class PlugInToolbarToolContainer
{
      public:
            PlugInToolbarToolContainer();
            ~PlugInToolbarToolContainer();

            opencpn_plugin    *m_pplugin;
            int               id;
            QString          label;
            QBitmap          *bitmap_day;
            QBitmap          *bitmap_dusk;
            QBitmap          *bitmap_night;
            QBitmap          *bitmap_Rollover_day;
            QBitmap          *bitmap_Rollover_dusk;
            QBitmap          *bitmap_Rollover_night;
            
            wxItemKind        kind;
            QString          shortHelp;
            QString          longHelp;
            QObject          *clientData;
            int               position;
            bool              b_viz;
            bool              b_toggle;
            int               tool_sel;
            QString          pluginNormalIconSVG;
            QString          pluginRolloverIconSVG;
            QString          pluginToggledIconSVG;
            
};

//    Define an array of PlugIn ToolbarTool Containers
typedef QList<PlugInToolbarToolContainer *> ArrayOfPlugInToolbarTools;



//-----------------------------------------------------------------------------------------------------
//
//          The PlugIn Manager Specification
//
//-----------------------------------------------------------------------------------------------------

class PlugInManager: public wxEvtHandler
{

public:
      PlugInManager(MyFrame *parent);
      virtual ~PlugInManager();

      bool LoadAllPlugIns(const QString &plugin_dir, bool enabled_plugins, bool b_enable_blackdialog = true);
      bool UnLoadAllPlugIns();
      bool DeactivateAllPlugIns();
      bool UpdatePlugIns();

      bool UpdateConfig();

      PlugInContainer *LoadPlugIn(QString plugin_file);
      ArrayOfPlugIns *GetPlugInArray(){ return &plugin_array; }

      bool RenderAllCanvasOverlayPlugIns( ocpnDC &dc, const ViewPort &vp, int canvasIndex);
      bool RenderAllGLCanvasOverlayPlugIns( wxGLContext *pcontext, const ViewPort &vp, int canvasIndex);
      void SendCursorLatLonToAllPlugIns( double lat, double lon);
      void SendViewPortToRequestingPlugIns( ViewPort &vp );
      void PrepareAllPluginContextMenus();

      void NotifySetupOptions();
      void CloseAllPlugInPanels( int );

      ArrayOfPlugInToolbarTools &GetPluginToolbarToolArray(){ return m_PlugInToolbarTools; }
      int AddToolbarTool(QString label, QBitmap *bitmap, QBitmap *bmpRollover,
                         wxItemKind kind, QString shortHelp, QString longHelp,
                         QObject *clientData, int position,
                         int tool_sel, opencpn_plugin *pplugin );

      void RemoveToolbarTool(int tool_id);
      void SetToolbarToolViz(int tool_id, bool viz);
      void SetToolbarItemState(int tool_id, bool toggle);
      void SetToolbarItemBitmaps(int item, QBitmap *bitmap, QBitmap *bmpDisabled);
      
      int AddToolbarTool(QString label, QString SVGfile, QString SVGRolloverfile, QString SVGToggledfile,
                         wxItemKind kind, QString shortHelp, QString longHelp,
                         QObject *clientData, int position,
                         int tool_sel, opencpn_plugin *pplugin );
      
      void SetToolbarItemBitmaps(int item, QString SVGfile,
                                 QString SVGfileRollover,
                                 QString SVGfileToggled);
      
      opencpn_plugin *FindToolOwner(const int id);
      QString GetToolOwnerCommonName(const int id);
      void ShowDeferredBlacklistMessages();

      ArrayOfPlugInMenuItems &GetPluginContextMenuItemArray(){ return m_PlugInMenuItems; }
      int AddCanvasContextMenuItem(QAction *pitem, opencpn_plugin *pplugin, const char *name = "" );
      void RemoveCanvasContextMenuItem(int item, const char *name = "" );
      void SetCanvasContextMenuItemViz(int item, bool viz, const char *name = "" );
      void SetCanvasContextMenuItemGrey(int item, bool grey, const char *name = "" );

      void SendNMEASentenceToAllPlugIns(const QString &sentence);
      void SendPositionFixToAllPlugIns(GenericPosDatEx *ppos);
      void SendAISSentenceToAllPlugIns(const QString &sentence);
      void SendJSONMessageToAllPlugins(const QString &message_id, QJsonValue v);
      void SendMessageToAllPlugins(const QString &message_id, const QString &message_body);
      int GetJSONMessageTargetCount();
      
      void SendResizeEventToAllPlugIns(int x, int y);
      void SetColorSchemeForAllPlugIns(ColorScheme cs);
      void NotifyAuiPlugIns(void);
      bool CallLateInit(void);
      
      bool IsPlugInAvailable(QString commonName);
      bool IsAnyPlugInChartEnabled();
      
      void SendVectorChartObjectInfo(const QString &chart, const QString &feature, const QString &objname, double &lat, double &lon, double &scale, int &nativescale);

      bool SendMouseEventToPlugins( wxMouseEvent &event);
      bool SendKeyEventToPlugins( QKeyEvent* event);

      void SendBaseConfigToAllPlugIns();
      void SendS52ConfigToAllPlugIns( bool bReconfig = false );
      
      QStringList GetPlugInChartClassNameArray(void);

      ListOfPI_S57Obj *GetPlugInObjRuleListAtLatLon( ChartPlugInWrapper *target, float zlat, float zlon,
                                                       float SelectRadius, const ViewPort& vp );
      QString CreateObjDescriptions( ChartPlugInWrapper *target, ListOfPI_S57Obj *rule_list );
      
      QString GetLastError();
      MyFrame *GetParentFrame(){ return pParent; }

      void DimeWindow(QWidget *win);
      
private:
      bool CheckBlacklistedPlugin(opencpn_plugin* plugin);
      bool DeactivatePlugIn(PlugInContainer *pic);
      QBitmap *BuildDimmedToolBitmap(QBitmap *pbmp_normal, unsigned char dim_ratio);
      bool UpDateChartDataTypes(void);
      bool CheckPluginCompatibility(QString plugin_file);
      bool LoadPlugInDirectory(const QString &plugin_dir, bool enabled_plugins, bool b_enable_blackdialog);

      MyFrame                 *pParent;

      ArrayOfPlugIns    plugin_array;
      QString          m_last_error_string;

      ArrayOfPlugInMenuItems        m_PlugInMenuItems;
      ArrayOfPlugInToolbarTools     m_PlugInToolbarTools;

      QString          m_plugin_location;

      int               m_plugin_tool_id_next;
      int               m_plugin_menu_item_id_next;
      QBitmap          m_cached_overlay_bm;

      bool              m_benable_blackdialog;
      bool              m_benable_blackdialog_done;
      QStringList     m_deferred_blacklist_messages;
      
      QStringList     m_plugin_order;
      void SetPluginOrder( QString serialized_names );
      QString GetPluginOrder();
    
#ifndef __OCPN__ANDROID__
#ifdef OCPN_USE_CURL
      
public:
      wxCurlDownloadThread *m_pCurlThread;
      // The libcurl handle being re used for the transfer.
      std::shared_ptr<wxCurlBase> m_pCurl;

      // returns true if the error can be ignored
      bool            HandleCurlThreadError(wxCurlThreadError err, wxCurlBaseThread *p,
                               const QString &url = wxEmptyString);
      void            OnEndPerformCurlDownload(wxCurlEndPerformEvent &ev);
      void            OnCurlDownload(wxCurlDownloadEvent &ev);
      
      wxEvtHandler   *m_download_evHandler;
      long           *m_downloadHandle;
      bool m_last_online;
      long m_last_online_chk;
#endif
#endif
};

typedef QList<PluginPanel *>    ArrayOfPluginPanel;

class PluginListPanel: public wxScrolledWindow
{
public:
      PluginListPanel( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, ArrayOfPlugIns *pPluginArray );
      ~PluginListPanel();

      void SelectPlugin( PluginPanel *pi );
      void MoveUp( PluginPanel *pi );
      void MoveDown( PluginPanel *pi );
      void UpdateSelections();
      void UpdatePluginsOrder();

private:
      ArrayOfPlugIns     *m_pPluginArray;
      ArrayOfPluginPanel  m_PluginItems;
      PluginPanel        *m_PluginSelected;
      
      wxBoxSizer         *m_pitemBoxSizer01;
};

class PluginPanel: public wxPanel
{
public:
      PluginPanel( PluginListPanel *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, PlugInContainer *p_plugin );
      ~PluginPanel();

      void OnPluginSelected( wxMouseEvent &event );
      void SetSelected( bool selected );
      void OnPluginPreferences( wxCommandEvent& event );
      void OnPluginEnable( wxCommandEvent& event );
      void OnPluginUp( wxCommandEvent& event );
      void OnPluginDown( wxCommandEvent& event );
      void SetEnabled( bool enabled );
      bool GetSelected(){ return m_bSelected; }
      PlugInContainer* GetPluginPtr() { return m_pPlugin; }

private:
      PluginListPanel *m_PluginListPanel;
      bool             m_bSelected;
      PlugInContainer *m_pPlugin;
      QLabel    *m_pName;
      QLabel    *m_pVersion;
      QLabel    *m_pDescription;
      wxFlexGridSizer      *m_pButtons;
      QPushButton        *m_pButtonEnable;
      QPushButton        *m_pButtonPreferences;
      
      wxBoxSizer      *m_pButtonsUpDown;
      QPushButton        *m_pButtonUp;
      QPushButton        *m_pButtonDown;
};


//  API 1.11 adds access to S52 Presentation library
//  These are some wrapper conversion utilities

class S52PLIB_Context
{
public:
    S52PLIB_Context(){
        bBBObj_valid = false;
        bCS_Added = false;
        bFText_Added = false;
        CSrules = NULL;
        FText = NULL;
        ChildRazRules = NULL;
        MPSRulesList = NULL;
        LUP = NULL;
        };
        
    ~S52PLIB_Context(){}
    
    wxBoundingBox           BBObj;                  // lat/lon BBox of the rendered object
    bool                    bBBObj_valid;           // set after the BBObj has been calculated once.
    
    Rules                   *CSrules;               // per object conditional symbology
    int                     bCS_Added;
    
    S52_TextC                *FText;
    int                     bFText_Added;
    QRect                  rText;
    
    LUPrec                  *LUP;
    ObjRazRules             *ChildRazRules;
    mps_container           *MPSRulesList;
};


void CreateCompatibleS57Object( PI_S57Obj *pObj, S57Obj *cobj, chart_context *pctx );
void UpdatePIObjectPlibContext( PI_S57Obj *pObj, S57Obj *cobj );

#endif            // _PLUGINMGR_H_

