#ifndef GUI_WIDGETS_EDIT___MACRO_APPLY_ITEMDATA__HPP
#define GUI_WIDGETS_EDIT___MACRO_APPLY_ITEMDATA__HPP
/*  $Id: macro_apply_itemdata.hpp 44990 2020-05-04 17:26:18Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 *  Classes to be used in the macro editor storing user specific data
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/macro_treeitemdata.hpp>

BEGIN_NCBI_SCOPE

class CMacroAuthorNamesPanel;
class CStructuredCommentPanel;

class CApplyQualTreeItemData : public CMacroActionItemData
{
public:
    CApplyQualTreeItemData(const string& description, EMacroFieldType type);
    virtual ~CApplyQualTreeItemData() {}
    
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyProteinQualTreeItemData : public CApplyQualTreeItemData
{
public:
    CApplyProteinQualTreeItemData();
    ~CApplyProteinQualTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;

private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyBsrcQualTreeItemData : public CApplyQualTreeItemData
{
public:
    CApplyBsrcQualTreeItemData();
    ~CApplyBsrcQualTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual const vector<string>& GetKeywords() const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyMolinfoQualTreeItemData : public CApplyQualTreeItemData
{
public:
    CApplyMolinfoQualTreeItemData();
    ~CApplyMolinfoQualTreeItemData() {}

    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyPubQualTreeItemData : public CApplyQualTreeItemData
{
public:
    CApplyPubQualTreeItemData();
    ~CApplyPubQualTreeItemData() {}

    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyDBLinkQualTreeItemData : public CApplyQualTreeItemData
{
public:
    CApplyDBLinkQualTreeItemData();
    ~CApplyDBLinkQualTreeItemData() {}

    virtual void UpdateTarget(const TConstraints& constraints);
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyAuthorsTreeItemData : public CMacroActionItemData
{
public:
    CApplyAuthorsTreeItemData();
    ~CApplyAuthorsTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    CMacroAuthorNamesPanel* m_AuthorsPanel{ nullptr };
    size_t m_NumAuthors{ 0 };  // number of authors that will be applied
};

class CApplyStrCommQualTreeItemData : public CApplyQualTreeItemData
{
public:
    CApplyStrCommQualTreeItemData();
    ~CApplyStrCommQualTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyStructCommentTreeItemData : public CMacroActionItemData
{
public:
    CApplyStructCommentTreeItemData();
    ~CApplyStructCommentTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    CStructuredCommentPanel* m_StructCommentPanel{ nullptr };
    CRef<objects::CSeqdesc> m_Seqdesc;
    unsigned int m_Index = 1;
};

class CApplyFeatQualTreeItemData : public CApplyQualTreeItemData
{
public:
    CApplyFeatQualTreeItemData();
    ~CApplyFeatQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyRNAQualTreeItemData : public CApplyQualTreeItemData
{
public:
    CApplyRNAQualTreeItemData();
    ~CApplyRNAQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CAddRNAFeatTreeItemData : public CMacroActionItemData
{
public:
    CAddRNAFeatTreeItemData();
    ~CAddRNAFeatTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CAddCDSFeatTreeItemData : public CMacroActionItemData
{
public:
    CAddCDSFeatTreeItemData();
    ~CAddCDSFeatTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CAddFeatureTreeItemData : public CMacroActionItemData
{
public:
    CAddFeatureTreeItemData();
    ~CAddFeatureTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyPmidToEntryTreeItemData : public CMacroActionItemData
{
public:
    CApplyPmidToEntryTreeItemData();
    ~CApplyPmidToEntryTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

// For replacing/applying new DOI number for a bioseq - used in RefSeq
class CApplyDOIToEntryTreeItemData : public CMacroActionItemData
{
public:
    CApplyDOIToEntryTreeItemData();
    ~CApplyDOIToEntryTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyDOILookupTreeItemData : public CMacroActionItemData
{
public:
    CApplyDOILookupTreeItemData();
    ~CApplyDOILookupTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_APPLY_ITEMDATA__HPP
