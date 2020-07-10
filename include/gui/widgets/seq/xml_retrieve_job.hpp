#ifndef GUI_WIDGETS_SEQ___XML_RETRIEVE_JOB__HPP
#define GUI_WIDGETS_SEQ___XML_RETRIEVE_JOB__HPP

/*  $Id: xml_retrieve_job.hpp 44629 2020-02-10 18:30:10Z asztalos $
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
#include <corelib/ncbimtx.hpp>
#include <gui/gui_export.h>

#include <stack>

#include <gui/utils/app_job.hpp>

#include <gui/widgets/seq/text_retrieve_job.hpp>

BEGIN_NCBI_SCOPE

class ITextItem;

class CClassTypeInfo;
class CContainerTypeInfo;

class NCBI_GUIWIDGETS_SEQ_EXPORT CXmlRetrieveJob :
    public CTextRetrieveJob
{
public:
    CXmlRetrieveJob(CTextPanelContext& context, const CSerialObject& so);
    virtual ~CXmlRetrieveJob();

    /// @name IAppJob implementation
    /// @{
    virtual string GetDescr() const { return "Retrieve XML data for text view"; }
    /// @}

protected:
    virtual string x_GetJobName() const { return "XmlRetrieveJob"; }
    virtual EJobState x_Run();

private:

    void x_Object(
        CCompositeTextItem* content,
        CConstObjectInfo objInfo,
        size_t indent);
    void x_Container(
        CCompositeTextItem* content,
        CConstObjectInfo objInfo,
        size_t indent);

    void x_Choice(
        CCompositeTextItem* content,
        CConstObjectInfo objInfo,
        size_t indent);

    void x_ChoiceMember(
        CCompositeTextItem* content,
        CConstObjectInfo objInfo,
        size_t indent);

    void x_Class(
        CCompositeTextItem* content,
        CConstObjectInfo objInfo,
        size_t indent);

    void x_ClassMembers(
        CCompositeTextItem* content,
        CConstObjectInfo objInfo,
        size_t indent);

    void x_Primitive(
        CCompositeTextItem* content,
        CConstObjectInfo objInfo,
        size_t indent);

    stack<string> m_TagStack;
    const CSerialObject& m_SO;
    const size_t m_OpenIndent;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___XML_RETRIEVE_JOB__HPP
