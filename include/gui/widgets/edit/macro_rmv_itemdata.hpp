#ifndef GUI_WIDGETS_EDIT___MACRO_RMV_ITEMDATA__HPP
#define GUI_WIDGETS_EDIT___MACRO_RMV_ITEMDATA__HPP
/*  $Id: macro_rmv_itemdata.hpp 44820 2020-03-23 14:59:16Z asztalos $
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

class CRmvQualTreeItemData : public CMacroActionItemData
{
public:
    CRmvQualTreeItemData(const string& description, EMacroFieldType type);
    ~CRmvQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRmvBsrcQualTreeItemData : public CMacroActionItemData
{
public:
    CRmvBsrcQualTreeItemData();
    ~CRmvBsrcQualTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual const vector<string>& GetKeywords() const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRmvStrCommQualTreeItemData : public CMacroActionItemData
{
public:
    CRmvStrCommQualTreeItemData();
    ~CRmvStrCommQualTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRmvMiscQualTreeItemData : public CRmvQualTreeItemData
{
public:
    CRmvMiscQualTreeItemData();
    ~CRmvMiscQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetFunction(TConstraints& constraints) const;
};

class CRmvProteinQualTreeItemData : public CRmvQualTreeItemData
{
public:
    CRmvProteinQualTreeItemData();
    ~CRmvProteinQualTreeItemData() {}
  
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRmvFeatQualTreeItemData : public CRmvQualTreeItemData
{
public:
    CRmvFeatQualTreeItemData();
    ~CRmvFeatQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRmvRNAQualTreeItemData : public CRmvQualTreeItemData
{
public:
    CRmvRNAQualTreeItemData();
    ~CRmvRNAQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};


class CRemoveDescrTreeItemData : public CMacroActionItemData
{
public:
    CRemoveDescrTreeItemData();
    ~CRemoveDescrTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRemoveFeatureTreeItemData : public CMacroActionItemData
{
public:
    CRemoveFeatureTreeItemData();
    ~CRemoveFeatureTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const;
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRemoveDuplFeatsTreeItemData : public CMacroActionItemData
{
public:
    CRemoveDuplFeatsTreeItemData();
    ~CRemoveDuplFeatsTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRemoveDbXrefTreeItemData : public CMacroActionItemData
{
public:
    CRemoveDbXrefTreeItemData();
    ~CRemoveDbXrefTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRmvOrgNameTreeItemData : public CMacroActionItemData
{
public:
    CRmvOrgNameTreeItemData();
    ~CRmvOrgNameTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_RMV_ITEMDATA__HPP

