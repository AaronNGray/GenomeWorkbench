/*  $Id: phy_tree_view.cpp 43374 2019-06-20 17:59:56Z katargir $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *    Phylogenetic tree viewer
 *    
 */

#include <ncbi_pch.hpp>

#include <algorithm>

#include <gui/packages/pkg_alignment/phy_tree_view.hpp>

#include <gui/widgets/data/query_panel_event.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_widget.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_reader.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <gui/core/project_service.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/app_job_task.hpp>
#include <gui/framework/status_bar_service.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/gl/attrib_menu.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/object_index.hpp>
#include <gui/objutils/taxid_sel_set.hpp>
#include <gui/objutils/biotree_selection.hpp>

#include <serial/serialimpl.hpp>


#include <objects/gbproj/AbstractProjectItem.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/biotree/FeatureDictSet.hpp>
#include <objects/biotree/FeatureDescr.hpp>
#include <objects/biotree/NodeSet.hpp>
#include <objects/biotree/Node.hpp>
#include <objects/biotree/NodeFeatureSet.hpp>
#include <objects/biotree/NodeFeature.hpp>
#include <objects/seqloc/Seq_interval.hpp>

#include <gui/packages/pkg_alignment/export_tree_dlg.hpp>
#include <gui/packages/pkg_alignment/phy_export_job.hpp>

#include <util/compress/lzo.hpp>
#include <corelib/ncbitime.hpp>

#include <wx/menu.h>
#include <wx/app.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/// Timers:
float m_CreateUndoCommand;
float m_ExecuteUndoCommand;

float m_CreateProperyUndoCommand;
float m_ExecutePropertyCommand;

float m_ExecOnBioTreeChanged;
float m_DataSourceInitTimer;

class CSelNodeHandle : public ISelObjectHandle
{
public:
    CSelNodeHandle(CPhyloTree::TTreeIdx node_idx) : m_NodeIdx(node_idx)    {}
    TTreeIdx GetNodeIdx() const { return m_NodeIdx; }

protected:
    TTreeIdx m_NodeIdx;
};

class CPhyRebuildIndexVisitor
{
public:
    CPhyRebuildIndexVisitor(CObjectIndex & idx) : m_Idx(&idx) {}
    ETreeTraverseCode operator()(CPhyloTree&  tree, CPhyloTree::TTreeIdx node_idx, int delta)
    {
        CPhyloTreeNode& node = tree[node_idx];
        if ((delta==1 || delta==0) && !(*node).GetSeqID().Empty()){
            m_Idx->Add(new CSelNodeHandle(node_idx), (*node).GetSeqID().GetObject());
        }
        return eTreeTraverse;
    }
private:
    CObjectIndex* m_Idx;
};


class CChangePhyloTreeCommand : public CObject, public IEditCommand
{
public:
    CChangePhyloTreeCommand( CPhyTreeView* view,
                             CConstRef<CBioTreeContainer> prev_tree,
                             CRef<CBioTreeContainer> new_tree) : m_View(view)
    {
        

        {
            CNcbiOstrstream  ostr;

            ostr << MSerial_AsnText << *prev_tree;
            string s = CNcbiOstrstreamToString(ostr);
            size_t len = s.size();
            size_t buf_size = m_Compressor.EstimateCompressionBufferSize(len);
            m_UtilVec.resize(buf_size);

            size_t compressed_len;
            m_Compressor.CompressBuffer(s.data(), len, 
                               (void*)(&m_UtilVec[0]), buf_size, &compressed_len);

            m_Tree.orig_tree.resize(compressed_len);
            memcpy((void*)(&m_Tree.orig_tree[0]),  (void*)(&m_UtilVec[0]),compressed_len);
        }
        {
            CNcbiOstrstream  ostr;

            ostr << MSerial_AsnText << *new_tree;
            string s = CNcbiOstrstreamToString(ostr);
            size_t len = s.size();
            size_t buf_size = m_Compressor.EstimateCompressionBufferSize(len);
            m_UtilVec.resize(buf_size);

            size_t compressed_len;

            m_Compressor.CompressBuffer((void*)s.data(), len, 
                (void*)(&m_UtilVec[0]), buf_size, &compressed_len);

            m_Tree.new_tree.resize(compressed_len);
            memcpy((void*)(&m_Tree.new_tree[0]),  (void*)(&m_UtilVec[0]),compressed_len);
        }
    }
    virtual void Execute()
    {        
        CStopWatch timer;
        timer.Start();

        size_t uncompressed_len;
        m_UtilVec.resize(m_UtilVec.capacity());

        m_Compressor.DecompressBuffer((void*)(&m_Tree.new_tree[0]), m_Tree.new_tree.size(),
            (void*)(&m_UtilVec[0]), m_UtilVec.size(), &uncompressed_len);

        CNcbiIstrstream istr(&(m_UtilVec[0]), uncompressed_len);
        m_View->CommitTheChanges(istr);

        CStopWatch timer2;
        timer2.Start();
		// Invoked from OnProjectChanged
        // m_View->OnBioTreeChanged(); 
        m_ExecOnBioTreeChanged = timer2.Elapsed();

        m_ExecuteUndoCommand = timer.Elapsed();
    }
    virtual void Unexecute()
    {     
        size_t uncompressed_len;
        m_UtilVec.resize(m_UtilVec.capacity());

        m_Compressor.DecompressBuffer((void*)(&m_Tree.orig_tree[0]), m_Tree.orig_tree.size(),
            (void*)(&m_UtilVec[0]), m_UtilVec.size(), &uncompressed_len);

        CNcbiIstrstream istr(&(m_UtilVec[0]), uncompressed_len);
        m_View->CommitTheChanges(istr);

        m_View->OnBioTreeChanged();
    }
    virtual string GetLabel() { return "Phylogenetic tree edit"; }

private:
    CPhyTreeView*        m_View;
    static vector<char>  m_UtilVec;
    static CLZOCompression m_Compressor;
   
    typedef struct SBTree {
        vector<char>  orig_tree;
        vector<char>  new_tree;
    } TBTree;
    TBTree m_Tree;
};

vector<char>  CChangePhyloTreeCommand::m_UtilVec;
CLZOCompression CChangePhyloTreeCommand::m_Compressor;



/// SelectionSets are stored in the biotreecontainer's user-data object
/// so we need a special change command object to handle it's undo-and redo.
/// CChangePhyloTreeCommand which saves the entire tree would also work,
/// but is much less (space) efficient
class CChangePhyloSelectionSetCmd : public CObject, public IEditCommand
{
public:
    CChangePhyloSelectionSetCmd(CPhyTreeView* view,
        CSelectionSetEdit* sel_edit,
        CRef<CPhyloTreeDataSource> ds,
        CConstRef <objects::CBioTreeContainer> bio_tree)
    {
        m_View = view;
        m_DS = ds;
        m_BioTree = bio_tree;
        m_SelectionEdit.Reset(sel_edit);
    }

    void PerformEdit()
    {
        // renumber cluster IDs in set to make sure the selection cluster IDs
        // don't overlap regular cluster IDs:
        // pick cluster id larger than existing ids:
        CPhyloTree::TClusterID max_id =
            m_DS->GetMaxClusterID() + 500 + m_DS->GetSelectionSets().GetSets().size();
        // set ids, first highest then lower.
        m_DS->GetSelectionSets().RenumberClusterIDs(max_id);

        m_DS->GetTree()->GetSelectionSets().SetSelectionSetProperty(m_DS->GetTree());

        // Need update access to biotree to commit the changes
        CBioTreeContainer* bio_tree =
            const_cast<CBioTreeContainer *> (m_BioTree.GetPointer());

        CBioTreeContainer_Base::TUser& uo = bio_tree->SetUser();
        if (!uo.CanGetType() || !uo.IsSetType() || uo.GetType().Which() == CObject_id_Base::e_not_set) {
            CRef<CObject_id> uo_id;
            uo_id.Reset(new CObject_id());
            uo_id->SetStr("Tree Metadata");
            uo.SetType(*uo_id);
        }

        m_DS->GetSelectionSets().SaveToUserObject(uo);

        // should check first that label updated (for efficiency)
        m_View->AppliedEditToDataSource();
    }

    virtual void Execute()
    {
        m_SelectionEdit->GetPrevSet() = m_DS->GetTree()->GetSelectionSets();
        m_DS->GetTree()->GetSelectionSets() = m_SelectionEdit->GetUpdatedSet();

        PerformEdit();
    }
    virtual void Unexecute()
    {
        m_DS->GetTree()->GetSelectionSets() = m_SelectionEdit->GetPrevSet();       

        PerformEdit();
    }

