#ifndef GUI_WIDGETS_EDIT___MACRO_SWAP_ITEMDATA__HPP
#define GUI_WIDGETS_EDIT___MACRO_SWAP_ITEMDATA__HPP
/*  $Id: macro_swap_itemdata.hpp 41636 2018-09-05 13:32:01Z asztalos $
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

class CSwapQualTreeItemData : public CMacroActionItemData
{
public:
    CSwapQualTreeItemData(const string& description, EMacroFieldType type);
    virtual ~CSwapQualTreeItemData() {}
    
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CSwapBsrcTreeItemData : public CSwapQualTreeItemData
{
public:
    CSwapBsrcTreeItemData();
    ~CSwapBsrcTreeItemData() {}

    virtual string GetFunction(TConstraints& constraints) const;
    virtual const vector<string>& GetKeywords() const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CSwapProteinQualTreeItemData : public CSwapQualTreeItemData
{
public:
    CSwapProteinQualTreeItemData();
    ~CSwapProteinQualTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;

private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CSwapRNAQualTreeItemData : public CSwapQualTreeItemData
{
public:
    CSwapRNAQualTreeItemData();
    ~CSwapRNAQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CSwapCdsGeneProtTreeItemData : public CSwapQualTreeItemData
{
public:
    CSwapCdsGeneProtTreeItemData();
    ~CSwapCdsGeneProtTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const;
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    string m_Srcfeat, m_Srcfield;
    string m_Destfeat, m_Destfield;
};

class CSwapFeatQualTreeItemData : public CSwapQualTreeItemData
{
public:
    CSwapFeatQualTreeItemData();
    ~CSwapFeatQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const;
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
    string m_Srcfeat, m_Srcfield;
    string m_Destfeat, m_Destfield;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_SWAP_ITEMDATA__HPP
