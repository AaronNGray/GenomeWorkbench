/*  $Id: editing_action_features.hpp 45101 2020-05-29 20:53:24Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */

#ifndef _EDITING_ACTION_FEATURES_H_
#define _EDITING_ACTION_FEATURES_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/RNA_qual.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/seqfeat/Code_break.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CEditingActionConstraint;

class IEditingActionFeat : public IEditingAction 
{
public:
    IEditingActionFeat(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &name);
    virtual void SetFeat(CSeq_feat_Handle fh);
    CSeq_feat_Handle GetFeatHandle(void);
    CSeqFeatData::ESubtype GetActionFeatSubtype() {return m_selector.GetFeatSubtype();}
    CSeq_feat_Handle x_FindGeneForFeature(const CSeq_loc& loc, CScope& scope, CSeqFeatData::ESubtype subtype);

protected:
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
    virtual void Modify(EActionType action);
    virtual void SwapContext(IEditingAction* source);
    virtual bool SameObject();
    bool OtherFeatIs(const CSeqFeatData::ESubtype subtype);
    void FindFeatForAnotherFeat(EActionType action, SAnnotSelector other_selector);
    void FindRelatedFeatForAnotherFeat(EActionType action, SAnnotSelector other_selector); // TODO - same as FindRelated ? 
    void FindOrCreateProtFeat(EActionType action);
    void FindRelatedOrCreateProtFeat(EActionType action);
    void GoToProtFeature(void);
    void GoToOrigFeature(void);

    bool x_TestGeneForFeature(const CSeq_loc& gene_loc, const CSeq_loc& feat_loc, CScope& scope, bool& exact, TSeqPos& diff);
    CRef<CSeq_loc> x_GetSublocOnBioseq(CBioseq_Handle bsh,  const CSeq_loc& loc);
    CRef<CSeq_loc> x_MergeFeatureLocation(const CSeq_loc &loc, CScope &scope);

    void RemoveGeneXref(void);
    void SetFeatForAnotherFeat(CSeq_feat_Handle fh, const CSeqFeatData::ESubtype subtype);
    CSeq_entry_Handle GetCurrentSeqEntryHandleFromFeatHandle(CSeq_feat_Handle fh);
    bool MatchBioseqConstraintForFeatHandle(CSeq_feat_Handle fh);

    virtual void ResetScope();
    CRef<CScope> m_scope;
    SAnnotSelector m_selector;
    CSeq_feat_Handle m_Feat;
    CRef<CSeq_feat> m_EditedFeat;

    CSeq_feat_Handle m_SavedFeat;
    CRef<CSeq_feat> m_SavedEditedFeat;
    bool m_saved_feat_enabled;
};

