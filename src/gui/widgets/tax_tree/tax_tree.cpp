/*  $Id: tax_tree.cpp 36929 2016-11-15 16:25:46Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/tax_tree/tax_tree.hpp>
#include <gui/widgets/tax_tree/tax_tree_ds.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/utils/reporter.hpp>
#include <gui/objutils/utils.hpp>
#include <objects/taxon1/taxon1.hpp>
#include <algorithm>
#include <stack>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);



//
// internal class to fill our tree structure
//

class CTreeFiller : public ITreeIterator::I4Each
{
public:

    virtual ~CTreeFiller() { }
    CTreeFiller(CTaxTreeDataSource& ds,
                const CTaxTreeBrowser::TTaxMap& tax_map)
        : m_TaxMap(tax_map),
          m_Curr(NULL),
          m_DS(&ds)
    {
    }

    ITreeIterator::EAction LevelBegin(const ITaxon1Node* tax_node)
    {
        if (m_Curr) {
            m_Nodes.push(m_Curr);
        }

        m_Curr = NULL;
        return ITreeIterator::eOk;
    }

    ITreeIterator::EAction Execute(const ITaxon1Node* tax_node)
    {
        CTaxTreeBrowser::STaxInfo info;
        info.data_source = m_DS;
        info.tax_node = tax_node;

        int tax_id = tax_node->GetTaxId();
        CTaxTreeBrowser::TTaxMap::const_iterator iter = m_TaxMap.find(tax_id);
        if (iter != m_TaxMap.end()) {
            info.uids = iter->second;
        } else {
            LOG_POST(Warning << "failed to find tax-id " << tax_id
                << " in list of sequences");
        }

        CTaxTreeBrowser::TTaxTree* node = NULL;
        if ( !m_Tree.get() ) {
            m_Tree.reset(new CTaxTreeBrowser::TTaxTree(info));
            node = m_Tree.get();
        } else if (m_Nodes.size()) {
            CTaxTreeBrowser::TTaxTree* parent = m_Nodes.top();
            node = parent->AddNode(info);
        } else {
            LOG_POST(Warning << "failed to find current node");
        }

        m_Curr = node;
        return ITreeIterator::eOk;
    }

    ITreeIterator::EAction LevelEnd(const ITaxon1Node* tax_node)
    {
        m_Curr = m_Nodes.top();
        m_Nodes.pop();
        return ITreeIterator::eOk;
    }

    const CTaxTreeBrowser::TTaxMap&     m_TaxMap;
    auto_ptr<CTaxTreeBrowser::TTaxTree> m_Tree;
    CTaxTreeBrowser::TTaxTree*          m_Curr;
    CRef<CTaxTreeDataSource>            m_DS;
    stack<CTaxTreeBrowser::TTaxTree*>   m_Nodes;
};


// callback - not a good way to go, but keeps FluTree unchanged
// in case it will be updated externally
//static void s_OnTaxTreeNotify_cb(Fl_Widget * widget, void * data)
//{
//    CTaxTreeBrowser* pTaxTree = dynamic_cast<CTaxTreeBrowser*>   (widget);
//
//    if (pTaxTree) {
//        CTaxTreeDataSource::TUidVec uids;
//        uids.clear();
//
//        pTaxTree->GetSelectedUids(uids);
//
//        switch (pTaxTree->callback_reason()) {
//        case FLU_SELECTED:
//        case FLU_UNSELECTED: {
//            CEventAttachmentFor< vector<CConstRef<objects::CSeq_id> > > csid;
//            csid = uids;
//            pTaxTree->Send(new CEvent(CEvent::eEvent_Message, CViewEvent::eSelectionChanged, &csid, pTaxTree));
//        }
//        }
//    }
//}

CTaxTreeBrowser::CTaxTreeBrowser(wxWindow *parent, wxWindowID id,
            const wxPoint& pos, const wxSize& size,
            long style, const wxValidator& validator,
            const wxString& name)
{
    m_Reporter = NULL;
    style = style & ~wxTR_MULTIPLE;
    wxTreeCtrl::Create(parent, id, pos, size, style, validator, name);      
}


CTaxTreeBrowser::CTaxTreeBrowser(wxWindow *parent, wxWindowID id,
            const wxPoint& pos, const wxSize& size,
            long style)
{
    m_Reporter = NULL;
    style = style & ~wxTR_MULTIPLE;
    wxTreeCtrl::Create(parent, id, pos, size, style);   
}



CTaxTreeBrowser::~CTaxTreeBrowser()
{
}


void CTaxTreeBrowser::SetReporter(IReporter* reporter)
{
    m_Reporter = reporter;
}


void CTaxTreeBrowser::SetDataSource(CTaxTreeDataSource& ds)
{
    m_DataSource.Reset(&ds);

    x_Refresh();
}


void CTaxTreeBrowser::x_Refresh()
{
    // clear the tree to start
    DeleteAllItems();
    if ( !m_DataSource ) {
        return;
    }

    if (m_Reporter) {
        m_Reporter->SetMessage("Retrieving taxonomic IDs...");
    }

    TTaxMap tax_map;
    m_DataSource->GetTaxMap(tax_map);

    ITreeIterator& iter = m_DataSource->GetIterator(m_DispMode);
    CTreeFiller filler(*m_DataSource, tax_map);
    iter.TraverseDownward(filler);
    m_TaxTree.reset(filler.m_Tree.release());

    // next, populate the browser
    if (m_TaxTree.get()) {
        x_PopulateTree(GetRootItem(), m_TaxTree.get());
    }

    if (m_Reporter) {
        m_Reporter->SetMessage("Done.");
        m_Reporter->SetPctCompleted(0);
    }
}


void CTaxTreeBrowser::SetDisplayMode(CTaxTreeDataSource::EDisplayMode mode)
{
    m_DispMode = mode;
    x_Refresh();
}


struct SUidCollector
{
    CTaxTreeDataSource::TUidVec uids;
    ETreeTraverseCode operator()(CTaxTreeBrowser::TTaxTree& node, int delta)
    {
        switch (delta) {
        case 0:
        case 1:
            uids.insert(uids.end(),
                        node.GetValue().uids.begin(),
                        node.GetValue().uids.end());
            break;

        case -1:
            break;
        }

        return eTreeTraverse;
    }
};


void CTaxTreeBrowser::GetSelectedUids( CTaxTreeDataSource::TUidVec& uids ) const
{
    SUidCollector       collector;
    wxArrayTreeItemIds  selections;
    
    if( GetSelections( selections ) ){
        for( size_t ix = 0; ix < selections.size(); ix++ ){
            wxTreeItemData* item_data = GetItemData( selections[ix] );
            TTaxTree* tree = dynamic_cast<wxTaxTreeData*>(item_data)->GetData();
            if( tree ){
                collector = TreeDepthFirstTraverse( *tree, collector );
            }
        }
    }

    std::sort( collector.uids.begin(), collector.uids.end() );
    collector.uids.erase( 
        std::unique( collector.uids.begin(), collector.uids.end() ), 
        collector.uids.end()
    );
    uids.swap( collector.uids );
}


void CTaxTreeBrowser::x_PopulateTree(wxTreeItemId node, TTaxTree* tree)
{
    // add a node for the current branch
    string name;
    x_GetName(*tree, name);

    node = AppendItem(node, ToWxString(name), -1, -1, new wxTaxTreeData(tree));

    // recurse our tree
    if ( !tree->IsLeaf()) {
        /// not a leaf node - just add the children recursively
        TTaxTree::TNodeList_CI begin = tree->SubNodeBegin();
        TTaxTree::TNodeList_CI begin2 = begin;
        TTaxTree::TNodeList_CI end = tree->SubNodeEnd();
        if (++begin2 == end) {
            // 1 child only
            Expand(node);
        }
        for ( ;  begin != end;  ++begin) {
            x_PopulateTree(node, *begin);
        }
    } else {
        // leaf node
        // we go ahead and transform the tree as well, making sure
        // that leaves represent sequences, not taxa
        string str;
        const STaxInfo& info = tree->GetValue();
        ITERATE (CTaxTreeDataSource::TUidVec, iter, tree->GetValue().uids) {
            /// add a leaf in the tax tree for the sequence as well
            STaxInfo new_info;
            new_info.data_source = info.data_source;
            new_info.uids.push_back(*iter);
            new_info.tax_node = info.tax_node;
            TTaxTree* new_tree = tree->AddNode(new_info);

            /// add a node for this sequence
            str.erase();
            m_DataSource->GetTitle(**iter, &str);

            AppendItem(node, ToWxString(str), -1, -1, new wxTaxTreeData(new_tree));         
        }
    }
}

void CTaxTreeBrowser::GetObjectSelection(TConstObjects& objs)
{
    // TO DO this is not a very effective solution, can be optimized
    CTaxTreeDataSource::TUidVec uids;
    GetSelectedUids(uids);

    ITERATE(CTaxTreeDataSource::TUidVec, it, uids)  {
        objs.push_back(CConstRef<CObject>(&**it));
    }
}


void CTaxTreeBrowser::SetObjectSelection(const CTaxTreeDataSource::TUidVec& uids)
{
    UnselectAll();
    SetSelectedUids(uids, GetRootItem());
}


// retrieve a name for a node
void CTaxTreeBrowser::x_GetName(TTaxTree& node, string& str) const
{
    size_t uids = x_CountNodes(node);
    m_DataSource->GetTitle(*node.GetValue().tax_node, &str);
    str += " - ";
    str += NStr::SizetToString(uids);
    str += " sequences";
}


struct SUidCounter
{
    SUidCounter()
        : uids(0) { }

    size_t uids;
    ETreeTraverseCode operator()(CTaxTreeBrowser::TTaxTree& node,
                                 int delta_level)
    {
        switch (delta_level) {
        case 0:
        case 1:
            uids += node.GetValue().uids.size();
            break;
        case -1:
            break;
        }
        return eTreeTraverse;
    }
};

// count the number of UIDs held underneath a given tree
size_t CTaxTreeBrowser::x_CountNodes(TTaxTree& node) const
{
    SUidCounter counter;
    counter = TreeDepthFirstTraverse(node, counter);
    return counter.uids;
}


void CTaxTreeBrowser::SetSelectedUids(const CTaxTreeDataSource::TUidVec& uids,
                                      wxTreeItemId node)
{
    TTaxTree * tree = (dynamic_cast<wxTaxTreeData*>(GetItemData(node)))->GetData();

    /**
    if ( !tree ) {
        /// the root doesn't have a tree associated with it
        /// descend always
        for (int i = 0;  i < node->children();  ++i) {
            SetSelectedUids(uids, node->child(i));
        }
    } else {
        /// check to see if the number of UIDs match
        CRef<CScope> scope = m_DataSource->GetScope();
        const CTaxTreeDataSource::TUidVec& this_uids = tree->GetValue().uids;
        if (this_uids.size() == uids.size()) {
            bool all_match = true;
            ITERATE (CTaxTreeDataSource::TUidVec, it1, this_uids) {
                bool found = false;
                ITERATE (CTaxTreeDataSource::TUidVec, it2, uids) {
                    if (CSeqUtils::Match(**it1, **it2, scope) ) {
                        found = true;
                        break;
                    }
                }
                if ( !found ) {
                    all_match = false;
                    break;
                }
            }

            if (all_match) {
                node->select(true);
            }
        } else if (this_uids.size() > uids.size()) {
            /// descend
            for (int i = 0;  i < node->children();  ++i) {
                SetSelectedUids(uids, node->child(i));
            }
        }
    }
    **/



    if (!GetChildrenCount(node)) {
        const CTaxTreeDataSource::TUidVec& uid_vec =
            tree->GetValue().uids;
        _ASSERT(uid_vec.size() == 1);
        if (uid_vec.size() != 1) {
            return;
        }

        CTaxTreeDataSource::TUid uid = uid_vec.front();
        CRef<CScope> scope = m_DataSource->GetScope();
        ITERATE (CTaxTreeDataSource::TUidVec, itt, uids) {
            if (CSeqUtils::Match(**itt, *uid, scope)) {
                SelectItem(node);

                wxTreeItemId parNode = node;
                while ( (parNode=GetItemParent(parNode)).IsOk() ) {
                    Expand(parNode);

                    wxTreeItemId chNode;
                    bool   bsel   = true;
                    wxTreeItemIdValue cookie;
                    do {
                        if (!chNode.IsOk()) {
                            chNode = GetFirstChild(parNode, cookie);
                        } else {
                            chNode = GetNextChild(parNode, cookie);
                        }
                        if ( !(bsel=IsSelected(chNode)) ) {
                            break;
                        }
                    } while (chNode!=GetLastChild(parNode));

                    SelectItem(parNode, bsel);
                }
            }
        }
    }

    wxTreeItemIdValue ck;
    wxTreeItemId  next = GetNextChild(node, ck);
    if (next.IsOk()) {
        SetSelectedUids(uids, next);
    }
}

END_NCBI_SCOPE
