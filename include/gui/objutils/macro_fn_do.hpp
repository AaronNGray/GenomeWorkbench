#ifndef GUI_OBJUTILS___MACRO_FN_DO__HPP
#define GUI_OBJUTILS___MACRO_FN_DO__HPP
/*  $Id: macro_fn_do.hpp 44806 2020-03-20 15:23:11Z asztalos $
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
 * File Description: 
 * Macro editing functions used in the DO - DONE section
 *
 */

/// @file macro_fn_do.hpp
#include <corelib/ncbistd.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CBioseq;
    class CAuthor;
    class CAffil;
    class CName_std;
    class CSeq_entry_Handle;
END_SCOPE(objects)
BEGIN_SCOPE(macro)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_Resolve)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_ResolveBioSourceQuals)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_ResolveNASeq)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RemoveLineageSourceNotes)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_FixUSAandStatesAbbrev)

DECLARE_FUNC_CLASS_WITH_ARGS(CMacroFunction_FixSourceQualCaps)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_FixMouseStrain)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_FixAuthorCaps)

DECLARE_FUNC_CLASS_WITH_ARGS(CMacroFunction_RemoveDescriptor)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RemoveSequence)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RemoveAlignment)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_ReorderStructComment)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_ChangeSeqInst)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_AddProteinTitles)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_CorrectGeneticCodes)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RmvDuplStructComments)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_TrimStringQual)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_PrintCSV)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_PrintTSV)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_PrintBankit)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_PrintLiteral)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_ToUnknownLengthGap)


class CMacroFunction_ModifyPrimerSeq : public IEditMacroFunction
{
public:
    enum EModifyType {
        eTrimJunk,
        eFixI
    };
    
    virtual ~CMacroFunction_ModifyPrimerSeq() {}

    virtual void TheFunction();
protected:
    CMacroFunction_ModifyPrimerSeq(EScopeEnum func_scope, EModifyType type)
        : IEditMacroFunction(func_scope), m_Type(type) {}

