/*  $Id: srcedit_util.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
 * Authors:  Colleen Bollin, J. Chen
 */
#ifndef _SRCEDIT_UTIL_H_
#define _SRCEDIT_UTIL_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objects/seqfeat/PCRReaction.hpp>
#include <objects/seqfeat/PCRPrimerSet.hpp>


#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/grid_widget/grid_widget.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_panel.hpp>
#include <gui/objutils/table_data.hpp>


#include <gui/objutils/cmd_composite.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/packages/pkg_sequence_edit/field_handler.hpp>

BEGIN_NCBI_SCOPE


class CSrcQual;
class CSourceRequirements;
typedef vector<CSrcQual *> TSrcQualList;


class CSrcQual 
{
public:
    /// Constructors
    CSrcQual();
    CSrcQual(string name, bool required, bool report_missing, string example = "");
    virtual string GetValue(const objects::CBioSource& src);
    virtual string GetFormatErrors(string value) {return ""; };
    virtual string GetExample() { return m_Example; };
    void SetExample(string example) { m_Example = example; };

    string GetName(void) { return m_Name; };
    bool IsRequired(void) { return m_Required; };
    bool ReportMissing(void) { return m_ReportMissing; };
    void SetRequired(bool required) { m_Required = required; };
    void SetReportMissing(bool report_missing) { m_ReportMissing = report_missing; };

    /// Destructor
    virtual ~CSrcQual();

protected:
    string m_Name;
    string m_Example;
    bool   m_Required;
    bool   m_ReportMissing;
};

class CTaxnameQual: public CSrcQual
{
public:
    /// Constructors
    CTaxnameQual() : CSrcQual() {};
    CTaxnameQual(string name, bool required, bool report_missing = false, string example = "") : CSrcQual(name, required, report_missing, example) {};
    virtual string GetValue(const objects::CBioSource& src);
    virtual string GetFormatErrors(string value) { return ""; };

    /// Destructor
    virtual ~CTaxnameQual() {};
};


class CUnculturedTaxnameQual: public CSrcQual
{
public:
    /// Constructors
    CUnculturedTaxnameQual() : CSrcQual() {};
    CUnculturedTaxnameQual(string name, bool required, bool report_missing = false, string example = "") : CSrcQual(name, required, report_missing, example) {};
    virtual string GetFormatErrors(string value); 

    /// Destructor
    virtual ~CUnculturedTaxnameQual() {};
};


class CGenomeQual: public CSrcQual
{
public:
    /// Constructors
    CGenomeQual() : CSrcQual() {};
    CGenomeQual(string name, bool required, bool report_missing = false, string example = "") : CSrcQual(name, required, report_missing, example) {};
    virtual string GetValue(const objects::CBioSource& src);
    virtual string GetFormatErrors(string value) { return ""; };

    /// Destructor
    virtual ~CGenomeQual() {};
};


class COrgModQual: public CSrcQual
{
public:
    /// Constructors
    COrgModQual() : CSrcQual(), m_Subtype(objects::COrgMod::eSubtype_other) {};
    COrgModQual(string name, bool required, bool report_missing = false, string example = "");
    virtual string GetValue(const objects::CBioSource& src);
    virtual string GetFormatErrors(string value);

    /// Destructor
    virtual ~COrgModQual() {};
private:
    objects::COrgMod::TSubtype m_Subtype;
};


class CSubSrcQual: public CSrcQual
{
public:
    /// Constructors
    CSubSrcQual() : CSrcQual(), m_Subtype(objects::CSubSource::eSubtype_other) {};
    CSubSrcQual(string name, bool required, bool report_missing = false, string example = "");
    virtual string GetValue(const objects::CBioSource& src);
    virtual string GetFormatErrors(string value);

    /// Destructor
    virtual ~CSubSrcQual() {};
private:
    objects::CSubSource::TSubtype m_Subtype;
    vector<string> m_BadIsolationSourceValues;
};


