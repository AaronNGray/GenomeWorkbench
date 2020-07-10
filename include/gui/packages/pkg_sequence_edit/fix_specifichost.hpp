#ifndef GUI_PACKAGES__FIX_SPECIFICHOST__HPP
#define GUI_PACKAGES__FIX_SPECIFICHOST__HPP
/*  $Id: fix_specifichost.hpp 30247 2014-04-17 21:42:23Z asztalos $
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
 * Authors: Andrea Asztalos
 *
 * File Description:
 *
 */

#include <objtools/validator/validatorp.hpp>
#include <wx/dataview.h>

BEGIN_NCBI_SCOPE

class CSpecificHostNode;
WX_DEFINE_ARRAY_PTR( CSpecificHostNode*, CSpecificHostNodePtrArray);

class CSpecificHostNode
{
public:
    CSpecificHostNode( CSpecificHostNode* parent, const wxString& current_spechost, 
        const wxString& correction, const wxString& category, const wxString& str_seqid);

    CSpecificHostNode( CSpecificHostNode* parent, const wxString& branch);

    ~CSpecificHostNode();
    // check if the node contains other nodes
    bool IsContainer(void) const { return m_Container; }
    // return the parent of this node
    CSpecificHostNode* GetParent() { return m_Parent; }
    // return the children of this node
    CSpecificHostNodePtrArray& GetChildren() { return m_Children; }
    // return the Nth child
    CSpecificHostNode* GetNthChild (unsigned int n ){ return m_Children.Item(n); }
    
    void Insert(CSpecificHostNode* child, unsigned int n) {
        m_Children.Insert( child, n );
    }
    void Append(CSpecificHostNode* child) { m_Children.Add(child); }
    
    unsigned int GetChildCount() const { return m_Children.GetCount(); }

    wxString m_CurSpechost;
    wxString m_Corrected;
    wxString m_Category;
    wxString m_Seqid;
    bool m_Container;
private:
    CSpecificHostNode* m_Parent;
    CSpecificHostNodePtrArray m_Children;
};


// ----- CSpecificHostModel
/*
    Current Specific-host    Suggested Correction    Category    SeqId
------------------------------------------------------------------------
1: Eschericia coli (2)
   2: Eschericia coli        Escherichia coli        Spelling    lcl|seq1
   3: Eschericia coli        Escherichia coli        Spelling    lcl|Seq4
4: Homo sapins                Homo                    Truncation    lcl|seq7
5: Homo supiens                Homo                    Truncation  lcl|seq9
6: giraffa giraffa(2)        All present, mixed        All present,mixed
   7: giraffa giraffa        Giraffa Giraffa            Capitalization lcl|seq6
   8: giraffa giraffa        Giraffa                    Truncation    lcl|seq2
9: vertebtrate                Unable to suggest        Unrecognized
                            correction for            
*/

// text should be wrapped in each cell
// the container row should have the rows filled in


class CSpecificHostModel : public wxDataViewModel
{
public:
    CSpecificHostModel();
    ~CSpecificHostModel() { delete m_Root; m_Root = NULL; }

    wxString GetCurSpechost( const wxDataViewItem& item) const;
    wxString GetCorrection( const wxDataViewItem& item) const;
    wxString GetCategory( const wxDataViewItem& item) const;
    wxString GetSeqId (const wxDataViewItem& item) const;

    // methods to change the model
    void AddSpecHostNode (const wxString& current_spechost, const wxString& correction, 
        const wxString& category, const wxString& str_seqid);
    // deleting one - once that host has been fixed
    void Delete ( const wxDataViewItem& item) ;
    int Compare ( const wxDataViewItem& item1, const wxDataViewItem& item2,
        unsigned int column, bool ascending) const;
    
    // implementing base class virtuals:
    virtual unsigned int GetColumnCount() const { return 4; } 
    // the number of columns is set to four
    virtual wxString GetColumnType( unsigned int col ) const { return wxT("string"); }
    virtual void GetValue( wxVariant &variant,
                           const wxDataViewItem &item, unsigned int col ) const;
    virtual bool SetValue( const wxVariant &variant,
                           const wxDataViewItem &item, unsigned int col );

    virtual bool IsEnabled( const wxDataViewItem &item,
                            unsigned int col ) const;

    virtual wxDataViewItem GetParent( const wxDataViewItem &item ) const;
    virtual bool IsContainer( const wxDataViewItem &item ) const;
    virtual unsigned int GetChildren( const wxDataViewItem &parent,
                                      wxDataViewItemArray &array ) const;

private:
    CSpecificHostNode* m_Root;
    // pointers to some of the special nodes in the tree - maybe it's not needed now
    CSpecificHostNode* m_esc; 
    CSpecificHostNode* m_hsap;
    CSpecificHostNode* m_hsup;
    CSpecificHostNode* m_giraf;
    CSpecificHostNode* m_ver;
};

END_NCBI_SCOPE

#endif
    // GUI_PACKAGES__FIX_SPECIFICHOST__HPP
