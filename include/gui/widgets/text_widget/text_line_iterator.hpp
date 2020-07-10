#ifndef GUI_WIDGETS_TEXT_WIDGET___TEXT_LINE_ITERATOR__HPP
#define GUI_WIDGETS_TEXT_WIDGET___TEXT_LINE_ITERATOR__HPP

/*  $Id: text_line_iterator.hpp 27273 2013-01-18 18:33:40Z katargir $
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
#include <gui/widgets/text_widget/text_position.hpp>

BEGIN_NCBI_SCOPE

class ITextItem;
class CCompositeTextItem;
class CExpandItem;
class CTextItemPanel;

class NCBI_GUIWIDGETS_TEXT_WIDGET_EXPORT CTextLineIterator
{
public:
    CTextLineIterator(CTextItemPanel* panel, const CTextPosition& pos, bool forward);

    CTextLineIterator& operator++ (void); // prefix
    string operator*  (void) const;

    CTextPosition GetPosition() const;

    operator bool() const;

private:
    void x_Next(void);
    bool x_Valid(void) const;

    CTextItemPanel* m_Panel;
    CTextPosition   m_StartPos;
    bool            m_Forward;
    int             m_CurLine;
    string          m_Line;
    bool            m_Begin;
    bool            m_End;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___TEXT_LINE_ITERATOR__HPP
