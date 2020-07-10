#ifndef GUI_WIDGETS_PHYLO_TREE__PHYLO_TREE_ALGORITHM_HPP
#define GUI_WIDGETS_PHYLO_TREE__PHYLO_TREE_ALGORITHM_HPP

/*  $Id: phylo_tree_algorithm.hpp 43693 2019-08-14 18:24:50Z katargir $
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
 *
 */


#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <corelib/ncbi_tree.hpp>
#include <gui/opengl/glpane.hpp>
#include <algo/phy_tree/bio_tree.hpp>
#include <algo/phy_tree/bio_tree_conv.hpp>
#include <objmgr/scope.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_label.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <util/ncbi_table.hpp>


BEGIN_NCBI_SCOPE

class IPhyloTreeVisitor
{
public:
    typedef CPhyloTree  TTreeType;
    typedef CPhyloTree::TTreeIdx TTreeIdx;
    typedef CPhyloTree::TNodeType TNodeType;

public:
    IPhyloTreeVisitor(TTreeType*  tree) : m_Tree(tree) {}
    virtual ~IPhyloTreeVisitor(){}

    ETreeTraverseCode operator()(TTreeType&  tree, TTreeIdx tree_node, int delta);
protected:
    virtual ETreeTraverseCode x_OnStepRight(TTreeIdx  x_node);
    virtual ETreeTraverseCode x_OnStepLeft(TTreeIdx  x_node);
    virtual ETreeTraverseCode x_OnStepDown(TTreeIdx  x_node);
    virtual ETreeTraverseCode x_OnStep(TTreeIdx  x_node, int delta);

    TTreeType* m_Tree;
};




class CPhyloTreeCalculator : public IPhyloTreeVisitor
{
public:
     typedef CPhyloTree::TClusterID        TClusterID;
     typedef map< TClusterID, vector<TTreeIdx> >  TClusterHash;
     typedef CNcbiTable<string, unsigned, string> TAttrTable;

public:
    CPhyloTreeCalculator(TTreeType*  tree, CRgbaGradColorTable* color_table);
    virtual ~CPhyloTreeCalculator(){}

    
    unsigned GetNumNodes()  { return m_NodeCount; }
    unsigned GetVisibleNodes() { return m_VisibleNodes; }
    // as a tree number of edges has to be m_NodeCount -1
    unsigned GetNumEdges()  { return m_NodeCount-1; }
    int GetWidth()          { return m_Width; }
    int GetHeight()         { return m_Height;}
    double GetMaxDistance() { return m_MaxDistance; }
    double GetMinDistance() { return m_MinDistance; }
    int    GetMaxLabelLen() { return m_MaxLabelLen; }
    void   DumpStats();
    const  TClusterHash & GetClusters()           { return m_Clusters;  }    
    void   SetClusters(const TClusterHash & clst) { m_Clusters  = clst; }
    TClusterID GetMaxClusterID() const            { return m_MaxClusterID; }
    void   SetAttrTable(const TAttrTable  & attr);
    void   ClearAttrTable();
    void   SetLabelFormat(string labelFmt)        { m_LabelFormat = labelFmt; }
    void   Init(CRgbaGradColorTable* ct);



protected:
    virtual ETreeTraverseCode x_OnStepRight(TTreeIdx  x_node);
    virtual ETreeTraverseCode x_OnStepLeft(TTreeIdx  x_node);
    virtual ETreeTraverseCode x_OnStepDown(TTreeIdx  x_node);
    virtual ETreeTraverseCode x_OnStep(TTreeIdx  x_node, int delta);

    /// Updates node properties using values from m_AttrTable
    void x_UpdateProperties(TNodeType& node, int row_idx);

private:
    unsigned        m_PamlID;
    unsigned        m_ID;
    unsigned        m_NodeCount;
    unsigned        m_VisibleNodes;
    int             m_PosX;
    int             m_PosY;
    int             m_Leaves;
    int             m_Childs;
    int             m_Width;
    int             m_Height;
    int             m_MaxLabelLen;
    double          m_MaxDistance;
    double          m_MinDistance;

    TClusterHash        m_Clusters;
    TClusterID          m_MaxClusterID;

    float          m_DistFromRoot;
    float          m_LastDist;
    
    // The top-level collapsed node (if any) to keep track of hidden nodes
    CPhyloTreeNode::TTreeIdx m_CollapsedParentIdx;

    /// Attributes with seq-ids optionally provided to update tree properties
    const TAttrTable  * m_AttrTable;
    string m_AttrKeyName;
    TBioTreeFeatureId  m_AttrKeyId;
    
    /// Structure allows us to more efficiently store and look up keys (usually seq-ids)
    /// in m_AttrTable. 
    struct AttrKey {
        AttrKey() : m_AttrTableIdx(0) {}
        AttrKey(const string& id, unsigned int idx) 
            : m_IDStr(id)
            , m_AttrTableIdx(idx) {}

        /// Sort keys on seq-id string value
        bool operator<(const AttrKey& rhs) const { return (m_IDStr < 
                                                           rhs.m_IDStr); }
        string m_IDStr;
        unsigned int m_AttrTableIdx;
    };


    /// Mapping for efficient lookup of seq-ids in m_AttrTable. 
    vector<AttrKey> m_AttrKeys;

    /// label calculation
    string              m_LabelFormat;
    CPhyloTreeLabel     m_PTL;
    CRgbaGradColorTable* m_ColorTable;
};

class CPhyloTreeRectCalculator : public IPhyloTreeVisitor
{
public:
    CPhyloTreeRectCalculator(TTreeType*  tree)
    : IPhyloTreeVisitor(tree)
    { Init(); }

    virtual ~CPhyloTreeRectCalculator() {}
    const TModelRect & GetRect()    const {return m_Rect; }
    void               Init();
protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);
private:
    TModelRect      m_Rect;
};