    virtual string GetLabel() { return "Phylogenetic tree SelectionSets update"; }

private:
    CPhyTreeView*  m_View;

    // project biotreecontainer
    CConstRef <objects::CBioTreeContainer> m_BioTree;

    CRef<CPhyloTreeDataSource> m_DS;
    CRef<CSelectionSetEdit> m_SelectionEdit;
};

class SUpdateFeatureCompare
{
public:
    SUpdateFeatureCompare(const CPhyloNodeData::TID id) : m_ID(id) {}

    bool operator() (const CUpdatedFeature &lhs, const CUpdatedFeature &rhs) const
    {
        return (lhs.m_NodeId < rhs.m_NodeId);
    }
    bool operator() (const CUpdatedFeature &lhs, const CPhyloNodeData::TID nod_id) const
    {
        return (lhs.m_NodeId < nod_id);
    }
    bool operator() (const CPhyloNodeData::TID node_id, const CUpdatedFeature &rhs) const
    {
        return (node_id < rhs.m_NodeId);
    }
private:
    CPhyloNodeData::TID m_ID;
};



class CChangePhyloPropertyCmd : public CObject, public IEditCommand
{
public:
    typedef CBioTreeContainer::TFdict  TContainerDict;
    typedef TContainerDict::Tdata::value_type::element_type TCFeatureDescr;

    typedef CBioTreeContainer::TNodes                  TCNodeSet;
    typedef TCNodeSet::Tdata                           TNodeList;
    typedef TNodeList::value_type::element_type        TCNode;
    typedef TCNode::TFeatures                          TCNodeFeatureSet;
    typedef TCNodeFeatureSet::Tdata                    TNodeFeatureList;
    typedef TNodeFeatureList::value_type::element_type TCNodeFeature;

public:
    /// IEditCommand merge commands (macros allow changes to both properties
    /// and selection sets, so allow those to merge):
    virtual bool CanMerge(IEditCommand* cmd) {
        CChangePhyloSelectionSetCmd* sel_cmd = dynamic_cast<CChangePhyloSelectionSetCmd*>(cmd);

        if (sel_cmd != NULL && true) {
            return true;
        }
        return false;
    }

    virtual void Merge(IEditCommand* cmd) {
        CChangePhyloSelectionSetCmd* sel_cmd = dynamic_cast<CChangePhyloSelectionSetCmd*>(cmd);

        if (sel_cmd != NULL && m_MergedCmd.IsNull())
            m_MergedCmd.Reset(sel_cmd);
    }
    
public:
    CChangePhyloPropertyCmd(CPhyTreeView* view,
                            CFeatureEdit* fedit, 
                            CRef<CPhyloTreeDataSource> ds,
                            CConstRef <objects::CBioTreeContainer> bio_tree)
    {
        m_View = view;
        m_DS = ds;
        m_BioTree = bio_tree;
        m_FeatureEdit.Reset(fedit);
        m_Label.SetScheme(m_View->GetWidget()->GetScheme(),  m_DS->GetTree());

        // Sort the features in feature edit because if there are a large number of features,
        // the linear search for them in the biotreecontainer can be very slow.
        std::sort(fedit->GetUpdated().begin(), fedit->GetUpdated().end());
    }

    void PerformFeatureDictEdit(CBioTreeFeatureDictionary& dict)
    {
        // Need update access to biotree to commit the changes
        CBioTreeContainer* bio_tree =
            const_cast<CBioTreeContainer *> (m_BioTree.GetPointer());

        // Update feature dictionary of biotree
        TContainerDict& fd = bio_tree->SetFdict();
        fd.Reset();
        TContainerDict::Tdata& feat_list = fd.Set();

        ITERATE(CBioTreeFeatureDictionary::TFeatureDict, it, dict.GetFeatureDict()) {
            TBioTreeFeatureId fid = it->first;
            const string& fvalue = it->second;

            {{
                    CRef<TCFeatureDescr> d(new TCFeatureDescr());
                    d->SetId(fid);
                    d->SetName(fvalue);

                    feat_list.push_back(d);
                }}
        } // ITERATE  
    }

    void PerformEdit(CBioTreeFeatureDictionary& dict, bool prev_features)
    {

        // Need update access to biotree to commit the changes
        CBioTreeContainer* bio_tree =
            const_cast<CBioTreeContainer *> (m_BioTree.GetPointer());

        //
        // Update node features of biotree (have to find node by id first):
        TNodeList node_list = bio_tree->SetNodes().Set();
        bool found_node = false;
        int update_count = 0;

        NON_CONST_ITERATE(TNodeList, it, node_list) {
            if (update_count == m_FeatureEdit->GetUpdated().size())
                break;

            CRef<TCNode>& cnode = *it;

            TCNode::TId uid = cnode->GetId();

            CPhyloTreeNode* node = NULL;
            auto iter = std::lower_bound(m_FeatureEdit->GetUpdated().begin(), m_FeatureEdit->GetUpdated().end(), uid, SUpdateFeatureCompare(uid));
            if (iter != m_FeatureEdit->GetUpdated().end() && iter->m_NodeId == uid) {
                ++update_count;

                if (iter->m_NodeIdx != CPhyloTree::Null()) {
                    node = &m_DS->GetTree()->GetNode(iter->m_NodeIdx);
                }
                else {
                    CPhyloTree::TTreeIdx idx = m_DS->GetTree()->FindNodeById(iter->m_NodeId);
                    if (idx != CPhyloTree::Null())
                        node = &m_DS->GetTree()->GetNode(idx);
                }

                if (node != NULL) {
                    // now perform the update:
                    if (prev_features)
                        (*node)->GetBioTreeFeatureList() = iter->GetPrevFeatures();
                    else
                        (*node)->GetBioTreeFeatureList() = iter->GetFeatures();

                    // Propogate features to node elements for display. 
                    (*node)->Init(dict, m_DS->GetModel().GetColorTable());
                    (*node)->SetLabel(m_Label.GetLabelForNode(*(m_DS->GetTree()), *node, m_View->GetWidget()->GetScheme().SetLabelFormat()));

                    found_node = true;
                    cnode->ResetFeatures();
                    TCNodeFeatureSet& fset = cnode->SetFeatures();

                    ITERATE(CBioTreeFeatureList::TFeatureList, it, (*node)->GetBioTreeFeatureList().GetFeatureList()) {
                        TBioTreeFeatureId fid = (*it).id;
                        const string& fvalue = (*it).value;

                        CRef<TCNodeFeature>  cfeat(new TCNodeFeature());
                        cfeat->SetFeatureid(fid);
                        cfeat->SetValue(fvalue);

                        fset.Set().push_back(cfeat);
                    } // ITERATE
                }
                else {
                    LOG_POST(Error << "Execute properties update error on node ID: " <<
                             iter->m_NodeId);
                }
            }
        }
    }

    virtual void Execute()
    {
        CStopWatch timer;
        timer.Start();

        CPhyloTreeNode* node = NULL;
        
        // Update dictionary and save previous version
        m_FeatureEdit->GetPrevDictionary() = m_DS->GetTree()->GetFeatureDict();
        m_DS->GetTree()->GetFeatureDict() = m_FeatureEdit->GetDictionary();

        PerformFeatureDictEdit(m_FeatureEdit->GetDictionary());

        PerformEdit(m_FeatureEdit->GetDictionary(), false);

        // should check first that label updated (for efficiency)
        m_View->AppliedEditToDataSource();

        if (m_FeatureEdit->SeqIdEdited())
            m_View->RebuildObjectIndex();

        if (!m_MergedCmd.IsNull())
            m_MergedCmd->Execute();

        m_ExecutePropertyCommand = timer.Elapsed();
    } 
    virtual void Unexecute()
    {
        CPhyloTreeNode* node = NULL;
        m_DS->GetTree()->GetFeatureDict() = m_FeatureEdit->GetPrevDictionary();

        PerformFeatureDictEdit(m_FeatureEdit->GetPrevDictionary());

        PerformEdit(m_FeatureEdit->GetPrevDictionary(), true);

        // should check first that label updated (for efficiency)
        m_View->AppliedEditToDataSource();

        if (m_FeatureEdit->SeqIdEdited())
            m_View->RebuildObjectIndex();

        if (!m_MergedCmd.IsNull())
            m_MergedCmd->Unexecute();
    }

    virtual string GetLabel() { return "Phylogenetic tree feature update"; }

private:
     CPhyTreeView*  m_View;

    // project biotreecontainer
    CConstRef <objects::CBioTreeContainer> m_BioTree;

    CRef<CPhyloTreeDataSource> m_DS;
    CRef<CFeatureEdit> m_FeatureEdit;
    CPhyloTreeLabel m_Label;

