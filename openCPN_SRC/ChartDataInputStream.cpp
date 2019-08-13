/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Support XZ compressed charts
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2016 by David S. Register                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 */

// For compilers that support precompilation, includes "wx.h".

#include "config.h"
#include "ChartDataInputStream.h"

#ifdef OCPN_USE_LZMA

wxCompressedFFileInputStream::wxCompressedFFileInputStream(const QString& fileName) : FileReadWrite(fileName, FileReadWrite::E_READ)
{
    init_lzma();
//    m_file = new wxFFile(fileName, "rb");
}

wxCompressedFFileInputStream::~wxCompressedFFileInputStream()
{
//    delete m_file;
    lzma_end(&strm);
}

size_t wxCompressedFFileInputStream::OnSysRead(void *buffer, size_t size)
{
    lzma_action action = LZMA_RUN;

    strm.next_out = (uint8_t*)buffer;
    strm.avail_out = size;
    
    for(;;) {
        if (strm.avail_in == 0)
        {
            if(!IsEof()) {
                strm.next_in = inbuf;
                strm.avail_in = Read(inbuf, sizeof inbuf);
                if(HasError()) return 0;
                
            } else
            {
                action = LZMA_FINISH;
            }
        }
        
        lzma_ret ret = lzma_code(&strm, action);
        
        if (strm.avail_out == 0 || ret == LZMA_STREAM_END)
            return size - strm.avail_out;
        
        if(ret != LZMA_OK) {
//            m_lasterror = wxSTREAM_READ_ERROR;
            return 0;
        }
    }
    return 0;
}

uint64_t wxCompressedFFileInputStream::OnSysSeek(uint64_t pos, FileReadWrite::FileSeekMode mode)
{
    // rewind to start is possible
    if(pos == 0 && mode == FileReadWrite::SEEK_FROM_START) {
        lzma_end(&strm);
        init_lzma();
        return Seek(pos, mode);
    }
    
    return 0;
}

uint64_t wxCompressedFFileInputStream::OnSysTell() const
{
    return strm.total_out;
}

void wxCompressedFFileInputStream::init_lzma()
{
    lzma_stream s = LZMA_STREAM_INIT;
    memcpy(&strm, &s, sizeof s);
    lzma_ret ret = lzma_stream_decoder(
        &strm, UINT64_MAX, LZMA_CONCATENATED);

//    if(ret != LZMA_OK)
//        m_lasterror = wxSTREAM_READ_ERROR;
}



ChartDataNonSeekableInputStream::ChartDataNonSeekableInputStream(const QString& fileName)
{
    if(fileName.toUpper().endsWith("XZ"))
        m_stream = new wxCompressedFFileInputStream(fileName);
    else
        m_stream = new FileReadWrite(fileName, FileReadWrite::E_READ);
}

ChartDataNonSeekableInputStream::~ChartDataNonSeekableInputStream()
{
    delete m_stream;
}

size_t ChartDataNonSeekableInputStream::OnSysRead(void *buffer, size_t size)
{
    m_stream->Read(buffer, size);
    return m_stream->LastRead();
}

uint64_t ChartDataNonSeekableInputStream::OnSysSeek(uint64_t pos, FileReadWrite::FileSeekMode mode)
{
    return m_stream->Seek(pos, mode);
}

uint64_t ChartDataNonSeekableInputStream::OnSysTell() const
{
    return m_stream->TellI();
}



ChartDataInputStream::ChartDataInputStream(const QString& fileName)
{
    if(fileName.toUpper().endsWith("XZ")) {
        // decompress to temp file to allow seeking
        m_tempfile.setFileTemplate(QFileInfo(fileName).fileName());
        if(m_tempfile.open()) m_tempfileName = m_tempfile.fileName();
        wxCompressedFFileInputStream stream(fileName);
        FileReadWrite tmp(m_tempfileName);

        char buffer[8192];
        int len;
        do {
            stream.Read(buffer, sizeof buffer);
            len = stream.LastRead();
            tmp.Write(buffer, len);
        } while(len == sizeof buffer);

        // do some error checking here?

        tmp.Close();
        m_stream = new FileReadWrite(m_tempfileName);
    } else
        m_stream = new FileReadWrite(fileName);
}

ChartDataInputStream::~ChartDataInputStream()
{
    // close it
    delete m_stream;
    // delete the temp file, how do we remove temp files if the program crashed?
//    if(!m_tempfilename.empty())
//        wxRemoveFile(m_tempfilename);
}

size_t ChartDataInputStream::OnSysRead(void *buffer, size_t size)
{
    m_stream->Read(buffer, size);
    return m_stream->LastRead();
}

uint64_t ChartDataInputStream::OnSysSeek(uint64_t pos, FileReadWrite::FileSeekMode mode)
{
    return m_stream->Seek(pos, mode);
}

uint64_t ChartDataInputStream::OnSysTell() const {
    return m_stream->TellI();
}

bool DecompressXZFile(const QString &input_path, const QString &output_path)
{
    if (!QFile::exists(input_path)) {
        return false;
    }
    wxCompressedFFileInputStream in(input_path);
    FileReadWrite out(output_path, FileReadWrite::E_Write);
    
    char buffer[8192];
    int len;
    do {
        in.Read(buffer, sizeof(buffer));
        len = in.LastRead();
        out.Write(buffer, len);
    } while(len == sizeof(buffer));

    return !in.HasError();
}

#else // OCPN_USE_LZMA

bool DecompressXZFile(const QString &input_path, const QString &output_path)
{
    ZCHX_LOGMSG(_T("Failed to decompress: ") + input_path);
    ZCHX_LOGMSG(_T("OpenCPN compiled without liblzma support"));
                 
    return false;
}

#endif // OCPN_USE_LZMA
