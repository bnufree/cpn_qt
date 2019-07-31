/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
#include <thread>
#include "zchxLog.h"
#include "OcpnWxSound.h"
#include <QFile>


bool OcpnWxSound::Load(const char* path, int deviceIndex)
{
    QString fileName = QString::fromStdString(path);
    if(!QFile::exists(fileName))
    {
        m_OK = false;
    } else
    {
        m_sound = new QSound(fileName);
        m_path = path;
        m_OK = true;
    }
    m_isPlaying = false;
    return m_OK;
}


bool OcpnWxSound::Stop(void)
{
    if(m_sound) m_sound->stop();
    m_OK = false;
    m_isPlaying = false;
    return false;
}


void OcpnWxSound::worker(void)
{
    ZCHX_LOGMSG("start");
    m_isPlaying = true;
    if(m_sound)m_sound->play();
    if  (m_onFinished) {
        m_onFinished(m_callbackData);
        m_onFinished = 0;
    }
    m_isPlaying = false;
}


bool OcpnWxSound::Play()
{
    ZCHX_LOGMSG("wxSound::Play()");
    if( !m_OK || m_isPlaying) {
        ZCHX_LOGMSG("OcpnWxSound: cannot play: not loaded or busy playing.");
        return false;
    }
    if  (m_onFinished) {
        std::thread t([this]() { worker(); });
        t.detach();
        return true;
    }
    if(!m_sound) return false;
    m_sound->play();
    return true;
}