class CSourceRequirements
{
public:
    /// Constructors
    CSourceRequirements();
    void AddRequirement(string name, bool required, string example = "");
    void PreferentiallyAddRequirement(CRef<objects::CSeq_table> values_table, string choice1, string choice2, bool required);
    void AddUniquenessList(vector<string> list);
    void AddUniquenessList(int num, ...);
    void AddOneOfList(vector<string> list);
    void AddOneOfList(int num, ...);
    CRef<objects::CSeqTable_column> CheckSourceQuals(CRef<objects::CSeq_table> values_table);
    void AddColumnsToSeqTable(CRef<objects::CSeq_table> values_table);

    /// Destructor
    virtual ~CSourceRequirements();


    enum EWizardType {
        eWizardType_standard            =  0,
        eWizardType_viruses             =  1,
        eWizardType_uncultured_samples  =  2,
        eWizardType_rrna_its_igs        =  3, 
        eWizardType_tsa                 =  4,
        eWizardType_igs                 =  5,
        eWizardType_microsatellite      =  6,
        eWizardType_d_loop              =  7
    };

    enum EWizardSrcType {
        eWizardSrcType_any                  = 0,
        eWizardSrcType_virus_norovirus      = 1,
        eWizardSrcType_virus_foot_and_mouth = 2,
        eWizardSrcType_virus_influenza      = 3,
        eWizardSrcType_virus_rotavirus      = 4,
        eWizardSrcType_bacteria_or_archaea  = 5,
        eWizardSrcType_uncultured_fungus    = 6,
        eWizardSrcType_cultured_fungus      = 7,
        eWizardSrcType_vouchered_fungus     = 8,
        eWizardSrcType_plant                = 9,
        eWizardSrcType_animal               = 10
    };

    void SetExamples(EWizardType wizard_type, EWizardSrcType src_type);

private:
    TSrcQualList m_Requirements;
    // if any uniqueness lists are specified, at least one of the combos listed must produce a unique identifier for each sequence
    vector<vector<string> > m_UniquenessLists;
    vector<vector<string> > m_OneOfLists;

    int x_AddUniquenessProblems(CRef<objects::CSeq_table> values_table, vector<string> uniqueness_list, vector<string>& row_problems);
    void x_AddOneOfProblems(CRef<objects::CSeq_table>, vector<string> one_of_list, CRef<objects::CSeqTable_column> problems);

};


// Note that a subclass that returns true for AllowMultipleValues 
// should implement the GetVals method
class CSrcTableColumnBase : public CObject
{
public:
    enum ESourceType {
        eNotSet = 0, // assume both descriptors and features
        eDescriptor,
        eFeature
    };
    // default is to do no command.  Subclasses will almost always override this
    virtual bool AddToBioSource(objects::CBioSource & in_out_bioSource, const string & newValue, objects::edit::EExistingText existing_text ) { return false; }
    virtual void ClearInBioSource(objects::CBioSource & in_out_bioSource ) { }
    virtual string GetFromBioSource(const objects::CBioSource & in_out_bioSource ) const = 0;
    virtual vector<string> GetValsFromBioSource(const objects::CBioSource &src) const; // used to be GetVals
    virtual string GetLabel() const { return ""; }
    virtual vector<string> IsValid(const vector<string>& values) { vector<string> x; return x; };
    virtual void SetConstraint(const string& field_name, CConstRef<edit::CStringConstraint> string_constraint) {};
    virtual bool AllowMultipleValues() { return false; };

    vector<CRef<objects::edit::CApplyObject> > GetApplyObjects(CBioseq_Handle bsh);
    vector<CConstRef<CObject> > GetObjects(CBioseq_Handle bsh);
    string GetVal(const CObject& object);
    vector<string> GetVals(const CObject& object);
    void ClearVal(CObject& object);

    void SetSourceType(ESourceType type) { m_Type = type; }
private:
    ESourceType m_Type;
};


