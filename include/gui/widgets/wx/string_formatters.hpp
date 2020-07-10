#ifndef GUI_WIDGETS_WX___STRING_FORMATTERS__HPP
#define GUI_WIDGETS_WX___STRING_FORMATTERS__HPP

/*  $Id: string_formatters.hpp 30889 2014-08-01 18:04:11Z ucko $
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
 * Authors: Yury Voronov
 *
 * File Description:
 *      
 */

#include <gui/gui.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/string.h>
#include <wx/variant.h>

#include <corelib/ncbiobj.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT IwxStringFormat
{
public:
    virtual ~IwxStringFormat() {}

    virtual wxString Format( wxVariant& aVar ) const;
    virtual void Format( wxVariant& aVar, wxString& aStr, bool afAppend = false ) const = 0;
};

class NCBI_GUIWIDGETS_WX_EXPORT CwxIntPrintfFormat
    : public CObject
    , public IwxStringFormat
{
    wxString m_Format;

public:
    CwxIntPrintfFormat( wxString aFormat ) : m_Format( aFormat ) {}
    virtual void Format( wxVariant& aVar, wxString& aStr, bool afAppend = false ) const;
};

class NCBI_GUIWIDGETS_WX_EXPORT CwxDoublePrintfFormat
    : public CObject
    , public IwxStringFormat
{
    wxString m_Format;

public:
    CwxDoublePrintfFormat( wxString aFormat ) : m_Format( aFormat ) {}
    virtual void Format( wxVariant& aVar, wxString& aStr, bool afAppend = false ) const;
};

class NCBI_GUIWIDGETS_WX_EXPORT CwxIntWithFlagsFormat
    : public CObject
    , public IwxStringFormat
{
    int mf_Flags;

public:
    CwxIntWithFlagsFormat( int aFlags ) : mf_Flags( aFlags ) {}
    virtual void Format( wxVariant& aVar, wxString& aStr, bool afAppend = false ) const;
};

END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___STRING_FORMATTERS__HPP
