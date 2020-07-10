#ifndef GUI_WIDGETS_EDIT___MACRO_APPLYTBL_ITEMDATA__HPP
#define GUI_WIDGETS_EDIT___MACRO_APPLYTBL_ITEMDATA__HPP
/*  $Id: macro_applytbl_itemdata.hpp 44731 2020-03-03 15:53:48Z asztalos $
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
 *  Classes to be used in the macro editor for applying qualifier tables
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/macro_treeitemdata.hpp>

BEGIN_NCBI_SCOPE

class CMacroApplySrcTablePanel;

class CApplyTableItemData : public CMacroActionItemData
{
public:
    virtual string GetMacroDescription() const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
protected:
    CApplyTableItemData(const string& description, EMacroFieldType type = EMacroFieldType::eNotSet)
        : CMacroActionItemData(description, type) {}
    
    void x_LoadParamPanel(wxWindow* parent, ECustomPanelType type);
    CMacroApplySrcTablePanel* m_TablePanel{ nullptr };
};

class CApplySrcTableTreeItemData : public CApplyTableItemData
{
public:
    CApplySrcTableTreeItemData();
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyStrCommTableTreeItemData : public CApplyTableItemData
{
public:
    CApplyStrCommTableTreeItemData();
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyPubTableTreeItemData : public CApplyTableItemData
{
public:
    CApplyPubTableTreeItemData();
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyDBlinkTableTreeItemData : public CApplyTableItemData
{
public:
    CApplyDBlinkTableTreeItemData();
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CApplyMolinfoTableTreeItemData : public CApplyTableItemData
{
public:
    CApplyMolinfoTableTreeItemData();
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    string m_TargetFeature;
};

class CApplyMiscTableTreeItemData : public CApplyTableItemData
{
public:
    CApplyMiscTableTreeItemData();
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    string m_TargetFeature;
};

class CApplyFeatTableTreeItemData : public CApplyTableItemData
{
public:
    CApplyFeatTableTreeItemData();
    virtual void UpdateTarget(const TConstraints& constraints);
    virtual string GetVariables();
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    string m_TargetFeature;
};

class CApplyCDSGeneProtTableTreeItemData : public CApplyTableItemData
{
public:
    CApplyCDSGeneProtTableTreeItemData();
    virtual void UpdateTarget(const TConstraints& constraints);
    virtual string GetVariables();
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    string m_TargetFeature;
};

class CApplyRNATableTreeItemData : public CApplyTableItemData
{
public:
    CApplyRNATableTreeItemData();
    virtual void UpdateTarget(const TConstraints& constraints);
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    string m_TargetRNAType;
    string m_TargetncRNAclass;
};


END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_APPLYTBL_ITEMDATA__HPP

