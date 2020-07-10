#ifndef GUI_WIDGETS_EDIT___MACRO_GENERAL_ITEMDATA__HPP
#define GUI_WIDGETS_EDIT___MACRO_GENERAL_ITEMDATA__HPP
/*  $Id: macro_general_itemdata.hpp 44572 2020-01-23 20:12:06Z asztalos $
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

class CRawSeqToDeltaByNPanel;

class CFixPrimerTreeItemData : public CMacroActionSimpleItemData
{
public:
    CFixPrimerTreeItemData(const string& description, bool fixI);
    ~CFixPrimerTreeItemData() {}

    virtual string GetFunction(TConstraints& constraints) const;
private:
    CTempString x_GetFncName() const;
    bool m_FixI{ false };
};

class CFixSubsrcFormatTreeItemData : public CMacroActionItemData
{
public:
    CFixSubsrcFormatTreeItemData(const string& description);
    ~CFixSubsrcFormatTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CFixSrcQualsTreeItemData : public CMacroActionItemData
{
public:
    CFixSrcQualsTreeItemData(const string& description);
    ~CFixSrcQualsTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual const vector<string>& GetKeywords() const { return m_SrcKeywords; }
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    vector<string> m_SrcKeywords{ "country", "isolation-source", "host" };
};

class CFixMouseStrainTreeItemData : public CMacroActionSimpleItemData
{
public:
    CFixMouseStrainTreeItemData(const string& description);
    ~CFixMouseStrainTreeItemData() {}

    virtual string GetFunction(TConstraints& constraints) const;
};


class CFixPubCapsTreeItemData : public CMacroActionItemData
{
public:
    CFixPubCapsTreeItemData(const string& description, EMacroFieldType type);
    ~CFixPubCapsTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CFixCapsLastNameTreeItemData : public CMacroActionSimpleItemData
{
public:
    CFixCapsLastNameTreeItemData(const string& description);
    virtual string GetFunction(TConstraints& constraints) const;
};

class CRetranslateCDSTreeItemData : public CMacroActionItemData
{
public:
    CRetranslateCDSTreeItemData(const string& description);
    ~CRetranslateCDSTreeItemData() {}

    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CUpdateReplacedECTreeItemData : public CMacroActionItemData
{
public:
    CUpdateReplacedECTreeItemData(const string& description);
    ~CUpdateReplacedECTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CAddGeneXrefTreeItemData : public CMacroActionItemData
{
public:
    CAddGeneXrefTreeItemData(const string& description);
    ~CAddGeneXrefTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CRemoveGeneXrefTreeItemData : public CMacroActionItemData
{
public:
    CRemoveGeneXrefTreeItemData(const string& description);
    ~CRemoveGeneXrefTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    string x_TranslateType(const string& dlg_type) const;
};

class CConvertClassTreeItemData : public CMacroActionItemData
{
public:
    CConvertClassTreeItemData();
    ~CConvertClassTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CDiscrepancyAutofixTreeItemData : public CMacroActionItemData
{
public:
    CDiscrepancyAutofixTreeItemData(const string& description);
    ~CDiscrepancyAutofixTreeItemData() {}
    
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    vector<string> m_TestVars;
};

class CAutodefTreeItemData : public CMacroActionItemData
{
public:
    CAutodefTreeItemData();
    ~CAutodefTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CAddAssemblyGapsNTreeItemData : public CMacroActionItemData
{
public:
    CAddAssemblyGapsNTreeItemData();
    ~CAddAssemblyGapsNTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    CRawSeqToDeltaByNPanel* m_RawSeqToDeltaPanel{ nullptr };
    bool m_IsAssemblyGap{ false };
    const vector<string> m_Vars{ "min_unknown", "max_unknown", "min_known", "max_known",
            "adjust_cds", "keep_gap_length", "gap_type", "linkage", "linkage_evidence" };
};

class CSetExceptionsTreeItemData : public CMacroActionItemData
{
public:
    CSetExceptionsTreeItemData();
    ~CSetExceptionsTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CConvertGapsTreeItemData : public CMacroActionItemData
{
public:
    CConvertGapsTreeItemData();

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_GENERAL_ITEMDATA__HPP
