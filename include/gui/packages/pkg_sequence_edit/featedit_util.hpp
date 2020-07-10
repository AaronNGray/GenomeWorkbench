/*  $Id: featedit_util.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
 * Authors:  Colleen Bollin
 */
#ifndef _FEATEDIT_UTIL_H_
#define _FEATEDIT_UTIL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objmgr/scope.hpp>
#include <gui/packages/pkg_sequence_edit/field_handler.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>

#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE


const string kStartColLabel = "Start";
const string kStopColLabel = "Stop";
const string kFeatureID = "FeatureID";
const string kPartialStart = "5' Partial";
const string kPartialStop = "3' Partial";
const string kProblems = "Problems";

static const string kSatelliteTypes[] = {"satellite", "microsatellite", "minisatellite" };
static const int kNumSatelliteTypes = sizeof (kSatelliteTypes) / sizeof (string);


class CFeatureSeqTableColumnBase : public CObject
{
public:
    CFeatureSeqTableColumnBase() 
        : m_Subtype(objects::CSeqFeatData::eSubtype_any),
          m_MajorType(objects::CSeqFeatData::e_not_set),
          m_Scope(NULL)
        {  }
    // default is to do no command.  Subclasses will almost always override this
    virtual bool AddToFeature(objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text ) { return false; }
    virtual void ClearInFeature(objects::CSeq_feat & in_out_feat ) { }
    virtual string GetFromFeat(const objects::CSeq_feat & in_out_feat ) { return ""; }
    virtual string GetLabel() const { return ""; }
    virtual bool RelaxedMatch(const objects::CSeq_feat & f1, const objects::CSeq_feat &f2) const { return true; };
    virtual vector<string> IsValid(const vector<string>& values) { vector<string> x; return x; };
    virtual vector<CConstRef<CObject> > GetObjects(objects::CBioseq_Handle bsh);
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);
    void SetSubtype(objects::CSeqFeatData::ESubtype subtype) { m_Subtype = subtype; };
    objects::CSeqFeatData::ESubtype GetSubtype() { return m_Subtype; };
    void SetMajorType(objects::CSeqFeatData::E_Choice major_type) { m_MajorType = major_type; };
    objects::CSeqFeatData::E_Choice GetMajorType() { return m_MajorType; };
    virtual vector <string> GetVals(const objects::CSeq_feat& feat) {vector <string> x; return x;};
    virtual vector<CRef<objects::edit::CApplyObject> > GetRelatedGeneApplyObjects(objects::CBioseq_Handle bsh);
    void SetScope(CRef<CScope> scope) { m_Scope = scope; };
    virtual bool AllowMultipleValues() { return false; };

protected:
    objects::CSeqFeatData::ESubtype m_Subtype;
    objects::CSeqFeatData::E_Choice m_MajorType;
    CRef<CScope> m_Scope;
};


typedef vector< CRef<CFeatureSeqTableColumnBase> > TFeatureSeqTableColumnList;

class CFeatureSeqTableColumnBaseFactory
{
public:
    static CRef<CFeatureSeqTableColumnBase> Create(const string &sTitle,
                                                   objects::CSeqFeatData::ESubtype subtype = objects::CSeqFeatData::eSubtype_any);
};

class CPartialStartColumn : public CFeatureSeqTableColumnBase
{
public:
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(
      objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat ) ;
    virtual string GetLabel() const { return kPartialStart; }
};


class CPartialStopColumn : public CFeatureSeqTableColumnBase
{
public:
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(
      objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat ) ;
    virtual string GetLabel() const { return kPartialStop; }
};


class CCommentColumn : public CFeatureSeqTableColumnBase
{
public:
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(
      objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat ) ;
    virtual string GetLabel() const { return "Comment"; }
};


class CExceptionColumn : public CFeatureSeqTableColumnBase
{
public:
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(
      objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat ) ;
    virtual string GetLabel() const { return "Exception"; }
};


class CGbQualColumn : public CFeatureSeqTableColumnBase
{
public:
    CGbQualColumn(string qual)  { m_QualName = qual; };
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(
        objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat );
    virtual string GetLabel() const { return m_QualName; }

protected:
    string m_QualName;
};


