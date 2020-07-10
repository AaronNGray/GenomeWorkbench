#ifndef GUI_WIDGETS_TEXT_WIDGET___CALC_SIZE_STREAM__HPP
#define GUI_WIDGETS_TEXT_WIDGET___CALC_SIZE_STREAM__HPP

/*  $Id: calc_size_stream.hpp 19853 2009-08-13 18:27:23Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/widgets/text_widget/draw_styled_text.hpp>

BEGIN_NCBI_SCOPE

class CCalcSizeCache;

// The height calculated is not used currently
// The line height is assumed to be constant

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CCalcSizeOStream : public CBaseTextOStream
{
public:
    typedef CBaseTextOStream TParent;
    
    CCalcSizeOStream(wxDC& dc, CCalcSizeCache& cache, size_t rowHeight, size_t indentStep);

    virtual StreamHint GetHint() const { return kDisplay; }

    virtual CStyledTextOStream& operator<< (const string& text_in);

    virtual void SetStyle(const CTextStyle* style);
    virtual void SetDefaultStyle();

    virtual void Indent (size_t steps);
    virtual void NewLine();

    wxSize  GetSize() const;
    size_t  GetLineCount() const;

private:
    CCalcSizeCache& m_SizeCache;
    wxSize  m_Size;
    size_t  m_LineWidth;
    size_t  m_LineHeight;
    size_t  m_LineCount;

    const CTextStyle* m_CachedStyle;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___CALC_SIZE_STREAM__HPP
