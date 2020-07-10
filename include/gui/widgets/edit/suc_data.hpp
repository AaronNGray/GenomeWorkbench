#ifndef GUI_WIDGETS_EDIT___SUC_DATA__HPP
#define GUI_WIDGETS_EDIT___SUC_DATA__HPP

/*  $Id: suc_data.hpp 41159 2018-06-04 18:32:28Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <objtools/format/flat_file_config.hpp>

#include <unordered_map>
#include <wx/string.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CSUCLine : public CObject
{
public:
    CSUCLine (const string& line, const string& accession, const CObject* obj);
    ~CSUCLine () {};

    void Add(const string& accession, const CObject* obj);
    void Print() { cerr << m_Count << "\t" << m_Line << endl; };
    int  Compare(const string& line) { return NStr::Compare(m_Line, line); };
    size_t GetCount() const { return m_Count; }
    const string& GetLine() const { return m_Line; }

    typedef pair<string, CConstRef<CObject> > TRelatedObject;
    size_t GetRelatedObjectCount() const { return m_RelatedObjects.size(); };
    CConstRef<CObject> GetRelatedObject(size_t row) const { return m_RelatedObjects[row].second; };
    const string& GetRelatedObjectAccession(size_t row) const { return m_RelatedObjects[row].first; };
    const string& GetAccession() {return m_RelatedObjects.empty() ? kEmptyStr : m_RelatedObjects.front().first;}
protected:
    string m_Line;
    size_t m_Count;
    vector<TRelatedObject> m_RelatedObjects;
};


class NCBI_GUIWIDGETS_EDIT_EXPORT CSUCBlock : public CObject
{
public:
    CSUCBlock(objects::CFlatFileConfig::FGenbankBlocks which_block) : m_Block(which_block), m_pos(-1), m_Expanded(false) {}
    ~CSUCBlock () {}

    objects::CFlatFileConfig::FGenbankBlocks GetBlockType() const { return m_Block; };
    void AddBlock(const string& block_text, const string& accession, const CObject* obj);
    void AddLine(const string& line, const string& accession, const CObject* obj, bool can_split);
    size_t CountLines() const { return m_Lines.size(); }
    void Print();

    CConstRef<CSUCLine> GetLine(size_t pos) const;

    static const char* GetBlockLabel(objects::CFlatFileConfig::FGenbankBlocks block_type);
    void SetExpanded(bool val) { m_Expanded = val; }
    bool GetExpanded() const { return m_Expanded; }
    void GetText(wxString& text) const;

    const map<CTempString, CRef<CSUCLine> >& GetLines(void) {return m_Lines;}
    bool HasSecondLevel(void);
    const map <string, map<CTempString, pair<bool, CRef<CSUCLine> > > >& GetSecondLevel(void) {return m_SecondLevel;}
    size_t CountChar(const string &str, const char c);
protected:
    objects::CFlatFileConfig::FGenbankBlocks m_Block;

    map<CTempString, CRef<CSUCLine> > m_Lines;
    map <string, map<CTempString, pair<bool, CRef<CSUCLine> > > > m_SecondLevel;
    mutable map<CTempString, CRef<CSUCLine> >::const_iterator m_pos_it;
    mutable int m_pos;
    bool m_Expanded;
    string m_label;
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CSUCResults : public CObject
{
public:
    CSUCResults ();
    ~CSUCResults () {} ;

    void AddText(string & block_text, objects::CFlatFileConfig::FGenbankBlocks which_block, const string& accession, CConstRef<CObject> obj);
    size_t CountLines() const;
    CConstRef<CSUCLine> GetLine(size_t pos) const;
    objects::CFlatFileConfig::FGenbankBlocks GetBlockType(size_t pos) const;
    void ExpandBlock(bool val, objects::CFlatFileConfig::FGenbankBlocks which_block);
    void ExpandAll(bool val);
    bool GetExpanded(objects::CFlatFileConfig::FGenbankBlocks which_block) const;
    void GetText(wxString& text);

    const unordered_map<unsigned int, CRef< CSUCBlock > >& GetBlocks(void) {return m_Blocks;}
    const vector<objects::CFlatFileConfig::FGenbankBlocks>& GetOrder(void) {return m_order;}
protected:
    unordered_map<unsigned int, CRef< CSUCBlock > > m_Blocks;
    vector<objects::CFlatFileConfig::FGenbankBlocks> m_order;

    bool x_GetBlockPos(size_t pos, objects::CFlatFileConfig::FGenbankBlocks& block_type, size_t& block_pos) const;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___SUC_DATA__HPP
