#ifndef GUI_WIDGETS_TEXT_WIDGET___CALC_NBPOS__HPP
#define GUI_WIDGETS_TEXT_WIDGET___CALC_NBPOS__HPP

/*  $Id: calc_nbpos_stream.hpp 23907 2011-06-21 15:13:43Z katargir $
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

// The height calculated is not used currently
// The line height is assumed to be constant

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CCalcNBPosOStream : public CStyledTextOStream
{
public:
    typedef CStyledTextOStream TParent;
    
    CCalcNBPosOStream();

    virtual StreamHint GetHint() const { return kDisplay; }
    virtual CStyledTextOStream& operator<< (const string& text);
    virtual int CurrentRowLength() const;

    virtual void SetStyle(const CTextStyle* WXUNUSED(style)) {}
    virtual void SetDefaultStyle() {}
    virtual void Indent (size_t steps);
    virtual void NewLine();
    virtual void NBtoggle();

    void GetNBPos(int& start, int& end, int& row) const;

private:
    int    m_Row;
    int    m_CurrentLineLength;
    bool   m_NB;
    int    m_NBStartChar;
    int    m_NBEndChar;
    int    m_NBRow;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___CALC_NBPOS__HPP