    CRef<CChangePhyloSelectionSetCmd> m_MergedCmd;
};

class CChangePhyloExpandCollapseCmd : public CObject, public IEditCommand
{
public:
    typedef CBioTreeContainer::TFdict  TContainerDict;
    typedef TContainerDict::Tdata::value_type::element_type TCFeatureDescr;

    typedef CBioTreeContainer::TNodes                  TCNodeSet;
    typedef TCNodeSet::Tdata                           TNodeList;
    typedef TNodeList::value_type::element_type        TCNode;
    typedef TCNode::TFeatures                          TCNodeFeatureSet;
    typedef TCNodeFeatureSet::Tdata                    TNodeFeatureList;
    typedef TNodeFeatureList::value_type::element_type TCNodeFeature;

public:
    CChangePhyloExpandCollapseCmd(CPhyTreeView* view,
        CExpandCollapseNodes* ec_edit,
        CRef<CPhyloTreeDataSource> ds,
        CConstRef <objects::CBioTreeContainer> bio_tree)
    {
        m_View = view;
        m_DS = ds;
        m_BioTree = bio_tree;
        m_ExpandCollapseEdit.Reset(ec_edit);
    }

    void PerformEdit(CPhyloNodeData::TDisplayChildren ec,
                     CNodeFeature_Base::TValue& collapse_value)
    {
        // Need update access to biotree to commit the changes
        CBioTreeContainer* bio_tree =
            const_cast<CBioTreeContainer *> (m_BioTree.GetPointer());

        // Update feature dictionary of biotree
        TContainerDict& fd = bio_tree->SetFdict();
        TContainerDict::Tdata& feat_list = fd.Set();
        bool has_feature = false;
        TBioTreeFeatureId collapse_fid;
        CFeatureDescr_Base::TId max_fid = (CFeatureDescr_Base::TId)0;

        // Get ID of collapse feature from dictionary (it should be there). If it is not
        // there, add it.
        ITERATE(TContainerDict::Tdata, it, feat_list) {
            CRef<TCFeatureDescr> fdesc = *it;

            if (fdesc->GetName() == "$NODE_COLLAPSED") {
                has_feature = true;
                collapse_fid = fdesc->GetId();
            }
            max_fid = std::max(max_fid, fdesc->GetId());
        }

        if (!has_feature) {
            collapse_fid = max_fid + 1;

            CRef<TCFeatureDescr> d(new TCFeatureDescr());
            d->SetId(collapse_fid);
            d->SetName("$NODE_COLLAPSED");

            feat_list.push_back(d);
        }


        //
        // Update node features of biotree (have to find node by id first):
        TNodeList node_list = bio_tree->SetNodes().Set();

        vector<CPhyloNodeData::TID> node_ids = m_ExpandCollapseEdit->GetIds();
        std::sort(node_ids.begin(), node_ids.end());
        int nodes_updated = 0;

        // Traverse all the nodes in the biotree container. For any nodes with ids
        // that are in the m_ExpandCollapseEdit object, expand or collapse that
        // node.
        NON_CONST_ITERATE(TNodeList, it, node_list) {
            CRef<TCNode>& cnode = *it;

            TCNode::TId uid = cnode->GetId();
            if (std::binary_search(node_ids.begin(), node_ids.end(), uid)) {
                ++nodes_updated;

                TTreeIdx node_idx = m_DS->GetTree()->FindNodeById(uid);
                if (node_idx == CPhyloTree::Null()) {
                    _TRACE("Node not found for id: " << uid);
                    continue;
                }
                CPhyloTreeNode& node = m_DS->GetTree()->GetNode(node_idx);
                node.ExpandCollapse(m_DS->GetTree()->GetFeatureDict(), ec);

                //if (uid == TCNode::TId(node->GetValue().GetId())) {
                TCNodeFeatureSet& fset = cnode->SetFeatures();
                CNodeFeatureSet_Base::Tdata& features = fset.Set();
                bool collapse_feature_found = false;

                ITERATE(CNodeFeatureSet_Base::Tdata, it, features) {
                    CRef< CNodeFeature > feature = *it;
                    if (feature->GetFeatureid() == collapse_fid) {
                        // Feature found. Set the updated expand/collapse value.
                        feature->SetValue(collapse_value);
                        collapse_feature_found = true;
                        break;
                    }
                }

                // Add the collapse feature if it's not already in the node
                if (!collapse_feature_found) {
                    CRef<TCNodeFeature>  cfeat(new TCNodeFeature());
                    cfeat->SetFeatureid(collapse_fid);
                    cfeat->SetValue(collapse_value);

                    features.push_back(cfeat);
                }
            }
        }
        if (nodes_updated != node_ids.size()) {
            _TRACE("Execute properties update error - CBioTreeContainer collapse node update count did not match");
        }
    }

    virtual void Execute()
    {
        CStopWatch timer;
        timer.Start();

        CPhyloNodeData::TDisplayChildren ec = m_ExpandCollapseEdit->GetExpanded();

        string feature_value = "0";
        if (ec == CPhyloNodeData::eHideChildren)
            feature_value = "1";

        PerformEdit(ec, feature_value);
        m_View->AppliedEditToDataSource();

        m_ExecutePropertyCommand = timer.Elapsed();
    }
    virtual void Unexecute()
    {
        CStopWatch timer;
        timer.Start();

        string feature_value;

        CPhyloNodeData::TDisplayChildren ec = m_ExpandCollapseEdit->GetExpanded();
        if (ec == CPhyloNodeData::eHideChildren) {
            ec = CPhyloNodeData::eShowChildren;
            feature_value = "1";
        }
        else {
            ec = CPhyloNodeData::eHideChildren;
            feature_value = "0";
        }

        PerformEdit(ec, feature_value);
        m_View->AppliedEditToDataSource();
    }

    virtual string GetLabel() { return "Phylogenetic tree expand/collapse update"; }

private:
    CPhyTreeView*  m_View;

    // project biotreecontainer
    CConstRef <objects::CBioTreeContainer> m_BioTree;

    CRef<CPhyloTreeDataSource> m_DS;
    CRef<CExpandCollapseNodes> m_ExpandCollapseEdit;
};

BEGIN_EVENT_MAP(CPhyTreeView, CProjectView)
    ON_EVENT(CEvent, eCmdExportTree, &CPhyTreeView::OnExportTree)
    ON_EVENT(CEvent, eCmdSomethingEdited, &CPhyTreeView::OnSomethingEdited)
    ON_EVENT(CEvent, eCmdFeaturesEdited, &CPhyTreeView::OnSomethingEdited)
    ON_EVENT(CEvent, eCmdGrpExpandCollapse, &CPhyTreeView::OnSomethingEdited)
    ON_EVENT(CEvent, eCmdNodeExpandCollapse, &CPhyTreeView::OnSomethingEdited)
    ON_EVENT(CEvent, eCmdTreeLabelSet, &CPhyTreeView::OnSomethingEdited)
    ON_EVENT(CEvent, eCmdLabelFormatChanged, &CPhyTreeView::OnSomethingEdited)
    ON_EVENT(CEvent, eCmdRenderingOptionsChanged, &CPhyTreeView::OnSomethingEdited)
    ON_EVENT(CEvent, eCmdTreeSorted, &CPhyTreeView::OnSomethingEdited)
    ON_EVENT(CEvent, eCmdSubtreeFromSelected, &CPhyTreeView::OnCreateSelectedSubtree)

    ON_EVENT(CQueryPanelEvent, CQueryPanelEvent::eStatusChange, &CPhyTreeView::x_QueryStatusChange)
END_EVENT_MAP()

CProjectViewTypeDescriptor s_PhyTreeViewTypeDescr(
    "Tree View", // type name
    "phylo_tree_view", // icon alias TODO
    "Tree View",
    "View for graphical presentation of hierarchical data (phylogenetic tree, taxonomy tree, etc.) using various layout methods",
    "PHY_TREE_VIEW", // help ID
    "Alignment",   // category
    false,       // not a singleton
    "BioTreeContainer",
    eOneObjectAccepted
);

CPhyTreeView::CPhyTreeView()
: m_Panel(nullptr)
, m_EditUpdate(false)
, m_CurrentNode(CPhyloTreeNode::Null())
{
#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenu& m = CAttribMenuInstance::GetInstance();
    CAttribMenu* sub_menu = m.AddSubMenuUnique("PhyView", this);

    sub_menu->AddFloatReadOnly("Create Undo Time", &m_CreateUndoCommand);
    sub_menu->AddFloatReadOnly("Execute Undo Time", &m_ExecuteUndoCommand);
    sub_menu->AddFloatReadOnly("Create Property Undo Time", &m_CreateProperyUndoCommand);
    sub_menu->AddFloatReadOnly("Execute Property Undo Count", &m_ExecutePropertyCommand);
    sub_menu->AddFloatReadOnly("Exec OnBioTreeChanged", &m_ExecOnBioTreeChanged);
    sub_menu->AddFloatReadOnly("Datasource Init time", &m_DataSourceInitTimer);
#endif
}

