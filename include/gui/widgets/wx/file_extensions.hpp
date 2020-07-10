#ifndef GUI_WIDGETS_WX___FILE_EXTENSIONS__HPP
#define GUI_WIDGETS_WX___FILE_EXTENSIONS__HPP

/* $Id: file_extensions.hpp 44436 2019-12-18 17:13:59Z katargir $
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
 * Author:  Roman Katargin
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

/// Class CFileExtensions
/// Helper class to provide info on particular (biological content) file types
/// To be used in File dialogs

class NCBI_GUIWIDGETS_WX_EXPORT CFileExtensions
{
public:
    enum EFileType {
        kFileTypeStart,
        kAllFiles = kFileTypeStart,
        kASN,
        kFASTA,
        kGenBank,
        kAGP,
        kNewick,
        kNexus,
        kTree,
        kGFF,
        kGTF,
        kVCF,
        kBED,
        kWIG,
        kTxt,
        kTable,
        k5Column,
        kTextAlign,
        kPSL,
        kFileTypeEnd = kTextAlign
    };

    // File type label e.g. "NCBI ASN.1 Files" or "FASTA files"
    static wxString GetLabel(EFileType fileType);

    // File type extensions semicolon delimited e.g. "*.fa;*.mpfa;*.fna;*.fsa;*.fas;*.fasta"
    static wxString GetExtensions(EFileType fileType);

    // Returns the first file extension for the given type
    static wxString GetDefaultExtension(EFileType fileType);

    // Appends the default extension for the given file type to the path, if one is not already appended
    static void AppendDefaultExtension(EFileType fileType, wxString &path);

    // String to be used as extension filter in File dialog
    // e.g. "FASTA files (*.fa;*.mpfa;*.fna;*.fsa;*.fas;*.fasta)|*.fa;*.mpfa;*.fna;*.fsa;*.fas;*.fasta"
    static wxString GetDialogFilter(EFileType fileType);

    static bool RecognizeExtension(EFileType fileType, const wxString& extension);
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___FILE_EXTENSIONS__HPP
