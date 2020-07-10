#ifndef GUI_OBJUTILS___MACRO_BIODATA_ITER__HPP
#define GUI_OBJUTILS___MACRO_BIODATA_ITER__HPP
/*  $Id: macro_biodata_iter.hpp 45050 2020-05-18 17:58:56Z asztalos $
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
 * Authors: Andrea Asztalos, Anatoly Osipov
 *
 * File Description: FOR EACH iterators used in macros
 *
 */

/// @file macro_biodata_iter.hpp
/// Biodata iterators for macro execution

#include <corelib/ncbiobj.hpp>
#include <serial/objectinfo.hpp>
#include <serial/iterator.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/general/User_object.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub.hpp>

#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <gui/gui_export.h>
#include <gui/objutils/objects.hpp>
#include <gui/objutils/macro_exec.hpp>
#include <gui/objutils/macro_cmd_composite.hpp>
#include <unordered_map>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)


///////////////////////////////////////////////////////////////////////////////
/// IMacroBioDataIter - common interface of all iterators used in the editing macros
/// The type of the iterator is defined by "foreach" selector in the macro.
///
class NCBI_GUIOBJUTILS_EXPORT IMacroBioDataIter : public CObject
{
public:
    virtual ~IMacroBioDataIter() {}

    virtual IMacroBioDataIter& Begin( void ) = 0;
    virtual IMacroBioDataIter& Next( void )  = 0;
    virtual bool               IsEnd( void ) const = 0;
    virtual bool               IsBegin( void ) = 0;

    virtual size_t             GetCount() const { return 0; }

    /// Dereferences the underlying iterator
    /// @return pointer to the object and its scope
    virtual SConstScopedObject GetScopedObject() const = 0;
    
    /// Used for logging
    /// @return unique description about the object
    virtual string GetBestDescr() const = 0;

    /// Creates a copy of the object that will be edited. 
    /// @note The function should be called prior to any modification of the original data in 
    /// memory and prior to calling RunEditCommand(). 
    /// Usually, each of these functions should be called once per iterator resolving.
    virtual void BuildEditedObject() = 0;
    

    CObjectInfo GetEditedObject() { return m_CurIterOI; }
    
    /// Constructs and executes the editing command. At the end it also stores this editing command.
    ///@param[in,out] cmd_composite
    ///  Composite command that stores all executed commands.
    ///@note Internally, each command swaps the read-only object with the updated object.
    ///@sa CMacroCmdComposite().
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite) = 0;

    /// Constructs and executes the delete command. At the end, the command is stored and 
    /// the iterator is advanced to the next object.
    ///@param[in,out] cmd_composite
    ///  Composite command that stores all executed commands.
    ///@sa CMacroCmdComposite().
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite) = 0;

    /// Method for executing a specific command.
    ///@param[in] cmd
    ///  Command to be executed. It has been previously constructed elsewhere.
    ///@param[in,out] cmd_composite
    ///  Composite command that stores all executed commands.
    ///@note This is used when changes have been made to an object other than the one pointed by the iterator.
    /// See for example, CMacroFunction_SynchronizeCDSPartials
    void RunCommand(CRef<CCmdComposite> cmd, CMacroCmdComposite* cmd_composite);

    bool IntendToDelete() const { return m_DeleteObject; }
    virtual void SetToDelete(bool intent) { m_DeleteObject = intent; }

    bool IsModified() const { return m_Modified; }
    void SetModified() { m_Modified |= true; }

    /// returns choice type string for a given identifier field_name
    ///@param[in] field_name
    ///  Name of the choice data member to be resolved
    ///@param[out] choice_type
    ///  Name of the choice variant
    ///@sa CMacroFunction_ChoiceType
    bool GetChoiceType(const string& field_name, string& choice_type) const;

    // functions that provide context on the object pointed by the iterator
    virtual bool IsDescriptor() const = 0;
    virtual bool IsFeature() const = 0;
    virtual objects::CBioseq_Handle GetBioseqHandle(void) const = 0;

    objects::CSeq_entry_Handle GetSEH() const { return m_Seh; }

    bool IsSetSeqSubmit( void ) const { return m_SeqSubmit.NotEmpty(); }
    const objects::CSeq_submit& GetSeqSubmit( void ) const { return *m_SeqSubmit; }
    void SetSeqSubmit(const objects::CSeq_submit& submit) { m_SeqSubmit.Reset(&submit); }

    void SetOutputStream(CNcbiOstream* str) { m_Ostream = str; }
    bool IsSetOutputStream() const { return (m_Ostream != nullptr);  }
    CNcbiOstream& GetOutputStream() const { return *m_Ostream; }

    // functions to handle tables
    void SetTableToApply(const string& file_name, size_t col,
        const string& delimiter, bool merge_de, bool split_firstcol, bool convert_multi, bool merge_firstcols);
    const string& GetTableName() const;

    string GetValueFromTable(size_t col);
    bool FindInTable(const CMQueryNodeValue::TObs& objs);
    bool FindInTable(const vector<string>& strs);

    string GetReport();

