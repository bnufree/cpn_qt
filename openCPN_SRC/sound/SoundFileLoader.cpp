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

#include "OCPN_Sound.h"
#include <algorithm>
#include <QFile>
#include "SoundFileLoader.h"
#include "zchxLog.h"


bool SoundFileLoader::Load(const char* path)
{

    QFile fileWave;
    fileWave.setFileName(QString::fromStdString(path));
    if (!fileWave.open(QIODevice::ReadOnly)) {
        ZCHX_LOGMSG(QString("").sprintf("Cannot open %s for read", path).toStdString().data());
        return false;
    }
    qint64 lenOrig = fileWave.size();
    if ( lenOrig < 0 ) {
        ZCHX_LOGMSG(QString("").sprintf("Loading %s: invalid offset", path).toStdString().data());
        return false;
    }
    size_t len = lenOrig;
    quint8 *data = new quint8[len];
    if ( fileWave.read((char*)data, len) != lenOrig ) {
        delete [] data;
        ZCHX_LOGMSG(QString("").sprintf("Couldn't load sound data from '%s'.", path).toStdString().data());
        return false;
    }
    if (!LoadWAV(data, len)) {
        delete [] data;
        ZCHX_LOGMSG(QString("").sprintf("Sound file '%s' is in unsupported format.", path).toStdString().data());
        return false;
    }
    m_next = 0;
    return true;
}


typedef struct
{
    quint32      uiSize;
    quint16      uiFormatTag;
    quint16      uiChannels;
    quint32      ulSamplesPerSec;
    quint32      ulAvgBytesPerSec;
    quint16      uiBlockAlign;
    quint16      uiBitsPerSample;
} WaveFormat;

#define WAVE_FORMAT_PCM  1
#define WAVE_INDEX       8
#define FMT_INDEX       12


bool SoundFileLoader::LoadWAV(const uint8_t* data, size_t length)
{
    // the simplest wave file header consists of 44 bytes:
    //
    //      0   "RIFF"
    //      4   file size - 8
    //      8   "WAVE"
    //
    //      12  "fmt "
    //      16  chunk size                  |
    //      20  format tag                  |
    //      22  number of channels          |
    //      24  sample rate                 | WAVEFORMAT
    //      28  average bytes per second    |
    //      32  bytes per frame             |
    //      34  bits per sample             |
    //
    //      36  "data"
    //      40  number of data bytes
    //      44  (wave signal) data
    //
    // so check that we have at least as much
    if ( length < 44 )
        return false;

    WaveFormat waveformat;
    memcpy(&waveformat, &data[FMT_INDEX + 4], sizeof(WaveFormat));
    //  Sanity checks
    if (memcmp(data, "RIFF", 4) != 0) {
        return false;
    }
    if (memcmp(&data[WAVE_INDEX], "WAVE", 4) != 0) {
        return false;
    }
    if (memcmp(&data[FMT_INDEX], "fmt ", 4) != 0) {
        return false;
    }
    // get the sound data size
    quint32 ul = 0;
    //  Get the "data" chunk length
    if (memcmp(&data[FMT_INDEX + waveformat.uiSize + 8], "data", 4) == 0) {
        memcpy(&ul, &data[FMT_INDEX + waveformat.uiSize + 12], 4);
    }

    //  There may be a "fact" chunk in the header, which will displace the first "data" chunk
    //  If so, find the "data" chunk 12 bytes further along
    else if (memcmp(&data[FMT_INDEX + waveformat.uiSize + 8], "fact", 4) == 0) {
        if (memcmp(&data[FMT_INDEX + waveformat.uiSize + 8 + 12], "data", 4) == 0) {
            memcpy(&ul, &data[FMT_INDEX + waveformat.uiSize + 12 + 12], 4);
        }
    }
    if ( length < ul + FMT_INDEX + waveformat.uiSize + 16 ) {
        return false;
    }
    if (waveformat.uiFormatTag != WAVE_FORMAT_PCM) {
        return false;
    }
    if (waveformat.ulSamplesPerSec !=
        waveformat.ulAvgBytesPerSec / waveformat.uiBlockAlign)
    {
        return false;
    }
    m_osdata = std::unique_ptr<SoundData>(new SoundData);
    m_osdata->m_channels = waveformat.uiChannels;
    m_osdata->m_samplingRate = waveformat.ulSamplesPerSec;
    m_osdata->m_bitsPerSample = waveformat.uiBitsPerSample;
    m_osdata->m_samples = ul / (m_osdata->m_channels * m_osdata->m_bitsPerSample / 8);
    m_osdata->m_dataBytes = ul;
    unsigned samplesPos = FMT_INDEX + waveformat.uiSize + 8;
    m_osdata->m_data.bytes = new quint8[length];
    memcpy(const_cast<uint8_t*>(m_osdata->m_data.bytes),
           data + samplesPos,
           length - samplesPos);
    return true;
}

bool SoundFileLoader::Reset()
{
    if (!m_osdata) {
        ZCHX_LOGMSG("SoundFileLoader: Attempt to reset without Load().");
        return false;
    }
    m_next = 0;
    return true;
}


size_t SoundFileLoader::Get(void* buff, size_t length)
{
    size_t len = std::min(length, m_osdata->m_dataBytes - m_next);
    memcpy(buff, m_osdata->m_data.bytes + m_next, len);
    m_next += len;
    return len;
}

unsigned SoundFileLoader::GetBytesPerSample() const 
{
    return m_osdata->m_channels * m_osdata->m_bitsPerSample / 8;
};


unsigned SoundFileLoader::GetChannelCount() const
{
    return m_osdata->m_channels;
}

unsigned SoundFileLoader::GetSamplingRate() const
{

    return m_osdata->m_samplingRate;
}