CPhyTreeView::~CPhyTreeView()
{
    DestroyViewWindow();

#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenuInstance::GetInstance().RemoveMenuR("PhyView", this);
#endif
}


wxWindow* CPhyTreeView::GetWindow()
{
    _ASSERT(m_Panel);
    return m_Panel;
}


void CPhyTreeView::x_CreateMenuBarMenu()
{   
    wxMenu* view_menu = new wxMenu(); 

    view_menu->Append( wxID_SEPARATOR, wxT("Actions") );

    wxMenu* bar_menu = new wxMenu();
    bar_menu->Append( wxID_ANY, wxT("&View"), view_menu );

    m_MenuBarMenu.reset( bar_menu );
}

class CPhyTreePanel : public wxPanel
{
public:
    CPhyTreePanel(wxWindow* parent, long childId) : wxPanel(parent), m_ChildId(childId) {}

    virtual bool ProcessEvent(wxEvent &event);
private:
    long m_ChildId;
};

bool CPhyTreePanel::ProcessEvent(wxEvent &event)
{
    if (event.IsCommandEvent()) {
        wxEventType type = event.GetEventType();
        if (type == wxEVT_UPDATE_UI || type == wxEVT_COMMAND_MENU_SELECTED) {
            wxWindow* child = FindWindow(m_ChildId);
            if (child) {
                wxEvtHandler* evtHandler = child->GetEventHandler();
                if (evtHandler && evtHandler->ProcessEventLocally(event))
                    return true;
            }
        }
    }
    return wxPanel::ProcessEvent(event);
}

void CPhyTreeView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT(!m_Panel);

    m_Panel = new CPhyTreePanel(parent, kPhylpWidgetID);

    CPhyloTreeWidget* phyWidget = new CPhyloTreeWidget(m_Panel, kPhylpWidgetID);
    phyWidget->Create();

    CQueryParsePanel* queryPanel = new CQueryParsePanel(phyWidget, m_DataSource.GetPointerOrNull());
    queryPanel->Create(m_Panel, kQueryPanelID);
    phyWidget->SetQueryPanel(queryPanel);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    m_Panel->SetSizer(sizer);

    sizer->Add(queryPanel, 0, wxEXPAND);
    sizer->Add(phyWidget, 1, wxEXPAND);

    AddListener(phyWidget, ePool_Child);
    phyWidget->AddListener(this, ePool_Parent);
}

CPhyloTreeWidget* CPhyTreeView::GetWidget() const
{
    if (!m_Panel)
        return nullptr;

    return dynamic_cast<CPhyloTreeWidget*>(m_Panel->FindWindow(kPhylpWidgetID));
}

void CPhyTreeView::DestroyViewWindow()
{
    if(m_Panel) {
        m_Panel->Destroy();
        m_Panel = NULL;
    }
}


const CViewTypeDescriptor& CPhyTreeView::GetTypeDescriptor() const
{
    return s_PhyTreeViewTypeDescr;
}


bool CPhyTreeView::InitView(TConstScopedObjects& objects, const objects::CUser_object*)
{
    // exploring objects to find biotree object 
    if(objects.size() > 0)  {
        CPhyloTreeWidget* phyWidget = GetWidget();

        ITERATE(TConstScopedObjects, it, objects) {
            const CObject* object = it->object.GetPointer();
            m_BioTreeContainer = dynamic_cast<const CBioTreeContainer *>(object);

              
            // first found biotree will be shown
            if(m_BioTreeContainer) {
                if (m_BioTreeContainer->GetNodeCount() == 0 ) {
                    m_BioTreeContainer.Reset();
                }
                else {
                    break;
                }
            } 
        }
        if (m_BioTreeContainer) {
            CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
            _ASSERT(srv);

            CScope* scope = objects[0].scope.GetPointer();

            CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
            if (!ws) return false;

            CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*objects[0].scope));
            _ASSERT(doc);
            
            phyWidget->LoadSettings();

            // Tree may have some rendering paameters saved inside asn user object.
            // Set those here when first loading (don't want to reset them each time
            // tree is edited).

            m_InitialUseDistances = -1;
            m_InitialLayout = -1;
            m_InitialRotateLabels = -1;
            if (m_BioTreeContainer->IsSetUser()) {
                const CBioTreeContainer_Base::TUser& uo = m_BioTreeContainer->GetUser();

                if (uo.HasField("layout") &&
                    uo.GetField("layout").GetData().IsInt()) {
                        m_InitialLayout = uo.GetField("layout").GetData().GetInt();
                }

                if (uo.HasField("use-distances") &&
                    uo.GetField("use-distances").GetData().IsBool()) {
                        m_InitialUseDistances = uo.GetField("use-distances").GetData().GetBool() ? 1 : 0;
                }

                if (uo.HasField("rotate-labels") &&
                    uo.GetField("rotate-labels").GetData().IsBool()) {
                        m_InitialRotateLabels = uo.GetField("rotate-labels").GetData().GetBool() ? 1 : 0;
                }
            }

            // If a specific layout was not saved in the file set it to whatever
            // layout the user last used
            if (m_InitialLayout == -1 && phyWidget->HasScheme()) {
                m_InitialLayout = phyWidget->GetScheme().GetLayoutIdx();
            }

            if (doc) {
                x_AttachToProject(*doc);                      
                CProjectViewEvent evt( doc->GetId(), CProjectViewEvent::eData);

                try {
                    OnProjectChanged(evt);
                }
                catch (CException& e) {
                    LOG_POST(Error << "Error loading tree: " << e.GetMsg());
                    return false;
                }

                return true;
            }
            else {
                return false;
            }
        }
        else {
            x_ReportInvalidInputData(objects);
            return false;
        }        
    }
    return false;
}


void CPhyTreeView::OnProjectChanged(CProjectViewEvent& evt)
{
    x_UpdateContentLabel();

    if(x_HasProject()) {      
        _ASSERT( ! m_BioTreeContainer.Empty());

        //
        if (evt.GetSubtype() == CProjectViewEvent::eDataChanging)
            return;

        // If the update is coming because of an edit through this view, there is no reason
        // to rebuild the data source since this is the view that changed it.  But we
        // do need to redraw the view to reflect the updated data.
        // m_EditUpdate is a bit of a hack to recognize event coming from this view since undo 
        // manager doesn't track the source view.
        if (m_EditUpdate) {
            CPhyloTreeWidget* phyWidget = GetWidget();

            if (phyWidget) {
                m_Scheme->SetTooltipFormat() = m_DataSource->GenerateTooltipFormat();             
                phyWidget->RedrawDataSource();
            }

            m_EditUpdate = false;
            return;
        } 
        OnBioTreeChanged();        
    }
    else {
        m_ObjectIndex.Clear(NULL);
    }
}