protected:
    IMacroBioDataIter(const objects::CSeq_entry_Handle& entry) 
        : m_Seh (entry), m_DeleteObject(false), m_Modified(false), m_TableToApply(kEmptyStr) {}

    /// Initializes member variable m_CurIterOI
    virtual void x_SetDefaultObjectInfo() = 0;
    void x_Init();


    CObjectInfo m_CurIterOI;
    objects::CSeq_entry_Handle m_Seh;
    CConstRef<objects::CSeq_submit> m_SeqSubmit;
    CNcbiOstream* m_Ostream{ nullptr };

    bool m_DeleteObject; ///< indicates the intention to delete the underlying object
    bool m_Modified;     ///< indicates whether the edited object has been modified. When it's TRUE, call RunEditCommand() 

    struct STableToApply
    {
        STableToApply(const string& file_name)
            : m_Name(file_name), m_NrCols(0), m_SplitFlags(0){}

        bool SetTableToApply(const string& file_name, size_t col);
        bool GetValueFromTable(size_t col, string& value);

        string m_Name;

        using TIndex = unordered_map<string, unsigned>;
        TIndex m_Index;  ///< stores the match_field value and the row number where that value appears in the table
        TIndex m_IdenticalItems;
        TIndex::const_iterator m_CurrentPos;
        size_t m_NrCols;  /// < number of actual columns in the table
        string m_Delimiter;  ///< column delimiter
        NStr::TSplitFlags m_SplitFlags{ 0 };
        bool m_SplitFirstCol{ false }; 
        bool m_ConvertMulti{ false };
        bool m_MergeFirstCols{ false };  ///< set to true, merges first two columns to form the seq-id match field

        using TPresent = vector<pair<string, bool>>;  ///< true value for a key indicates that was present in the record
        TPresent m_State; ///< to report values of the matching field that are present in the table, but not present in the data

        using TTableValues = vector<string>;
        list<pair<size_t, TTableValues>> m_TableValues;
    private:
        void x_InitAndCheck(const string& file_name, size_t col);
        bool x_CacheColFromTable(size_t col);
    };

    STableToApply m_TableToApply;
};


////////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_FeatIterBase - iterates over any type of features
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_FeatIterBase : public IMacroBioDataIter
{
public:
    CMacroBioData_FeatIterBase(const objects::CSeq_entry_Handle& entry, const objects::SAnnotSelector& sel);
    CMacroBioData_FeatIterBase(const objects::CBioseq_Handle& bsh, const objects::SAnnotSelector& sel, const objects::CSeq_loc& loc);
    virtual ~CMacroBioData_FeatIterBase() {}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
    virtual bool IsEnd(void) const;
    virtual bool IsBegin(void);
    virtual size_t             GetCount() const;

    virtual SConstScopedObject GetScopedObject() const;
    virtual string GetBestDescr() const;

    virtual void BuildEditedObject();
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite);
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite);
    
    virtual bool IsDescriptor() const { return false; }
    virtual bool IsFeature() const { return true; }

    virtual objects::CBioseq_Handle GetBioseqHandle(void) const;

    virtual void SetToDelete(bool intent);

    const vector<string>& GetTaxnames() const { return m_Taxnames; }
    vector<string>& SetTaxnames() { return m_Taxnames; }

    void SetCreateGeneralIdFlag();
    bool GetCreateGeneralIdFlag() const { return m_CreateGeneralId; }
protected:
    virtual void x_SetDefaultObjectInfo();

    objects::CFeat_CI m_FeatIter;
    CConstRef<objects::CSeq_feat> m_OrigFeat;
    CRef<objects::CSeq_feat> m_EditedFeat;
    CConstRef<objects::CSeq_feat> m_FirstFeat;
    
    // caching the taxnames for CMacroFunction_FixProteinFormat
    vector<string> m_Taxnames;
    // auxiliary flag for creating protein seq-ids
    bool m_CreateGeneralId{ false };
    // auxiliary data used for deleting features
    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > m_ProductToCDS;