class CPhyloTreePriorityNode : public IPhyloTreeVisitor
{
public:
    CPhyloTreePriorityNode(TTreeType*  tree, TTreeIdx node_idx)
        : IPhyloTreeVisitor(tree)
        , m_PriorityLeafIdx(TTreeType::Null())     
        , m_LeafMidpoint(0.0f)
        , m_LeafCount(0)
        , m_PriorityId(-1)
        , m_MaxPriority(-1)
        , m_MaxPriorityLeafNum(-1)
    {
        Init(node_idx);
    }

    virtual  ~CPhyloTreePriorityNode() {}
    void Init(TTreeIdx node_idx);

    TTreeIdx GetMaxPriorityNode() const { return m_PriorityLeafIdx; }

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);

private:
    /// Leaf with highest priority number
    TTreeIdx m_PriorityLeafIdx; 
    /// Index of leaf at center of subtree (leaves in subtree/2)
    float    m_LeafMidpoint;
    /// Current leaf count (as we iterate over the subtree)
    size_t m_LeafCount;
    
    /// Id of priority feature in tree
    TBioTreeFeatureId m_PriorityId;

    /// Max priority value found in priority field in subtree
    int m_MaxPriority;
    /// Leaf index of node with m_MaxPriority. Used to resolve conflicts if 
    /// more than one node in subtree has same max priority value.
    int m_MaxPriorityLeafNum;
};

class CPhyloTreeMaxIdCalculator : public IPhyloTreeVisitor
{
public:
    CPhyloTreeMaxIdCalculator(TTreeType* tree)
    : IPhyloTreeVisitor(tree)
    , m_Id(0) {}

    virtual ~CPhyloTreeMaxIdCalculator() {}

    CPhyloNodeData::TID GetMaxId() const {return m_Id; }

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);
    CPhyloNodeData::TID m_Id;
};

class CPhyloTreeSorter : public IPhyloTreeVisitor
{
public:
    CPhyloTreeSorter(TTreeType*  tree, bool x_order) 
        : IPhyloTreeVisitor(tree)
        , m_Order(x_order) {}

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);

private:
    bool m_Order;
};

struct SChildMaxDist {
    SChildMaxDist() : m_NodeIdx(0), m_MaxChildDist(0.0f) {}
    SChildMaxDist(TTreeIdx idx, float dist)
        : m_NodeIdx(idx)
        , m_MaxChildDist(dist) {}

    /// Sort keys on seq-id string value
    bool operator<(const SChildMaxDist& rhs) const {
        return (m_MaxChildDist < rhs.m_MaxChildDist);
    }
    TTreeIdx m_NodeIdx;
    float m_MaxChildDist;
};


// Pass in a function derived from SCollapsable that returns
// false for any nodes that should not be collapsed.
struct SCollapsable : public CObject {
    virtual bool operator() (const CPhyloTreeNode& n) { return true; }
};

/// For each node this finds the maximum distance of any of its immediate
/// (not recursively searched) children from the root.
class CPhyloTreeMaxDirectChildDist : public IPhyloTreeVisitor
{
public:
    CPhyloTreeMaxDirectChildDist(TTreeType*  tree, SCollapsable* collapsable_func = new SCollapsable())
        : IPhyloTreeVisitor(tree)
        , m_Distances(tree->GetSize(), SChildMaxDist(0, 0.0f))
        , m_LastDist(0.0f)
        , m_DistFromRoot(0.0f)
        , m_CheckCollapseFunc(collapsable_func)
    {
        if (m_CheckCollapseFunc.IsNull()) {
            m_CheckCollapseFunc.Reset(new SCollapsable());
        }
    }

    vector<SChildMaxDist>&  GetDistances() { return m_Distances; }

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);

private:
    /// Max distace of any direct child of a node from the root, saved in same order
    /// as node array in tree
    vector<SChildMaxDist> m_Distances;
    float m_LastDist;
    float m_DistFromRoot;
    CRef<SCollapsable> m_CheckCollapseFunc;
};

/// For each node, this returns the maximum distances of any of its children
/// (recursively searched) from the root node.
class CPhyloTreeMaxChildDist : public IPhyloTreeVisitor
{
public:
    CPhyloTreeMaxChildDist(TTreeType*  tree) 
        : IPhyloTreeVisitor(tree)
        , m_Distances(tree->GetSize(), 0.0f)
        , m_LastDist(0.0f)
        , m_DistFromRoot(0.0f)
        , m_MaxDist(-std::numeric_limits<float>::max())
        , m_MinDist(std::numeric_limits<float>::max())
    {
    }

    vector<float>&  GetDistances() { return m_Distances; }
    float GetMaxDist() const { return m_MaxDist; }
    float GetMinDist() const { return m_MinDist; }

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);

private:
    /// Max distace of any child of a node from the root, saved in same order
    /// as node array in tree
    vector<float> m_Distances;
    float m_LastDist;
    float m_DistFromRoot;

    /// Keep track of overall max and min distances
    float m_MaxDist;
    float m_MinDist;
};

// Build a vector of a the distance of each node from the root node.
// This is needed by the CPhyloTreeMidpointDist algorithm .
class CPhyloTreeDistFromRoot : public IPhyloTreeVisitor
{
public:
    CPhyloTreeDistFromRoot(TTreeType* tree)
        : IPhyloTreeVisitor(tree)
        , m_Distances(tree->GetSize(), 0.0f)
        , m_LastDist(0.0f)
        , m_DistFromRoot(0.0f)
        , m_MaxBranchDist(0.0f)
        , m_MaxDistNode(CPhyloTree::Null()) {}

    vector<float>&  GetDistances() { return m_Distances; }
    TTreeIdx GetMaxDistNode() const { return m_MaxDistNode;  }

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);

private:
    /// Distance of each node from root
    vector<float> m_Distances;

    float m_LastDist;
    float m_DistFromRoot;
    float m_MaxBranchDist;
    
    /// Index of node that is furthest from the root.
    TTreeIdx m_MaxDistNode;
};

