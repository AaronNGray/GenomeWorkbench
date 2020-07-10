#ifndef GUI_WIDGETS_TEXT_WIDGET___COUNT_CHARS_STREAM__HPP
#define GUI_WIDGETS_TEXT_WIDGET___COUNT_CHARS_STREAM__HPP

/*  $Id: count_chars_stream.hpp 23908 2011-06-21 15:40:48Z katargir $
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

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CCalcCharsOStream : public CStyledTextOStream
{
public:
    CCalcCharsOStream();

    virtual CStyledTextOStream& operator<< (const string& text);
    virtual int CurrentRowLength() const;

    virtual void SetStyle(const CTextStyle* WXUNUSED(style)) {}
    virtual void SetDefaultStyle() {}
    virtual void NewLine();

    size_t GetLineCount() const { return m_CharCount.size(); }
    size_t GetCharCount(size_t row = 0);

private:
    vector<size_t> m_CharCount;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___COUNT_CHARS_STREAM__HPP