class CRptUnitRangeColumn : public CGbQualColumn
{
public:
    CRptUnitRangeColumn(const vector<size_t> lens);
    virtual vector<string> IsValid(const vector<string>& values);
    virtual string GetLabel() const { return "rpt_unit_range\nExample: 1..10"; }
protected:
    vector<size_t> m_Lens;
};


class CRptUnitSeqColumn : public CGbQualColumn
{
public:
    CRptUnitSeqColumn(); 

    virtual vector<string> IsValid(const vector<string>& values);
};

class CSatelliteColumn : public CGbQualColumn
{
public:
    CSatelliteColumn(const string& satellite_field);
    virtual bool AddToFeature(objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text);
    virtual void ClearInFeature(objects::CSeq_feat & in_out_feat);
    virtual string GetFromFeat(const objects::CSeq_feat & in_out_feat);
    virtual string GetLabel() const { return kEmptyStr; }
private:
    string x_AddSatellitePrefix(const string& value, objects::edit::EExistingText existing_text) const;
    bool x_AddSatelliteType(objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text);
    bool x_AddSatelliteName(objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text);
    void x_ClearSatelliteSubFields(objects::CSeq_feat & in_out_feat) const;
    
    string x_GetSatelliteType(const string& field) const;
    string x_GetSatelliteName(const string& field) const;
    
    enum EFieldType {
        eSatellite,  // satellite-type:satellite-name
        eSatelliteType, // satellite-type = one of {satellite, microsatellite, minisatellite}
        eSatelliteName  // satellite-name
    };
    EFieldType m_Field;
};


class CSatelliteTypeNameColumn : public CFeatureSeqTableColumnBase
{
public:
    CSatelliteTypeNameColumn(string satellite_type, bool need_unique = false) 
      : m_SatelliteType(satellite_type), m_RequireUniqueness(need_unique) {};
    virtual bool AddToFeature(objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(const objects::CSeq_feat & in_out_feat );
    virtual string GetLabel() const { return m_SatelliteType + " name"; }
    virtual bool RelaxedMatch(const objects::CSeq_feat & f1, const objects::CSeq_feat &f2) const;
    void SetUniqueness(bool val) { m_RequireUniqueness = val; };
    virtual vector<string> IsValid(const vector<string>& values);
protected:
    string m_SatelliteType;
    bool   m_RequireUniqueness;
    bool x_IsQualAMatch(const CGb_qual& qual) const;
};


class CProductColumn : public CGbQualColumn
{
public:
    CProductColumn() : CGbQualColumn("product") {};
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(
        objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat );
    virtual string GetLabel() const { return "product"; }
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);

};


class CCodonStartColumn : public CGbQualColumn
{
public:
    CCodonStartColumn() : CGbQualColumn("codon_start") {};
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(
        objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat );
    virtual string GetLabel() const { return "codon_start"; }
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);
};


class CGeneticCodeColumn : public CGbQualColumn
{
public:
    CGeneticCodeColumn() : CGbQualColumn("transl_table") {}
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text);
    virtual void ClearInFeature(
        objects::CSeq_feat & in_out_feat);
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat);
    virtual string GetLabel() const { return "transl_table"; }
};


class CDescriptionColumn : public CGbQualColumn
{
public:
    CDescriptionColumn() : CGbQualColumn("description") {};
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(
        objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat );
    virtual string GetLabel() const { return "description"; }
};


class CGeneRefColumn : public CGbQualColumn
{
public:
    CGeneRefColumn(string qual);
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(
        objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat );
    virtual vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(objects::CBioseq_Handle bsh);
    static bool IsGeneRefField(string qual);

private:
    enum EFieldType {
        eFieldType_locus = 0,
        eFieldType_locus_tag,
        eFieldType_maploc,
        eFieldType_synonym,
        eFieldType_allele,
        eFieldType_other
    };

    EFieldType m_FieldType;

    static EFieldType x_GetFieldTypeFromString(string qual);
};


class CProtRefColumn : public CGbQualColumn
{
public:
    CProtRefColumn(string qual);
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(
        objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat );
    static bool IsProtRefField(string qual);

private:
    enum EFieldType {
        eFieldType_name = 0,
        eFieldType_ec_number,
        eFieldType_activity,
        eFieldType_other
    };

