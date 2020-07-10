#ifndef GUI_OBJUTILS___MACRO_FN_STRING_CONSTR__HPP
#define GUI_OBJUTILS___MACRO_FN_STRING_CONSTR__HPP
/*  $Id: macro_fn_string_constr.hpp 40255 2018-01-17 21:51:10Z asztalos $
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
 * File Description: Macro functions implementing string constraints
 *
 */

/// @file macro_fn_string_constr.hpp
#include <objects/macro/String_constraint.hpp>
#include <objects/macro/Word_substitution.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>
/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_StringConstraints : public IEditMacroFunction
{
public:
    enum EStrConstrCase {
        eNotSet,
        eUpper,
        eLower,
        ePunct
    };

    enum ELogicType {
        eOR,
        eAND
    };

    static const string sm_syn;
    CMacroFunction_StringConstraints(EScopeEnum func_scope,
        const string& type, EStrConstrCase testcase = eNotSet, ELogicType op_type = eOR);

    virtual ~CMacroFunction_StringConstraints() {}
    virtual void TheFunction();

    static const char* sm_Upper;
    static const char* sm_UpperAll;
    static const char* sm_Lower;
    static const char* sm_LowerAll;
    static const char* sm_Punct;
    static const char* sm_PunctAll;
    static const char* sm_Start;
    static const char* sm_End;
    static const char* sm_Equal;
    static const char* sm_Contain;
    static const char* sm_InList;

protected:
    /// checks whether the arguments number and their type is good and sets the m_Modifier flag in case of a modifier
    virtual bool x_ValidArguments() const;
    virtual void x_ResetState() { m_Modifier = false; m_Constraint.Reset(); }
    virtual bool x_IsNestedFunctionReturnValid() const;

    void x_SetModifierFlag();
    /// builds a new string constraint to later match with the value of the field
    void x_BuildStringConstraint(void);
    void x_BuildCaseStringConstraint(void);
    void x_BuildNoCaseStringConstraint(void);
    CRef<objects::CWord_substitution> x_ReadSynonymsFor(const string& phrase);

    bool x_CheckStringConstraint(const CObjectInfo& oi);
    bool x_CheckConstraintPrimitive(const string& field);
    bool x_CheckConstraintContainer(const string& container, const string& subtype);
    bool x_CheckConstraintWithinClass(const CObjectInfo& obj, const string& subtype);
    bool x_CheckConstraintObjects(const CMQueryNodeValue::TObs& objs);
    bool x_CheckAuthorNames(const CObjectInfo& obj);

    /// indicates to test for the case of the phrase
    EStrConstrCase m_TestCase;
    /// specifies the type of pattern matching
    string m_Type;
    /// flag that indicates that the field to be checked is a modifier
    bool m_Modifier;
    /// flag to indicate whether to use OR or AND operation for the input values
    ELogicType m_Operation;
    /// the string constraint
    CRef<objects::CString_constraint> m_Constraint;
};

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_FN_STRING_CONSTR__HPP
