#ifndef GUI_OBJUTILS___MACRO_FN_WHERE__HPP
#define GUI_OBJUTILS___MACRO_FN_WHERE__HPP
/*  $Id: macro_fn_where.hpp 45077 2020-05-22 23:43:56Z rudnev $
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
 * Authors: Anatoly Osipov, Andrea Asztalos
 *
 * File Description: Macro functions used in the WHERE clause
 *
 */

/// @file macro_fn_where.hpp

#include <gui/objutils/macro_edit_fn_base.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_ChoiceType)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_Features_For_Object)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RelatedFeatures)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_CDSTranslation)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_SeqID)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_Accession)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_InconsistentTaxa)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_StringLength)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_StructCommField)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_StructCommDatabase)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_StructCommFieldname)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_GetDBLink)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_InTable)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_Contained)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_IllegalDbXref)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_StructVoucherPart)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_GeneType)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_Label)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_VariationType)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RefAllele)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_AltAlleles)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_Consequence)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_SnpID)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_VcfSevenCol)

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_IsPresent : public IEditMacroFunction
{
public:
    CMacroFunction_IsPresent(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
    void x_IsSimpleTypePresent(const string& field_name);
    void x_IsContainerElementPresent(const string& container, const string& field_name);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_GetSeqdesc : public IEditMacroFunction
{
public:
    CMacroFunction_GetSeqdesc(EScopeEnum func_scope, objects::CSeqdesc::E_Choice type)
        : IEditMacroFunction(func_scope), m_SeqdescType(type) {}

    virtual void TheFunction();
    static const char* sm_BsrcForMolinfo;
    static const char* sm_MolinfoForBsrc;
    static const char* sm_BsrcForSeq;
    static const char* sm_BsrcForFeat;
    static const char* sm_MolinfoForFeat;
    static const char* sm_BsrcForSeqdesc;

protected:
    virtual bool x_ValidArguments() const;
    objects::CSeqdesc::E_Choice m_SeqdescType;
};


class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_GetSequence : public IEditMacroFunction
{
public:
    CMacroFunction_GetSequence(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual void TheFunction();
    static const char* sm_SeqForDescr;
    static const char* sm_SeqForFeat;

protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FirstOrLastItem  : public IEditMacroFunction
{
public:
    CMacroFunction_FirstOrLastItem(EScopeEnum func_scope, bool first_item)
        : IEditMacroFunction(func_scope), m_First(first_item) {}

    virtual void TheFunction();
    static const char* sm_First;
    static const char* sm_Last;

protected:
    virtual bool x_ValidArguments() const;
    bool m_First;
};


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_FN_WHERE__HPP
