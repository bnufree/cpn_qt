/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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

#ifndef __CONFIGMGR_H__
#define __CONFIGMGR_H__


#include "pugixml.hpp"

class OCPNConfigCatalog;
class OCPNConfigObject;

WX_DECLARE_LIST(OCPNConfigObject, ConfigObjectList);

/**
 * Manages the user config matrix.
 *
 * Singleton.
 */
class ConfigMgr
{
    public:
        static ConfigMgr & Get();
        static void Shutdown();
        
        QString CreateNamedConfig( const QString &title, const QString &description, QString UUID );
        bool DeleteConfig(QString GUID);
        wxArrayString GetConfigGUIDArray();
        
        wxPanel *GetConfigPanel( wxWindow *parent, QString GUID );
        QString GetTemplateTitle( QString GUID );
        bool ApplyConfigGUID( QString GUID );
        bool CheckTemplateGUID( QString GUID );
        
    private: // private for singleton
        ConfigMgr();
        ~ConfigMgr();
        ConfigMgr(const ConfigMgr &) {}
        ConfigMgr & operator=(const ConfigMgr &) { return *this; }
        static ConfigMgr *instance;
        
        void Init();
        bool LoadCatalog();
        bool SaveCatalog();
        QString GetUUID(void);
        bool SaveTemplate( QString fileName);
        QString GetConfigDir(){ return m_configDir; }
        ConfigObjectList *GetConfigList(){ return configList; }
        OCPNConfigObject *GetConfig( QString GUID );
        bool CheckTemplate( QString fileName);
        
        QString m_configDir;
        QString m_configCatalogName;
        OCPNConfigCatalog *m_configCatalog;
        ConfigObjectList *configList;
};

class ConfigPanel: public wxPanel
{
public:
    ConfigPanel( OCPNConfigObject *config, wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize );
    ~ConfigPanel();
    
    QString GetConfigGUID();
    
private:
    void OnConfigPanelMouseSelected( wxMouseEvent &event);
    OCPNConfigObject *m_config;
};

#endif
