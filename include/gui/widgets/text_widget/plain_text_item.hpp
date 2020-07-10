#ifndef GUI_WIDGETS_TEXT_WIDGET___PLAIN_TEXT_ITEM__HPP
#define GUI_WIDGETS_TEXT_WIDGET___PLAIN_TEXT_ITEM__HPP

/*  $Id: plain_text_item.hpp 34288 2015-12-08 22:53:38Z katargir $
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

#include <gui/widgets/text_widget/text_select_item.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>

BEGIN_NCBI_SCOPE

class CStyledTextOStream;

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CPlainTextItem : public CTextSelectItem
{
public:
    CPlainTextItem();

    void            AddLine(const string& line) { m_Text.push_back(line); }

    void            SetStyle(const CTextStyle& style) { m_Style = style; }

    void SetIndent(int indent) { m_Indent = indent; }
    virtual int GetIndent() const { return m_Indent; }

protected:
    virtual void    x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const;

private:
    list<string>    m_Text;
    CTextStyle      m_Style;

    int m_Indent;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___PLAIN_TEXT_ITEM__HPP