// Find the pair of nodes in the tree with the longest path between
// them, and return that path and it's total distance.
class CPhyloTreeMidpointDist : public IPhyloTreeVisitor
{
public:
    CPhyloTreeMidpointDist(TTreeType* tree,
        vector<float>& distances,
        TTreeIdx max_dist_node_idx)
        : IPhyloTreeVisitor(tree)
        , m_Distances(distances)
        , m_MaxDistNode(max_dist_node_idx)
        , m_MaxDist(0.0f) 
    {
        m_MaxDistFromRoot = m_Distances[m_MaxDistNode];

        // Get all nodes from m_MaxDistNode to root:
        TTreeIdx idx = m_MaxDistNode;
        while (idx != CPhyloTree::Null()) {
            m_MaxDistPathToRoot.insert(m_MaxDistPathToRoot.begin(), idx);
            idx = (*m_Tree)[idx].GetParent();
        }

        m_SortedMaxDistPathToRoot = m_MaxDistPathToRoot;
        std::sort(m_SortedMaxDistPathToRoot.begin(), 
                  m_SortedMaxDistPathToRoot.end());
    }

    void GetLongest(vector<TTreeIdx>& path, float& length);

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);

private:
    /// Distance of each node from root node
    vector<float> m_Distances;

    /// Node at greatest distance from root, its distance and vector
    /// of nodes from root to m_MaxDistNode (sorted and unsorted)
    TTreeIdx m_MaxDistNode;
    float m_MaxDistFromRoot;
    vector<TTreeIdx> m_MaxDistPathToRoot;
    vector<TTreeIdx> m_SortedMaxDistPathToRoot;

    /// Path (set of nodes) that is the longest path in the tree
    vector<TTreeIdx> m_MaxPath;
    /// Total distance along m_MaxPath
    float m_MaxDist;       
};

// For each node int the tree find from its subnodes the lexiocographical
// min and max label values, e.g.  if lables of the children of 'q' are 
// 'c', 'f' and 't', the min is 'c' and the max is 't'.
class CPhyloTreeLabelRange : public IPhyloTreeVisitor
{
public:
    CPhyloTreeLabelRange(TTreeType*  tree, bool x_order) 
        : IPhyloTreeVisitor(tree)
        , m_LabelRanges(tree->GetSize()) {}

    vector<pair<string,string> >&  GetLabelRanges() { return m_LabelRanges; }

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);

private:
    /// Min and max label values (according to lexicographic 
    vector<pair<string,string> > m_LabelRanges;
    string m_MaxBranchLabel;
    string m_MinBranchLabel;
};

class CPhyloTreeSorterSubtreeDist : public IPhyloTreeVisitor
{
public:
    CPhyloTreeSorterSubtreeDist(TTreeType*  tree, 
                                const vector<float>& distances, 
                                bool x_order) 
        : IPhyloTreeVisitor(tree)
        , m_Distances(distances)
        , m_Order(x_order) {}

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);

private:
    const vector<float>& m_Distances;
    bool m_Order;
};

class CPhyloTreeSorterLabel : public IPhyloTreeVisitor
{
public:
    CPhyloTreeSorterLabel(TTreeType*  tree, bool x_order) 
        : IPhyloTreeVisitor(tree)
        , m_Order(x_order) {}

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);

private:
    bool m_Order;
};


class CPhyloTreeSorterLabelRange : public IPhyloTreeVisitor
{
public:
    CPhyloTreeSorterLabelRange(TTreeType*  tree, 
                               const vector<pair<string,string> >& label_ranges,
                               bool x_order) 
        : IPhyloTreeVisitor(tree)
        , m_LabelRanges(label_ranges)
        , m_Order(x_order) {}

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx node_idx, int delta);

private:
    const vector<pair<string,string> >& m_LabelRanges;
    bool m_Order;
};

class IPhyloTreeFilter : public IPhyloTreeVisitor
{
public:
    IPhyloTreeFilter(TTreeType*  tree) : IPhyloTreeVisitor(tree) {}

protected:
    virtual ETreeTraverseCode x_OnStep(TTreeIdx x_node, int delta);

    // all allowed
    virtual bool x_Allowed(TNodeType& /*node*/) {return true;}

private:
    vector<TTreeIdx>    m_TreeStack;
};

// filter out not selected nodes
class CPhyloTreeFilter_Selector : public IPhyloTreeFilter
{
public:
    CPhyloTreeFilter_Selector(TTreeType*  tree) : IPhyloTreeFilter(tree) {}
protected:
    virtual bool x_Allowed(TNodeType&  node)
    {
        return (((*node).GetSelectedState()==CPhyloNodeData::eSelected) ||
               ((*node).GetSelectedState()==CPhyloNodeData::eShared));
    }
};

// filter out nodes, having childs number == 1
class CPhyloTreeFilter_Export : public IPhyloTreeFilter
{
public:
    CPhyloTreeFilter_Export(TTreeType*  tree) : IPhyloTreeFilter(tree) {}
protected:
    virtual bool x_Allowed(TNodeType&  node)
    {
        if (node.GetChildren().size() == 1 && node.GetParent() != TNodeType::Null()) {
			TTreeIdx child_idx = node.GetChildren()[0];
			if (m_Tree->GetNode(child_idx).GetChildren().size() == 0)
				return false;          
		}
		return true;
    }
};


// filter out nodes, with distances falling under threshold
class CPhyloTreeFilter_Distance : public IPhyloTreeFilter
{
public:
    CPhyloTreeFilter_Distance(TTreeType*  tree, float x_dist)  
        : IPhyloTreeFilter(tree) 
        , m_Dist((float)x_dist) {}

protected:
    virtual bool x_Allowed(TNodeType&  node)
    {
       return !(node.GetParent()==TNodeType::Null()) || 
           ((*node).GetDistance() > m_Dist);
    }
private:
    double m_Dist;
};

