#ifndef GUI_WIDGETS_PHY_TREE___PHYLO_TREE_DS__HPP
#define GUI_WIDGETS_PHY_TREE___PHYLO_TREE_DS__HPP

/*  $Id: phylo_tree_ds.hpp 40268 2018-01-18 21:25:47Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <corelib/ncbi_tree.hpp>
#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_reader.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_algorithm.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <gui/widgets/phylo_tree/tree_graphics_model.hpp>
#include <gui/objutils/query_func_promote.hpp>
#include <gui/objutils/query_data_source.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_query_exec.hpp>
#include <gui/widgets/phylo_tree/tree_graphics_model.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/glcolortable.hpp>
#include <objects/biotree/BioTreeContainer.hpp>
#include <algo/phy_tree/bio_tree.hpp>
#include <algo/phy_tree/bio_tree_conv.hpp>
#include <gui/objutils/biotree_attr_reader.hpp>
#include <objmgr/scope.hpp>


BEGIN_NCBI_SCOPE

class CPhyloSelectionSetMgr;

struct CUpdatedFeature {
    void SetNode(CPhyloNodeData::TID id, CPhyloTree::TTreeIdx tidx) {
        m_NodeId = id;
        m_NodeIdx = tidx;
    }

    // So we can sort them when there are in CFeatureEdit array
    bool operator<(const CUpdatedFeature& rhs) const { return m_NodeId < rhs.m_NodeId; }

    CBioTreeFeatureList& GetFeatures() { return m_Features; }
    CBioTreeFeatureList& GetPrevFeatures() { return m_PrevFeatures; }

    bool FeatureUpdated(TBioTreeFeatureId fid) {
        return (m_Features.GetFeatureValue(fid) != m_PrevFeatures.GetFeatureValue(fid));           
    }

    /// Updated feature list (includes all features, not just those that changed)
    CBioTreeFeatureList  m_Features;
    /// Feature list prior to edit (for undo)
    CBioTreeFeatureList  m_PrevFeatures;

    /// Node id to find node in either tree or biotree
    CPhyloNodeData::TID m_NodeId;
    /// Node index or pointer (set to NULL after first execute)
    CPhyloTree::TTreeIdx m_NodeIdx;
};

/// Edits for one or more nodes
struct CFeatureEdit : public CObject, public IEventAttachment
{
    vector<CUpdatedFeature>& GetUpdated() { return m_Updated; }
    CBioTreeFeatureDictionary& GetDictionary() { return  m_Dictionary; }
    CBioTreeFeatureDictionary& GetPrevDictionary() { return  m_PrevDictionary; }

    bool SeqIdEdited() {
        TBioTreeFeatureId id = m_Dictionary.GetId("seq-id");
        if (id != TBioTreeFeatureId(-1)) {
            for (size_t i =0; i<m_Updated.size(); ++i) {
                if (m_Updated[i].FeatureUpdated(id))
                    return true;
            }
        }
        return false;
    }

    CBioTreeFeatureDictionary  m_Dictionary;  

    /// Store value of dictionary before edit for undo
    CBioTreeFeatureDictionary  m_PrevDictionary;  

    /// One entry for each updated node
    vector<CUpdatedFeature> m_Updated;
};

// Edits for expanding or collapsing a group of nodes
struct CExpandCollapseNodes : public CObject, public IEventAttachment
{
    void AddNode(CPhyloNodeData::TID id) {
        m_NodeIds.push_back(id);
    }

    void SetExpanded(CPhyloNodeData::TDisplayChildren ec) { m_Expanded = ec; }
    CPhyloNodeData::TDisplayChildren GetExpanded() const { return m_Expanded; }

    vector<CPhyloNodeData::TID>& GetIds() { return m_NodeIds; }

    /// Indicates if all nodes are to be expanded or collapsed
    CPhyloNodeData::TDisplayChildren m_Expanded;

    /// Node ids to find node in either tree or biotree
    vector<CPhyloNodeData::TID> m_NodeIds;
};

/// Edits for one or more nodes
struct CSelectionSetEdit : public CObject, public IEventAttachment
{
    CPhyloSelectionSetMgr&  GetUpdatedSet() { return m_UpdatedSet; }
    CPhyloSelectionSetMgr& GetPrevSet() { return m_PrevSet; }

    bool Updated() {
        m_UpdatedSet.RemoveDuplicateIDs();
        m_PrevSet.RemoveDuplicateIDs();
        return !(m_UpdatedSet == m_PrevSet);
    }

    /// Updated set
    CPhyloSelectionSetMgr  m_UpdatedSet;
    /// Previous values (for undo)
    CPhyloSelectionSetMgr  m_PrevSet;
};

/// Function to exclude nodes during a collapse those nodes that meet
/// a priority threshold.
struct SPriorityCollapse : public SCollapsable {
    SPriorityCollapse(CPhyloTree& m_TreeModel)
    {
        // If feature is not in tree, -1 is returned
        m_PriorityId = m_TreeModel.GetFeatureDict().GetId("$PRIORITY");

    }

    virtual bool operator() (const CPhyloTreeNode& n)
    {
        if (m_PriorityId == TBioTreeFeatureId(-1))
            return true;

        string feature_val = n->GetBioTreeFeatureList().GetFeatureValue(m_PriorityId);
        int val = NStr::StringToInt(feature_val, NStr::fConvErr_NoThrow);
        if (val >= 1000)
            return false;
        else
            return true;
    }

protected:
    TBioTreeFeatureId m_PriorityId;
};

class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CTreeLabel
{
public:
    CTreeLabel() 
        : m_FontName(CGlTextureFont::FaceToString(CGlTextureFont::eFontFace_Helvetica))
        , m_FontSize(18)
        , m_Color(0.0f, 0.0f, 0.0f, 1.0f)
        , m_XPos(50)
        , m_YPos(95) {}
    CTreeLabel(string label, string font, int font_size, 
               const CRgbaColor& c, int xpos, int ypos)
        : m_Label(label)
        , m_FontName(font)
        , m_FontSize(font_size)
        , m_Color(c)
        , m_XPos(xpos)
        , m_YPos(ypos) {}

    void SetLabel(const string& l) { m_Label = l; }
    string GetLabel() const { return m_Label; }

    void SetFontName(const string& f) { m_FontName = f; }
    string GetFontName() const { return m_FontName; }

    void SetFontSize(int s) { m_FontSize = s; }
    int GetFontSize() const { return m_FontSize; }

    void SetXPos(int x) { m_XPos = x; }
    int GetXPos() const { return m_XPos; }

    void SetYPos(int y) { m_YPos = y; }
    int GetYPos() const { return m_YPos; }

    void SetColor(const CRgbaColor& c) { m_Color = c; }
    CRgbaColor GetColor() const { return m_Color; }

    void InitializeFromUserObject(const CBioTreeContainer_Base::TUser& uo);
    void SaveToUserObject(CBioTreeContainer_Base::TUser& uo);

protected:
    string m_Label;
    string m_FontName;   
    int m_FontSize;
    CRgbaColor m_Color;

    int m_XPos;
    int m_YPos;
};

class  NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloTreeDataSource 
    : public CObject
    , public IQueryDataSource
{
public:
    typedef CAppJobDispatcher::TJobID   TJobID;
    typedef CPhyloTree TTreeType;
    typedef CPhyloTree::TTreeIdx TTreeIdx;
    typedef CPhyloTree::TNodeType TNodeType;
    typedef CPhyloTree::TClusterID TClusterID;
    typedef std::map<int, size_t> TClusterToColorMap;
    
public:
    /// Throws exception if tree is empty - at least 1 node is required.
    CPhyloTreeDataSource(const objects::CBioTreeContainer& tree, 
                         objects::CScope& scope, 
                         bool expand_all=false);

    ~CPhyloTreeDataSource();

    /// Throws exception if tree is empty - at least 1 node is required.
    void Init(const objects::CBioTreeContainer& tree, 
              objects::CScope& scope, 
              bool expand_all=false);
    void Clear();

    void SetColorIndices(CPhyloTreeScheme* scheme);

    string GenerateTooltipFormat();

    /// Create sorted array of distances of nodes from the parent 
    /// (the m_LenDistribution array)
    void ComputeLengthsFromRoot();
    /// Look in vector of sorted distances of nodes from the root and given a
    /// pct (0..1), return the distance closest to that value.
    float GetClosestLen(float pct) const;
    /// Return true if data is available (for trees without distances, may not be)
    bool HasLenDistribution() const { return m_LenDistribution.size() > 0; }

    void MeasureTree(TTreeIdx node);
    void MeasureTree();    

    void ApplyAttributes(CBioTreeAttrReader::TAttrTable & attrs,
                         CPhyloTreeScheme* scheme,
                         const string& labelfmt="");

    // clusters check
    bool HasClusters();

    // clusterizing with colors
    void Clusterize(CPhyloTreeScheme* scheme);

    /// Set the root node of the tree to the node at root_idx
    void ReRoot(TTreeIdx root_idx);

    /// Re-root on the edge between the selected node and its parent
    void ReRootEdge(TTreeIdx edge_child_node);

    /// Re-root tree using midpoint-method
    void ReRootMidpoint();

    /// Collapse, based on distance, enough nodes in the tree to get the total
    /// number of leaves down to the requested number. (or just over if exact 
    /// match is not possible)
    set<CPhyloNodeData::TID> CollapseByDistance(int leaf_count_target, SCollapsable* collapse_func);

    void Relabel(CPhyloTreeScheme* scheme, string labelFmt);

    void Sort(bool ascending);
    void SortDist(bool ascending);
    void SortLabel(bool ascending);
    void SortLabelRange(bool ascending);

    TModelRect GetBoundRect();

    void Clean();
    void Filter();
    void FilterDistances(double x_dist);

    CPhyloTree* GetTree() { return m_TreeModel.GetPointerOrNull(); }
    const CPhyloTree* GetTree() const { return m_TreeModel.GetPointerOrNull(); }

    /// Label for tree (optional element of biotreecontainer)
    void SetTreeLabel(const CTreeLabel& l) { m_TreeLabel = l; }
    CTreeLabel GetTreeLabel() const { return m_TreeLabel; }

    /// Get model for rendering
    CTreeGraphicsModel& GetModel() { return m_TreeModel->GetGraphicsModel(); }


    const CBioTreeFeatureDictionary& GetDictionary() const    { return m_TreeModel->GetFeatureDict(); }

    unsigned int GetNumNodes(void) { return m_Calc ? m_Calc->GetNumNodes() : 0;    }
    unsigned int GetVisibleNodes(void) { return m_Calc ? m_Calc->GetVisibleNodes() : 0;}
    unsigned int GetNumEdges(void) { return m_Calc ? m_Calc->GetNumEdges() : 0;    }
    // width is max. depth of tree and height is beadth (# of leaves)
    int GetWidth(void)             { return m_Calc ? m_Calc->GetWidth() : 0;       }
    int GetSize(void)              { return m_Calc ? m_Calc->GetHeight() : 0;      }
    int GetMaxLabelLen(void)       { return m_Calc ? m_Calc->GetMaxLabelLen() : 0; }
    double GetNormDistance(void)   { return m_Calc ? m_Calc->GetMaxDistance() : 0.0; }
    double GetMinDistance(void)    { return m_Calc ? m_Calc->GetMinDistance() : 0.0; }
    TClusterID GetMaxClusterID()   { return m_Calc ? m_Calc->GetMaxClusterID() : 0; }



    /// @name IQueryDataSource interface implementation
    /// @{
    // need indexd access to map.  Just iterate?
    virtual string GetColumnLabel(size_t col) const;
    virtual size_t GetColsCount() const;
    virtual CMacroQueryExec* GetQueryExec(bool casesensitive, CStringMatching::EStringMatching matching);
    virtual void SetQueryResults(CMacroQueryExec* q);
    virtual void ClearQueryResults();
    virtual CRef<objects::CScope> GetScope() { return m_Scope; }

    virtual void ExecuteStringQuery(const string &query, 
                                    size_t& num_selected, 
                                    size_t& num_queried, 
                                    CStringMatching::EStringMatching string_matching = CStringMatching::ePlainSearch, 
                                    NStr::ECase use_case = NStr::eCase);
    /// @}

    /// search for nodes via the specified text matching algorithm
    vector<TTreeIdx> FindNodes(const string &query, size_t& num_queried, CStringMatching::EStringMatching string_matching, NStr::ECase use_case) const;
    /// Get IDs selected by most recent call to FindNodes
    const vector<CPhyloTree::TID>& GetStringQueryIDs() const { return m_StringQueryIDs; }

    // Iterate or toggle display mode of the selected set
    TTreeIdx IterateOverSelNodes( int direction, bool highlight);
    TTreeIdx GetCurrentSearchNode() const { return m_SearchCurrentNode; }
    void SetCurrentSearchNode(TTreeIdx node_idx) { m_SearchCurrentNode = node_idx; }

    CPhyloSelectionSetMgr& GetSelectionSets() { return m_TreeModel->GetSelectionSets(); }
    // Set selection set propery in nodes from m_SelectionSets and re-clusterize the tree
    void UpdateSelectionSets(CPhyloTreeScheme* scheme);
    
    /// Get map that translates cluster-ids to color indices
    const TClusterToColorMap&  GetClusterToColorMap() const { return m_ClusterToColorMap;}

    // cut/copy/paste operations on selected node(s)
    void Cut();
    void Paste();
    TTreeIdx NewNode(bool after = true);
    void Remove(bool subtree = true);
    void RemoveSelected();
    void MoveNode(bool up);
    bool ClipboardEmpty() const { return m_sTreeClipboard.GetRootIdx() == TNodeType::Null(); }

    /// Collapse single node, do any relabeling if needed
    void SetCollapsedLabel(CPhyloTree::TTreeIdx idx);

    /// Collapse all nodes in node_ids, doing relabeling if needed
    void SetCollapsedLabels(const vector<CPhyloNodeData::TID>& node_ids);

    // Collapse all selected nodes (collapse if parent is not also selected)
    void CollapseSelected();

protected:
    CRef<CPhyloTree> m_TreeModel;

    // A single (optional) label placed somewhere in the tree view
    CTreeLabel m_TreeLabel;

    // visitor/calculator
    CPhyloTreeCalculator* m_Calc;

    CRef<objects::CScope> m_Scope;

    TTreeIdx         m_SearchCurrentNode;

    TClusterToColorMap  m_ClusterToColorMap;

    // Length distribution is an array of all the distances of child nodes 
    // from the root and is used (sometimes) to enhance longer edges when 
    // we render very large trees but are zoomed out so that edges run togeather
    std::vector<float> m_LenDistribution;

    /// Temporary storage for string query results to get ID's back to caller
    vector<CPhyloTree::TID> m_StringQueryIDs;


    /// static (to allow copying between trees)    
    static CPhyloTree  m_sTreeClipboard;

private:
    class visitor_read_properties
    {
    public:
        visitor_read_properties(CRgbaGradColorTable* t) : m_ColorTable(t) {}
        ETreeTraverseCode operator()(TTreeType&  tree, TTreeType::TTreeIdx node, int delta)
        {
             if (delta==1 || delta==0){
                tree[node].GetValue().Init(tree.GetFeatureDict(), m_ColorTable);
             }
             return eTreeTraverse;
        }
         
        CRgbaGradColorTable* m_ColorTable;
    };
};

END_NCBI_SCOPE

#endif //GUI_WIDGETS_PHY_TREE___PHYLO_TREE_DS__HPP

