#ifndef GUI_WIDGETS_EDIT___MACRO_FEATLOC_ITEMDATA__HPP
#define GUI_WIDGETS_EDIT___MACRO_FEATLOC_ITEMDATA__HPP
/*  $Id: macro_featloc_itemdata.hpp 40991 2018-05-09 13:17:44Z asztalos $
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

class CEditFeatLocTreeItemData : public CMacroActionItemData
{
public:
    virtual ~CEditFeatLocTreeItemData() {}
    virtual bool UpdateTarget();
    virtual bool HasVariables() const { return true; }
protected:
    CEditFeatLocTreeItemData(const string& description, const string &panel_name, const TArgumentsVector &arguments);
    string x_CommonEditFeatLocDescr(size_t index) const;
    string x_CommonEditFeatLocVars(size_t index) const;
    string x_CommonEditFeatFunc(size_t index) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    const string m_PanelName;
    const TArgumentsVector* m_Arguments;
};

class CSet5PartialTreeItemData : public CEditFeatLocTreeItemData
{
public:
    CSet5PartialTreeItemData();
    ~CSet5PartialTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    map<string, pair<string, string>> m_5PartialMap;
};

class CSet3PartialTreeItemData : public CEditFeatLocTreeItemData
{
public:
    CSet3PartialTreeItemData();
    ~CSet3PartialTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    map<string, pair<string, string>> m_3PartialMap;
};

class CSetBothPartialTreeItemData : public CEditFeatLocTreeItemData
{
public:
    CSetBothPartialTreeItemData();
    ~CSetBothPartialTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    map<string, pair<string, string>> m_BothPartialMap;
};

class CClear5PartialTreeItemData : public CEditFeatLocTreeItemData
{
public:
    CClear5PartialTreeItemData();
    ~CClear5PartialTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    map<string, pair<string, string>> m_5PartialMap;
};

class CClear3PartialTreeItemData : public CEditFeatLocTreeItemData
{
public:
    CClear3PartialTreeItemData();
    ~CClear3PartialTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    map<string, pair<string, string>> m_3PartialMap;
};

class CClearBothPartialTreeItemData : public CEditFeatLocTreeItemData
{
public:
    CClearBothPartialTreeItemData();
    ~CClearBothPartialTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    map<string, pair<string, string>> m_BothPartialMap;
};

class CConvertStrandTreeItemData : public CEditFeatLocTreeItemData
{
public:
    CConvertStrandTreeItemData();
    ~CConvertStrandTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
};

class CConvertLocTypeTreeItemData : public CEditFeatLocTreeItemData
{
public:
    CConvertLocTypeTreeItemData();
    ~CConvertLocTypeTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
};

class CExtendToSeqEndTreeItemData : public CEditFeatLocTreeItemData
{
public:
    CExtendToSeqEndTreeItemData(EMActionType action_type);
    ~CExtendToSeqEndTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    EMActionType m_ActionType;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_FEATLOC_ITEMDATA__HPP