class CSrcTableOrganismNameColumn : public CSrcTableColumnBase
{
public:
    virtual bool AddToBioSource(
        objects::CBioSource & src, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInBioSource(
        objects::CBioSource & in_out_bioSource );
    virtual string GetFromBioSource(
        const objects::CBioSource & in_out_bioSource ) const;
    virtual string GetLabel() const { return "Organism Name"; }
};

class CSrcTaxnameAfterBinomialColumn : public CSrcTableOrganismNameColumn
{
public:
    virtual bool AddToBioSource(
        objects::CBioSource & src, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInBioSource(
        objects::CBioSource & in_out_bioSource );
    virtual string GetFromBioSource(
        const objects::CBioSource & in_out_bioSource ) const;
    virtual string GetLabel() const { return "Taxname after binomial"; }
private:
    string x_GetTextAfterNomial(const string& taxname) const;
};

class CSrcTableGenomeColumn : public CSrcTableColumnBase
{
public:
    virtual bool AddToBioSource(
        objects::CBioSource & src, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInBioSource(
        objects::CBioSource & in_out_bioSource );
    virtual string GetFromBioSource(
        const objects::CBioSource & in_out_bioSource ) const;
    virtual string GetLabel() const { return "Genome"; }
    virtual vector<string> IsValid(const vector<string>& values);
    CSrcTableGenomeColumn(const string& organelle = kEmptyStr);      
private:
    int m_organelle;
};

class CSrcTableOriginColumn : public CSrcTableColumnBase
{
public:
    virtual bool AddToBioSource(
        objects::CBioSource & src, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInBioSource(
        objects::CBioSource & in_out_bioSource );
    virtual string GetFromBioSource(
        const objects::CBioSource & in_out_bioSource ) const;
    virtual string GetLabel() const { return "Origin"; }
    virtual vector<string> IsValid(const vector<string>& values);
};


class CSrcTableTaxonIdColumn : public CSrcTableColumnBase
{
public:
    virtual bool AddToBioSource(
        objects::CBioSource & in_out_bioSource, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInBioSource(
        objects::CBioSource & in_out_bioSource );
    virtual string GetFromBioSource(
        const objects::CBioSource & in_out_bioSource ) const;
    virtual string GetLabel() const { return "Tax ID"; }
};

class CSrcTableCommonNameColumn : public CSrcTableColumnBase
{
public:
    virtual string GetFromBioSource( const objects::CBioSource & biosrc ) const;
};

class CSrcTableLineageColumn : public CSrcTableColumnBase
{
public:
    virtual string GetFromBioSource( const objects::CBioSource & biosrc ) const;
};

class CSrcTableDivisionColumn : public CSrcTableColumnBase
{
public:
    virtual string GetFromBioSource( const objects::CBioSource & biosrc ) const;
};

class CSrcTableDbxrefColumn : public CSrcTableColumnBase
{
public:
    CSrcTableDbxrefColumn(const string& db_name = kEmptyStr)
        : m_DbName(db_name) {}
    virtual string GetFromBioSource( const objects::CBioSource & biosrc ) const;
    virtual vector <string> GetValsFromBioSource(const objects::CBioSource & biosrc) const;
    virtual bool AllowMultipleValues();
    virtual bool AddToBioSource(
        objects::CBioSource & in_out_bioSource, const string & newValue, objects::edit::EExistingText existing_text);
    virtual void ClearInBioSource(objects::CBioSource & in_out_bioSource );
private:
    // designates the DB name
    // if it is empty, all dbxrefs are considered within the biosource
    string m_DbName;
};

class CSrcTableAllNotesColumn : public CSrcTableColumnBase
{
public:
    virtual vector <string> GetValsFromBioSource(const objects::CBioSource & biosrc) const;
    virtual string GetFromBioSource( const objects::CBioSource & in_out_bioSource ) const;
    virtual void ClearInBioSource(objects::CBioSource & in_out_bioSource );
    virtual bool AllowMultipleValues() { return true; };
}; 

class CSrcTableSubSourceColumn : public CSrcTableColumnBase
{
public:
  CSrcTableSubSourceColumn(objects::CSubSource::TSubtype subtype) { m_Subtype = subtype; };
    virtual bool AddToBioSource(
        objects::CBioSource & src, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInBioSource(
        objects::CBioSource & in_out_bioSource );
    virtual string GetFromBioSource(
        const objects::CBioSource & in_out_bioSource ) const;
    virtual vector <string> GetValsFromBioSource(const objects::CBioSource & biosrc) const;
    virtual string GetLabel() const { return objects::CSubSource::GetSubtypeName(m_Subtype); }
    virtual bool AllowMultipleValues() { return true; };
private:
    objects::CSubSource::TSubtype m_Subtype;
};

class CSrcTableOrgModColumn : public CSrcTableColumnBase
{
public:
    CSrcTableOrgModColumn(objects::COrgMod::TSubtype subtype) : m_Subtype(subtype) {}
    virtual bool AddToBioSource(
        objects::CBioSource & src, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInBioSource(
        objects::CBioSource & in_out_bioSource );
    virtual string GetFromBioSource(
        const objects::CBioSource & in_out_bioSource ) const;
    virtual vector <string> GetValsFromBioSource(const objects::CBioSource & biosrc) const;
    virtual string GetLabel() const { return objects::COrgMod::GetSubtypeName(m_Subtype); }
    virtual bool AllowMultipleValues() { return true; };
protected:
    objects::COrgMod::TSubtype m_Subtype;
};

class CSrcStructuredVoucherPartColumn : public CSrcTableOrgModColumn
{
public:
    enum EStructVouchPart {
        eUnknown =0,
        eColl,
        eInst,
        eSpecid
    };
    CSrcStructuredVoucherPartColumn(objects::COrgMod::TSubtype subtype, EStructVouchPart epart)
        : CSrcTableOrgModColumn(subtype), m_SubtypePart(epart) {}
    virtual bool AddToBioSource(
        objects::CBioSource & src, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInBioSource(
        objects::CBioSource & in_out_bioSource );
    virtual string GetFromBioSource(
        const objects::CBioSource & in_out_bioSource ) const;
    virtual vector<string> GetValsFromBioSource(const objects::CBioSource &src) const;
    virtual string GetLabel() const { 
        return objects::COrgMod::GetSubtypeName(m_Subtype) + GetName_StrVoucherPart(m_SubtypePart); }

    static const string& GetName_StrVoucherPart( EStructVouchPart stype_part );
    static EStructVouchPart GetStrVoucherPart_FromName ( const string& name );
private:
    void x_ParsePartsFromStructuredVoucher(const string& qualifier, string& inst, string& coll, string& id) const;
    EStructVouchPart m_SubtypePart;
};


class CSrcTablePrimerColumn : public CSrcTableColumnBase
{
public:
    enum EPrimerColType {
        eFwdSeq = 0,
        eFwdName,
        eRevSeq,
        eRevName,
        eNotPrimerCol
    };

