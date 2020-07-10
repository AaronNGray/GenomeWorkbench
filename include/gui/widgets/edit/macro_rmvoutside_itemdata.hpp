#ifndef GUI_WIDGETS_EDIT___MACRO_RMVOUTSIDE_ITEMDATA__HPP
#define GUI_WIDGETS_EDIT___MACRO_RMVOUTSIDE_ITEMDATA__HPP
/*  $Id: macro_rmvoutside_itemdata.hpp 41508 2018-08-09 19:54:22Z asztalos $
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

class CRmvOutsideQualTreeItemData : public CMacroActionItemData
{
public:
    CRmvOutsideQualTreeItemData(const string& description, EMacroFieldType type);
    virtual ~CRmvOutsideQualTreeItemData() {}
    
    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();

protected:
    string x_GetMarkedTextFunction() const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRmvOutsideBsrcTreeItemData : public CRmvOutsideQualTreeItemData
{
public:
    CRmvOutsideBsrcTreeItemData();
    ~CRmvOutsideBsrcTreeItemData() {}

    virtual string GetFunction(TConstraints& constraints) const;
    virtual const vector<string>& GetKeywords() const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRmvOutsideStrCommQualTreeItemData : public CRmvOutsideQualTreeItemData
{
public:
    CRmvOutsideStrCommQualTreeItemData();
    ~CRmvOutsideStrCommQualTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRmvOutsideRNAQualTreeItemData : public CRmvOutsideQualTreeItemData
{
public:
    CRmvOutsideRNAQualTreeItemData();
    ~CRmvOutsideRNAQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};


class CRmvOutsideProteinQualTreeItemData : public CRmvOutsideQualTreeItemData
{
public:
    CRmvOutsideProteinQualTreeItemData();
    ~CRmvOutsideProteinQualTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRmvOutsideFeatQualTreeItemData : public CRmvOutsideQualTreeItemData
{
public:
    CRmvOutsideFeatQualTreeItemData();
    ~CRmvOutsideFeatQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_RMVOUTSIDE_ITEMDATA__HPP