// filter out nodes, exceeding specified depth
class CPhyloTreeFilter_Depth : public IPhyloTreeFilter
{
public:
    CPhyloTreeFilter_Depth(TTreeType*  tree, int x_depth)  
        : IPhyloTreeFilter(tree)
        , m_Depth(x_depth) {}
protected:
    virtual bool x_Allowed(TNodeType&  node)
    {
       return ((*node).IDX().first <= m_Depth);
    }
private:
    int m_Depth;
};


/// Find all nodes within 'd' distance of the nodes 'node_ids'
/// If tree does not have distance information, return node_ids.
/// Traverse the tree both up and down (as if it were a graph) when
/// accumulating distance.
/// Avoid using recursion to eliminate possibility of stack overflow.
template<class TPhyloTree>
set<CPhyloTree::TTreeIdx> GetTreeNeighborhood(TPhyloTree&   phylo_tree,                                             
                                              const vector<CPhyloTree::TID>& node_ids,
                                              float d)
{
    typedef typename CPhyloTree::TTreeIdx TTreeIdx;
    typedef typename CPhyloTree::TNodeType TNodeType;
    typedef pair<TTreeIdx, float>  TNodeDist;

    // The current set of nodesnd their distance from the first node
    // node_idx
    vector<TNodeDist> node_distances;
    set<CPhyloTree::TTreeIdx> results;
    set<CPhyloTree::TTreeIdx> visited_nodes;

    for (size_t i = 0; i < node_ids.size(); ++i) {
        TTreeIdx node_idx = phylo_tree.FindNodeById(node_ids[i]);
        if (node_idx != CPhyloTree::Null()) {
            node_distances.push_back(TNodeDist(node_idx, 0.0f));
            results.insert(node_idx);
            visited_nodes.insert(node_idx);
        }
    }

    // Make sure tree has a distance parameter (it's not required)
    // Also return if d==0 since that means we do not expand the neighborhood
    if (!phylo_tree.GetFeatureDict().HasFeature("dist") || d <= 0.0f)
        return results;   

    while (node_distances.size() > 0) {
        vector<TNodeDist> prev_node_distances = node_distances;

        node_distances.clear();

        // Add children and parent of nodes in prev_node_distances to results if
        // they are not already in results and accumulated distance (for that node)
        // is not greater than 'd'
        for (size_t i = 0; i < prev_node_distances.size(); ++i) {
            TNodeType& n = phylo_tree.GetNode(prev_node_distances[i].first);
            float accumulated_dist = prev_node_distances[i].second;

            if (n.HasParent()) {
                TTreeIdx  parent_idx = n.GetParent();
                float parent_dist = n.GetValue().GetDistance();
                if (accumulated_dist + parent_dist <= d && visited_nodes.find(parent_idx) == visited_nodes.end()) {
                    node_distances.push_back(TNodeDist(parent_idx, accumulated_dist + parent_dist));
                    results.insert(parent_idx);
                }                
            }

            TNodeType::TNodeList_I child_iter = n.GetChildren().begin();
            for (; child_iter != n.GetChildren().end(); ++child_iter) {
                TTreeIdx  child_idx = *child_iter;
                float child_dist = phylo_tree.GetNode(child_idx).GetValue().GetDistance();
                if (accumulated_dist + child_dist <= d && visited_nodes.find(child_idx) == visited_nodes.end()) {
                    node_distances.push_back(TNodeDist(child_idx, accumulated_dist + child_dist));
                    results.insert(child_idx);
                }
            }

            visited_nodes.insert(prev_node_distances[i].first);
        }
    }

    return results;
}


