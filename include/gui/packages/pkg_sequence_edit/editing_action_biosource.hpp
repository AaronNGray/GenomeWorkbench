/*  $Id: editing_action_biosource.hpp 45101 2020-05-29 20:53:24Z asztalos $
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

#ifndef _EDITING_ACTION_BIOSOURCE_H_
#define _EDITING_ACTION_BIOSOURCE_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/RNA_qual.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/PCRReaction.hpp>
#include <objects/seqfeat/PCRPrimer.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CEditingActionConstraint;

class IEditingActionBiosource : public IEditingAction 
{
public:
    IEditingActionBiosource(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature, const string &name);
    void SetFeat(CSeq_feat_Handle fh);
    void SetDesc(const CSeqdesc* desc);
protected:
    virtual void Find(EActionType action);
    virtual void FindRelated(EActionType action);
    virtual void Modify(EActionType action);
    virtual void SwapContext(IEditingAction* source);
    virtual bool SameObject();

    CRef<CSeqdesc> m_EditedDesc;
    CRef<CSeq_feat> m_EditedFeat;
    CBioSource *m_EditedBiosource;
    const CSeqdesc* m_Desc;
    CSeq_feat_Handle m_Feat;
    CSeq_entry_Handle m_DescContext;
    bool m_is_descriptor;
    bool m_is_feature;
};

class CEditingActionBiosourceTaxname : public IEditingActionBiosource
{
public:
    CEditingActionBiosourceTaxname(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionBiosourceTaxnameAfterBinomial : public IEditingActionBiosource
{
public:
    CEditingActionBiosourceTaxnameAfterBinomial(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
private:
    string x_GetTextAfterNomial(const string& taxname) const;
};

class CEditingActionBiosourceCommonName : public IEditingActionBiosource
{
public:
    CEditingActionBiosourceCommonName(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionBiosourceDivision : public IEditingActionBiosource
{
public:
    CEditingActionBiosourceDivision(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionBiosourceLineage : public IEditingActionBiosource
{
public:
    CEditingActionBiosourceLineage(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionBiosourceLocation : public IEditingActionBiosource
{
public:
    CEditingActionBiosourceLocation(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionBiosourceOrigin : public IEditingActionBiosource
{
public:
    CEditingActionBiosourceOrigin(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature);
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
};

class CEditingActionBiosourcePrimers : public IEditingActionBiosource
{
public:
    CEditingActionBiosourcePrimers(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature, bool is_fwd, bool is_rev, bool is_name, bool is_seq);
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
private:
    bool m_is_fwd, m_is_rev, m_is_name, m_is_seq;
    string *m_val;
    CRef< CPCRReaction > m_reaction;
    CRef< CPCRPrimer > m_primer;
};


class CEditingActionBiosourceOrgMod : public IEditingActionBiosource
{
public:
    CEditingActionBiosourceOrgMod(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature, const COrgMod::ESubtype subtype, const string &name = "CEditingActionBiosourceOrgMod");
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
protected:
    COrgMod::ESubtype m_qual;
    CRef< COrgMod > m_OrgMod;
    bool m_erase;
};

class CEditingActionBiosourceSubSource : public IEditingActionBiosource
{
public:
    CEditingActionBiosourceSubSource(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature, const CSubSource::ESubtype subtype, const string &name = "CEditingActionBiosourceSubSource");
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
protected:
    CSubSource::ESubtype m_qual;
    CRef< CSubSource > m_SubSource;
    bool m_erase;
};

class CEditingActionBiosourceAllNotes : public IEditingActionBiosource
{
public:
    CEditingActionBiosourceAllNotes(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature);
protected:
    virtual void Modify(EActionType action);
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);
protected:
    CRef< COrgMod > m_OrgMod;
    CRef< CSubSource > m_SubSource;
    bool m_erase;
};

class CEditingActionBiosourceStructuredVoucher : public CEditingActionBiosourceOrgMod
{
public:
    CEditingActionBiosourceStructuredVoucher(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature, const COrgMod::ESubtype subtype, const int voucher_type);
    enum eVoucherType
    {
        eVoucher_inst = 0,
        eVoucher_coll,
        eVoucher_id
    };
protected:
    virtual bool IsSetValue(void);
    virtual void SetValue(const string &value);
    virtual string GetValue(void);
    virtual void ResetValue(void);

private: 
    void Parse(const string &str, string &inst, string &coll, string &id);
    int m_voucher_type;
};

class CEditingActionBiosourceDbxref : public IEditingActionBiosource
{
public:
    CEditingActionBiosourceDbxref(CSeq_entry_Handle seh, bool is_descriptor, bool is_feature);
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


IEditingActionBiosource* CreateActionBiosource(CSeq_entry_Handle seh, string field);

END_NCBI_SCOPE

#endif
    // _EDITING_ACTION_BIOSOURCE_H_
