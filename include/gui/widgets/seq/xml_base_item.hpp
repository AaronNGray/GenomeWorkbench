#ifndef GUI_WIDGETS_SEQ___XML_BASE_ITEM__HPP
#define GUI_WIDGETS_SEQ___XML_BASE_ITEM__HPP

/*  $Id: xml_base_item.hpp 29707 2014-01-31 16:59:45Z katargir $
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
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <gui/widgets/seq/xml_styles.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_SEQ_EXPORT CXmlBaseItem : public CTextSelectItem
{
public:
    CXmlBaseItem(size_t indent, const string& tag)
        : m_Indent(indent), m_Tag(tag) {}

    virtual int     GetIndent() const;

protected:
    virtual void    x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const;

    virtual void    x_RenderXml(CStyledTextOStream& ostream, CTextPanelContext* context) const = 0;

    const size_t m_Indent;
    const string m_Tag;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___XML_BASE_ITEM__HPP