/// Convert ASN.1 BioTree container to phylo tree
/// @param phylo_tree Ouput phylo tree
/// @param tree_container Input tree container
/// @param preserve_node_ids If true, node ids in the resulting phylo tree
/// will be the same is in the input tree container
///
template<class TBioTreeContainer, class TPhyloTree>
void BioTreeConvertContainer2Tree(TPhyloTree&   phylo_tree,
                                  const TBioTreeContainer&  tree_container, 
                                  CBioTreeFeatureDictionary* dictionary,
                                  bool preserve_node_ids = true,
                                  bool expand_all = false)
{
    phylo_tree.Clear();

    int node_id = 1;
	
    // Convert feature dictionary

    typedef typename TBioTreeContainer::TFdict  TContainerDict;

    CBioTreeFeatureDictionary& dict = phylo_tree.GetFeatureDict();
    const TContainerDict& fd = tree_container.GetFdict();
    const typename TContainerDict::Tdata& feat_list = fd.Get();
    TBioTreeFeatureId expand_collapse_id = -1;
    //TBioTreeFeatureId max_id = 0;

    ITERATE(typename TContainerDict::Tdata, it, feat_list) {
        TBioTreeFeatureId fid = (*it)->GetId();
        const string& fvalue = (*it)->GetName();
        if (fvalue == "$NODE_COLLAPSED")
            expand_collapse_id = fid;
        //max_id = std::max(fid, max_id);
		
		dict.Register(fid, fvalue);
    }

    // Vector allows us to remove unused features from dictionary. Really
    // only important for "dist" which we want to remove from the dictinoary
    // if it does not occur in any nodes. If we don't, we may default to 
    // distance-based layout of trees with no distance data. (If even 1 node has
    // has 'dist' property, we keep it and assign 0 distance to all other nodes).
    // --This has now been fixed via MakeDistanceSensitiveBioTreeContainer
    //vector<bool> has_property(size_t(max_id+1), false);

	// convert tree data (nodes)
    typedef typename TBioTreeContainer::TNodes            TCNodeSet;
    typedef typename TCNodeSet::Tdata                     TNodeList;
    typedef typename TNodeList::value_type::element_type  TCNode;

    const TNodeList node_list = tree_container.GetNodes().Get();

    std::map<TBioTreeNodeId, typename TPhyloTree::TTreeIdx> pmap;    

    phylo_tree.Reserve(node_list.size());

    ITERATE(typename TNodeList, it, node_list) {
        const CRef<TCNode>& cnode = *it;

        TBioTreeNodeId uid = cnode->GetId();

        typedef typename TPhyloTree::TNodeType TNodeType;
        typedef typename TNodeType::TValueType TNodeValueType;

        TNodeValueType v;
        if (preserve_node_ids) {
            v.SetId(uid);
        }     
    
        typedef typename TCNode::TFeatures TCNodeFeatureSet;

        if (cnode->CanGetFeatures()) {
            const TCNodeFeatureSet& fset = cnode->GetFeatures();

            const typename TCNodeFeatureSet::Tdata& flist = fset.Get();

            ITERATE(typename TCNodeFeatureSet::Tdata, fit, flist) {
                unsigned int fid = (*fit)->GetFeatureid();
                const string& fvalue = (*fit)->GetValue();

                // Should always be true unless tree was corrupted
                //if (size_t(fid) < has_property.size())
                //    has_property[size_t(fid)] = true;   //indicate property is in-use

                if (expand_all && fid == expand_collapse_id) {
                    v.GetBioTreeFeatureList().SetFeature(fid, "0");
                    continue;
                }

                v.GetBioTreeFeatureList().SetFeature(fid, fvalue);
            } // ITERATE 
        }

        if (cnode->CanGetParent()) {
            TBioTreeNodeId parent_id = cnode->GetParent();
            typename TPhyloTree::TTreeIdx parent_node_idx = pmap[parent_id];

            typename TPhyloTree::TTreeIdx node_idx = TPhyloTree::Null();

            if (parent_node_idx != TPhyloTree::Null()) {              
                node_idx = phylo_tree.AddNode();
                phylo_tree[node_idx].SetValue(v);
                phylo_tree.AddChild(parent_node_idx, node_idx);
                if (!preserve_node_ids) {
                    phylo_tree[node_idx].GetValue().SetId(++node_id);
                }
            }
            else {
                NCBI_THROW(CException, eUnknown, "Parent not found");
            }
                      
            pmap[uid] = node_idx;            
        } else {
            typename TPhyloTree::TTreeIdx node_idx = phylo_tree.AddNode();
            phylo_tree[node_idx].SetValue(v);
            phylo_tree.SetRootIdx(node_idx);

            if (!preserve_node_ids) {                
                phylo_tree[node_idx].GetValue().SetId(++node_id);
            }
            pmap[uid] = node_idx;            
        }
    } // ITERATE TNodeList

    // Problem of empty distance property is now taken care of in Newick to Biotree conversion
    // function (MakeDistanceSensitiveBioTreeContainer)
    //for (size_t i = 0; i < has_property.size(); ++i) {
    //    if (!has_property[i] && dict.HasFeature(TBioTreeFeatureId(i))) {
    //        dict.Unregister(TBioTreeFeatureId(i));
    //    }
    //}
}


// --------------------------------------------------------------------------

/// Visitor functor to convert phylo tree nodes to ASN.1 BioTree container
///
/// @internal
template<class TBioTreeContainer, class TTreeType>
class CTreeConvert2ContainerFunc
{
protected:
    typedef typename TTreeType::TNodeType               TTreeNodeType;
    typedef typename TTreeType::TTreeIdx                TTreeIdx;
    typedef typename TTreeNodeType::TValueType          TNodeValueType;

    typedef typename TBioTreeContainer::TNodes           TCNodeSet;
    typedef typename TCNodeSet::Tdata                    TNodeList;
    typedef typename TNodeList::value_type::element_type TCNode;
    typedef typename TCNode::TFeatures                   TCNodeFeatureSet;
    typedef typename TCNodeFeatureSet::Tdata             TNodeFeatureList;
    typedef typename 
       TNodeFeatureList::value_type::element_type        TCNodeFeature;
public:
    CTreeConvert2ContainerFunc(TBioTreeContainer* tree_container,
                               TTreeIdx root_idx)
    : m_Container(tree_container)
    , m_RootIdx(root_idx)
    {
        m_NodeList = &(tree_container->SetNodes().Set());
    }

    ETreeTraverseCode 
    operator()(TTreeType&  tree, TTreeIdx node_idx, int delta_level)               
    {
        if (delta_level < 0) {
            return eTreeTraverse;
        }

        TTreeNodeType& node = tree[node_idx];
        
        const TNodeValueType& v = node.GetValue();
        TBioTreeNodeId uid = v.GetId();

        CRef<TCNode> cnode(new TCNode);
        cnode->SetId(uid);

        if (node_idx != m_RootIdx && node.HasParent()) {          
            const TTreeNodeType& node_parent = 
                            (TTreeNodeType&) tree.GetParent(node);
            cnode->SetParent(node_parent.GetValue().GetId());            
        }
        
        typedef typename 
           TNodeValueType::TNodeFeaturesType::TFeatureList TFList;
        const TFList& flist = v.GetBioTreeFeatureList().GetFeatureList();

        if (!flist.empty()) {
            
            TCNodeFeatureSet& fset = cnode->SetFeatures();

            ITERATE(typename TFList, it, flist) {
                TBioTreeFeatureId fid = it->id;
                const string fvalue = it->value;

                CRef<TCNodeFeature>  cfeat(new TCNodeFeature);
                cfeat->SetFeatureid(fid);
                cfeat->SetValue(fvalue);

                fset.Set().push_back(cfeat);
            } // ITERATE
        }

        m_NodeList->push_back(cnode);

        return eTreeTraverse;
    }

private:
    TBioTreeContainer*   m_Container;
    TNodeList*           m_NodeList;
    TTreeIdx             m_RootIdx;
};

template<class TBioTreeContainer, class TTreeType>
class CTreeConvertSelected2ContainerFunc
{
protected:
    typedef typename TTreeType::TNodeType               TTreeNodeType;
    typedef typename TTreeType::TTreeIdx                TTreeIdx;
    typedef typename TTreeNodeType::TValueType          TNodeValueType;

