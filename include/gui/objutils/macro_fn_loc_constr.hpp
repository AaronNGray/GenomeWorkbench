#ifndef GUI_OBJUTILS___MACRO_FN_LOC_CONSTR__HPP
#define GUI_OBJUTILS___MACRO_FN_LOC_CONSTR__HPP
/*  $Id: macro_fn_loc_constr.hpp 44446 2019-12-19 16:23:05Z asztalos $
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
 * Authors: Andrea Asztalos
 *
 * File Description: 
 * Macro constraint functions related to locations
 *
 */

/// @file macro_fn_loc_constr.hpp

#include <objects/macro/Location_constraint.hpp>
#include <objects/macro/Location_pos_constraint.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_LocationStrand)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_LocationStrandSymbol)

enum class ELocEndType { eStart, eStop };

class CMacroFunction_LocPartialTest : public IEditMacroFunction
{
public:
    CMacroFunction_LocPartialTest(EScopeEnum func_scope, ELocEndType testcase)
        : IEditMacroFunction(func_scope), m_TestCase(testcase) {}

    virtual void TheFunction();
    static const char* sm_PartialStart;
    static const char* sm_PartialStop;
protected:
    virtual bool x_ValidArguments() const;
    ELocEndType m_TestCase;
};

class CMacroFunction_LocEnd : public IEditMacroFunction
{
public:
    CMacroFunction_LocEnd(EScopeEnum func_scope, ELocEndType testcase)
        : IEditMacroFunction(func_scope), m_TestCase(testcase) {}

    virtual void TheFunction();
    static const char* sm_Start;
    static const char* sm_Stop;
protected:
    virtual bool x_ValidArguments() const;
    ELocEndType m_TestCase;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_LocationType : public IEditMacroFunction
{
public:
    CMacroFunction_LocationType(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
     
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
    bool x_Match(const objects::CSeq_loc& loc, const objects::CLocation_constraint& loc_cons);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_LocationSeqType : public IEditMacroFunction
{
public:
    CMacroFunction_LocationSeqType(EScopeEnum func_scope,
        const objects::ESeqtype_constraint& seq_type = objects::eSeqtype_constraint_any)
        : IEditMacroFunction(func_scope), m_Seqtype(seq_type) {}

    virtual void TheFunction();
    static objects::CSeq_loc* s_GetLocation(const string& field_name, CIRef<IMacroBioDataIter> iter);
    static const char* sm_SeqNa;
    static const char* sm_SeqAa;

protected:
    virtual bool x_ValidArguments() const;
    objects::ESeqtype_constraint m_Seqtype;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_LocationDistConstraint : public IEditMacroFunction
{
public:
    CMacroFunction_LocationDistConstraint(EScopeEnum func_scope, ELocEndType testcase)
        : IEditMacroFunction(func_scope), m_TestCase(testcase) {}

    virtual void TheFunction();
    static const char* sm_FromStart;
    static const char* sm_FromStop;
protected:
    virtual bool x_ValidArguments() const;
    ELocEndType m_TestCase;

};

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_FN_LOC_CONSTR__HPP
