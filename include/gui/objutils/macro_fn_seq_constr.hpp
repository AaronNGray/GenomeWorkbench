#ifndef GUI_OBJUTILS___MACRO_FN_SEQ_CONSTR__HPP
#define GUI_OBJUTILS___MACRO_FN_SEQ_CONSTR__HPP
/*  $Id: macro_fn_seq_constr.hpp 40255 2018-01-17 21:51:10Z asztalos $
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
 * File Description: Macro functions implementing sequence constraints
 *
 */

/// @file macro_fn_seq_constr.hpp

#include <objects/macro/Feature_stranded_constrain.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>
/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FeatStrandednessConstraint : public IEditMacroFunction
{
public:
    CMacroFunction_FeatStrandednessConstraint(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
   
    virtual ~CMacroFunction_FeatStrandednessConstraint() {}
    virtual void TheFunction();
    static const char* sm_FunctionName;
protected:
    virtual bool x_ValidArguments() const;
    objects::EFeature_strandedness_constraint x_GetStrandednessFromName(const string& strand_name);
    bool x_Match(const objects::CBioseq_Handle& bsh, objects::EFeature_strandedness_constraint strandedness);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_NumberOfFeatures : public IEditMacroFunction
{
public:
    CMacroFunction_NumberOfFeatures(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
   
    virtual ~CMacroFunction_NumberOfFeatures() {}
    virtual void TheFunction();

    /// returns the number of features of type feat_type present on the bioseq 
    static int s_GetFeatTypeCount(const objects::CBioseq_Handle& bsh, const string& feat_type);
    static const char* sm_FunctionName;
protected:
    virtual bool x_ValidArguments() const;
};

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_FN_SEQ_CONSTR__HPP
