/*  $Id: fix_specifichost.cpp 30247 2014-04-17 21:42:23Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/packages/pkg_sequence_edit/fix_specifichost.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CSpecificHostNode::CSpecificHostNode( CSpecificHostNode* parent, const wxString& current_spechost, 
        const wxString& correction, const wxString& category, const wxString& str_seqid)
        : m_CurSpechost(current_spechost), m_Corrected(correction), 
        m_Category(category), m_Seqid(str_seqid), m_Container(false)
{
}

CSpecificHostNode::CSpecificHostNode( CSpecificHostNode* parent, const wxString& branch)
    : m_CurSpechost(branch), m_Corrected(wxEmptyString), m_Category(wxEmptyString), 
    m_Seqid(wxEmptyString), m_Container(true), m_Parent(parent)
{
}

CSpecificHostNode::~CSpecificHostNode()
{
    // free all the children nodes
    size_t count = m_Children.GetCount();  // can be replaced by a member function
    for (size_t n = 0; n < count; ++n) {
        CSpecificHostNode* child = m_Children[n];
        delete child;
        child = NULL;
    }
}

 /// Implementing CSpecificHostModel
 CSpecificHostModel::CSpecificHostModel()
 {
     // fill up the model with actual data from the seq_entry
     m_Root = new CSpecificHostNode( NULL, "Specific Hosts to be Corrected" );
     // This is not the root itself!
     // It is the sole child of the actual, invisible root. 
     // special case: when the tool is applied and the model might be empty
     m_esc = new CSpecificHostNode( m_Root, "Eschericia coli (2)");
     m_esc->Append(new CSpecificHostNode(m_esc, "Eschericia coli", "Escherichia coli", "Spelling", "lcl|seq1"));
     m_esc->Append(new CSpecificHostNode(m_esc, "Eschericia coli", "Escherichia coli", "Spelling", "lcl|seq3"));
     m_Root->Append(m_esc);
     
     m_hsap = new CSpecificHostNode(NULL, "Homo sapins", "Homo", "Truncation", "lcl|seq5");
     m_Root->Append(m_hsap);

     m_hsup = new CSpecificHostNode(NULL, "Homo supiens", "Homo", "Truncation", "lcl|seq2");
     m_Root->Append(m_hsup);

     m_giraf = new CSpecificHostNode(m_Root, "giraffa giraffa (2)");
     m_giraf->Append(new CSpecificHostNode(m_giraf, "giraffa giraffa", "Giraffa Giraffa", "Capitalization", "lcl|seq4"));
     m_giraf->Append(new CSpecificHostNode(m_giraf, "giraffa giraffa", "Giraffa", "Truncation", "lcl|seq9"));
     m_Root->Append(m_giraf);

     m_ver = new CSpecificHostNode(m_Root, "vertebrate", "Unable to suggest correction for", "Unrecognized", "lcl|seq11");
     m_Root->Append(m_ver);  

 }

wxString CSpecificHostModel::GetCurSpechost( const wxDataViewItem& item) const
{
    CSpecificHostNode *node = (CSpecificHostNode*) item.GetID();
    if (!node)      
        return wxEmptyString;

    return node->m_CurSpechost;
}

wxString CSpecificHostModel::GetCorrection( const wxDataViewItem& item) const
{
    CSpecificHostNode *node = (CSpecificHostNode*) item.GetID();
    if (!node)
        return wxEmptyString;

    return node->m_Corrected;
}
wxString CSpecificHostModel::GetCategory( const wxDataViewItem& item) const
{
    CSpecificHostNode *node = (CSpecificHostNode*) item.GetID();
    if (!node)
        return wxEmptyString;

    return node->m_Category;
}
wxString CSpecificHostModel::GetSeqId (const wxDataViewItem& item) const
{
    CSpecificHostNode *node = (CSpecificHostNode*) item.GetID();
    if (!node)      // happens if item.IsOk()==false
        return wxEmptyString;

    return node->m_Seqid;
}

void CSpecificHostModel::AddSpecHostNode (const wxString& current_spechost, const wxString& correction, 
        const wxString& category, const wxString& str_seqid)
{
    _ASSERT(m_Root);
    CSpecificHostNode* child_node = new CSpecificHostNode(m_Root, current_spechost, correction, category, str_seqid);
    m_Root->Append(child_node);

    if (true)
    {
        // notify control
        wxDataViewItem child( (void*) child_node );
        wxDataViewItem parent( (void*) m_Root );
        ItemAdded( parent, child );
    }
}

void CSpecificHostModel::Delete ( const wxDataViewItem& item)
{
    CSpecificHostNode *node = (CSpecificHostNode*) item.GetID();
    if (!node)      // happens if item.IsOk()==false
        return;

    wxDataViewItem parent( node->GetParent() );
    if (!parent.IsOk())
    {
        //wxASSERT(node == m_Root);
        //wxLogError( "Cannot remove the root item!" );
        return;
    }

    if (node == m_esc)
        m_esc = NULL;
    else if (node == m_hsap)
        m_hsap = NULL;
    else if (node == m_hsup)
        m_hsup = NULL;
    else if (node == m_ver)
        m_ver = NULL;
    else if (node == m_giraf)
        m_giraf = NULL;

    node->GetParent()->GetChildren().Remove( node );
    delete node;
    node = NULL;  

    // notify control
    ItemDeleted( parent, item );
}

int CSpecificHostModel::Compare ( const wxDataViewItem& item1, const wxDataViewItem& item2,
        unsigned int column, bool ascending) const
{
    _ASSERT(item1.IsOk() && item2.IsOk());

    if (IsContainer(item1) && IsContainer(item2))
    {
        wxVariant value1, value2;
        GetValue( value1, item1, 0 );
        GetValue( value2, item2, 0 );

        wxString str1 = value1.GetString();
        wxString str2 = value2.GetString();
        int res = str1.Cmp( str2 );
        if (res) return res;

        // items must be different
        wxUIntPtr litem1 = (wxUIntPtr) item1.GetID();
        wxUIntPtr litem2 = (wxUIntPtr) item2.GetID();

        return litem1-litem2;
    }

    return wxDataViewModel::Compare( item1, item2, column, ascending );
}

void CSpecificHostModel::GetValue( wxVariant &variant,
                           const wxDataViewItem &item, unsigned int col ) const
{
    _ASSERT(item.IsOk());

    CSpecificHostNode *node = (CSpecificHostNode*) item.GetID();
    switch (col)
    {
    case 0:
        variant = node->m_CurSpechost;
        break;
    case 1:
        variant = node->m_Corrected;
        break;
    case 2:
        variant = node->m_Category;
        break;
    case 3:
        variant = node->m_Seqid;
        break;
    default:
        //wxLogError( "CSpecificHostModel::GetValue: wrong column %d", col );
        break;
    }
}

bool CSpecificHostModel::SetValue( const wxVariant &variant,
                           const wxDataViewItem &item, unsigned int col )
{    _ASSERT(item.IsOk());

    CSpecificHostNode *node = (CSpecificHostNode*) item.GetID();
    switch (col)
    {
        case 0:
            node->m_CurSpechost = variant.GetString();
            return true;
        case 1:
            node->m_Corrected = variant.GetString();
            return true;
        case 2:
            node->m_Category = variant.GetString();
            return true;
        case 3:
            node->m_Seqid = variant.GetString();
            return true;

        default:
            //wxLogError( "CSpecificHostModel::SetValue: wrong column" );
            break;
    }
    return false;
}

bool CSpecificHostModel::IsEnabled( const wxDataViewItem &item,
                            unsigned int col ) const
{
    _ASSERT(item.IsOk());

    //CSpecificHostNode *node = (CSpecificHostNode*) item.GetID();
    
    return true;
}

wxDataViewItem CSpecificHostModel::GetParent( const wxDataViewItem &item ) const
{
    if (!item.IsOk())
        return wxDataViewItem(0);

    CSpecificHostNode *node = (CSpecificHostNode*) item.GetID();

    if (node == m_Root)
        return wxDataViewItem(0);

    return wxDataViewItem( (void*) node->GetParent() );
}

bool CSpecificHostModel::IsContainer( const wxDataViewItem &item ) const
{
    if (!item.IsOk())
        return true;

    CSpecificHostNode *node = (CSpecificHostNode*) item.GetID();
    return node->IsContainer(); 
}

unsigned int CSpecificHostModel::GetChildren( const wxDataViewItem &parent, wxDataViewItemArray &array ) const
{
    CSpecificHostNode *node = (CSpecificHostNode*) parent.GetID();
    if (!node)
    {
        array.Add( wxDataViewItem( (void*) m_Root ) );
        return 1;
    }

    if (node == m_esc)
    {
        //CSpecificHostModel *model = (CSpecificHostModel*)(const CSpecificHostModel*) this;
    }

    if (node->GetChildCount() == 0)
    {
        return 0;
    }

    unsigned int count = node->GetChildren().GetCount();
    for (unsigned int pos = 0; pos < count; pos++)
    {
        CSpecificHostNode *child = node->GetChildren().Item( pos );
        array.Add( wxDataViewItem( (void*) child ) );
    }

    return count;
}

END_NCBI_SCOPE