void CPhyTreeView::OnBioTreeChanged()
{  
    CScope* scope = x_GetScope();
    _ASSERT(scope);

    CPhyloTreeWidget* phyWidget = GetWidget();

    bool initial = m_DataSource.Empty();
    bool first_update = (phyWidget->GetDS() == NULL);

    // This will cancel any background processes using the data source
    phyWidget->RemoveCurrentDataSource();

    // Delete and recreate the data source  
    CStopWatch timer;
    timer.Start();

    // set default scheme
    m_Scheme.Reset(new CPhyloTreeScheme());
    if (phyWidget->HasScheme())
        *m_Scheme = phyWidget->GetScheme();

    m_Scheme->LoadCurrentSettings();
    string label_format;

    /// If label format is saved with tree, set it in the scheme
    if (m_BioTreeContainer->IsSetUser()) {
        const CBioTreeContainer_Base::TUser& uo = m_BioTreeContainer->GetUser();

        // Only copy label-format since that's all that should be there
        if (uo.HasField("label-format") &&
            uo.GetField("label-format").GetData().IsStr()) {
                label_format = uo.GetField("label-format").GetData().GetStr();
                m_Scheme->SetLabelFormat() = label_format;
        }
    }

    // set default label style for tax tree if the tree didn't have a 
    // label-format saved with it in the user-object
    if (m_BioTreeContainer->CanGetTreetype() && 
        label_format == "" &&
        (m_BioTreeContainer->GetTreetype() == "CommonTaxTree")){
            m_Scheme->SetLabelFormat() = "$(name)$(seq-id)";
            m_Scheme->SetLabelVisibility(CPhyloTreeScheme::eLabelsVisible);      
    }
    // Forcing label to "$(name)$(seq-id)" is a little odd since it will
    // still be set when user is done looking at taxtrees. Users may be confused
    // why no label appears in non-taxtrees after viewing a taxtree, so we set
    // label back to default it is set to taxtree value...
    else if ((!m_BioTreeContainer->CanGetTreetype() ||
              (!m_BioTreeContainer->CanGetTreetype() &&
               m_BioTreeContainer->GetTreetype() != "CommonTaxTree")) &&
             label_format == "" &&
             m_Scheme->SetLabelFormat() == "$(name)$(seq-id)"){
            m_Scheme->SetLabelFormat() = "$(label)";
            m_Scheme->SetLabelVisibility(CPhyloTreeScheme::eLabelsForLeavesOnly);
    }

    if (label_format == "")
        label_format = m_Scheme->SetLabelFormat();

    if (m_DataSource.IsNull())
        m_DataSource.Reset(new CPhyloTreeDataSource(*m_BioTreeContainer, *scope));
    else {
        m_DataSource->Clear();
        m_DataSource->Init(*m_BioTreeContainer, *scope);
    }
    m_DataSourceInitTimer = timer.Elapsed();

    // sort if needed
    if (m_DataSource->GetTree()->GetFeatureDict().HasFeature("do_sort") && initial) {
        m_DataSource->Sort(true);  
    }


    // On linux, the gui updates forced by SetDataSource() can
    // cause a crash if the window/widget is not yet fully created.
    phyWidget->SetDataSourceNoUpdate(m_DataSource);

    // apply distances rendering, if distances available
    if (initial) {
        phyWidget->SetUseDistances(
            m_DataSource->GetTree()->GetFeatureDict().HasFeature("dist"));
             
        if (m_DataSource->GetTree()->GetFeatureDict().HasFeature("label") &&
            label_format == "") {
             m_Scheme->SetLabelFormat() = "$(label)";
             m_Scheme->SetLabelStyle(CPhyloTreeScheme::eFancyLabels);
        }
    }

    m_Scheme->SetTooltipFormat() = m_DataSource->GenerateTooltipFormat();
    phyWidget->SetScheme(*m_Scheme);

    // datasource labeling (creating labels cach)
    m_DataSource->Relabel(m_Scheme, m_Scheme->SetLabelFormat());

    RebuildObjectIndex();

    // Set rendering parameters which are (optionally) stored in biotreecontainer
    // only after file is first loaded
    if (m_InitialUseDistances != -1) {
        phyWidget->SetUseDistances((m_InitialUseDistances==1) ? true : false);
        m_InitialUseDistances = -1;
    }   
    if (m_InitialRotateLabels != -1) {
        phyWidget->SetRotateLabels((m_InitialRotateLabels==1) ? true : false);
        m_InitialRotateLabels = -1;
    }

    if (m_InitialLayout != -1) {
        // this also does layout so we do not need to call RedrawDataSource
        // if this is called
        phyWidget->SetCurrRenderer(m_InitialLayout);
        m_InitialLayout = -1;
    }
    else {
        // This does layout, which needs labels which are set above.  
        phyWidget->RedrawDataSource();
    }
}


void CPhyTreeView::RebuildObjectIndex()
{
    _ASSERT(m_DataSource);

    CScope* scope = x_GetScope();
    m_ObjectIndex.Clear(scope);

    TreeDepthFirst(*(m_DataSource->GetTree()),
        CPhyRebuildIndexVisitor(m_ObjectIndex));
}


void CPhyTreeView::GetSelection(TConstScopedObjects& objs) const
{
     //This is called from the idle update to recheck selection constantly. For that 
     //reason, it can be a bit expensive.

    CPhyloTreeWidget* phyWidget = GetWidget();

    if (phyWidget && m_DataSource && x_HasProject()) {

        CScope* scope = x_GetScope();
        
        // Tree traversel doesn't have a const version
        const CPhyloTreeDataSource* ds = m_DataSource.GetPointer();
        CPhyloTree* tree = const_cast<CPhyloTree*>(ds->GetTree());

        // 
        if (ds->GetTree()->HasCurrentNode()) {
            if (m_CurrentNode != ds->GetTree()->GetCurrentNodeIdx()) {
                m_CurrentNode = ds->GetTree()->GetCurrentNodeIdx();
                m_Cont.Reset(new CBioTreeContainer());
                TreeConvert2Container(*m_Cont, 
                    *tree,
                    m_CurrentNode);

                m_Cont->SetTreetype("Phylogenetic Tree"); 
            }
            objs.push_back(SConstScopedObject(m_Cont, scope));     
        }

        vector<CPhyloTree::TTreeIdx> sel_nodes;
        ds->GetTree()->GetSelected(sel_nodes);
              
        ITERATE(vector<CPhyloTree::TTreeIdx>, it, sel_nodes) {
            const CPhyloTreeNode& n = ds->GetTree()->GetNode(*it);
            if (!(*n).GetSeqID().Empty()) {
                objs.push_back(SConstScopedObject((*n).GetSeqID().GetPointer(), scope));
            }
        }        
    } 
}


void CPhyTreeView::GetSelection(CSelectionEvent& evt) const
{   
    CPhyloTreeWidget* phyWidget = GetWidget();

    if (phyWidget && m_DataSource && x_HasProject()) {

        const CPhyloTreeDataSource* ds = m_DataSource.GetPointer();

        vector<CPhyloTree::TTreeIdx>  sel_nodes;
        ds->GetTree()->GetSelected(sel_nodes);
        if (sel_nodes.size() > 0) { 
            CBioTreeSelection* bioTreeSelection = new CBioTreeSelection(m_BioTreeContainer);
            CBioTreeSelection::TNodeIdsVector &node_ids = bioTreeSelection->GetNodeIds();
            CBioTreeSelection::TNodeIdsVector &explicitly_selected_node_ids = bioTreeSelection->GetExplicitlySelectedNodeIds();
            ds->GetTree()->GetSelectedIDs(node_ids);
            ds->GetTree()->GetExplicitlySelectedIDs(explicitly_selected_node_ids);

            evt.AddIndexed("tree", *bioTreeSelection);
        }

        if (ds->GetDictionary().HasFeature("seq-id")) {
            TConstObjects objs;
            ITERATE(vector<CPhyloTree::TTreeIdx>, it, sel_nodes) {
                const CPhyloTreeNode& n = ds->GetTree()->GetNode(*it);
                if (!(*n).GetSeqID().Empty()) {            
                    objs.push_back(CConstRef<CObject>((*n).GetSeqID().GetPointer()));
                }
            }
            evt.AddObjectSelection(objs);        
        }
        
        if (ds->GetDictionary().HasFeature("tax-id") || ds->GetDictionary().HasFeature("taxid")) {
            TBioTreeFeatureId feature_id = ds->GetDictionary().GetId("tax-id");
            // allow for common misspelling
            if (feature_id ==  (TBioTreeFeatureId)-1)
                feature_id = ds->GetDictionary().GetId("taxid");

            ITERATE(vector<CPhyloTree::TTreeIdx>, it, sel_nodes) {
                const CPhyloTreeNode& n = ds->GetTree()->GetNode(*it);
                const string& tax_id = (*n).GetBioTreeFeatureList()[feature_id];
                if (tax_id != "") {
                    CTaxIdSelSet::TTaxId id = (CTaxIdSelSet::TTaxId)NStr::StringToUInt(tax_id);
                    evt.AddTaxIDSelection(id);
                }
            }
        }    
    }    
}

class visitor_taxid_query
{
public:
    visitor_taxid_query(const CSelectionEvent::TTaxIds& tids,
                        TBioTreeFeatureId tax_feat_id) 
        : m_TaxIds(tids) 
        , m_TaxFeatureId(tax_feat_id) {}
    
    ETreeTraverseCode operator()(CPhyloTree&  tree, CPhyloTree::TTreeIdx node_idx, int delta)
    {
        if (delta==1 || delta==0){
            CPhyloTreeNode& tree_node = tree[node_idx];
            const string& tax_id = (*tree_node).GetBioTreeFeatureList()[m_TaxFeatureId];
            if (tax_id != "") {
                CTaxIdSelSet::TTaxId id = (CTaxIdSelSet::TTaxId)NStr::StringToUInt(tax_id);
                if (m_TaxIds.IsSelected(id)) {
                    m_SelNodes.push_back(node_idx);
                }
            }   
        }
        return eTreeTraverse;
    }

