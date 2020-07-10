#ifndef GUI_WIDGETS_SEQ___EDIT_TB__HPP
#define GUI_WIDGETS_SEQ___EDIT_TB__HPP

/*  $Id: edit_tb.hpp 39803 2017-11-07 15:36:58Z katargir $
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

#include <gui/widgets/text_widget/glyph.hpp>

BEGIN_NCBI_SCOPE

class ITextItem;

class CEditToolbar : public CGlyph
{
public:
    CEditToolbar(ITextItem* item, int editFlags);

    // Mouse handling
    virtual void MouseEvent(wxMouseEvent& event);

    virtual void    Draw(wxDC& dc, CTextPanelContext* context);
    virtual void    CalcSize(wxDC& /*dc*/, CTextPanelContext* /*context*/) {}

private:
    void MouseMove(wxMouseEvent& event);
    void MouseLeftUp(wxMouseEvent& event);

    ITextItem* m_Item;
    int        m_EditFlags;

    static wxBitmap m_EditBmp;
    static wxBitmap m_DeleteBmp;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___EDIT_TB__HPP
