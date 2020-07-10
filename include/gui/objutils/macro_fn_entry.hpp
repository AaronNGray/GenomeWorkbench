#ifndef GUI_OBJUTILS___MACRO_FN_ENTRY__HPP
#define GUI_OBJUTILS___MACRO_FN_ENTRY__HPP
/*  $Id: macro_fn_entry.hpp 44325 2019-11-29 20:29:44Z asztalos $
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
 *   Macro editing functions used in the DO/DONE section that in general 
 *   modify the whole top seq-entry
 *
 */

/// @file macro_fn_entry.hpp
/// Functions used in the DO/DONE section affecting the top seq-entry

#include <corelib/ncbistd.hpp>
#include <objtools/edit/autodef_with_tax.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeq_entry_Handle;
    class CBioseq_set_Handle;
    class CAutoDef;
    class CAutoDefOptions;
    class CAutoDefModifierCombo;
    class CAutoDefAvailableModifier;
END_SCOPE(objects)

BEGIN_SCOPE(macro)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_TaxLookup)

DECLARE_FUNC_CLASS_WITH_ARGS(CMacroFunction_DiscrepancyAutofix)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_ConvertRawToDeltabyNs)

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FixSpelling : public IEditMacroFunction
{
public:
    CMacroFunction_FixSpelling(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual ~CMacroFunction_FixSpelling() {}
    virtual void TheFunction();
    static void s_SpellingFixes(CSerialObject& object, CNcbiOstrstream& oss);
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_RemoveSingleItemSet : public IEditMacroFunction
{
public:
    CMacroFunction_RemoveSingleItemSet(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual ~CMacroFunction_RemoveSingleItemSet() {}
    virtual void TheFunction();
    static short s_RemoveSingleItemSet(objects::CSeq_entry_Handle& seh);
    static bool s_IsSingletonSet(const objects::CBioseq_set_Handle& bioseq_set);
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_Autodef : public IEditMacroFunction
{
public:
    CMacroFunction_Autodef(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope), m_Descr(kEmptyStr) {}
    
    virtual ~CMacroFunction_Autodef() {}
    virtual void TheFunction();

    static CRef<CCmdComposite> s_AutodefSeqEntry(const objects::CSeq_entry_Handle& seh, const objects::CAutoDefOptions& options);
    static CRef<CCmdComposite> s_AutodefSequence(const objects::CSeq_entry_Handle& seh, const objects::CBioseq_Handle& target, const objects::CAutoDefOptions& options);

    static void s_AutodefBioseqs(const objects::CSeq_entry_Handle& seh, objects::CAutoDefWithTaxonomy& autodef,
                                objects::CAutoDefModifierCombo* mod_combo, CRef<CCmdComposite> composite_cmd);
    static void s_AutodefSets(const objects::CSeq_entry_Handle& seh, objects::CAutoDefWithTaxonomy& autodef, CRef<CCmdComposite> composite_cmd);
    static void s_AddAutodefOptions(const objects::CAutoDef& autodef, const objects::CBioseq_Handle& bsh, CRef<CCmdComposite> composite_cmd);
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
protected:
    virtual bool x_ValidArguments() const;
    virtual void x_ResetState() { m_Descr = kEmptyStr; }

    void x_AddModifiersToOptions(objects::CAutoDefOptions& opts);
    bool x_IsRequested(const objects::CAutoDefAvailableModifier& modifier);
    string x_GetDescription();
    string m_Descr;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_AutodefId : public IEditMacroFunction
{
public:
    CMacroFunction_AutodefId(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual ~CMacroFunction_AutodefId() {}
    virtual void TheFunction();

    static void s_ConfigureAutodefOptionsForID(objects::CAutoDefSourceDescription::TAvailableModifierVector& modifiers, objects::CSeq_entry_Handle seh);
    static CTempString GetFuncName();
protected:
    bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_RenormalizeNucProtSet : public IEditMacroFunction
{
public:
    CMacroFunction_RenormalizeNucProtSet(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual void TheFunction();

    void s_RenormalizeNucProtSets(objects::CSeq_entry_Handle seh, Int4& count);
    static CTempString GetFuncName();
protected:
    bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_CreateProteinFeats : public IEditMacroFunction
{
public:
    CMacroFunction_CreateProteinFeats(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    bool x_ValidArguments() const;
    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle>> m_ProductToCds;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_DeltaSeqToRaw : public IEditMacroFunction
{
public:
    CMacroFunction_DeltaSeqToRaw(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual void TheFunction();
    static CTempString GetFuncName();
    static CRef<CCmdComposite> s_ConvertDeltaToRaw(objects::CSeq_entry_Handle seh, Int4& count);
protected:
    bool x_ValidArguments() const;
    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle>> m_ProductToCds;
};


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif
    // GUI_OBJUTILS___MACRO_FN_ENTRY__HPP