    CSrcTablePrimerColumn(EPrimerColType col_type) 
        : m_ColType(col_type),
          m_ConstraintCol(eNotPrimerCol),
          m_StringConstraint(NULL)
        { };

    static EPrimerColType GetPrimerColumnType (const string& field_name);
    virtual bool AddToBioSource(
        objects::CBioSource & src, const string & newValue, objects::edit::EExistingText existing_text );
    virtual void ClearInBioSource(
        objects::CBioSource & src );
    virtual string GetFromBioSource(
        const objects::CBioSource & src ) const;
    virtual vector<string> GetValsFromBioSource(const CBioSource &src) const;
    
    virtual void SetConstraint(const string& field_name, CConstRef<edit::CStringConstraint> string_constraint);
    virtual bool AllowMultipleValues() { return true; };

protected:
    EPrimerColType m_ColType;
    EPrimerColType m_ConstraintCol;
    CRef<edit::CStringConstraint> m_StringConstraint;

    bool x_DoesReactionHaveSpace(const CPCRReaction& reaction);
    bool x_DoesPrimerSetHaveSpace(const CPCRPrimerSet& set);
    bool x_DoesReactionMatchConstraint(const CPCRReaction& reaction) const;
    vector<string> x_GetValues(EPrimerColType col, const CBioSource& src) const;
    vector<string> x_GetValues(EPrimerColType col, const CPCRReaction& reaction) const;
    vector<string> x_GetValues(EPrimerColType col, const CPCRPrimerSet& set) const;
    bool x_ApplyStringToReaction(const string& val, CPCRReaction& reaction, objects::edit::EExistingText existing_text);
    bool x_ApplyStringToPrimerSet(const string& val, CPCRPrimerSet& set, objects::edit::EExistingText existing_text);
    bool x_AddFieldToPrimerSet(const string& val, CPCRPrimerSet& set);
    bool x_ApplyStringToSrc(const string& val, CBioSource& src, objects::edit::EExistingText existing_text);
    void x_RemoveEmptyValues(CBioSource& src);
    void x_RemoveEmptyValues(CPCRReaction& reaction);
    void x_RemoveEmptyValues(CPCRPrimerSet& set);
};


class CSrcTableFwdPrimerSeqColumn : public CSrcTablePrimerColumn
{
public:
    CSrcTableFwdPrimerSeqColumn() : CSrcTablePrimerColumn(eFwdSeq) { };
    virtual string GetLabel() const { return "fwd-primer-seq"; }
};


class CSrcTableRevPrimerSeqColumn : public CSrcTablePrimerColumn
{
public:
    CSrcTableRevPrimerSeqColumn() : CSrcTablePrimerColumn(eRevSeq) { };
    virtual string GetLabel() const { return "rev-primer-seq"; }
};


class CSrcTableFwdPrimerNameColumn : public CSrcTablePrimerColumn
{
public:
    CSrcTableFwdPrimerNameColumn() : CSrcTablePrimerColumn(eFwdName) { };
    virtual string GetLabel() const { return "fwd-primer-name"; }
};


class CSrcTableRevPrimerNameColumn : public CSrcTablePrimerColumn
{
public:
    CSrcTableRevPrimerNameColumn() : CSrcTablePrimerColumn(eRevName) { };
    virtual string GetLabel() const { return "rev-primer-name"; }
};


class CSrcTableAllPrimersColumn : public CSrcTablePrimerColumn
{
public:
    CSrcTableAllPrimersColumn() : CSrcTablePrimerColumn(eNotPrimerCol) { };
    virtual string GetFromBioSource( const objects::CBioSource & src ) const;
    virtual void ClearInBioSource(objects::CBioSource & in_out_bioSource );
    virtual vector<string> GetValsFromBioSource(const objects::CBioSource &src) const;
    virtual bool AllowMultipleValues() { return true; };
};

class CSrcTableColumnBaseFactory
{
public:
    static CRef<CSrcTableColumnBase> Create(const objects::CSeqTable_column &column);
    static CRef<CSrcTableColumnBase> Create(const string &column_name);
};

CRef<objects::CSeq_table> GetSeqTableFromSeqEntry (objects::CSeq_entry_Handle seh);
CRef<objects::CSeq_table> GetSeqTableForSrcQualFromSeqEntry (objects::CSeq_entry_Handle seh, vector<string> qual_names);
CRef<CCmdComposite> ApplySrcTableToSeqEntry (CRef<objects::CSeq_table>values_table, objects::CSeq_entry_Handle seh);
CSourceRequirements *GetSrcRequirements(CSourceRequirements::EWizardType wizard_type, 
                                        CSourceRequirements::EWizardSrcType source_type, 
                                        CRef<objects::CSeq_table> values_table);
bool RelaxedMatch (CRef<objects::CSeq_id> id1, CRef<objects::CSeq_id> id2);

bool IsFwdPrimerName (string name);
bool IsRevPrimerName (string name);
bool IsFwdPrimerSeq (string name);
bool IsRevPrimerSeq (string name);

END_NCBI_SCOPE

#endif
    // _SRCEDIT_UTIL_H_