private:
    CMacroBioData_FeatIterBase(const CMacroBioData_FeatIterBase&);
    CMacroBioData_FeatIterBase& operator=(const CMacroBioData_FeatIterBase&);
};


struct SFeatInterval
{
    TSeqRange m_Range;
    bool left_closed{ true };
    bool right_closed{ true };
    // default behavior - is the same as given by the OM
    /*
    [left, right) - features that straddle the 'open' boundary of the interval are NOT included in the selection
    OM's selectors/iterators always include all features that straddle the left and right boundaries of an interval.
    There is no flag to modify this behavior.
    */
    SFeatInterval(const TSeqRange& range, bool left = true , bool right = true)
        : m_Range(range), left_closed(left), right_closed(right) {}
};

////////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_FeatIntervalIter - iterates over features within [|(left, right]|) intervals
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_FeatIntervalIter : public IMacroBioDataIter
{
public:
    CMacroBioData_FeatIntervalIter(const objects::CBioseq_Handle& bsh, 
        const objects::SAnnotSelector& sel, const SFeatInterval& feat_interval);
    virtual ~CMacroBioData_FeatIntervalIter() {}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
    virtual bool IsEnd(void) const;
    virtual bool IsBegin(void);

    virtual SConstScopedObject GetScopedObject() const;
    virtual string GetBestDescr() const;

    virtual void BuildEditedObject();
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite) {}
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite) {}

    virtual bool IsDescriptor() const { return false; }
    virtual bool IsFeature() const { return true; }

    virtual objects::CBioseq_Handle GetBioseqHandle(void) const;

protected:
    virtual void x_SetDefaultObjectInfo();

    void x_StepToNextFeat();

    objects::CFeat_CI m_FeatIter;
    CConstRef<objects::CSeq_feat> m_OrigFeat;
    CRef<objects::CSeq_feat> m_EditedFeat;
    CConstRef<objects::CSeq_feat> m_FirstFeat;
    SFeatInterval m_FeatInterval;

private:
    CMacroBioData_FeatIntervalIter(const CMacroBioData_FeatIntervalIter&);
    CMacroBioData_FeatIntervalIter& operator=(const CMacroBioData_FeatIntervalIter&);
};


////////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_BioSourceIter - iterates over all biosource descriptors and features in the seq-entry
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_BioSourceIter : public IMacroBioDataIter
{
public:
    CMacroBioData_BioSourceIter(const objects::CSeq_entry_Handle& entry);
    virtual ~CMacroBioData_BioSourceIter(){}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
    virtual bool IsEnd(void) const;
    virtual bool IsBegin(void);

    virtual SConstScopedObject GetScopedObject() const;
    virtual string GetBestDescr() const;

    virtual void BuildEditedObject();
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite);
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite);

    virtual bool IsDescriptor() const;
    virtual bool IsFeature() const;

    virtual objects::CBioseq_Handle GetBioseqHandle(void) const;

    enum ECollDateType {
        eCollDateType_NotSet,
        eAmbiguous_True,
        eAmbiguous_False
    };

    ECollDateType GetCollDateType() const { return m_CollDateType; }
    void SetCollDateType(bool ambiguous) { m_CollDateType = (ambiguous) ? eAmbiguous_True : eAmbiguous_False; }

protected:
    virtual void x_SetDefaultObjectInfo();
private:
    objects::CFeat_CI m_SrcFeatIter;
    objects::CSeqdesc_CI m_SrcDescrIter;
    objects::CBioseq_CI m_SeqIter; ///> provides context information
    CConstRef<objects::CBioSource> m_OrigSource;
    CRef<objects::CBioSource> m_EditedSource;
    CConstRef<objects::CBioSource> m_FirstSource;

    ECollDateType m_CollDateType;
    const objects::CBioSource* x_GetBioSource();
private:
    // prohibit copying and assigning
    CMacroBioData_BioSourceIter(const CMacroBioData_BioSourceIter&);
    CMacroBioData_BioSourceIter& operator=(const CMacroBioData_BioSourceIter&);
};


