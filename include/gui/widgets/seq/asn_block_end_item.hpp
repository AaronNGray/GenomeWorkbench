#ifndef GUI_WIDGETS_SEQ___ASN_BLOCK_END_ITEM__HPP
#define GUI_WIDGETS_SEQ___ASN_BLOCK_END_ITEM__HPP

/*  $Id: asn_block_end_item.hpp 19315 2009-05-20 18:33:27Z katargir $
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

BEGIN_NCBI_SCOPE

class CStyledTextOStream;

class CAsnBlockEndItem : public CTextSelectItem
{
public:
    CAsnBlockEndItem(size_t indent, bool trailComma = true) :
        m_Indent(indent), m_TrailComma(trailComma) {}

    virtual int     GetIndent() const;

    void            SetTrailComma(bool fTrailComma) { m_TrailComma = fTrailComma; }

protected:
    virtual void    x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const;

private:
    size_t          m_Indent;
    bool            m_TrailComma;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___ASN_BLOCK_END_ITEM__HPP