    typedef typename TBioTreeContainer::TNodes           TCNodeSet;
    typedef typename TCNodeSet::Tdata                    TNodeList;
    typedef typename TNodeList::value_type::element_type TCNode;
    typedef typename TCNode::TFeatures                   TCNodeFeatureSet;
    typedef typename TCNodeFeatureSet::Tdata             TNodeFeatureList;
    typedef typename
        TNodeFeatureList::value_type::element_type        TCNodeFeature;
public:
    CTreeConvertSelected2ContainerFunc(TBioTreeContainer* tree_container,
        TTreeIdx root_idx)
        : m_Container(tree_container)
        , m_RootIdx(root_idx)
    {
        m_NodeList = &(tree_container->SetNodes().Set());
    }

    ETreeTraverseCode
        operator()(TTreeType&  tree, TTreeIdx node_idx, int delta_level)
    {
        if (delta_level < 0) {
            return eTreeTraverse;
        }

        TTreeNodeType& node = tree[node_idx];
        if (!((*node).GetSelectedState() == CPhyloNodeData::eSelected) &&
            !((*node).GetSelectedState() == CPhyloNodeData::eTraced) &&
            !((*node).GetSelectedState() == CPhyloNodeData::eShared))
            return eTreeTraverseStepOver;

        const TNodeValueType& v = node.GetValue();
        TBioTreeNodeId uid = v.GetId();

        CRef<TCNode> cnode(new TCNode);
        cnode->SetId(uid);

        if (node_idx != m_RootIdx && node.HasParent()) {
            const TTreeNodeType& node_parent =
                (TTreeNodeType&)tree.GetParent(node);
            cnode->SetParent(node_parent.GetValue().GetId());
        }

        typedef typename
            TNodeValueType::TNodeFeaturesType::TFeatureList TFList;
        const TFList& flist = v.GetBioTreeFeatureList().GetFeatureList();

        if (!flist.empty()) {

            TCNodeFeatureSet& fset = cnode->SetFeatures();

            ITERATE(typename TFList, it, flist) {
                TBioTreeFeatureId fid = it->id;
                const string fvalue = it->value;

                CRef<TCNodeFeature>  cfeat(new TCNodeFeature);
                cfeat->SetFeatureid(fid);
                cfeat->SetValue(fvalue);

                fset.Set().push_back(cfeat);
            } // ITERATE
        }

        m_NodeList->push_back(cnode);

        return eTreeTraverse;
    }

private:
    TBioTreeContainer*   m_Container;
    TNodeList*           m_NodeList;
    TTreeIdx             m_RootIdx;
};

template<class TBioTreeContainer, class TTreeType>
class CTreeConvertNonSingleChild2ContainerFunc
{
protected:
    typedef typename TTreeType::TNodeType               TTreeNodeType;
    typedef typename TTreeType::TTreeIdx                TTreeIdx;
    typedef typename TTreeNodeType::TValueType          TNodeValueType;

    typedef typename TBioTreeContainer::TNodes           TCNodeSet;
    typedef typename TCNodeSet::Tdata                    TNodeList;
    typedef typename TNodeList::value_type::element_type TCNode;
    typedef typename TCNode::TFeatures                   TCNodeFeatureSet;
    typedef typename TCNodeFeatureSet::Tdata             TNodeFeatureList;
    typedef typename
        TNodeFeatureList::value_type::element_type        TCNodeFeature;
public:
    CTreeConvertNonSingleChild2ContainerFunc(TTreeType&  tree,
        TBioTreeContainer* tree_container,
        TTreeIdx root_idx)
        : m_Container(tree_container)
        , m_RootIdx(root_idx)
        , m_ParentDist(0)
        , m_HasDist(false)
        , m_DistFeatureId(-1)
        , m_ParentID(-1)
        , m_SkippedParent(false)
    {
        m_NodeList = &(tree_container->SetNodes().Set());
        if (tree.GetFeatureDict().HasFeature("dist")) {
            m_HasDist = true;
            m_DistFeatureId = tree.GetFeatureDict().GetId("dist");
        }
    }

    ETreeTraverseCode
        operator()(TTreeType&  tree, TTreeIdx node_idx, int delta_level)
    {
        if (delta_level < 0) {
            m_ParentDist = 0.0f;
            m_ParentID = -1;
            m_SkippedParent = false;
            return eTreeTraverse;
        }

        TTreeNodeType& node = tree[node_idx];

        const TNodeValueType& v = node.GetValue();
        TBioTreeNodeId uid = v.GetId();

        // If this node only has one child, record the parent ID
        // (unless its parent also only had a single child, and
        // then skip over it (visit its children recursively until
        // a leaf or non-single child is found)
        if (node.GetChildren().size() == 1) {            
            if (m_HasDist) {
                m_ParentDist += v.GetDistance();
            }
            if (!m_SkippedParent && node.HasParent()) {
                const TTreeNodeType& node_parent =
                    (TTreeNodeType&)tree.GetParent(node);
                m_ParentID = node_parent.GetValue().GetId();
            }
            m_SkippedParent = true;
            return eTreeTraverse;
        }

        CRef<TCNode> cnode(new TCNode);
        cnode->SetId(uid);

        if (node_idx != m_RootIdx && node.HasParent()) {
            if (m_SkippedParent) {
                // If m_ParentID is -1, then the skipped parent
                // was the root so we do not set ay value for 
                // cnode->SetParent(..) in this case
                if (m_ParentID != -1) {
                    cnode->SetParent(m_ParentID);
                }
            }
            else {
                const TTreeNodeType& node_parent =
                    (TTreeNodeType&)tree.GetParent(node);
                cnode->SetParent(node_parent.GetValue().GetId());
            }
        }

        typedef typename
            TNodeValueType::TNodeFeaturesType::TFeatureList TFList;
        const TFList& flist = v.GetBioTreeFeatureList().GetFeatureList();

        if (!flist.empty()) {

            TCNodeFeatureSet& fset = cnode->SetFeatures();

            ITERATE(typename TFList, it, flist) {
                TBioTreeFeatureId fid = it->id;
                string fvalue = it->value;

                // We may update distances here when we skip parents (removing them from
                // the tree) we want to retain their distance information
                if (fid == m_DistFeatureId){
                    if (!fvalue.empty()) {
                        try {
                            double ddist = NStr::StringToDouble(fvalue);
                            if (ddist < 0.0)
                                ddist = 0.0;
                            ddist += (double)m_ParentDist;
                            fvalue = NStr::DoubleToString(ddist);
                        }
                        catch (std::exception&) {}
                    }
                }

                CRef<TCNodeFeature>  cfeat(new TCNodeFeature);
                cfeat->SetFeatureid(fid);
                cfeat->SetValue(fvalue);

                fset.Set().push_back(cfeat);
            } // ITERATE
        }

        m_NodeList->push_back(cnode);

        m_ParentDist = 0.0f;
        m_ParentID = -1;
        m_SkippedParent = false;

        return eTreeTraverse;
    }

private:
    TBioTreeContainer*   m_Container;
    TNodeList*           m_NodeList;
    TTreeIdx             m_RootIdx;
    float                m_ParentDist;
    bool                 m_HasDist;
    TBioTreeFeatureId    m_DistFeatureId;
    TBioTreeNodeId       m_ParentID;
    bool                 m_SkippedParent;
};