////////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_PubdescIter - iterates over all publication descriptors and features in the seq-entry
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_PubdescIter : public IMacroBioDataIter
{
public:
    CMacroBioData_PubdescIter(const objects::CSeq_entry_Handle& entry);
    virtual ~CMacroBioData_PubdescIter(){}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
    virtual bool IsEnd(void) const;
    virtual bool IsBegin(void);

    virtual SConstScopedObject GetScopedObject() const;
    virtual string GetBestDescr() const;

    virtual void BuildEditedObject();
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite);
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite);

    virtual bool IsDescriptor() const;
    virtual bool IsFeature() const;

    virtual objects::CBioseq_Handle GetBioseqHandle(void) const;

    using TPmidLookupMap = map<long int, CRef<objects::CPub>>;

    const TPmidLookupMap& GetPmidLookupMap() const { return m_PmidLookupMap; }
    TPmidLookupMap& SetPmidLookupMap() { return m_PmidLookupMap; }

    using TDOILookupMap = map<string, CRef<objects::CPubdesc>>;
    TDOILookupMap& SetDOILookupMap() { return m_DOILookupMap; }

protected:
    virtual void x_SetDefaultObjectInfo();
private:

    class CSubmitblockIter : public CObject
    {
    public:
        CSubmitblockIter(CConstRef<objects::CSeq_submit> seqsubmit);
        ~CSubmitblockIter() {}

        bool Processed() const { return m_Processed; }
        void MarkProcessed() { m_Processed = true; }

        CRef<objects::CSubmit_block> BuildEditedObject();
        const objects::CSubmit_block& GetOrigSubmitBlock() const { return m_OrigSubmitBlock.GetObject(); }
        objects::CSubmit_block& GetEditedSubmitBlock() const { return m_EditedSubmitBlock.GetNCObject(); }

    private:
        CConstRef<objects::CSubmit_block> m_OrigSubmitBlock{ nullptr };
        CRef<objects::CSubmit_block> m_EditedSubmitBlock{ nullptr };
        bool m_Processed{ false };
    };

    objects::CFeat_CI m_PubFeatIter;
    objects::CSeqdesc_CI m_PubDescrIter;
    objects::CBioseq_CI m_SeqIter; ///> provides context information

    CConstRef<objects::CPubdesc> m_OrigPubdesc;
    CRef<objects::CPubdesc> m_EditedPubdesc;
    CConstRef<objects::CPubdesc> m_FirstPubdesc;

    // this will be the last in the iterated objects list
    CRef<CSubmitblockIter> m_SubmitBlockIter;

    // cache the looked up pmids
    TPmidLookupMap m_PmidLookupMap;

    // cache the looked up dois
    TDOILookupMap m_DOILookupMap;

    // prohibit copying and assigning
    CMacroBioData_PubdescIter(const CMacroBioData_PubdescIter&);
    CMacroBioData_PubdescIter& operator=(const CMacroBioData_PubdescIter&);
};

/////////////////////////////////////////////////////////////////
/// CMacroBioData_SeqdescIter- iterates over all types of sequence descriptors
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_SeqdescIter : public IMacroBioDataIter
{
public:
    CMacroBioData_SeqdescIter(const objects::CSeq_entry_Handle& entry);
    virtual ~CMacroBioData_SeqdescIter(){}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
    virtual bool IsEnd(void) const;
    virtual bool IsBegin(void);

    virtual SConstScopedObject GetScopedObject() const;
    virtual string GetBestDescr() const;

    virtual void BuildEditedObject();
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite);
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite);

    virtual bool IsDescriptor() const { return true; }
    virtual bool IsFeature() const { return false; }

    virtual objects::CBioseq_Handle GetBioseqHandle(void) const;
protected:
    virtual void x_SetDefaultObjectInfo();

    string x_GetSeqId() const;
    objects::CBioseq_CI m_SeqIter;  ///> provides context information
    objects::CSeqdesc_CI m_DescIter;

private:
    CConstRef<objects::CSeqdesc> m_OrigSeqdesc;
    CRef<objects::CSeqdesc> m_EditedSeqdesc;
    CConstRef<objects::CSeqdesc> m_FirstSeqdesc;

    CMacroBioData_SeqdescIter(const CMacroBioData_SeqdescIter&);
    CMacroBioData_SeqdescIter& operator=(const CMacroBioData_SeqdescIter&);
};

////////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_MolInfoIter - iterates over all molinfos in the seq-entry
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_MolInfoIter : public CMacroBioData_SeqdescIter
{
public:
    CMacroBioData_MolInfoIter(const objects::CSeq_entry_Handle& entry);
    virtual ~CMacroBioData_MolInfoIter(){}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
    virtual bool IsEnd(void) const;
    virtual bool IsBegin(void);

    virtual SConstScopedObject GetScopedObject() const;
    virtual string GetBestDescr() const;

    virtual void BuildEditedObject();
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite);
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite);