    visitor_taxid_query& operator=(const visitor_taxid_query& rhs) 
        { m_SelNodes = rhs.m_SelNodes; return *this; }

    const vector<CPhyloTree::TTreeIdx>& GetNodes() { return m_SelNodes; }
private:
    vector<CPhyloTree::TTreeIdx> m_SelNodes;

    const CSelectionEvent::TTaxIds& m_TaxIds;
    TBioTreeFeatureId m_TaxFeatureId;
};


class visitor_string_query
{
public:
    visitor_string_query(TBioTreeFeatureId feat_id,
                         const set<string>& feat_values) 
        : m_FeatureId(feat_id) 
        , m_FeatureValues(feat_values) 
    {       
    }
    
    ETreeTraverseCode operator()(CPhyloTree& tree, CPhyloTree::TTreeIdx node_idx, int delta)
    {
        if (delta==1 || delta==0) {
            CPhyloTreeNode& tree_node = tree[node_idx];
            const string& feat_val = (*tree_node).GetBioTreeFeatureList()[m_FeatureId];
            if (feat_val != "") {
                if (m_FeatureValues.find(feat_val) != m_FeatureValues.end()) {              
                    m_SelNodes.push_back(node_idx);
                }
            }   
        }
        return eTreeTraverse;
    }

    visitor_string_query& operator=(const visitor_string_query& rhs) 
        { m_SelNodes = rhs.m_SelNodes; return *this; }

    const vector<CPhyloTree::TTreeIdx>& GetNodes() { return m_SelNodes; }
private:
    vector<CPhyloTree::TTreeIdx> m_SelNodes;

    TBioTreeFeatureId m_FeatureId;
    const set<string>& m_FeatureValues;   
};

// handles incoming selection broadcast (overriding CView virtual function)
void CPhyTreeView::x_OnSetSelection(CSelectionEvent& evt)
{
    CPhyloTreeWidget* phyWidget = GetWidget();

    if (phyWidget == NULL)
        return;

    vector<TTreeIdx> sel_nodes;
    vector<CPhyloTree::TID> sel_node_ids;

    CPhyloTreeDataSource* ds = m_DataSource.GetPointer();

    TConstObjects objs;
    evt.GetIndexed("tree", objs);
    
    // The selection events only contain selection from a single object, and since
    // trees only generate one 'tree' indexed object for the selection, 'objs' should
    // only have one element. 

    // If we have a tree broadcast and select a feature other than seq-id or taxid, we will
    // ignore seq-id and taxid selection. If the feature type IS seq-id or taxid, we will process 
    // those ids further down under HasObjectSelection since it should be more efficient.
    bool select_seqids = true;
    bool select_taxids = true;
    const CBioTreeSelection* bioTreeSelection = NULL;

    if (objs.size() > 0) {
        select_seqids = false;
        select_taxids = false;
    
        bioTreeSelection = dynamic_cast<const CBioTreeSelection*>(objs[0].GetNonNullPointer());

        if (bioTreeSelection != NULL && bioTreeSelection->GetNodeIds().size()) {
            CConstRef<objects::CBioTreeContainer> bioTree = bioTreeSelection->GetBioTree();

            // broadcasting selections from a tree to itself is a special case.  CSelectionEvent::sm_TreeBroadcastOneToOne
            // returns the selection criterion that says that we want to match the trees own
            // nodes one-to-one if and only if the two trees are built from the same underlying
            // CBioTreeContainer.
            bool select_by_id = false;
            if (bioTree.GetNonNullPointer() == m_BioTreeContainer.GetNonNullPointer() &&
                CSelectionEvent::sm_TreeBroadcastOneToOne ) {
                select_by_id = true;
            }
   
            string feat_name;
            TBioTreeFeatureId feature_id = -1;

            if (!select_by_id && bioTree->IsSetFdict()) {
                ITERATE(vector<string>, it, CSelectionEvent::sm_TreeBroadcastProperties) {
                    if (ds->GetTree()->GetFeatureDict().HasFeature(*it) &&
                        bioTreeSelection->HasFeature(*it)) {
                        feat_name = *it;
                        feature_id = bioTreeSelection->GetFeatureId(feat_name);
                        break;
                    }
                }
            }

            set<string> selected_feature_values;

            // check for special case of selecting by id:
            if (select_by_id) {
                const CBioTreeSelection::TNodeIdsVector &nodeIds = bioTreeSelection->GetNodeIds();
                sel_node_ids.reserve(nodeIds.size());
                std::copy(nodeIds.begin(), nodeIds.end(), back_inserter(sel_node_ids));
            }
            else if (feature_id != -1) {
                if (feat_name == "seq-id") 
                    select_seqids = true;
                if (feat_name == "tax-id" || feat_name == "taxid")
                    select_taxids = true;

                if (!select_seqids && !select_taxids) {
                    // Iterate over the selection tree and collect all the selected values. Could also
                    // iterate over the selected nodes for each node in the tree..
                    // could use set or put all in vector, sort vector and remove non-unique.
                    std::vector<size_t>::iterator iter;
                    ITERATE(CBioTreeSelection::TNodeIdsVector, iter, bioTreeSelection->GetNodeIds()) {                
                        string feat_val = bioTreeSelection->GetFeatureValue(*iter,feature_id);
                        if (feat_val.empty())
                            continue;
                        selected_feature_values.insert(feat_val);
                    }

                    // Iterate over the tree to be updated (potentially) and select all those nodes
                    // that have 'feat_name' equal to one of the selected values
                    feature_id = ds->GetTree()->GetFeatureDict().GetId(feat_name);  // feature_id for the tree in this view
                    visitor_string_query vsq(feature_id, selected_feature_values);
                    vsq = TreeDepthFirst(*ds->GetTree(), vsq);
                    sel_nodes =  vsq.GetNodes();
                }
            }
        }
    }    

    if(evt.HasObjectSelection() && (select_seqids || select_taxids))    {
        // get Ids
        CSelectionEvent::TIds  ids = evt.GetIds();

        // get Ids from TIdLocs
        const CSelectionEvent::TIdLocs&  locs = evt.GetIdLocs();
        ITERATE(CSelectionEvent::TIdLocs, it_loc, locs)   {
            ids.push_back((*it_loc)->m_Id);
        }

        if (ids.size() != 0 && select_seqids) {

            CObjectIndex::TResults results;

            ITERATE(CSelectionEvent::TIds, it_id, ids) {
                const CSeq_id& id = **it_id;
                CSeq_id_Descr descr(NULL, id, evt.GetScope());    // TODO - put descr in SelectionEvent

                results.clear();
                m_ObjectIndex.GetMatches(descr, results);

                ITERATE(CObjectIndex::TResults, it_r, results)  {
                    const CSelNodeHandle* handle =
                        reinterpret_cast<const CSelNodeHandle*>(*it_r);
                    sel_nodes.push_back(handle->GetNodeIdx());
                }
            }
        }
        else if (select_taxids) {            
            if (phyWidget && m_DataSource && x_HasProject()) {
                CPhyloTreeDataSource* ds = m_DataSource.GetPointer();

                if (ds->GetDictionary().HasFeature("tax-id") || ds->GetDictionary().HasFeature("taxid")) {

                    const CSelectionEvent::TTaxIds& tids = evt.GetTaxIDs();\

                    TBioTreeFeatureId feature_id = ds->GetDictionary().GetId("tax-id");
                    // allow for common misspelling
                    if (feature_id ==  (TBioTreeFeatureId)-1)
                        feature_id = ds->GetDictionary().GetId("taxid");

                    if (feature_id != -1) {
                        visitor_taxid_query vtq(tids, feature_id);
                        vtq = TreeDepthFirst(*ds->GetTree(), vtq);
                        sel_nodes =  vtq.GetNodes();
                    }
                }
            }
        }
    }

    m_DataSource->GetTree()->ClearSelection();
    if (!sel_nodes.empty()) {
        m_DataSource->GetTree()->SetSelection(sel_nodes);
        // Since we don't know which nodes were explicitly selected in the source, we just
        // set the leaves as being the primary selection set since that is most often the case.
        // This only has an effect on using the iteration arrows (forward backward) to step through
        // the selection set.
        m_DataSource->GetTree()->SetLeavesExplicitlySelected();
    }

    if (!sel_node_ids.empty()) {
        m_DataSource->GetTree()->SetSelectionIDs(sel_node_ids);
        // For selection coming from same biotree, preserve knowledge of which selected
        // nodes were explicitly selected (e.g. by meeting query criteria) vs. which were
        // selected as the parents or children of those selected nodes.
        if (bioTreeSelection != NULL) {
            m_DataSource->GetTree()->SetExplicitlySelected(bioTreeSelection->GetExplicitlySelectedNodeIds());
        }
    }
    phyWidget->OnUpdateSelChanged();
}

