/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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
 ***************************************************************************
 */

#include "config.h"

#include "s57RegistrarMgr.h"
#include "S57ClassRegistrar.h"
#include <QDir>
#include <QDebug>
#include "_def.h"

extern S57ClassRegistrar *g_poRegistrar;

static void s57_initialize( const QString& csv_dir )
{
    //      Get one instance of the s57classregistrar,
    //      And be prepared to give it to any module that needs it

    if( g_poRegistrar == NULL ) {
        g_poRegistrar = new S57ClassRegistrar();

        if( !g_poRegistrar->LoadInfo( csv_dir.toUtf8().data(), false ) ) {
            qDebug("   Error: Could not load S57 ClassInfo from %s", csv_dir.toUtf8().data() );

            delete g_poRegistrar;
            g_poRegistrar = NULL;
        }
    }
}

s57RegistrarMgr::s57RegistrarMgr( const QString& csv_dir )
{
    s57_initialize( csv_dir );
    
    //  Create and initialize the S57 Attribute helpers
    s57_attr_init( csv_dir );
    //  Create and initialize the S57 Feature code helpers
    s57_feature_init( csv_dir );
}

s57RegistrarMgr::~s57RegistrarMgr()
{
    delete g_poRegistrar;
    g_poRegistrar = NULL;
}

bool s57RegistrarMgr::s57_attr_init( const QString& csv_dir ){
    
    //  Find, open, and read the file {csv_dir}/s57attributes.csv
    QString csv_t = csv_dir;
    QString sep = zchxFuncUtil::separator();
    if( csv_t.right(1) != sep ) csv_t.append( sep );

    QString targetFile = csv_t + ("s57attributes.csv");
    QFile tFile(targetFile);
    
    if(!tFile.open(QIODevice::ReadOnly ) ){
        QString msg(("   Error: Could not load S57 Attribute Info from ") );
        msg.append( csv_dir );
        qDebug()<< msg;
        return false;
    }

    //  populate the member hashmaps
    
    //First map: Key is char[] attribute acronym, value is standard ID
    //Second map: Key is standard ID, value is char[] attribute acronym
    
    while(!tFile.atEnd())
    {
        QString str = QString::fromUtf8(tFile.readLine());
        QStringList tk = str.split(QRegExp("[,\r\n]"));
        int i = 0;
        QString ident = tk[i++];
        bool ok = false;
        long nID = ident.toLong(&ok);
        if( ok){
            QString description = tk[i++];
            QString acronym = tk[i++];
            m_attrHash1[acronym] = nID;
            m_attrHash2[nID] = acronym.toStdString();
            
        }
    }

    return true;     
    
}

bool s57RegistrarMgr::s57_feature_init( const QString& csv_dir ){
    
    //  Find, open, and read the file {csv_dir}/s57objectclasses.csv
    QString csv_t = csv_dir;
    QString sep = zchxFuncUtil::separator();
    if( csv_t.right(1) != sep ) csv_t.append( sep );

    QString targetFile = csv_t + ("s57objectclasses.csv");
    QFile file(targetFile);
    if(!file.open( QIODevice::ReadOnly ) ){
        QString msg(("   Error: Could not load S57 Feature Info from ") );
        msg.append( csv_dir );
        qDebug()<<msg;
        return false;
    }
    
    //  populate the member hashmaps
    
    //First map: Key is char[] feature acronym, value is standard ID
    //Second map: Key is standard ID, value is char[] feature acronym
    
    while(!file.atEnd())
    {
        QString str = QString::fromUtf8(file.readLine());
        QStringList tk = str.split(QRegExp("[,\r\n]"));
        int i = 0;
        QString ident = tk[i++];
        bool ok = false;
        long nID = ident.toLong(&ok);
        if( ok){
            QString description = tk[i++];
//            QString d2;
            while(!description.endsWith("\""))
                description += tk[i++];
            
            QString acronym = tk[i++];
            
            m_featureHash1[acronym] = nID;
            m_featureHash2[nID] = acronym.toStdString();
            
        }
    }
    
    return true;     
    
}

int s57RegistrarMgr::getAttributeID(const char *pAttrName){
    QString key(pAttrName);
    
    if( m_attrHash1.find( key ) == m_attrHash1.end())
        return -1;
    else
        return m_attrHash1[key];
}

std::string s57RegistrarMgr::getAttributeAcronym(int nID){
    
    if( m_attrHash2.find( nID ) == m_attrHash2.end())
        return "";
    else
        return m_attrHash2[nID];
}


std::string s57RegistrarMgr::getFeatureAcronym(int nID){
    
    if( m_featureHash2.find( nID ) == m_featureHash2.end())
        return "";
    else
        return m_featureHash2[nID];
}