protected:
    virtual void x_SetDefaultObjectInfo();

private:
    CConstRef<objects::CMolInfo> m_OrigMolinfo;
    CRef<objects::CMolInfo> m_EditedMolinfo;
    CConstRef<objects::CMolInfo> m_FirstMolinfo;

    const objects::CMolInfo* x_GetMolInfo();

    // prohibit copying and assigning
    CMacroBioData_MolInfoIter(const CMacroBioData_MolInfoIter&);
    CMacroBioData_MolInfoIter& operator=(const CMacroBioData_MolInfoIter&);
};


/////////////////////////////////////////////////////////////////
/// CMacroBioData_UserObjectIter - iterates over all user objects in the seq-entry
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_UserObjectIter : public CMacroBioData_SeqdescIter
{
public:
    CMacroBioData_UserObjectIter(const objects::CSeq_entry_Handle& entry);
    virtual ~CMacroBioData_UserObjectIter(){}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
    virtual bool IsEnd(void) const;
    virtual bool IsBegin(void);

    virtual SConstScopedObject GetScopedObject() const;
    virtual string GetBestDescr() const;

    virtual void BuildEditedObject();
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite);
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite);

protected:
    virtual void x_SetDefaultObjectInfo();

    CConstRef<objects::CUser_object> m_OrigUserObj;
    CRef<objects::CUser_object> m_EditedUserObj;
    CConstRef<objects::CUser_object> m_FirstUserObj;

private:
    // prohibit copying and assigning
    CMacroBioData_UserObjectIter(const CMacroBioData_UserObjectIter&);
    CMacroBioData_UserObjectIter& operator=(const CMacroBioData_UserObjectIter&);

};

////////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_StructCommentIter - iterates over all structured comments in the seq-entry
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_StructCommentIter : public CMacroBioData_UserObjectIter
{
public:
    CMacroBioData_StructCommentIter(const objects::CSeq_entry_Handle& entry);
    ~CMacroBioData_StructCommentIter(){}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);

    virtual string GetBestDescr() const;

private:
    void x_StepToNextStructComment();

    // prohibit copying and assigning
    CMacroBioData_StructCommentIter(const CMacroBioData_StructCommentIter&);
    CMacroBioData_StructCommentIter& operator=(const CMacroBioData_StructCommentIter&);
};


////////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_DBLinkIter - iterates over all DBlinks in the seq-entry
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_DBLinkIter : public CMacroBioData_UserObjectIter
{
public:
    CMacroBioData_DBLinkIter(const objects::CSeq_entry_Handle& entry);
    ~CMacroBioData_DBLinkIter(){}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
 
    virtual string GetBestDescr() const;

private:
    const objects::CUser_object* x_GetDBLink();

    // prohibit copying and assigning
    CMacroBioData_DBLinkIter(const CMacroBioData_DBLinkIter&);
    CMacroBioData_DBLinkIter& operator=(const CMacroBioData_DBLinkIter&);
};

/////////////////////////////////////////////////////////////////
/// CMacroBioData_SeqAlignIter - iterates over sequence alignments
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_SeqAlignIter : public IMacroBioDataIter
{
public:
    CMacroBioData_SeqAlignIter(const objects::CSeq_entry_Handle& entry);
    virtual ~CMacroBioData_SeqAlignIter() {}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
    virtual bool IsEnd(void) const;
    virtual bool IsBegin(void);

    virtual SConstScopedObject GetScopedObject() const;
    virtual string GetBestDescr() const;

    virtual void BuildEditedObject();
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite);
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite);

    virtual bool IsDescriptor() const { return false; }
    virtual bool IsFeature() const { return false; }

    virtual objects::CBioseq_Handle GetBioseqHandle(void) const { return objects::CBioseq_Handle(); }
protected:
    virtual void x_SetDefaultObjectInfo();
private:
    objects::CAlign_CI m_AlignIter;
    CConstRef<objects::CSeq_align> m_OrigSeqalign;
    CRef<objects::CSeq_align> m_EditedSeqalign;
private:
    CMacroBioData_SeqAlignIter(const CMacroBioData_SeqAlignIter&);
    CMacroBioData_SeqAlignIter& operator=(const CMacroBioData_SeqAlignIter&);
};