void CPhyTreeView::GetVisibleRanges(CVisibleRange& vrange) const
{
}

void CPhyTreeView::SetRegistryPath(const string& reg_path)
{
    IRegSettings* reg_set = dynamic_cast<IRegSettings*>(GetWidget());
    if (reg_set)
        reg_set->SetRegistryPath(reg_path);
}

void CPhyTreeView::LoadSettings()
{
    IRegSettings* reg_set = dynamic_cast<IRegSettings*>(GetWidget());
    if (reg_set)
        reg_set->LoadSettings();
}


void CPhyTreeView::SaveSettings() const
{
    IRegSettings* reg_set = dynamic_cast<IRegSettings*>(GetWidget());
    if (reg_set)
        reg_set->SaveSettings();
}


const CObject* CPhyTreeView::x_GetOrigObject() const
{
    if(m_BioTreeContainer)    {
        return m_BioTreeContainer.GetPointer();
    }
    return NULL;
}

// This finds the closest node to the root (closest in number of edges,
// not phylogenetic distance) that is either selected or shared (a node that
// has multiple selected children)
class CPhyloTreeSelectionRoot
{
public:
    CPhyloTreeSelectionRoot()
        : m_SelectionRootIdx(-1)
        , m_CurrentRootDistance(0)
        , m_SelectionRootDistance(std::numeric_limits<int>::max()) {}

    TTreeIdx GetSelectionRootIdx() const { return m_SelectionRootIdx; }
 
    ETreeTraverseCode operator()(CPhyloTree&  tree, CPhyloTree::TTreeIdx node_idx, int delta)
    {
        CPhyloTreeNode& node = tree[node_idx];

        m_CurrentRootDistance += delta;

        if (delta == 0 || delta == 1) {
            if (((*node).GetSelectedState() == CPhyloNodeData::eSelected) ||
                ((*node).GetSelectedState() == CPhyloNodeData::eShared)) {

                if (m_CurrentRootDistance < m_SelectionRootDistance) {
                    m_SelectionRootDistance = m_CurrentRootDistance;
                    m_SelectionRootIdx = node_idx;
                }
            }
        }
        return eTreeTraverse;
    }

    TTreeIdx m_SelectionRootIdx;
    int m_CurrentRootDistance;
    int m_SelectionRootDistance;
};

void CPhyTreeView::OnCreateSelectedSubtree(CEvent* evt)
{
    // this can be slow (for very large trees) so let user know we are working
    CPhyloTreeWidget* phyWidget = GetWidget();
    phyWidget->SetCursor(*wxHOURGLASS_CURSOR);

    CRef<objects::CBioTreeContainer> btc(new objects::CBioTreeContainer());
    
    // We will create a subtree with all selected and shared nodes (shared on common
    // path back towards root).  Unless there are no nodes selected in which case
    // we will create a subtree from the current node (last node clicked on, including
    // via right-mouse)
    if (m_DataSource->GetTree()->GetNumSelected() > 0) {
        CPhyloTreeSelectionRoot sel_dist = TreeDepthFirst(*m_DataSource->GetTree(),
            m_DataSource->GetTree()->GetRootIdx(),
            CPhyloTreeSelectionRoot());

        CPhyloTree::TTreeIdx node_idx = sel_dist.GetSelectionRootIdx();

        if (node_idx == CPhyloTree::Null()) {
            CIRef<IStatusBarService> sb_srv = m_Workbench->GetServiceByType<IStatusBarService>();
            sb_srv->SetStatusMessage("No selected nodes found");
            return;
        }

        TreeConvertSelected2Container(*btc, *m_DataSource->GetTree(), node_idx);
    }
    else if (m_DataSource->GetTree()->GetCurrentNodeIdx() != CPhyloTree::Null()) {
        CPhyloTree::TTreeIdx idx = m_DataSource->GetTree()->GetCurrentNodeIdx();
        TreeConvert2Container(*btc, *m_DataSource->GetTree(), idx);
    }
    else {
        // We should not see this because menu items should not be enabled
        // without selected or current node
        CIRef<IStatusBarService> sb_srv = m_Workbench->GetServiceByType<IStatusBarService>();
        sb_srv->SetStatusMessage("No selected nodes found");
        return;
    }

    if (m_BioTreeContainer->IsSetUser()) {
        btc->SetUser().Assign(m_BioTreeContainer->GetUser());
    }

    CDataLoadingAppJob* job = new CPhyloTreeJob(btc.GetPointer());

    CSelectProjectOptions options;
    options.Set_AddToExistingProject(m_ProjectId);

    CIRef<CProjectService> prjService = m_Workbench->GetServiceByType<CProjectService>();
    CDataLoadingAppTask* task = new CDataLoadingAppTask(prjService, options, *job);
    m_Workbench->GetAppTaskService()->AddTask(*task);

    phyWidget->SetCursor(*wxSTANDARD_CURSOR);

    // Creating a subtree causes the project to be updated which in turn updates (rebuilds
    // trees from biotreecontainer) all views. We do not want that since it loses our current
    // query selection (and is inefficient)
    m_EditUpdate = true;
}


void CPhyTreeView::OnExportTree(CEvent* evt)
{
    _ASSERT(m_Workbench);
    CPhyloTreeWidget* phyWidget = GetWidget();
    _ASSERT(phyWidget);
    
    if (!phyWidget->HasScheme())
        return;

    CExportTreeDlg dlgExport(phyWidget);

    if (dlgExport.ShowModal() != wxID_OK)
        return;

    CRef<CBioTreeContainer> new_container(new CBioTreeContainer());
    TreeConvert2Container(*new_container, *m_DataSource->GetTree());

    CPhyExportParams& params = dlgExport.GetData();
    params.SetLabelFormat(phyWidget->GetScheme().SetLabelFormat().c_str());
    params.SetBioTree(new_container);

    CIRef<IAppJob> job(new CPhyExportJob(params));
    CRef<CAppJobTask> export_task(new CAppJobTask(*job, true, "Exporting tree..."));
    m_Workbench->GetAppTaskService()->AddTask(*export_task);
}

