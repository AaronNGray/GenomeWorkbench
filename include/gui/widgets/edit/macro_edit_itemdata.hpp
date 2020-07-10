#ifndef GUI_WIDGETS_EDIT___MACRO_EDIT_ITEMDATA__HPP
#define GUI_WIDGETS_EDIT___MACRO_EDIT_ITEMDATA__HPP
/*  $Id: macro_edit_itemdata.hpp 41497 2018-08-08 15:31:57Z asztalos $
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


class CEditQualTreeItemData : public CMacroActionItemData
{
public:
    CEditQualTreeItemData(const string& description, EMacroFieldType type);
    ~CEditQualTreeItemData() {}
    
    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CEditProteinQualTreeItemData : public CEditQualTreeItemData
{
public:
    CEditProteinQualTreeItemData();
    ~CEditProteinQualTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CEditFeatQualTreeItemData : public CEditQualTreeItemData
{
public:
    CEditFeatQualTreeItemData();
    ~CEditFeatQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CEditRNAQualTreeItemData : public CEditQualTreeItemData
{
public:
    CEditRNAQualTreeItemData();
    ~CEditRNAQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CEditStrCommQualTreeItemData : public CEditQualTreeItemData
{
public:
    CEditStrCommQualTreeItemData();
    ~CEditStrCommQualTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CEditMolinfoFieldsTreeItemData : public CMacroActionItemData
{
public:
    CEditMolinfoFieldsTreeItemData();
    ~CEditMolinfoFieldsTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    void x_GetDescrChangeTo(const string& field, const CTempString& from, const CTempString& to, string& change_to) const;
    void x_GetDescrChangeFrom(const string& field, const CTempString& from, const CTempString& to, string& change_from) const;
    string x_GetFunctionFor(const string& field, const CTempString& from, const CTempString& to, TConstraints& constraints) const;
};


END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_EDIT_ITEMDATA__HPP
