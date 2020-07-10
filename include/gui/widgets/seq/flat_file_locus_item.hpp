#ifndef GUI_WIDGETS_SEQ___FLAT_FILE_LOCUS_ITEM__HPP
#define GUI_WIDGETS_SEQ___FLAT_FILE_LOCUS_ITEM__HPP

/*  $Id: flat_file_locus_item.hpp 32643 2015-04-06 20:20:36Z katargir $
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

#include <gui/widgets/seq/flat_file_text_item.hpp>
#include <gui/widgets/text_widget/hyperlink_glyph.hpp>

#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects) // namespace ncbi::objects::
class IFlatItem;
END_SCOPE(objects)

class CHyperlinkGlyph;

class CFlatFileLocusItem : public CFlatFileTextItem, public IHyperlinkClick
{
public:
    CFlatFileLocusItem(CConstRef<objects::IFlatItem> item) :
      CFlatFileTextItem(item), m_HyperlinkGlyph() {}
    virtual ~CFlatFileLocusItem();

    virtual void MouseEvent(wxMouseEvent& event, CTextPanelContext& context);

    virtual void    CalcSize(wxDC& dc, CTextPanelContext* context);
    virtual void    Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context);

	objects::CBioseq_Handle	GetBioseqHandle();
    SConstScopedObject GetSeqLoc();


    /// @name IHyperlinkClick  interface implementation
    /// @{
    virtual void OnHyperlinkClick(CTextPanelContext* context);
    /// @}

protected:
    CHyperlinkGlyph* m_HyperlinkGlyph;
    SConstScopedObject m_SeqLoc;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FLAT_FILE_LOCUS_ITEM__HPP
