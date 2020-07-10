#ifndef GUI_WIDGETS_EDIT___MACRO_PARSE_ITEMDATA__HPP
#define GUI_WIDGETS_EDIT___MACRO_PARSE_ITEMDATA__HPP
/*  $Id: macro_parse_itemdata.hpp 41837 2018-10-18 18:39:51Z asztalos $
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

class CParseQualTreeItemData : public CMacroActionItemData
{
public:
    CParseQualTreeItemData(const string& description, EMacroFieldType type);
    virtual ~CParseQualTreeItemData() {}
    
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual bool HasVariables() const { return true; }
    virtual string GetVariables();

protected:
    string x_GetParsedTextDescription() const;
    // pair<resolve_function, variable_name/path_to_asn_member>
    pair<string, string> x_GetResolveFuncForMultValQuals(const string& field, const string& rt_var, TConstraints& constraints, bool remove_constraint) const;
    string x_GetParsedTextFunction(const string& src_field, const string& parsed_text) const;
    bool x_ParseEntireText() const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);

    pair<string, bool> m_IncludeLeft;
    pair<string, bool> m_IncludeRight;
    pair<string, bool> m_CaseSensitive;
};


class CParseBsrcTreeItemData : public CParseQualTreeItemData
{
public:
    CParseBsrcTreeItemData();
    ~CParseBsrcTreeItemData() {}

    virtual string GetFunction(TConstraints& constraints) const;
    virtual const vector<string>& GetKeywords() const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CParseToBsrcTreeItemData : public CParseQualTreeItemData
{
public:
    CParseToBsrcTreeItemData();
    ~CParseToBsrcTreeItemData() {}

    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
    virtual const vector<string>& GetKeywords() const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CParseToCdsGeneProtTreeItemData : public CParseQualTreeItemData
{
public:
    CParseToCdsGeneProtTreeItemData();
    ~CParseToCdsGeneProtTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
    virtual const vector<string>& GetKeywords() const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CParseProteinQualTreeItemData : public CParseQualTreeItemData
{
public:
    CParseProteinQualTreeItemData();
    ~CParseProteinQualTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CParseRNAQualTreeItemData : public CParseQualTreeItemData
{
public:
    CParseRNAQualTreeItemData();
    ~CParseRNAQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CParseFeatQualTreeItemData : public CParseQualTreeItemData
{
public:
    CParseFeatQualTreeItemData();
    ~CParseFeatQualTreeItemData() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CParseCdsGeneProtTreeItemdata : public CParseQualTreeItemData
{
public:
    CParseCdsGeneProtTreeItemdata();
    ~CParseCdsGeneProtTreeItemdata() {}

    virtual bool UpdateTarget();
    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual string GetVariables();
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

class CParseBsrcToStructCommTreeItemData : public CParseQualTreeItemData
{
public:
    CParseBsrcToStructCommTreeItemData();
    ~CParseBsrcToStructCommTreeItemData() {}

    virtual string GetMacroDescription() const;
    virtual string GetFunction(TConstraints& constraints) const;
    virtual const vector<string>& GetKeywords() const;
private:
    virtual void x_AddParamPanel(wxWindow* parent);
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_PARSE_ITEMDATA__HPP