    EFieldType m_FieldType;

    static EFieldType x_GetFieldTypeFromString(string qual);
};

class CNcrnaClassColumn : public CGbQualColumn
{
public:
    CNcrnaClassColumn() : CGbQualColumn("ncRNA_class") {};
    virtual bool AddToFeature(
        objects::CSeq_feat & in_out_feat, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInFeature(
        objects::CSeq_feat & in_out_feat );
    virtual string GetFromFeat(
        const objects::CSeq_feat & in_out_feat );
};

class CDbXrefColumn :  public CGbQualColumn
{
public:
    CDbXrefColumn() : CGbQualColumn("db_xref") {};
    virtual string GetFromFeat(const objects::CSeq_feat & in_out_feat );
    virtual void ClearInFeature( objects::CSeq_feat & in_out_feat );
};

class CEvidenceColumn :  public CGbQualColumn
{
public:
    CEvidenceColumn() : CGbQualColumn("evidence") {};
    virtual string GetFromFeat(const objects::CSeq_feat & in_out_feat );
};

class CRegionColumn : public CGbQualColumn
{
public:
    CRegionColumn() : CGbQualColumn("region") {};
    virtual string GetFromFeat(const objects::CSeq_feat & in_out_feat );
};

class CLocationColumn : public CFeatureSeqTableColumnBase 
{
public:
    virtual string GetFromFeat(const objects::CSeq_feat & in_out_feat );
};

class CCodonsRecColumn : public CGbQualColumn
{
public:
    CCodonsRecColumn() : CGbQualColumn("codons_recognized") {};
    virtual string GetFromFeat(const objects::CSeq_feat & in_out_feat );
};

class CAntiCodonColumn : public CGbQualColumn
{
public:
    CAntiCodonColumn() : CGbQualColumn("anticodon") {};
    virtual string GetFromFeat(const objects::CSeq_feat & in_out_feat );
};

class CTagPeptideColumn : public CGbQualColumn
{
public:
    CTagPeptideColumn() : CGbQualColumn("tag_peptide") {};
    virtual string GetFromFeat(const objects::CSeq_feat & in_out_feat );
};

class CCitationColumn : public CGbQualColumn
{
public:
    CCitationColumn() : CGbQualColumn("citation") {};
    virtual string GetFromFeat(const objects::CSeq_feat & in_out_feat );
};


class CInferenceColumn : public CGbQualColumn
{
public:
    CInferenceColumn() : CGbQualColumn("inference") {};
    virtual bool AllowMultipleValues() { return true; };
};


int FindHighestFeatureId(const objects::CSeq_entry_Handle& entry);
CRef<objects::CSeq_table> BuildFeatureSeqTableFromSeqEntry(objects::CSeq_entry_Handle entry, const SAnnotSelector& sel);
void AddColumnsToFeatureTable(const vector<string> & fields, CRef<objects::CSeq_table> table, objects::CSeq_entry_Handle entry);
CRef<CCmdComposite> ApplyFeatureSeqTableToSeqEntry(CRef<objects::CSeq_table> table, objects::CSeq_entry_Handle entry);

CRef<objects::CSeq_loc> NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CreateFeatLocation(const objects::CBioseq_Handle& bsh, int from, int to, bool ispartial5, bool ispartial3);
CRef<objects::CSeq_feat> NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT AddRibosomalRNA(const objects::CBioseq_Handle& bsh, string name, int from, int to, bool ispartial5, bool ispartial3);
CRef<CCmdComposite> NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT AddNamedrRna(objects::CSeq_entry_Handle entry, string rna_name, bool ispartial5, bool ispartial3);
CRef<objects::CSeq_feat> NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT AddMiscRNA(const objects::CBioseq_Handle& bsh, string name, int from, int to, bool ispartial5, bool ispartial3);

CRef<CCmdComposite> NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CreateControlRegion(objects::CSeq_entry_Handle entry);
CRef<CCmdComposite> NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CreateMicrosatellite(objects::CSeq_entry_Handle entry);

void SetRnaProductName( objects::CRNA_ref & rna, const string& product, string& remainder, objects::edit::EExistingText existing_text );

END_NCBI_SCOPE

#endif
    // _FEATEDIT_UTIL_H_
