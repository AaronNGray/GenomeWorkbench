#ifndef GUI_WIDGETS_SEQ___XML_VALUE_ITEM__HPP
#define GUI_WIDGETS_SEQ___XML_VALUE_ITEM__HPP

/*  $Id: xml_value_item.hpp 19246 2009-05-06 19:02:00Z katargir $
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

#include <gui/widgets/seq/xml_base_item.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_SEQ_EXPORT CXmlValueItem : public CXmlBaseItem
{
public:
    CXmlValueItem(size_t indent, const string& tag, CConstObjectInfo& objInfo)
        : CXmlBaseItem(indent, tag), m_ObjectInfo(objInfo) {}

protected:
    virtual void  x_RenderXml(CStyledTextOStream& ostream, CTextPanelContext* context) const;

    void x_WriteString(CStyledTextOStream& ostream, const string& str) const;
    void x_WriteOctetString(CStyledTextOStream& ostream, const vector<char>& value) const;

    CConstObjectInfo m_ObjectInfo;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___XML_VALUE_ITEM__HPP
