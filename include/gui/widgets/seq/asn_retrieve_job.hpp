#ifndef GUI_WIDGETS_SEQ___ASN_RETRIEVE_JOB__HPP
#define GUI_WIDGETS_SEQ___ASN_RETRIEVE_JOB__HPP

/*  $Id: asn_retrieve_job.hpp 44629 2020-02-10 18:30:10Z asztalos $
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

#include <gui/widgets/seq/text_retrieve_job.hpp>

BEGIN_NCBI_SCOPE

class ITextItem;
class CAsnBioContext;
class CAsnBioContextContainer;

class CClassTypeInfo;
class CContainerTypeInfo;

class NCBI_GUIWIDGETS_SEQ_EXPORT CAsnRetrieveJob :
        public CTextRetrieveJob
{
public:
    CAsnRetrieveJob(CTextPanelContext& context, const CSerialObject& so, objects::CScope& scope);
    virtual ~CAsnRetrieveJob();

    /// @name IAppJob implementation
    /// @{
    virtual string GetDescr() const { return "Retrieve ASN data for text view"; }
    /// @}

protected:
    virtual string x_GetJobName() const { return "AsnRetrieveJob"; }
    virtual EJobState x_Run();

private:
    virtual void x_FillBlock(
        CCompositeTextItem* content,
        TTypeInfo containerType,
        TConstObjectPtr containerPtr,
        size_t indent, size_t maxIndent);
    virtual void x_FillClass(
        CCompositeTextItem* content,
        const CClassTypeInfo* classType,
        TConstObjectPtr classPtr,
        size_t indent, size_t maxIndent);
    virtual void x_FillContainer(
        CCompositeTextItem* content,
        const CContainerTypeInfo* type,
        TConstObjectPtr ptr,
        size_t indent, size_t maxIndent);

    static void x_SetNoComma(CCompositeTextItem* block);
    static void x_GetMemberObject(TTypeInfo& type, TConstObjectPtr& ptr);

    const CSerialObject& m_SO;
    CRef<objects::CScope> m_Scope;
    CAsnBioContextContainer* m_ContextContainer;
    CAsnBioContext*  m_BioContext;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___ASN_RETRIEVE_JOB__HPP
