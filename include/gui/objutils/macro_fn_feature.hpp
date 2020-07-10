#ifndef GUI_OBJUTILS___MACRO_FN_FEATURE__HPP
#define GUI_OBJUTILS___MACRO_FN_FEATURE__HPP
/*  $Id: macro_fn_feature.hpp 43315 2019-06-12 14:02:14Z asztalos $
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
 * Macro editing functions applied to features used in the DO - DONE section
 *
 */

/// @file macro_fn_feature.hpp
#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/macro/Gene_xref_suppression_type.hpp>
#include <objects/macro/Gene_xref_necessary_type.hpp>
#include <objects/macro/Location_edit_type.hpp>
#include <objtools/edit/loc_edit.hpp>

#include <gui/objutils/macro_edit_fn_base.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class CConvertFeatureBase;

BEGIN_SCOPE(macro)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_TrimStopFromCompleteCDS)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_SynchronizeCDSPartials)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_AdjustConsensusSpliceSites)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RemoveInvalidECNumbers)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_UpdatemRNAProduct)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_CopyNameToCDSNote)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RemoveFeature)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RemoveDuplFeatures)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_AddGeneXref)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_JoinShorttRNAs)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_UpdateReplacedECNumbers)

DECLARE_FUNC_CLASS_WITH_FNCNAME(CMacroFunction_RetranslateCDS)

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_FixProteinFormat : public IEditMacroFunction
{
public:
    CMacroFunction_FixProteinFormat(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual ~CMacroFunction_FixProteinFormat() {}
    virtual void TheFunction();
    static string s_FixProteinNameFormat(const string& protein, const vector<string>& taxnames);
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class CMacroFunction_EditFeatLocation : public IEditMacroFunction
{
public:
    virtual ~CMacroFunction_EditFeatLocation() {}
    virtual void TheFunction() {}
protected:
    virtual bool x_ValidArguments() const { return false; }

    bool x_CheckInitFeature();
    void x_RetranslateCDS(CNcbiOstrstream& log);
    void x_RetranslateCDSAdjustGene(const string& descr);

    CMacroFunction_EditFeatLocation(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope), m_RetranslateCDS(false), m_AdjustGene(false), m_Modified(false) {}

    CRef<objects::CSeq_feat> m_Seqfeat;  // edited feature
    CConstRef<objects::CSeq_feat> m_Origfeat;
    CRef<objects::CScope> m_Scope;
    bool m_RetranslateCDS;
    bool m_AdjustGene;
    bool m_Modified;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_SetBothPartials : public CMacroFunction_EditFeatLocation
{
public:
    CMacroFunction_SetBothPartials(EScopeEnum func_scope)
        : CMacroFunction_EditFeatLocation(func_scope) {}
    
    virtual void TheFunction();
    static bool s_SetBothEndsPartial(objects::CSeq_feat& feat, objects::CScope& scope, const string& descr, bool extend = false);
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_RemoveBothPartials : public CMacroFunction_EditFeatLocation
{
public:
    CMacroFunction_RemoveBothPartials(EScopeEnum func_scope)
        : CMacroFunction_EditFeatLocation(func_scope) {}
    
    virtual void TheFunction();
    static bool s_RemoveBothPartials(objects::CSeq_feat& feat, objects::CScope& scope, const string& descr);
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_Set5Partial : public CMacroFunction_EditFeatLocation
{
public:
    CMacroFunction_Set5Partial(EScopeEnum func_scope)
        : CMacroFunction_EditFeatLocation(func_scope) {}
    
    virtual void TheFunction();
    static bool s_Set5EndPartial(objects::CSeq_feat& feat, objects::CScope& scope, const string& descr, bool extend5 = false);
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_Set3Partial : public CMacroFunction_EditFeatLocation
{
public:
    CMacroFunction_Set3Partial(EScopeEnum func_scope)
        : CMacroFunction_EditFeatLocation(func_scope) {}
    
    virtual void TheFunction();
    static bool s_Set3EndPartial(objects::CSeq_feat& feat, objects::CScope& scope, const string& descr, bool extend3 = false);
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_Clear5Partial : public CMacroFunction_EditFeatLocation
{
public:
    CMacroFunction_Clear5Partial(EScopeEnum func_scope)
        : CMacroFunction_EditFeatLocation(func_scope) {}
   
    virtual void TheFunction();
    static bool s_Clear5EndPartial(objects::CSeq_feat& feat, objects::CScope& scope, const string& descr);
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_Clear3Partial : public CMacroFunction_EditFeatLocation
{
public:
    CMacroFunction_Clear3Partial(EScopeEnum func_scope)
        : CMacroFunction_EditFeatLocation(func_scope) {}
    
    virtual void TheFunction();
    static bool s_Clear3EndPartial(objects::CSeq_feat& feat, objects::CScope& scope, const string& descr);
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ConvertLocStrand : public CMacroFunction_EditFeatLocation
{
public:
    CMacroFunction_ConvertLocStrand(EScopeEnum func_scope)
        : CMacroFunction_EditFeatLocation(func_scope) {}
    
    virtual void TheFunction();
    static objects::ENa_strand s_GetStrandFromString(const string& str);
    static bool s_ConvertLocationStrand(objects::CSeq_feat& feat, objects::CScope& scope, const string& str_from, const string& str_to);
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ConvertLocType : public CMacroFunction_EditFeatLocation
{
public:
    CMacroFunction_ConvertLocType(EScopeEnum func_scope)
        : CMacroFunction_EditFeatLocation(func_scope) {}

    virtual void TheFunction();
    static bool s_ConvertLocationType(objects::CSeq_feat& feat, objects::CScope& scope, const string& descr);
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT  CMacroFunction_Extend5Feature : public CMacroFunction_EditFeatLocation
{
public:
    CMacroFunction_Extend5Feature(EScopeEnum func_scope)
        : CMacroFunction_EditFeatLocation(func_scope) {}
   
    virtual void TheFunction();
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT  CMacroFunction_Extend3Feature : public CMacroFunction_EditFeatLocation
{
public:
    CMacroFunction_Extend3Feature(EScopeEnum func_scope)
        : CMacroFunction_EditFeatLocation(func_scope) {}
    
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_RemoveAllFeatures : public IEditMacroFunction
{
public:
    CMacroFunction_RemoveAllFeatures(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle>> m_ProductToCds;
};


class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ApplyCDSFrame : public IEditMacroFunction
{
public:
    enum ECdsFrame {
        eNotSet,
        eBest,	// choose the frame that produces the longest sequence of aas before a stop codon is produced
        eMatch  // choose the frame that matches the protein sequence if it can find one
    };
    CMacroFunction_ApplyCDSFrame(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual ~CMacroFunction_ApplyCDSFrame() {}
    virtual void TheFunction();

    static bool s_SetCDSFrame(objects::CSeq_feat& cds, ECdsFrame frame_type, objects::CScope& scope);
    static objects::CCdregion::TFrame s_FindMatchingFrame(const objects::CSeq_feat& cds, objects::CScope& scope);
    static ECdsFrame s_GetFrameFromName(const string& name);
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ConvertFeature : public IEditMacroFunction
{
public:
    typedef objects::CSeqFeatData::ESubtype TFeatSubtype;

    CMacroFunction_ConvertFeature(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    virtual ~CMacroFunction_ConvertFeature() {}

    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
    void x_SetConvertOptions(CRef<CConvertFeatureBase> converter);
    TFeatSubtype m_FromType{ objects::CSeqFeatData::eSubtype_any };
    TFeatSubtype m_ToType{ objects::CSeqFeatData::eSubtype_any };
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ConvertCDS : public CMacroFunction_ConvertFeature
{
public:
    CMacroFunction_ConvertCDS(EScopeEnum func_scope)
        : CMacroFunction_ConvertFeature(func_scope) {}
    ~CMacroFunction_ConvertCDS() {}
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};


class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_RemoveGeneXref : public IEditMacroFunction
{
public:
    CMacroFunction_RemoveGeneXref(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual ~CMacroFunction_RemoveGeneXref() {}
    virtual void TheFunction();

    static bool s_GeneXrefMatchesSuppression(const objects::CGene_ref& gene, objects::EGene_xref_suppression_type suppr_type);
    static bool s_GeneXrefMatchesNecessary(const objects::CGene_ref& gene, 
        const objects::CSeq_feat& feat, objects::CScope& scope, objects::EGene_xref_necessary_type necessary_type);
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ReplaceStopWithSelenocysteine : public IEditMacroFunction
{
public:
    CMacroFunction_ReplaceStopWithSelenocysteine(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}
    
    virtual ~CMacroFunction_ReplaceStopWithSelenocysteine() {}
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
    CRef<objects::CSeq_loc> x_GetProteinLoc(const objects::CSeq_id& prot_id, TSeqPos pos) const;
};


class CMacroFunction_MakeLocation : public IEditMacroFunction
{
public:
    virtual ~CMacroFunction_MakeLocation() {}
    virtual void TheFunction() {}
protected:
    virtual bool x_ValidArguments() const { return false; }
    bool x_SetCurrentBioseq();
    void x_HandleNegativeCoordinates(size_t index);
    objects::ENa_strand x_GetStrand(const string& str);
   
    CMacroFunction_MakeLocation(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    objects::CBioseq_Handle m_Bsh;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_MakeInterval : public CMacroFunction_MakeLocation
{
public:
    CMacroFunction_MakeInterval(EScopeEnum func_scope)
        : CMacroFunction_MakeLocation(func_scope) {}
    
    virtual void TheFunction();
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_MakeWholeSeqInterval : public CMacroFunction_MakeLocation
{
public:
    CMacroFunction_MakeWholeSeqInterval(EScopeEnum func_scope)
        : CMacroFunction_MakeLocation(func_scope) {}
    
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_MakePoint : public CMacroFunction_MakeLocation
{
public:
    CMacroFunction_MakePoint(EScopeEnum func_scope)
        : CMacroFunction_MakeLocation(func_scope) {}
    
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};


class CMacroFunction_ApplyFeature : public IEditMacroFunction
{
public:
    virtual void TheFunction() {}
    //static CMacroFunction_ApplyFeature* s_MakeApplyFeatureType(EScopeEnum func_scope, objects::CSeqFeatData::E_Choice type);
protected:
    virtual bool x_ValidArguments() const { return false; }
    bool x_SetLocation(size_t index);
    bool x_ShouldAddRedundant(size_t index);
    void x_SetAdditionalFields(size_t index);

    CMacroFunction_ApplyFeature(EScopeEnum func_scope)
        : IEditMacroFunction(func_scope) {}

    objects::CBioseq_Handle m_Bsh;
    CRef<objects::CSeq_feat> m_NewFeat;
    CRef<objects::CSeq_loc> m_Location;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ApplyCDS : public CMacroFunction_ApplyFeature
{
public:
    CMacroFunction_ApplyCDS(EScopeEnum func_scope)
        : CMacroFunction_ApplyFeature(func_scope) {}
    
    virtual void TheFunction();
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ApplyRNA : public CMacroFunction_ApplyFeature
{
public:
    CMacroFunction_ApplyRNA(EScopeEnum func_scope)
        : CMacroFunction_ApplyFeature(func_scope) {}
    
    virtual void TheFunction();
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ApplyGene : public CMacroFunction_ApplyFeature
{
public:
    CMacroFunction_ApplyGene(EScopeEnum func_scope)
        : CMacroFunction_ApplyFeature(func_scope) {}
    
    virtual void TheFunction();
    static CTempString GetFuncName();
protected:
    virtual bool x_ValidArguments() const;
};

class NCBI_GUIOBJUTILS_EXPORT CMacroFunction_ApplyOtherFeature : public CMacroFunction_ApplyFeature
{
public:
    CMacroFunction_ApplyOtherFeature(EScopeEnum func_scope)
        : CMacroFunction_ApplyFeature(func_scope) {}
   
    virtual void TheFunction();
    static CTempString GetFuncName();
    static const vector<SArgMetaData> sm_Arguments;
protected:
    virtual bool x_ValidArguments() const;
};


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_FN_FEATURE__HPP
