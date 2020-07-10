#ifndef GUI_WIDGETS_SEQ___ASN_ELEMENT_BLOCK_COLLAPSED__HPP
#define GUI_WIDGETS_SEQ___ASN_ELEMENT_BLOCK_COLLAPSED__HPP

/*  $Id: asn_element_block_collapsed.hpp 25759 2012-05-03 20:40:31Z katargir $
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

#include <serial/serialdef.hpp>

#include <gui/widgets/seq/asn_element_item.hpp>

BEGIN_NCBI_SCOPE

class CMemberId;
class CStyledTextOStream;

class CAsnElementBlockCollapsed : public CAsnElementItem
{
public:
    CAsnElementBlockCollapsed(
                    const CMemberId& memberId,
                    TTypeInfo type,
                    TConstObjectPtr ptr,
                    size_t indent,
                    CAsnBioContext* bioContext) :
        CAsnElementItem(memberId, type, ptr, indent, bioContext), m_ExpandedValue(0) {}

    CAsnElementBlockCollapsed(const CAsnElementItem& asnBlock) :
            CAsnElementItem(asnBlock), m_ExpandedValue(0) {}

    void SetExpandedValue(ITextItem* value) { m_ExpandedValue = value; }

    // Expand object will handle selection
    virtual bool    IsSelectable() const { return false; }

    virtual void TextPosToExpanded(CTextPosition& pos, CTextPanelContext* context) const;
    virtual void TextPosToCollapsed(CTextPosition& pos, CTextPanelContext* context) const;

protected:
    virtual void    RenderValue(CStyledTextOStream& ostream, CTextPanelContext* context,
                                TTypeInfo type, TConstObjectPtr ptr) const;

    ITextItem* m_ExpandedValue;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___ASN_ELEMENT_BLOCK_COLLAPSED__HPP