class CEditingActionFeatComment : public IEditingActionFeat
{
public:
    CEditingActionFeatComment(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatException : public IEditingActionFeat
{
public:
    CEditingActionFeatException(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatGbQual : public IEditingActionFeat
{
public:
    CEditingActionFeatGbQual(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &qual, const string &name = "CEditingActionFeatGbQual");
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
protected:
    string m_qual;
    CRef< CGb_qual > m_GBqual;
    bool m_erase;
};

class CEditingActionFeatGbQualTwoNames : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatGbQualTwoNames(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &qual1, const string &qual2);
protected:
    virtual void Modify(EActionType action);
    string m_qual2;
};


class CEditingActionFeatDualVal1 : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatDualVal1(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &qual);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatDualVal2 : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatDualVal2(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &qual);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatDbxref : public IEditingActionFeat
{
public:
    CEditingActionFeatDbxref(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
private:
    CRef< CDbtag > m_Dbtag;
    bool m_erase;
};

class CEditingActionFeatEvidence : public IEditingActionFeat
{
public:
    CEditingActionFeatEvidence(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatPseudo : public IEditingActionFeat
{
public:
    CEditingActionFeatPseudo(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatCodonsRecognized : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatCodonsRecognized(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatAnticodon : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatAnticodon(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
private:
    string GetIntervalString(const CSeq_interval& seq_int);
};

class CEditingActionFeatTranslExcept : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatTranslExcept(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
private:
    CRef< CCode_break > m_CodeBreak;
};


class CEditingActionFeatRnaQual : public IEditingActionFeat
{
public:
    CEditingActionFeatRnaQual(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &qual);
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
private:
    string m_qual;
    CRef< CGb_qual > m_GBqual;
    CRef< CRNA_qual > m_RnaQual;
    bool m_erase;
};

class CEditingActionFeatRegion : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatRegion(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatCodonStart : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatCodonStart(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatTranslTable : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatTranslTable(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
private:
    const CGenetic_code_table& m_code_table;
};

class CEditingActionFeatDesc : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatDesc(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatNcRnaClass : public IEditingActionFeat
{
public:
    CEditingActionFeatNcRnaClass(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatLocus_tag : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatLocus_tag(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatMaploc : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatMaploc(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatSynonym : public IEditingActionFeat
{
public:
    CEditingActionFeatSynonym(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
private:
    string *m_syn;
    bool m_erase;
    string m_qual;
    CRef< CGb_qual > m_GBqual;
};

class CEditingActionFeatAllele : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatAllele(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatActivity : public IEditingActionFeat
{
public:
    CEditingActionFeatActivity(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
private:
    string *m_activity;
    bool m_erase;
    string m_qual;
    CRef< CGb_qual > m_GBqual;
};

class CEditingActionFeatPartial : public IEditingActionFeat
{
public:
    CEditingActionFeatPartial(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatTranslation : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatTranslation(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionFeatEcNumber : public IEditingActionFeat
{
public:
    CEditingActionFeatEcNumber(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
private:
    string *m_ec;
    bool m_erase;
    string m_qual;
    CRef< CGb_qual > m_GBqual;
};

class CEditingActionFeatFunction : public IEditingActionFeat
{
public:
    CEditingActionFeatFunction(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
//    virtual void Find(EActionType action);
//    virtual void FindRelated(EActionType action);
private:
    string *m_function;
    bool m_erase;
    string m_qual;
    CRef< CGb_qual > m_GBqual;
};

class CEditingActionFeatProduct : public IEditingActionFeat
{
public:
    CEditingActionFeatProduct(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
private:
    string *m_name;
    bool m_erase;
    string m_qual;
    CRef< CGb_qual > m_GBqual;
};


class CEditingActionFeatGeneLocus : public CEditingActionFeatGbQual
{
public:
    CEditingActionFeatGeneLocus(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &name);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
    virtual void FindRelated(EActionType action);
};

class CEditingActionFeatGeneLocusPlain : public CEditingActionFeatGeneLocus
{
public:
    CEditingActionFeatGeneLocusPlain(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
        : CEditingActionFeatGeneLocus(seh, subtype, feat_type, "CEditingActionFeatGeneLocusPlain") {}
protected:
    virtual void Find(EActionType action);
};

class CEditingActionFeatGeneLocusRna : public CEditingActionFeatGeneLocus
{
public:
    CEditingActionFeatGeneLocusRna(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
        : CEditingActionFeatGeneLocus(seh, subtype, feat_type, "CEditingActionFeatGeneLocusRna") {}
    virtual void SetFeat(CSeq_feat_Handle fh);
protected:
    virtual bool SameObject();
    virtual void Find(EActionType action);
};

class CEditingActionFeatGeneLocusCdsGeneProt : public CEditingActionFeatGeneLocus
{
public:
    CEditingActionFeatGeneLocusCdsGeneProt(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
        : CEditingActionFeatGeneLocus(seh, subtype, feat_type, "CEditingActionFeatGeneLocusCdsGeneProt") {}
    virtual void SetFeat(CSeq_feat_Handle fh);
protected:
    virtual void Find(EActionType action);
};


class CEditingActionFeatRnaToGeneDesc :  public CEditingActionFeatDesc
{
public:
    CEditingActionFeatRnaToGeneDesc(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype);
    virtual void SetFeat(CSeq_feat_Handle fh);
protected:
    virtual bool SameObject();
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
};

class CEditingActionFeatRnaToGeneMaploc :  public CEditingActionFeatMaploc
{
public:
    CEditingActionFeatRnaToGeneMaploc(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype);
    virtual void SetFeat(CSeq_feat_Handle fh);
protected:
    virtual bool SameObject();
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
};

class CEditingActionFeatRnaToGeneLocus_tag :  public CEditingActionFeatLocus_tag
{
public:
    CEditingActionFeatRnaToGeneLocus_tag(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype);
    virtual void SetFeat(CSeq_feat_Handle fh);
protected:
    virtual bool SameObject();
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
};

class CEditingActionFeatRnaToGeneSynonym :  public CEditingActionFeatSynonym
{
public:
    CEditingActionFeatRnaToGeneSynonym(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype);
    virtual void SetFeat(CSeq_feat_Handle fh);
protected:
    virtual bool SameObject();
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
};

class CEditingActionFeatRnaToGeneComment :  public CEditingActionFeatComment
{
public:
    CEditingActionFeatRnaToGeneComment(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype);
    virtual void SetFeat(CSeq_feat_Handle fh);
protected:
    virtual bool SameObject();
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
};

class CEditingActionFeatTranscriptId : public CEditingActionFeatGbQualTwoNames
{
public:
    CEditingActionFeatTranscriptId(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

IEditingActionFeat* CreateActionFeat(CSeq_entry_Handle seh, string field, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type);
IEditingActionFeat* CreateActionRna(CSeq_entry_Handle seh, string field, const CSeqFeatData::ESubtype subtype);
IEditingActionFeat* CreateActionCdsGeneProt(CSeq_entry_Handle seh, string field, const CSeqFeatData::ESubtype subtype);

END_NCBI_SCOPE

#endif
    // _EDITING_ACTION_FEATURES_H_