    virtual bool x_ValidArguments() const;
    EModifyType m_Type;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_TrimJunkFromPrimers : public CMacroFunction_ModifyPrimerSeq
{
public:
    CMacroFunction_TrimJunkFromPrimers(EScopeEnum func_scope)
        : CMacroFunction_ModifyPrimerSeq(func_scope, CMacroFunction_ModifyPrimerSeq::eTrimJunk) {}
    ~CMacroFunction_TrimJunkFromPrimers() {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FixIInPrimers : public CMacroFunction_ModifyPrimerSeq
{
public:
    CMacroFunction_FixIInPrimers(EScopeEnum func_scope)
        : CMacroFunction_ModifyPrimerSeq(func_scope, CMacroFunction_ModifyPrimerSeq::eFixI) {}
    ~CMacroFunction_FixIInPrimers() {}
    static CTempString GetFuncName();
};


class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FixFormat : public IEditMacroFunction
{
public:
    CMacroFunction_FixFormat(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual ~CMacroFunction_FixFormat() {};
    virtual void TheFunction();
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;

    // if unable to correct lat-lon, it returns an empty string
    static string s_FixLatLonFormat(objects::CBioSource& bsrc, const string& value);
    static bool s_AddAltitudeToSubSourceNote(objects::CBioSource& bsrc, const string& value);
    static string s_ConvertAltitudeToMeters( const string& value );
protected:
    virtual bool x_ValidArguments() const;
};


class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FixPubCaps : public IEditMacroFunction
{
public:
    enum EPubType {
        eTitle,
        eAuthor,
        eAffiliation,
        eAffilExceptAffilDiv,
        eAffilCountry
    };
    virtual ~CMacroFunction_FixPubCaps() {}

    virtual void TheFunction();
    
    static string s_FixCapitalizationInTitle(const string& title, bool first_is_upper, const objects::CSeq_entry_Handle& seh);
    static Int4   s_FixCapitalizationInAuthor(objects::CName_std& auth_names);
    static bool   s_FixCapitalizationInElement(string& value, bool fix_abbrev, bool fix_short_words, bool cap_after_apostrophe);
    static void   s_FixCapitalizationInCountryString(string& country, bool punct_only);
    static Int4   s_FixCapsInPubAffil(objects::CAffil& affil, bool punct_only);
    static Int4   s_FixCapsInPubAffil_NoAffilDiv(objects::CAffil& affil, bool punct_only);
    static Int4   s_FixCapsInPubAffil_Affil(objects::CAffil& affil, bool punct_only);
    static Int4   s_FixCapsInPubAffil_Div(objects::CAffil& affil, bool punct_only);
    static Int4   s_FixCapsInPubAffil_City(objects::CAffil& affil, bool punct_only);
    static Int4   s_FixCapsInPubAffil_Street(objects::CAffil& affil, bool punct_only);
    static Int4   s_FixCapsInPubAffil_Sub(objects::CAffil& affil, bool punct_only);
    static Int4   s_FixCapsInPubAffil_Country(objects::CAffil& affil, bool punct_only);
protected:
    CMacroFunction_FixPubCaps(EScopeEnum func_scope, EPubType type)
        : IEditMacroFunction(func_scope), m_Type(type) {}

    virtual bool x_ValidArguments() const;
    void x_FixCapsInPubAffilCountry(objects::CAffil& affil, bool punct_only);
    EPubType m_Type;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FixPubCapsTitle : public CMacroFunction_FixPubCaps
{
public:
    CMacroFunction_FixPubCapsTitle(EScopeEnum func_scope)
        : CMacroFunction_FixPubCaps(func_scope, CMacroFunction_FixPubCaps::eTitle) {}
    ~CMacroFunction_FixPubCapsTitle() {}
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FixPubCapsAuthor : public CMacroFunction_FixPubCaps
{
public:
    CMacroFunction_FixPubCapsAuthor(EScopeEnum func_scope)
        : CMacroFunction_FixPubCaps(func_scope, CMacroFunction_FixPubCaps::eAuthor) {}
    ~CMacroFunction_FixPubCapsAuthor() {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FixPubCapsAffil : public CMacroFunction_FixPubCaps
{
public:
    CMacroFunction_FixPubCapsAffil(EScopeEnum func_scope)
        : CMacroFunction_FixPubCaps(func_scope, CMacroFunction_FixPubCaps::eAffiliation) {}
    ~CMacroFunction_FixPubCapsAffil() {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FixPubCapsAffilWithExcept : public CMacroFunction_FixPubCaps
{
public:
    CMacroFunction_FixPubCapsAffilWithExcept(EScopeEnum func_scope)
        : CMacroFunction_FixPubCaps(func_scope, CMacroFunction_FixPubCaps::eAffilExceptAffilDiv) {}
    ~CMacroFunction_FixPubCapsAffilWithExcept() {}
    static CTempString GetFuncName();
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FixPubCapsAffilCountry : public CMacroFunction_FixPubCaps
{
public:
    CMacroFunction_FixPubCapsAffilCountry(EScopeEnum func_scope)
        : CMacroFunction_FixPubCaps(func_scope, CMacroFunction_FixPubCaps::eAffilCountry) {}
    ~CMacroFunction_FixPubCapsAffilCountry() {}
    static CTempString GetFuncName();
};




class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_RemoveSegGaps : public IEditMacroFunction
{
public:
    CMacroFunction_RemoveSegGaps(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual ~CMacroFunction_RemoveSegGaps() {}
    virtual void TheFunction();
    static bool s_RemoveSegGaps(objects::CSeq_align& align);
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_RemoveOrgName : public IEditMacroFunction
{
public:
    CMacroFunction_RemoveOrgName(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_RemoveOrgName() {}
    virtual void TheFunction();
    static bool s_RemoveOrgName(const objects::CBioSource& bsrc, string& field);

    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_MakeBoldXrefs : public IEditMacroFunction
{
public:
    CMacroFunction_MakeBoldXrefs(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual ~CMacroFunction_MakeBoldXrefs() {}
    virtual void TheFunction();

    // returns an empty string if there is no such ID
    static string s_GetBarcodeId(const objects::CBioseq& bseq);
    static bool s_HasBarcodeDbXref(const objects::CBioseq& bseq, const string& barcode_id);
    static CRef<objects::CDbtag> s_MakeBarcodeDbXref(const string& barcode_id);

    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_TrimTerminalNs : public IEditMacroFunction
{
public:
    CMacroFunction_TrimTerminalNs(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual void TheFunction();
    static CTempString GetFuncName();

protected:
    virtual bool x_ValidArguments() const;
    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle>> m_ProductToCds;
};


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_FN_DO__HPP
