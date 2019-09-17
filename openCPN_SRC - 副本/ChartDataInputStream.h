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

#include "config.h"

#ifdef OCPN_USE_LZMA
#include "lzma.h"
#include "_def.h"
#include <QTemporaryFile>


// this implements a non-seekable input stream of xz compressed file
class wxCompressedFFileInputStream : public FileReadWrite
{
public:
    wxCompressedFFileInputStream(const QString& fileName);
    virtual ~wxCompressedFFileInputStream();

//    virtual bool IsOk() const { return FileReadWrite::isOK()/* && m_file->IsOpened()*/; }
    bool IsSeekable() const { return false; }

protected:
    size_t OnSysRead(void *buffer, size_t size);
    uint64_t OnSysSeek(uint64_t pos, FileReadWrite::FileSeekMode mode);
    uint64_t OnSysTell() const;

//    wxFFile *m_file;
    lzma_stream strm;

private:
    void init_lzma();

    uint8_t inbuf[BUFSIZ];

    Q_DISABLE_COPY(wxCompressedFFileInputStream)
};

// non-seekable stream for either non-compressed or compressed files
class ChartDataNonSeekableInputStream : public FileReadWrite
{
public:
    ChartDataNonSeekableInputStream(const QString& fileName);
    virtual ~ChartDataNonSeekableInputStream();

//    virtual bool IsOk() const { return m_stream->IsOk(); }
    bool IsSeekable() const { return false; }

protected:
    size_t OnSysRead(void *buffer, size_t size);
    uint64_t OnSysSeek(uint64_t pos, FileReadWrite::FileSeekMode mode);
    uint64_t OnSysTell() const;

private:
    FileReadWrite *m_stream;

    Q_DISABLE_COPY(ChartDataNonSeekableInputStream)
};


// seekable stream for either non-compressed or compressed files
// it must decompress the file to a temporary file to make it seekable
class ChartDataInputStream : public FileReadWrite
{
public:
    ChartDataInputStream(const QString& fileName);
    virtual ~ChartDataInputStream();

//    virtual bool IsOk() const { return m_stream->IsOk(); }
    bool IsSeekable() const { return /*m_stream->IsSeekable()*/true; }

    QString TempFileName() const { return m_tempfileName; }

protected:
    size_t OnSysRead(void *buffer, size_t size);
    uint64_t OnSysSeek(uint64_t pos, FileReadWrite::FileSeekMode mode);
    uint64_t OnSysTell() const;


private:
    QTemporaryFile  m_tempfile;
    QString         m_tempfileName;
    FileReadWrite *m_stream;

    Q_DISABLE_COPY(ChartDataInputStream)
};

#else

typedef wxFFileInputStream ChartDataInputStream;
typedef wxFFileInputStream ChartDataNonSeekableInputStream;

#endif // OCPN_USE_LZMA

bool DecompressXZFile(const QString &input_path, const QString &output_path);