/// Convert  tree to ASN.1 BioTree container
///
template<class TBioTreeContainer, class TPhyloTree>
void TreeConvert2Container(TBioTreeContainer& tree_container,
                           TPhyloTree&        phylo_tree,
                           typename TPhyloTree::TTreeIdx  node_idx=TPhyloTree::Null())
{
    // Convert feature dictionary

    typedef typename TBioTreeContainer::TFdict  TContainerDict;

    const CBioTreeFeatureDictionary& dict = phylo_tree.GetFeatureDict();
    const CBioTreeFeatureDictionary::TFeatureDict& dict_map = 
                                                dict.GetFeatureDict();

    TContainerDict& fd = tree_container.SetFdict();
    typename TContainerDict::Tdata& feat_list = fd.Set();
    typedef 
    typename TContainerDict::Tdata::value_type::element_type TCFeatureDescr;
    
    ITERATE(CBioTreeFeatureDictionary::TFeatureDict, it, dict_map) {
        TBioTreeFeatureId fid = it->first;
        const string& fvalue = it->second;

        {{
        CRef<TCFeatureDescr> d(new TCFeatureDescr);
        d->SetId(fid);
        d->SetName(fvalue);

        feat_list.push_back(d);
        }}
    } // ITERATE

    CTreeConvert2ContainerFunc<TBioTreeContainer, TPhyloTree>
        func(&tree_container, node_idx);

    if (node_idx==TPhyloTree::Null())
        TreeDepthFirst(phylo_tree, func);
    else
        TreeDepthFirst(phylo_tree, node_idx, func);
}

/// Convert  selected nodes from tree to ASN.1 BioTree container
///
template<class TBioTreeContainer, class TPhyloTree>
void TreeConvertSelected2Container(TBioTreeContainer& tree_container,
    TPhyloTree&        phylo_tree,
    typename TPhyloTree::TTreeIdx  node_idx = TPhyloTree::Null())
{
    // Convert feature dictionary

    typedef typename TBioTreeContainer::TFdict  TContainerDict;

    const CBioTreeFeatureDictionary& dict = phylo_tree.GetFeatureDict();
    const CBioTreeFeatureDictionary::TFeatureDict& dict_map =
        dict.GetFeatureDict();

    TContainerDict& fd = tree_container.SetFdict();
    typename TContainerDict::Tdata& feat_list = fd.Set();
    typedef
        typename TContainerDict::Tdata::value_type::element_type TCFeatureDescr;

    ITERATE(CBioTreeFeatureDictionary::TFeatureDict, it, dict_map) {
        TBioTreeFeatureId fid = it->first;
        const string& fvalue = it->second;

        {{
                CRef<TCFeatureDescr> d(new TCFeatureDescr);
                d->SetId(fid);
                d->SetName(fvalue);

                feat_list.push_back(d);
            }}
    } // ITERATE

    CTreeConvertSelected2ContainerFunc<TBioTreeContainer, TPhyloTree>
        func(&tree_container, node_idx);

    if (node_idx == TPhyloTree::Null())
        TreeDepthFirst(phylo_tree, func);
    else
        TreeDepthFirst(phylo_tree, node_idx, func);
}

/// Convert  selected nodes from tree to ASN.1 BioTree container
///
template<class TBioTreeContainer, class TPhyloTree>
void TreeConvertNonSingleChild2Container(TBioTreeContainer& tree_container,
    TPhyloTree&        phylo_tree,
    typename TPhyloTree::TTreeIdx  node_idx = TPhyloTree::Null())
{
    // Convert feature dictionary

    typedef typename TBioTreeContainer::TFdict  TContainerDict;

    const CBioTreeFeatureDictionary& dict = phylo_tree.GetFeatureDict();
    const CBioTreeFeatureDictionary::TFeatureDict& dict_map =
        dict.GetFeatureDict();

    TContainerDict& fd = tree_container.SetFdict();
    typename TContainerDict::Tdata& feat_list = fd.Set();
    typedef
        typename TContainerDict::Tdata::value_type::element_type TCFeatureDescr;

    ITERATE(CBioTreeFeatureDictionary::TFeatureDict, it, dict_map) {
        TBioTreeFeatureId fid = it->first;
        const string& fvalue = it->second;

        {{
                CRef<TCFeatureDescr> d(new TCFeatureDescr);
                d->SetId(fid);
                d->SetName(fvalue);

                feat_list.push_back(d);
            }}
    } // ITERATE

    CTreeConvertNonSingleChild2ContainerFunc<TBioTreeContainer, TPhyloTree>
        func(phylo_tree, &tree_container, node_idx);

    if (node_idx == TPhyloTree::Null())
        TreeDepthFirst(phylo_tree, func);
    else
        TreeDepthFirst(phylo_tree, node_idx, func);
}