void CPhyTreeView::OnSomethingEdited(CEvent* evt)
{        
    CPhyloTreeWidget* phyWidget = GetWidget();
    ICommandProccessor* undoManager = x_GetUndoManager();

    if (evt->GetID() == eCmdRenderingOptionsChanged) {
        // There is no undo for this so we don't create a command object
        CBioTreeContainer* in_project = 
            const_cast<CBioTreeContainer *>(m_BioTreeContainer.GetPointer());

        CBioTreeContainer_Base::TUser& uo = in_project->SetUser();

        if (!uo.CanGetType() || !uo.IsSetType()  || uo.GetType().Which()==CObject_id_Base::e_not_set ) {   
            CRef<CObject_id> uo_id;
            uo_id.Reset(new CObject_id());
            uo_id->SetStr("Tree Metadata");
            uo.SetType(*uo_id);
        }

        Int4 current_layout = phyWidget->GetCurrRenderer();
        if (uo.HasField("layout") && 
            uo.GetField("layout").GetData().IsInt()) {
                uo.SetField("layout").SetData().SetInt(current_layout);    
        }
        else {
            uo.AddField("layout", current_layout);
        }    

        // Save distance rendering option too      
        bool use_distances = phyWidget->GetUseDistances();
        if (uo.HasField("use-distances") && 
            uo.GetField("use-distances").GetData().IsBool()) {
                uo.SetField("use-distances").SetData().SetBool(use_distances);    
        }
        else {
            uo.AddField("use-distances", use_distances);
        }  

        bool rotate_labels = phyWidget->GetRotateLabels();
        if (uo.HasField("rotate-labels") && 
            uo.GetField("rotate-labels").GetData().IsBool()) {
                uo.SetField("rotate-labels").SetData().SetBool(rotate_labels);    
        }
        else {
            uo.AddField("rotate-labels", rotate_labels);
        } 

        // hack to prevent event from x_FireProjectChangedEvent from 
        // Causing OnProjectChangedEvent to force the model to be recreated from the biotree.
        // that should only happen if another view causes the data update.
        m_EditUpdate = true;

        IEventAttachment* attach = evt->GetAttachment();
        CSelectionSetEdit* sel_edit = dynamic_cast<CSelectionSetEdit*>(attach);
        if (sel_edit != NULL) {

            CChangePhyloSelectionSetCmd* phycmd = new CChangePhyloSelectionSetCmd(this,
                sel_edit,
                m_DataSource,
                m_BioTreeContainer);

            if (undoManager) {
                CIRef<IEditCommand> command(phycmd);
                undoManager->Execute(command);
            }
        }
    }
    else if (evt->GetID() == eCmdLabelFormatChanged) {
        // There is no undo for this so we don't create a command object
        CBioTreeContainer* in_project = 
            const_cast<CBioTreeContainer *>(m_BioTreeContainer.GetPointer());

        CBioTreeContainer_Base::TUser& uo = in_project->SetUser();

        if (!uo.CanGetType() || !uo.IsSetType()  || uo.GetType().Which()==CObject_id_Base::e_not_set ) {   
            CRef<CObject_id> uo_id;
            uo_id.Reset(new CObject_id());
            uo_id->SetStr("Tree Metadata");
            uo.SetType(*uo_id);
        }

        if (phyWidget->HasScheme())
            *m_Scheme = phyWidget->GetScheme();
        string label_format = m_Scheme->SetLabelFormat();
        if (uo.HasField("label-format") && 
            uo.GetField("label-format").GetData().IsStr()) {
                uo.SetField("label-format").SetData().SetStr(label_format);    
        }
        else {
            uo.AddField("label-format", label_format);
        }   

        
    }
    else if (evt->GetID() == eCmdTreeLabelSet) {
        // There is no undo for this so we don't create a command object
        CBioTreeContainer* in_project = 
            const_cast<CBioTreeContainer *>(m_BioTreeContainer.GetPointer());

        CBioTreeContainer_Base::TUser& uo = in_project->SetUser();

        if (!uo.CanGetType() || !uo.IsSetType()  || uo.GetType().Which()==CObject_id_Base::e_not_set ) {   
            CRef<CObject_id> uo_id;
            uo_id.Reset(new CObject_id());
            uo_id->SetStr("Tree Metadata");
            uo.SetType(*uo_id);
        }

        phyWidget->GetDS()->GetTreeLabel().SaveToUserObject(uo);
    }
    else if (evt->GetID() == eCmdFeaturesEdited || 
             evt->GetID() == eCmdNodeExpandCollapse) {
        
        // hack to prevent event from x_FireProjectChangedEvent from 
        // Causing OnProjectChangedEvent to force the model to be recreated from the biotree.
        // that should only happen if another view causes the data update.
        m_EditUpdate = true;
        IEventAttachment* attach = evt->GetAttachment();
        CFeatureEdit* fedit = dynamic_cast<CFeatureEdit*>(attach);
        if (fedit != NULL) {
            CStopWatch timer;
            timer.Start();

            CChangePhyloPropertyCmd* phycmd = new CChangePhyloPropertyCmd(this,
                fedit,
                m_DataSource, 
                m_BioTreeContainer);

            if (undoManager) {
                m_CreateProperyUndoCommand = timer.Restart();

                CIRef<IEditCommand> command(phycmd);
                undoManager->Execute(command); 

                m_ExecutePropertyCommand = timer.Elapsed();
            }
        }
    }
    else if (evt->GetID() == eCmdGrpExpandCollapse) {
        // hack to prevent event from x_FireProjectChangedEvent from 
        // Causing OnProjectChangedEvent to force the model to be recreated from the biotree.
        // that should only happen if another view causes the data update.
        m_EditUpdate = true;
        IEventAttachment* attach = evt->GetAttachment();
        CExpandCollapseNodes* expand_collapse = dynamic_cast<CExpandCollapseNodes*>(attach);
        if (expand_collapse != NULL) {
            CStopWatch timer;
            timer.Start();

            CChangePhyloExpandCollapseCmd* phycmd = new CChangePhyloExpandCollapseCmd(this,
                expand_collapse,
                m_DataSource,
                m_BioTreeContainer);

            if (undoManager) {
                m_CreateProperyUndoCommand = timer.Restart();

                CIRef<IEditCommand> command(phycmd);
                undoManager->Execute(command);

                m_ExecutePropertyCommand = timer.Elapsed();
            }
        }
    }
    else {
        CRef<CBioTreeContainer> new_container(new CBioTreeContainer());
        TreeConvert2Container(*new_container, *m_DataSource->GetTree());
        
        // Copy any user data (selection sets, rendering options etc.) into tree
        if (m_BioTreeContainer->IsSetUser()) {
            new_container->SetUser().Assign(m_BioTreeContainer->GetUser());
        }

        /* Previously we did not have undo for sort, but (by commenting this  out) we 
           have added it back in
		if (eCmdTreeSorted == evt->GetID()) {
			CNcbiStrstream  str;
			str << MSerial_AsnText << *new_container;
			CommitTheChanges((CNcbiIstrstream&)str);
			OnBioTreeChanged();
		}
		else
        */
        {
			CStopWatch timer;
			timer.Start();

			CChangePhyloTreeCommand* phyCommand = new CChangePhyloTreeCommand(
				this,
				m_BioTreeContainer,
				new_container);
			
			m_CreateUndoCommand = timer.Restart();

			if (undoManager) {
				// We do not execute the command here because the 
				CIRef<IEditCommand> command(phyCommand);
				m_EditUpdate = true;
	            undoManager->Execute(command); 

				m_ExecuteUndoCommand = timer.Elapsed();
			}
		}
    }
 }

void CPhyTreeView::CommitTheChanges(CNcbiIstrstream& istr)
{
    // no alternative for this; special command to update project item may be better
    // then dropping const
    CBioTreeContainer* in_project = 
        const_cast<CBioTreeContainer *>(m_BioTreeContainer.GetPointer());

    CBioTreeContainer_Base::TTreetype tt;
    bool has_tt = in_project->IsSetTreetype();
   
    if (has_tt)
        tt = in_project->GetTreetype();
   
    in_project->Reset();
    istr >> MSerial_AsnText >> *in_project;

    if (has_tt)
        in_project->SetTreetype(tt);
}

void CPhyTreeView::x_QueryStatusChange(CEvent* evt)
{
    CQueryPanelEvent* queryEvt = dynamic_cast<CQueryPanelEvent*>(evt);
    if (!queryEvt) return;

    CIRef<IStatusBarService> sb_srv = m_Workbench->GetServiceByType<IStatusBarService>();
    sb_srv->SetStatusMessage(queryEvt->GetStatus());
}

///////////////////////////////////////////////////////////////////////////////
/// CPhyTreeViewFactory
string  CPhyTreeViewFactory::GetExtensionIdentifier() const
{
    static string sid("phy_tree_view_factory");
    return sid;
}


string CPhyTreeViewFactory::GetExtensionLabel() const
{
    static string slabel("Phylogenetic Tree View Factory");
    return slabel;
}


void CPhyTreeViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("phylo_tree_view.png"));
}

void CPhyTreeViewFactory::RegisterCommands (CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
    CPhyloTreeWidget::RegisterCommands(cmd_reg, provider);
}

const CProjectViewTypeDescriptor& CPhyTreeViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_PhyTreeViewTypeDescr;
}


IView* CPhyTreeViewFactory::CreateInstance() const
{
    return new CPhyTreeView();
}


IView* CPhyTreeViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    /*TFingerprint print(CPhyTreeView::m_TypeDescr.GetLabel(), false);
    if(print == fingerprint)   {
        return new CPhyTreeView();
    }*/
    return NULL;
}



int CPhyTreeViewFactory::TestInputObjects(TConstScopedObjects& objects)
{
    bool found_good = false;
    bool found_bad = false;
    for(  size_t i = 0;  i < objects.size();  i++)  {
        const CObject* obj = objects[i].object;
        const type_info& type = typeid(*obj);
        if(typeid(CBioTreeContainer) == type)  {
            found_good = true;
        } else {
            found_bad = true;
        }
    }
    if(found_good)   {
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);
    }
    return 0; // can show nothing
}


CPhyloTreeJob::CPhyloTreeJob(const objects::CBioTreeContainer* btc)
{
    CFastMutexGuard lock(m_Mutex);
	m_Btc.Reset(const_cast<objects::CBioTreeContainer*>(btc)); 
	m_Descr = "Phylogenetic Tree Cutter Tool";
}


void CPhyloTreeJob::x_CreateProjectItems()
{   
	CRef<objects::CProjectItem> item(new objects::CProjectItem());

	string label = "";
    CLabel::GetLabel(*m_Btc, &label, CLabel::eDefault, m_Scope.GetPointer());
    
	item->SetItem().SetOther().Set(*m_Btc);
    item->SetLabel(label);
    AddProjectItem(*item);
}


END_NCBI_SCOPE
