/*  $Id: pubdesc_editor.hpp 42440 2019-02-26 20:48:50Z asztalos $
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
#ifndef _PUBDESC_EDITOR_H_
#define _PUBDESC_EDITOR_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/feat_ci.hpp>
#include <util/xregexp/regexp.hpp>
#include <objtools/cleanup/capitalization_string.hpp>

BEGIN_NCBI_SCOPE

class IPubDescEditor
{
public:
    virtual ~IPubDescEditor() {}
    CRef<CCmdComposite> GetCommand(objects::CSeq_entry_Handle tse, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(nullptr));

protected:
    IPubDescEditor(const string& title) : m_Title(title) {}
    IPubDescEditor(objects::CSeq_entry_Handle tse, const string& title) : m_tse(tse), m_Title(title) {}
    void x_ApplyToSeqAndFeat(CCmdComposite* composite);
    void x_ApplyToDescriptors(const objects::CSeq_entry& se, CCmdComposite* composite);
    
    virtual void x_ApplyToSeqSubmit(CCmdComposite* composite) {}
    virtual bool x_ApplyToPubdesc(objects::CPubdesc& pubdesc) = 0;

    objects::CSeq_entry_Handle m_tse;
    CConstRef<objects::CSubmit_block> m_SubmitBlock;
    string m_Title;
};

class CFixTitleCap : public IPubDescEditor
{
public:
    CFixTitleCap() : IPubDescEditor("Fix Title Capitalization") {}
    CFixTitleCap(objects::CSeq_entry_Handle seh) : IPubDescEditor(seh, "Fix Title Capitalization") {}

    static string s_GetTitle(const objects::CTitle& title);
    static void s_SetTitle(objects::CTitle& title, const string& value);

protected:
    virtual bool x_ApplyToPubdesc(objects::CPubdesc& pubdesc);
    friend class CFixAllCap;
private:
    bool x_FixTitle(string& title);
};

class CFixAllCap : public IPubDescEditor
{
public:
    CFixAllCap() : IPubDescEditor("Fix All Capitalization") {}
protected:
    virtual bool x_ApplyToPubdesc(objects::CPubdesc& pubdesc);
};

class CReloadPublications : public IPubDescEditor
{
public:
    CReloadPublications() : IPubDescEditor("Reload Publications") {}
    void LookUpByPmid(objects::CPub& pub, int pmid, bool& modified);
protected:
    virtual bool x_ApplyToPubdesc(objects::CPubdesc& pubdesc);
};


// Classes that modify authors

class IAuthorEditor : public IPubDescEditor
{
public:
    virtual ~IAuthorEditor() {}
    static string s_CapitalizeInitial(const string &orig);
    static bool s_FixInitials(objects::CPerson_id& pid);
    static bool s_FixInitialsNew(objects::CPerson_id& pid);
    static void s_ExtractSuffixFromInitials(objects::CName_std& name);
    static void s_FixEtAl(objects::CName_std& name);
protected:
    IAuthorEditor(const string& title) : IPubDescEditor(title) {}
    IAuthorEditor(objects::CSeq_entry_Handle tse, const string& title) : IPubDescEditor(tse, title) {}
    virtual bool x_ApplyToPubdesc(objects::CPubdesc& pubdesc);
    virtual void x_ApplyToSeqSubmit(CCmdComposite* composite);
    virtual bool x_ApplyToCAuth(objects::CAuth_list& authors) = 0;

    friend class CFixAllCap;
private:
    void x_ApplyToPubequiv(objects::CPub_equiv& pe, bool& modified);
};


class CReverseAuthorNames : public IAuthorEditor
{
public:
    CReverseAuthorNames() : IAuthorEditor("Reverse Author Last and First Name") {}
    bool ApplyToPubdesc(CConstRef<CObject> obj, objects::CSeq_entry_Handle tse, CCmdComposite& composite);
protected:
    virtual bool x_ApplyToCAuth(objects::CAuth_list& authors);
};


class CFixAuthorInitials : public IAuthorEditor
{
public:
    CFixAuthorInitials() : IAuthorEditor("Truncate Author Middle Initials") {}
protected:
    virtual bool x_ApplyToCAuth(objects::CAuth_list& authors);
};

class CFixAuthorCap : public IAuthorEditor
{
public:
    CFixAuthorCap() : IAuthorEditor("Fix Author Capitalization") {}
    CFixAuthorCap(objects::CSeq_entry_Handle seh) : IAuthorEditor(seh, "Fix Author Capitalization") {}
protected:
    virtual bool x_ApplyToCAuth(objects::CAuth_list& authors);
};

class CStripAuthorSuffix : public IAuthorEditor
{
public:
    CStripAuthorSuffix() : IAuthorEditor("Strip Author Suffixes") {}
protected:
    virtual bool x_ApplyToCAuth(objects::CAuth_list& authors);
};

class CRemoveAuthorConsortium : public IAuthorEditor
{
public:
    CRemoveAuthorConsortium() : IAuthorEditor("Remove Author Consortiums") {}
protected:
    virtual bool x_ApplyToCAuth(objects::CAuth_list& authors);
};

class CConvertAllAuthorToConsortium : public IAuthorEditor
{
public:
    CConvertAllAuthorToConsortium() : IAuthorEditor("Convert All Authors To Consortiums") {}
protected:
    virtual bool x_ApplyToCAuth(objects::CAuth_list& authors);
};

class CConvertAuthorToConsortiumWhereConstraint : public IAuthorEditor
{
public:
    CConvertAuthorToConsortiumWhereConstraint() : IAuthorEditor("Convert Author to Consortiums Where Last Name Contains Consortium") {}
    ~CConvertAuthorToConsortiumWhereConstraint();
    void SetConstraint(const string &input);
protected:
    virtual bool x_ApplyToCAuth(objects::CAuth_list& authors);
private:
    CRegexp* m_Regex;
};

// Classes that modify affiliation fields

class IAffilEditor : public IAuthorEditor
{
public:
    virtual ~IAffilEditor() {}
protected:
    IAffilEditor(const string& title) : IAuthorEditor(title) {}
    IAffilEditor(objects::CSeq_entry_Handle seh, const string& title) : IAuthorEditor(seh, title) {}
    virtual bool x_ApplyToCAuth(objects::CAuth_list& authors);
    virtual bool x_ApplyToCAffil(objects::CAffil& affil) = 0;
};


class CFixUSA : public IAffilEditor
{
public:
    CFixUSA() : IAffilEditor("Fix USA and States") {}
protected:
    virtual bool x_ApplyToCAffil(objects::CAffil& affil);
};


class CFixCountryCap : public IAffilEditor
{
public:
    CFixCountryCap() : IAffilEditor("Fix Country Capitalization") {}
protected:
    virtual bool x_ApplyToCAffil(objects::CAffil& affil);
};


class CFixAffilCap : public IAffilEditor
{
public:
    CFixAffilCap() : IAffilEditor("Fix Affiliation Capitalization") {}
    CFixAffilCap(objects::CSeq_entry_Handle seh) : IAffilEditor(seh, "Fix Affiliation Capitalization") {}
protected:
    virtual bool x_ApplyToCAffil(objects::CAffil& affil);
};


// Classes that remove publication fields

class IRemovePubs
{
public:
    virtual ~IRemovePubs() {}
    CRef<CCmdComposite> GetCommand(objects::CSeq_entry_Handle tse);

protected:
    IRemovePubs(const string& title) : m_Title(title) {}

    virtual void x_ApplyToSeqAndFeat(CCmdComposite* composite);
    void x_ApplyToDescriptors(const objects::CSeq_entry& se, CCmdComposite* composite);

    virtual bool x_ApplyToPubdesc(objects::CPubdesc& pubdesc) = 0;

    objects::CSeq_entry_Handle m_tse;
    string m_Title;
};


class CRemoveUnpublishedPubs : public IRemovePubs
{
public:
    CRemoveUnpublishedPubs() : IRemovePubs("Delete Unpublished Publications") {}
protected:
    virtual bool x_ApplyToPubdesc(objects::CPubdesc& pubdesc);
};


class CRemoveInPressPubs : public IRemovePubs
{
public:
    CRemoveInPressPubs() : IRemovePubs("Delete In Press Publications") {}
protected:
    virtual bool x_ApplyToPubdesc(objects::CPubdesc& pubdesc);
};


class CRemovePublishedPubs : public IRemovePubs
{
public:
    CRemovePublishedPubs() : IRemovePubs("Delete Published Publications") {}
protected:
    virtual bool x_ApplyToPubdesc(objects::CPubdesc& pubdesc);
};


class CRemoveCollidingPubs : public IRemovePubs
{
public:
    CRemoveCollidingPubs() : IRemovePubs("Delete Colliding Publications") {}
protected:
    virtual void x_ApplyToSeqAndFeat(CCmdComposite* composite);
    virtual bool x_ApplyToPubdesc(objects::CPubdesc& pubdesc);
private:
    bool m_SecondPass;
    map<string,int> m_collisions;
};


class CRemoveAllPubs : public IRemovePubs
{
public:
    CRemoveAllPubs() : IRemovePubs("Delete All Publications") {}
protected:
    virtual bool x_ApplyToPubdesc(objects::CPubdesc& pubdesc);
};


END_NCBI_SCOPE

#endif
    // _PUBDESC_EDITOR_H_