/////////////////////////////////////////////////////////////////
/// CMacroBioData_SeqIter- iterates over sequences (na and aa)
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_SeqIter : public IMacroBioDataIter
{
public:
    virtual ~CMacroBioData_SeqIter(){}
    static CMacroBioData_SeqIter* s_MakeSeqIterator(const objects::CSeq_entry_Handle& entry, objects::CSeq_inst::EMol mol_type);

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
    virtual bool IsEnd(void) const;
    virtual bool IsBegin(void);

    virtual SConstScopedObject GetScopedObject() const;
    virtual string GetBestDescr() const;

    virtual void BuildEditedObject();
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite);
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite);

    virtual bool IsDescriptor() const { return false; }
    virtual bool IsFeature() const { return false; }

    virtual objects::CBioseq_Handle GetBioseqHandle(void) const;
protected:
    virtual void x_SetDefaultObjectInfo();
    CMacroBioData_SeqIter(const objects::CSeq_entry_Handle& entry);
protected:
    objects::CBioseq_CI m_SeqIter;
    CConstRef<objects::CBioseq> m_OrigSeq;
    CRef<objects::CSeq_entry> m_EditedEntry;
    CRef<objects::CBioseq> m_EditedSeq;

private:
    CMacroBioData_SeqIter(const CMacroBioData_SeqIter&);
    CMacroBioData_SeqIter& operator=(const CMacroBioData_SeqIter&);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_SeqNAIter : public CMacroBioData_SeqIter
{
public:
    CMacroBioData_SeqNAIter(const objects::CSeq_entry_Handle& entry);

    virtual IMacroBioDataIter& Begin(void);
    virtual bool IsBegin(void);
};

class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_SeqAAIter : public CMacroBioData_SeqIter
{
public:
    CMacroBioData_SeqAAIter(const objects::CSeq_entry_Handle& entry);

    virtual IMacroBioDataIter& Begin(void);
    virtual bool IsBegin(void);
};

/////////////////////////////////////////////////////////////////
/// CMacroBioData_SeqSetIter- iterates over sets
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_SeqSetIter : public IMacroBioDataIter
{
public:
    CMacroBioData_SeqSetIter(const objects::CSeq_entry_Handle& entry);
    virtual ~CMacroBioData_SeqSetIter(){}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
    virtual bool IsEnd(void) const;
    virtual bool IsBegin(void);

    virtual SConstScopedObject GetScopedObject() const;
    virtual string GetBestDescr() const;

    virtual void BuildEditedObject();
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite);
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite);

    virtual bool IsDescriptor() const { return false; }
    virtual bool IsFeature() const { return false; }

    virtual objects::CBioseq_Handle GetBioseqHandle(void) const { return objects::CBioseq_Handle(); }
protected:
    virtual void x_SetDefaultObjectInfo();
private:
    objects::CSeq_entry_CI m_SeqSetIter;
    CConstRef<objects::CBioseq_set> m_OrigSeqSet;
    CRef<objects::CBioseq_set> m_EditedSeqSet;
private:
    CMacroBioData_SeqSetIter(const CMacroBioData_SeqSetIter&);
    CMacroBioData_SeqSetIter& operator=(const CMacroBioData_SeqSetIter&);
};


////////////////////////////////////////////////////////////////////////////////
/// CMacroBioData_TSEntryIterBase - iterator pointing to the top seq-entry
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData_TSEntryIter : public IMacroBioDataIter
{
public:
    CMacroBioData_TSEntryIter(const objects::CSeq_entry_Handle& entry);
    virtual ~CMacroBioData_TSEntryIter() {}

    virtual IMacroBioDataIter& Begin(void);
    virtual IMacroBioDataIter& Next(void);
    virtual bool IsEnd(void) const;
    virtual bool IsBegin(void);

    virtual SConstScopedObject GetScopedObject() const;
    virtual string GetBestDescr() const;

    virtual void BuildEditedObject();
    virtual void RunEditCommand(CMacroCmdComposite* cmd_composite);
    virtual void RunDeleteCommand(CMacroCmdComposite* cmd_composite);

    virtual bool IsDescriptor() const { return false; }
    virtual bool IsFeature() const { return false; }

    virtual objects::CBioseq_Handle GetBioseqHandle(void) const { return objects::CBioseq_Handle(); }

protected:
    virtual void x_SetDefaultObjectInfo();

    objects::CSeq_entry_CI m_EntryIter;
    CConstRef<objects::CSeq_entry> m_OrigTSE;
    CRef<objects::CSeq_entry> m_EditedTSE;

private:
    CMacroBioData_TSEntryIter(const CMacroBioData_TSEntryIter&);
    CMacroBioData_TSEntryIter& operator=(const CMacroBioData_TSEntryIter&);
};


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_BIODATA_ITER__HPP
