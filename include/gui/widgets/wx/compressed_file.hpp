#ifndef GUI_WIDGETS_WX___COMPRESSED_FILE__HPP
#define GUI_WIDGETS_WX___COMPRESSED_FILE__HPP

/*  $Id: compressed_file.hpp 38076 2017-03-23 18:10:10Z evgeniev $
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
 * Authors:  Roman Katargin, Vladislav Evgeniev
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbifile.hpp>
#include <gui/gui_export.h>

#include <wx/string.h>

#include <util/compress/stream.hpp>
#include <util/format_guess.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CCompressedFile
{
public:
    CCompressedFile(const wxString& fileName);

    CNcbiIstream& GetIstream();

    CFormatGuess::EFormat GuessFormat();

    void Reset();

    static bool FileExists(const wxString &fileName);
protected:

    enum EType {
        eLocal      = 0x1,
        eNetwork    = 0x2,
        eCompressed = 0x4
    };

    enum EProtocol {
        eUnknown,
        eFile,
        eHTTP,
        eFTP
    };

protected:
    void x_AttachDecompressor();
    CNcbiIstream& x_DownloadFile(CNcbiIstream& remote);
    CFormatGuess::EFormat x_GuessFormat(CNcbiIstream &input, bool resetPosition = true) const;
    CFormatGuess::EFormat x_GuessFormatLocal();
    CFormatGuess::EFormat x_GuessFormatNetwork() const;
    EProtocol x_GetProtocol(const wxString& path) const;

protected:
    wxString                        m_FileName;
    EProtocol                       m_Protocol;
    unique_ptr<CNcbiIstream>        m_FileStream;
    CFormatGuess::EFormat           m_ContentFmt;
    CFormatGuess::EFormat           m_ArchiveFmt;
    unique_ptr<CCompressionIStream> m_Compressed;
    unique_ptr<CTmpFile>            m_TempFile;
    CNcbiIstream*                   m_Istream;
    uint8_t                         m_Type;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___COMPRESSED_FILE__HPP
