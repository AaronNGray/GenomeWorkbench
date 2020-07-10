/*  $Id: compressed_file.cpp 38102 2017-03-28 20:34:22Z evgeniev $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Roman Katargin, Vladislav Evgeniev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/compressed_file.hpp>

#include <wx/filename.h>

#include <corelib/ncbiexpt.hpp>
#include <corelib/ncbi_url.hpp>
#include <connect/ncbi_conn_stream.hpp>
#include <util/compress/zlib.hpp>
#include <util/compress/bzip2.hpp>

BEGIN_NCBI_SCOPE

CCompressedFile::CCompressedFile(const wxString& fileName)
    : m_FileName(fileName)
    , m_ContentFmt(CFormatGuess::eUnknown)
    , m_ArchiveFmt(CFormatGuess::eUnknown)
    , m_Istream(nullptr)
    , m_Type(0)
{
    m_Protocol = x_GetProtocol(fileName);
    switch (m_Protocol) {
    case eUnknown:
    {
        string msg("Unsupported protocol: ");
        msg += (const char*) fileName.ToUTF8();
        NCBI_THROW(CCoreException, eInvalidArg, msg);
    }
        break;
    case eFile:
        m_FileStream.reset(new CNcbiIfstream(fileName.fn_str(), ios::binary));
        m_Type |= eLocal;
        break;
    case eHTTP:
        m_FileStream.reset(new CConn_HttpStream((const char*)m_FileName.ToUTF8()));
        m_Type |= eNetwork;
        break;
    case eFTP:
    {
        CUrl url((const char*)fileName.ToUTF8());
        m_FileStream.reset(new CConn_FTPDownloadStream(url.GetHost(), url.GetPath()));
        m_Type |= eNetwork;
    }
    break;
    }

    CFormatGuess guess(*m_FileStream.get());
    guess.GetFormatHints().AddPreferredFormat(CFormatGuess::eGZip).AddPreferredFormat(CFormatGuess::eBZip2);
    m_ArchiveFmt = guess.GuessFormat();
    m_ContentFmt = m_ArchiveFmt;
    if (m_Type & eLocal) {
        m_FileStream->clear();
        m_FileStream->seekg(0, ios_base::beg);
    }
}

CNcbiIstream& CCompressedFile::GetIstream()
{
    _ASSERT(m_FileStream);
    if (m_Istream)
        return *m_Istream;

    x_AttachDecompressor();
   
    if (m_Type & eNetwork) { // Download the file to a temp location for the purpose of seeking
        if (m_Compressed)
            m_Istream = &(x_DownloadFile(*m_Compressed.get()));
        else
            m_Istream = &(x_DownloadFile(*m_FileStream.get()));
    }

    return *m_Istream;
}

CFormatGuess::EFormat CCompressedFile::GuessFormat()
{
    switch (m_ContentFmt)
    {
        case CFormatGuess::eUnknown:
        case CFormatGuess::eGZip:
        case CFormatGuess::eBZip2:
        {
            if (m_Type & eLocal)
                m_ContentFmt = x_GuessFormatLocal();
            else
                m_ContentFmt = x_GuessFormatNetwork();
            break;
        }
        default:
            break;
    }
    return m_ContentFmt;
}

void CCompressedFile::Reset()
{
    _ASSERT(m_FileStream);
    if (m_Type & eLocal) {
        m_Istream = 0;
        m_Compressed.reset();
        m_FileStream->clear();
        m_FileStream->seekg(0, ios_base::beg);
    }
    if (m_TempFile && (nullptr != m_Istream)) {
        m_Istream->clear();
        m_Istream->seekg(0, ios_base::beg);
    }
}

bool CCompressedFile::FileExists(const wxString &fileName)
{
    CUrl url((const char*)fileName.ToUTF8());
    string protocol = url.GetScheme();
    if (protocol.empty() || url.GetHost().empty()) {
        return wxFileName::FileExists(fileName);
    }
    else if ((0 == NStr::CompareNocase(protocol.c_str(), "http")) || (0 == NStr::CompareNocase(protocol.c_str(), "https")) || (0 == NStr::CompareNocase(protocol.c_str(), "ftp"))) {
        return true;
    }
    return false;
}

void CCompressedFile::x_AttachDecompressor()
{
    m_Istream = m_FileStream.get();

    switch (m_ArchiveFmt) {
    case CFormatGuess::eGZip:
        m_Compressed.reset(new CCompressionIStream(*m_Istream,
            new CZipStreamDecompressor(CZipCompression::fGZip), CCompressionStream::fOwnProcessor));
        m_Istream = m_Compressed.get();
        m_Type |= eCompressed;
        break;
    case CFormatGuess::eBZip2:
        m_Compressed.reset(new CCompressionIStream(*m_Istream,
            new CBZip2StreamDecompressor(), CCompressionStream::fOwnProcessor));
        m_Istream = m_Compressed.get();
        m_Type |= eCompressed;
        break;
    default:
        break;
    }
}

CNcbiIstream& CCompressedFile::x_DownloadFile(CNcbiIstream& remote)
{
    m_TempFile.reset(new CTmpFile());
    CNcbiOstream &temp = m_TempFile->AsOutputFile(CTmpFile::eIfExists_Reset, IOS_BASE::binary);

    NcbiStreamCopy(temp, remote);

    CNcbiIstream &istream = m_TempFile->AsInputFile(CTmpFile::eIfExists_Reset, IOS_BASE::binary);
    return istream;
}

CFormatGuess::EFormat CCompressedFile::x_GuessFormat(CNcbiIstream &input, bool resetPosition) const
{
    CFormatGuess guess(input);
    CFormatGuess::EFormat result = guess.GuessFormat();
    if (resetPosition) {
        input.clear();
        input.seekg(0, ios_base::beg);
    }
    return result;
}

CFormatGuess::EFormat CCompressedFile::x_GuessFormatLocal()
{
    if (!m_Compressed) 
        x_AttachDecompressor();
    
    if (!m_Compressed)
        return CFormatGuess::eUnknown;

    return x_GuessFormat(*m_Compressed.get());
}

CFormatGuess::EFormat CCompressedFile::x_GuessFormatNetwork() const
{
    unique_ptr<CNcbiIstream> fileStream;
    switch (m_Protocol) {
    case eUnknown:
    {
        string msg("Unsupported protocol: ");
        msg += (const char*) m_FileName.ToUTF8();
        NCBI_THROW(CCoreException, eInvalidArg, msg);
    }
    break;
    case eFile:
        fileStream.reset(new CNcbiIfstream(m_FileName.fn_str(), ios::binary));
        break;
    case eHTTP:
        fileStream.reset(new CConn_HttpStream((const char*)m_FileName.ToUTF8()));
        break;
    case eFTP:
    {
        CUrl url((const char*)m_FileName.ToUTF8());
        fileStream.reset(new CConn_FTPDownloadStream(url.GetHost(), url.GetPath()));
    }
    break;
    }

    CFormatGuess guess(*fileStream.get());
    guess.GetFormatHints().AddPreferredFormat(CFormatGuess::eGZip).AddPreferredFormat(CFormatGuess::eBZip2);
    CFormatGuess::EFormat fmt = guess.GuessFormat();
    
    unique_ptr<CNcbiIstream> compressedStream;
    switch (fmt) {
    case CFormatGuess::eGZip:
        compressedStream.reset(new CCompressionIStream(*fileStream.get(),
            new CZipStreamDecompressor(CZipCompression::fGZip), CCompressionStream::fOwnProcessor));
        return x_GuessFormat(*compressedStream.get(), false);
        break;
    case CFormatGuess::eBZip2:
        compressedStream.reset(new CCompressionIStream(*fileStream.get(),
            new CBZip2StreamDecompressor(), CCompressionStream::fOwnProcessor));
        return x_GuessFormat(*compressedStream.get(), false);
        break;
    default:
        break;
    }
    return fmt;
}

CCompressedFile::EProtocol CCompressedFile::x_GetProtocol(const wxString& path) const
{
    CUrl url((const char*)m_FileName.ToUTF8());
    string protocol = url.GetScheme();
    if (protocol.empty() || url.GetHost().empty()) {
        return eFile;
    }
    else if ((0 == NStr::CompareNocase(protocol.c_str(), "http")) || (0 == NStr::CompareNocase(protocol.c_str(), "https"))) {
        return eHTTP;
    }
    else if (0 == NStr::CompareNocase(protocol.c_str(), "ftp")) {
        return eFTP;
    }
    return eUnknown;
}

END_NCBI_SCOPE