/// Visitor functor to convert phylo tree nodes to ASN.1 BioTree container
/// not including all features
/// @internal
template<class TBioTreeContainer, class TTreeType>
class CTreeConvert2ContainerPartialFunc
{
protected:
    typedef typename TTreeType::TNodeType               TTreeNodeType;
    typedef typename TTreeType::TTreeIdx                TTreeIdx;
    typedef typename TTreeNodeType::TValueType          TNodeValueType;

    typedef typename TBioTreeContainer::TNodes           TCNodeSet;
    typedef typename TCNodeSet::Tdata                    TNodeList;
    typedef typename TNodeList::value_type::element_type TCNode;
    typedef typename TCNode::TFeatures                   TCNodeFeatureSet;
    typedef typename TCNodeFeatureSet::Tdata             TNodeFeatureList;
    typedef typename 
       TNodeFeatureList::value_type::element_type        TCNodeFeature;
public:
    CTreeConvert2ContainerPartialFunc(TBioTreeContainer* tree_container,
                                      TTreeIdx root_idx,
                                      const CBioTreeFeatureDictionary& return_dict)
    : m_Container(tree_container)
    , m_RootIdx(root_idx)
    , m_ReturnDict(return_dict)
    {
        m_NodeList = &(tree_container->SetNodes().Set());
    }

    ETreeTraverseCode 
    operator()(TTreeType&  tree, TTreeIdx node_idx, int delta_level)               
    {
        if (delta_level < 0) {
            return eTreeTraverse;
        }

        TTreeNodeType& node = tree[node_idx];
        
        const TNodeValueType& v = node.GetValue();
        TBioTreeNodeId uid = v.GetId();

        CRef<TCNode> cnode(new TCNode);
        cnode->SetId(uid);

        if (node_idx != m_RootIdx && node.HasParent()) {          
            const TTreeNodeType& node_parent = 
                            (TTreeNodeType&) tree.GetParent(node);
            cnode->SetParent(node_parent.GetValue().GetId());            
        }
        
        typedef typename 
           TNodeValueType::TNodeFeaturesType::TFeatureList TFList;
        const TFList& flist = v.GetBioTreeFeatureList().GetFeatureList();

        if (!flist.empty()) {
            
            TCNodeFeatureSet& fset = cnode->SetFeatures();

            // Only get features already in the dictionary. That allows us to export
            // trees with restricted sets of features (TreeConvert2ContainerPartial).
            ITERATE(typename TFList, it, flist) {
                TBioTreeFeatureId fid = it->id;
                const string fvalue = it->value;

                if (m_ReturnDict.HasFeature(fid)) {
                    CRef<TCNodeFeature>  cfeat(new TCNodeFeature);
                    cfeat->SetFeatureid(fid);
                    cfeat->SetValue(fvalue);

                    fset.Set().push_back(cfeat);
                }
            } // ITERATE
        }

        m_NodeList->push_back(cnode);

        return eTreeTraverse;
    }

private:
    TBioTreeContainer*   m_Container;
    TNodeList*           m_NodeList;
    TTreeIdx             m_RootIdx;
    const CBioTreeFeatureDictionary&  m_ReturnDict;
};

/// Convert  tree to ASN.1 BioTree container but only include the requested features
///
template<class TBioTreeContainer, class TPhyloTree>
void TreeConvert2ContainerPartial(TBioTreeContainer& tree_container,
                                  TPhyloTree&        phylo_tree,
                                  const set<string>& features,
                                  typename TPhyloTree::TTreeIdx  node_idx = TPhyloTree::Null())
{
    // Convert feature dictionary
    typedef typename TBioTreeContainer::TFdict  TContainerDict;

    const CBioTreeFeatureDictionary& dict = phylo_tree.GetFeatureDict();
    const CBioTreeFeatureDictionary::TFeatureDict& dict_map =
        dict.GetFeatureDict();
    CBioTreeFeatureDictionary return_dict;

    TContainerDict& fd = tree_container.SetFdict();
    typename TContainerDict::Tdata& feat_list = fd.Set();
    typedef
        typename TContainerDict::Tdata::value_type::element_type TCFeatureDescr;

    ITERATE(CBioTreeFeatureDictionary::TFeatureDict, it, dict_map) {
        TBioTreeFeatureId fid = it->first;
        const string& fvalue = it->second;
        if (features.find(fvalue) != features.end()) {            
            CRef<TCFeatureDescr> d(new TCFeatureDescr);
            d->SetId(fid);
            d->SetName(fvalue);
            return_dict.Register(fid, fvalue);

            feat_list.push_back(d);         
        }
    } // ITERATE

    CTreeConvert2ContainerPartialFunc<TBioTreeContainer, TPhyloTree>
        func(&tree_container, node_idx, return_dict);

    if (node_idx == TPhyloTree::Null())
        TreeDepthFirst(phylo_tree, func);
    else
        TreeDepthFirst(phylo_tree, node_idx, func);
}

/// Convert CBioTreeFeatureDictionary to ASN.1 BioTree dictionary
///
template<class TContainerDict>
void DictConvert2ContainerFeatureDict(TContainerDict& fd,
                                      CBioTreeFeatureDictionary& dict)
{
    // Convert feature dictionary
    const CBioTreeFeatureDictionary::TFeatureDict& dict_map =
        dict.GetFeatureDict();

    typename TContainerDict::Tdata& feat_list = fd.Set();
    typedef typename TContainerDict::Tdata::value_type::element_type TCFeatureDescr;

    ITERATE(CBioTreeFeatureDictionary::TFeatureDict, it, dict_map) {
        TBioTreeFeatureId fid = it->first;
        const string& fvalue = it->second;

        {{
                CRef<TCFeatureDescr> d(new TCFeatureDescr);
                d->SetId(fid);
                d->SetName(fvalue);

                feat_list.push_back(d);
            }}
    } // ITERATE
}


END_NCBI_SCOPE

#endif
