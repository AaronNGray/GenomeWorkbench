#ifndef GUI_WIDGETS_PHY_TREE___PHYLO_TREE_NODE__HPP
#define GUI_WIDGETS_PHY_TREE___PHYLO_TREE_NODE__HPP

/*  $Id: phylo_tree_node.hpp 38724 2017-06-13 12:22:09Z falkrb $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <corelib/ncbi_tree.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/biotree/Node.hpp>
#include <algo/phy_tree/bio_tree.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>
#include <gui/opengl/glrect.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/widgets/phylo_tree/tree_model.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>

BEGIN_NCBI_SCOPE

class CSubtreeBoundary;
class CTreeGraphicsModel;
class CGlTextureFont;
class CRgbaGradColorTable;

// We put larger, infrequently used parameters for the node
// into a separate structure so that we can keep the node
// sizes smaller on average - since the pointer to this
// structure will usually just be NULL.
struct SExtendedNodeParms {
    SExtendedNodeParms()
		: m_MarkerSize(0.0f)
		, m_Boundary(NULL) {};
    SExtendedNodeParms(const SExtendedNodeParms& rhs);
    SExtendedNodeParms& operator=(const SExtendedNodeParms& rhs);

    ~SExtendedNodeParms();

	// marker colors (0 or 1 at a leaf, 0..n otherwise)
	vector<CRgbaColor> m_MarkerColors;
	// marker size, as a multiple of node size
	float m_MarkerSize;

	// Boundary for subtree at this node (if m_Bounded is eBounded)
	CSubtreeBoundary*  m_Boundary;

	// selection clusters
	vector<int> m_SelClusters;
};

class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloNodeData {
public:
    typedef objects::CNode::TId TID;
    typedef string TLabel;
    typedef float TDistance;
    typedef int TClusterID;
    typedef vector<int> TSelClusterIDs;
    typedef CBioTreeFeatureList TNodeFeaturesType;

    typedef enum {
        eNotSelected,   // node is not selected
        eSelected,      // node is selected (descending)
        eTraced,        // node is traced (up to root)
        eShared         // node is on path between selected nodes
    } TSelectedState;

    typedef enum {
        eShowChildren,  // node expanded
        eHideChildren   // node collapsed
    } TDisplayChildren;

    typedef enum {
        eUnBounded,
        eBounded
    } TBoundingState;

    typedef CVect2<float>        TPoint;
    typedef pair<int, int>        TIndex;
    typedef string               TColor;

    static const TColor TNoColor;

public:
    CPhyloNodeData()
        : m_NumLeaves(0)
        , m_NumLeavesEx(0)
        , m_PamlCounter(0)
        , m_ClusterID(-1)
        , m_Distance(TDistance(0))
        , m_DistanceFromRoot(TDistance(0))
        , m_MaxChildDistance(TDistance(0))
        , m_Angle(0.0f)
        , m_DrawAngle(0.0f)
        , m_EdgeScore(0.0f)
        , m_NodeOffset(0.0f, 0.0f)
        , m_NodePixelOffset(0.0f, 0.0f)
        , m_Selected(CPhyloNodeData::eNotSelected)
        , m_ColorType(CPhyloTreeScheme::eColor)
        , m_ExtendedParms(NULL)
        , m_NodeColor(1.0f, 0.0f, 1.0f, 1.0f)
        , m_FgColor(1.0f, 0.0f, 1.0f, 1.0f)
        , m_Label("")
        , m_Size(-1.0f)
        , m_NodeWidth(0.0f)
        , m_NodeEdColorIdx(-1)
        , m_NodeFgColorIdx(-1)
        , m_ClusterColorIdx(-1)
        , m_LabelFgColorIdx(-1)
        , m_LabelBgColorIdx(-1)
        , m_EdgeColorGradient(false)
        , m_Children(true) {}

    CPhyloNodeData(const CPhyloNodeData& rhs);
    ~CPhyloNodeData();
    CPhyloNodeData& operator=(const CPhyloNodeData& rhs);

    void Init(const CBioTreeFeatureDictionary& dict,
        CRgbaGradColorTable* t);

    void  Sync(CBioTreeFeatureDictionary& dict);

    const CRef<objects::CSeq_id>& GetSeqID() const  { return m_SeqID; }
    void SetSeqID(objects::CSeq_id* sid) { m_SeqID.Reset(sid); }

    CBioTreeFeatureList& GetBioTreeFeatureList();
    const CBioTreeFeatureList& GetBioTreeFeatureList() const { return m_Features; }
    void  SetBioTreeFeatureList(const CBioTreeFeatureList & btcfl);

    // cache accessors
    int     GetNumLeaves() const    { return m_NumLeaves; }
    void    SetNumLeaves(int x_ln)  { m_NumLeaves = x_ln; }

    int     GetNumLeavesEx() const    { return m_NumLeavesEx; }
    void    SetNumLeavesEx(int x_ln)  { m_NumLeavesEx = x_ln; }

    /// With selection clusters a node may have multiple cluster ids. Return
    /// highest-priority cluster (first selection cluster, if present, 
    /// m_ClusterID otherwise)
    int  GetPrimaryCluster() const;

    /// Return total # of applicable clusters - selection + m_ClusterID
    int GetNumClusters() const;

    TClusterID GetClusterID(void) const         { return m_ClusterID; }
    void       SetClusterID(TClusterID x_id)    { m_ClusterID = x_id; }

    int   GetPamlCounter() const    { return m_PamlCounter; }
    void  SetPamlCounter(int x_ctr) { m_PamlCounter = x_ctr; }

    TID GetId() const       { return m_ID; }
    void SetId(TID x_id);

    TDistance GetDistance() const   { return fabs(m_Distance); }
    void SetDistance(TDistance x_dist)    { m_Distance = x_dist; }

    TDistance GetDistanceFromRoot() const   { return fabs(m_DistanceFromRoot); }
    void SetDistanceFromRoot(TDistance x_dist)    { m_DistanceFromRoot = x_dist; }

    TDistance GetMaxChildDistance() const   { return fabs(m_MaxChildDistance); }
    void SetMaxChildDistance(TDistance x_dist)    { m_MaxChildDistance = x_dist; }

    void SetLabel(const TLabel & x_label) { m_Label = x_label; }
    const TLabel& GetLabel() const { return m_Label; }

    void Set(TID x_id, TDistance x_dist,
        const TLabel & x_label)
    {
        m_ID = x_id;
        m_Label = x_label;
        m_Distance = x_dist;
    }


    TPoint& XY()   { return m_XY; }
    float& X()     { return m_XY.X(); }
    float& Y()     { return m_XY.Y(); }

    TPoint XY() const   { return m_XY; }
    float X() const     { return m_XY.X(); }
    float Y() const     { return m_XY.Y(); }

    TIndex& IDX()  { return m_IDX; }
    const TIndex& IDX() const { return m_IDX; }

    TSelectedState GetSelectedState() const       { return m_Selected; }
    void SetSelectedState(TSelectedState x_state) { m_Selected = x_state; }

    CPhyloTreeScheme::TPhyloTreeColors GetColoration() const { return m_ColorType; }
    void SetColoration(CPhyloTreeScheme::TPhyloTreeColors ct) { m_ColorType = ct; }

    TDisplayChildren  GetDisplayChildren() const      { return m_Children ? eShowChildren : eHideChildren; }
    void SetDisplayChildren(TDisplayChildren show)  { m_Children = (show == eShowChildren); }

    TBoundingState  GetBoundedDisplay() const;

    short GetNodeEdColorIdx() const         { return m_NodeEdColorIdx; }
    short GetNodeFgColorIdx() const         { return m_NodeFgColorIdx; }
    short GetClusterColorIdx() const        { return m_ClusterColorIdx; }
    short GetLabelFgColorIdx() const        { return m_LabelFgColorIdx; }
    short GetLabelBgColorIdx()              { return m_LabelBgColorIdx; }

    void SetNodeFgColorIdx(short idx)       { m_NodeFgColorIdx = idx; }
    void SetNodeEdgeColorIdx(short idx)     { m_NodeEdColorIdx = idx; }
    void SetClusterColorIdx(short idx)      { m_ClusterColorIdx = idx; }
    void SetLabelColorIdx(short idx)        { m_LabelFgColorIdx = idx; }
    void SetLabelBgColorIdx(short idx)      { m_LabelBgColorIdx = idx; }

    void SetLabelRect(const CGlRect<float>& r) { m_LabelRect = r; }
    CGlRect<float>  GetLabelRect() const { return m_LabelRect; }

    void SetNodeOffset(const CVect2<float>& o) { m_NodeOffset = o; }
    CVect2<float> GetNodeOffset() const { return m_NodeOffset; }

    void SetNodePixelOffset(const CVect2<float>& o) { m_NodePixelOffset = o; }
    CVect2<float> GetNodePixelOffset() const { return m_NodePixelOffset; }

	bool            HasSelClusters() const;
	TSelClusterIDs& GetSelClusters();
	void            SetSelClusters(const TSelClusterIDs& sc);

	void SetMarkerColors(const string& str);
	string GetMarkerColorsAsString();
	bool HasNodeMarker() const;
	vector<CRgbaColor>& GetMarkerColors();
	float GetMarkerSize() const;
	void SetMarkerSize(float f);
	/// Gets current boundary or creates a new one if m_Boundary is NULL
	CSubtreeBoundary* GetSubtreeBoundary();
	/// Deletes current boundary (if not NULL).
	void DeleteSubtreeBoundary();


    float GetNodeSize() const         { return m_Size;    }
    void SetNodeSize(float x_sz)      { m_Size = x_sz;    }

    float GetAngle(void) const        { return m_Angle;  }
    void  SetAngle(float angle)       { m_Angle = angle; }

    float GetDrawAngle(void) const        { return m_DrawAngle; }
    void  SetDrawAngle(float angle)       { m_DrawAngle = angle; }

    float GetEdgeScore() const { return m_EdgeScore; }
    void SetEdgeScore(float s) { m_EdgeScore = s;  }

    void    SetFeature(CBioTreeFeatureDictionary& dict,
                       const string & name, const string & value);
    void    RemoveFeature(CBioTreeFeatureDictionary& dict, const string & name);

    // init excluding seq-id
    void    InitFeatures(const CBioTreeFeatureDictionary& dict,
                         CRgbaGradColorTable* t);

    /// Utility to get screen rotation angle after scaling
    float GetScaledAngle(const CVect3<float>& scale, float a, bool for_label = true) const;

    /// Get modified center of node position - this works for collapsed 
    /// nodes in rotated layouts and with offsets between label and node
    CVect2<float> GetNodePosEx(const CVect3<float>& scale, 
                               CPhyloTreeScheme* scheme, 
                               float& node_size,
                               float size_scaler = 1.0f) const;

    /// Draw the label using scale to compute its offset from m_BasePos
    void Render(const CGlPane& pane, 
                const CGlTextureFont& font,
                CRgbaGradColorTable* color_table,
                const CVect3<float>& scale, 
                bool truncate_labels,
                TModelUnit max_len = -1.0f);

    /// Draw the label and rotate it to match m_Angle (radial text)
    /// This form does not support label truncation
    void RenderRotated(const CGlPane& pane, 
                       const CGlTextureFont& font,
                       CRgbaGradColorTable* color_table,
                       const CVect3<float>& scale,
                       float orig_angle,
                       bool truncate_labels);

    /// Return the drawing position and (possibly truncated) text
    CVect2<float> GetTextOut(CGlPane& pane, 
                             const CGlTextureFont& font,
                             const CVect3<float>& scale,
                             bool truncate_labels,
                             string& text_out);

    /// Find the rectangle for the label given the current zoom level (scale)
    void GetLabelRect(const CVect2<float>& scale,
                      CVect2<float>& lower_left,
                      CVect2<float>& upper_right,
                      bool rotated=false);

    /// Return true if pt is inside of this nodes text box
    bool PointInTextBox(const CVect2<float>& scale,
                        CVect2<float> pt, 
                        bool rotated=false);

    /// Find the rectangle for the label in pixel coordinates, taking rotation into account
    CGlRect<float> GetLabelRect(const CVect2<float>& scale, bool rotated);

    /// Find the rectangle for the node and label combined given the 
    /// current zoom level (scale)
    void GetBoundingRect(const CVect2<float>& scale, 
                         CVect2<float>& lower_left,
                         CVect2<float>& upper_right,
                         bool rotated=false);

    /// Return text box (m_LabelRect) rotated to align with radial edges
    CVect4<CVect2<float> > GetRotatedTextBox(const CVect3<float>& scale, 
                                             float& angle, float orig_angle = 0.0f) const;

    /// Return the text starting point rotated to align with radial edges
    CVect2<float> GetRotatedTextPos(const CVect3<float>& scale,
                                    float& angle) const;

    /// Return labels minimal X coordinate
    float GetMinX(const CVect2<float>& scale);

    /// Get modified node size - includes scaled (BOA) size
    float GetDefaultNodeSize(const CPhyloTreeScheme* scheme) const;

    /// Get modified node size - use this instead of GetDefaultNodeSize
    /// when doing layout (doesn't go to 0 if leaf node size does)
    /// This is basically the height of the node for layout purposes.
    float GetNodeLayoutSize(const CPhyloTreeScheme* scheme) const;

    /// Get modified node width (length) for collapsed nodes. Default is 1
    float GetNodeWidthScaler(const CPhyloTreeScheme* scheme) const;

    /// Get extenstion of node beyond end of edge in pixels. In circular views,
    /// this takes tfhe offset into the circle into account as well.
    float GetNodeOverhang(const CVect2<float>& scale) const;

    /// Flag to renderer to determine whether to draw label
    void SetVisible(bool b);
    bool GetVisible() const { return m_Visible; }

    /// set/get label text
    void SetDisplayLabel(const string& t) { m_Text = t; }
    const string& GetDisplayLabel() const { return m_Text; }

    /// set/get node size (this is the result of GetDefaultNodeSize())
    void SetNodeDisplaySize(float s) { m_NodeSize = s; }
    float GetNodeDisplaySize() const { return m_NodeSize; }

    /// Enable or disable gradient coloring of edges between nodes
    void SetEdgeColorGradient(bool g) { m_EdgeColorGradient = g; }
    bool GetEdgeColorGradient() const { return m_EdgeColorGradient; }

    /// Set/get node color
    void SetNodeDisplayColor(const CRgbaColor& c) { m_NodeColor = c; }
    CRgbaColor GetNodeDisplayColor() const { return m_NodeColor; }

    /// Set/get text color
    void SetTextDisplayColor(const CRgbaColor& c) { m_FgColor = c; }
    CRgbaColor GetTextDisplayColor() const { return m_FgColor; }

    /// Flag set after each drawing and cleared when rendering frame done
    void SetAlreadyDrawn(bool b) { m_AlreadyDrawn = b; }
    bool GetAlreadyDrawn() const { return m_AlreadyDrawn; }

protected:
    string x_GetParameter(const string& features, const string& parm);
	bool x_HasExtendedParms() const { return m_ExtendedParms != NULL; }

    CRef<objects::CSeq_id>  m_SeqID;
    CBioTreeFeatureList     m_Features;

    // cached values
	int     m_NumLeaves;
	// tweaked for close/open
	int     m_NumLeavesEx;

    // Paml-style counter, leaf-skipping
    int     m_PamlCounter;
    
    // cluster ID
    TClusterID m_ClusterID;

    TID         m_ID;
    // Distance of this node from it's parent
    TDistance   m_Distance;
    // distance of node from root node
    TDistance   m_DistanceFromRoot;
    // maximum distance of any child of this node from the root
    TDistance m_MaxChildDistance;

    // location / coordinates
    TPoint      m_XY;
    TIndex      m_IDX;
    // This is the angle of the node from the center of the (drawn) tree
    float       m_Angle;
    // This angle is the angle at which text was drawn, which may differ (for 
    // minimally rotated angles in particular) with m_Angle
    float       m_DrawAngle;

    // Scaled value to allow more significant (currently that means longer)
    // edges to be highlighted in a zoomed-out (many edges overlapping) images
    float m_EdgeScore;
    
    // Pixel coordinates of label relative to node position
    CGlRect<float> m_LabelRect;
    // For nodes offset from normal position (initially only collapsed nodes
    // in circular layout).  This is in world coordinates.
    CVect2<float> m_NodeOffset;
    /// Pixel offset to apply the node 
    CVect2<float> m_NodePixelOffset;

    // selection state
    TSelectedState   m_Selected;
    CPhyloTreeScheme::TPhyloTreeColors m_ColorType;
	
    // NULL unless one or more of the infrequently-used parameters are invoked
    SExtendedNodeParms* m_ExtendedParms;

	/// Current node color
    CRgbaColor m_NodeColor;
    /// Current text color
    CRgbaColor m_FgColor;    

	/// Full label
	TLabel m_Label;
    /// Text to render
    string m_Text;

    // node size
    float m_Size;

    /// Scaled node size
    float m_NodeSize;

    /// This is computed by GetNodeWidthScaler(..) for collapsed nodes and
    /// we need to hang onto it for node label positioning
    mutable float m_NodeWidth;

    /// Node colors defined by scheme
    short m_NodeEdColorIdx;
    short m_NodeFgColorIdx;

    // Node and edge color, if cluster present
    short m_ClusterColorIdx;

    /// Label color
    short m_LabelFgColorIdx;
    short m_LabelBgColorIdx;

    /// Controls use of gradient coloring for edges - if false, edges will be a 
    /// single color, if true the edge will transition from child to parent color
    bool m_EdgeColorGradient;
    /// expand-collapse state (expanded==true)
    bool m_Children;
    /// True if node is visible
    bool m_Visible;
    /// Text is alpha-blended so we add a flag to prevent drawing multiple times
    bool m_AlreadyDrawn;
};


class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloTreeNode : public CTreeModelNode<CPhyloNodeData>
{
public:
    /// ctor
    CPhyloTreeNode();
    /// Set Id (must be unique), dist to parent and label
    CPhyloTreeNode(int x_id, double x_dist, const string& x_label);   

    /// Return true the index 'idx' in the array is not currently part of the tree.
    /// This can happen if a node is deleted, for example.
    bool IsUnused() const { return GetValue().GetId() == CPhyloNodeData::TID(-1); }

    /// Return true if node is currently not collapsed
    bool    Expanded() const { return GetValue().GetDisplayChildren() == CPhyloNodeData::eShowChildren; }
    /// Set this node to be expanded/collapsed
    void    ExpandCollapse(CBioTreeFeatureDictionary& dict, 
                           CPhyloNodeData::TDisplayChildren chds);
    /// Return true if node can have its expand/collapsed state changed to chds
    bool    CanExpandCollapse(CPhyloNodeData::TDisplayChildren chds);
    /// Set label string and synch value to the feature list for this ndoe
    void    SetLabel(CBioTreeFeatureDictionary& dict,
                     const string & label);

    /// Return the number of immediate children of this node
    int NumChildren() const { return m_ChildNodes.size(); }

    /// Return true if node is a leaf or is collapsed
    bool    IsLeafEx() const        { return (m_ChildNodes.empty() || (!Expanded())); }

    /// Return the child nodes only if visible
    TNodeList_I SubNodeBeginEx()    { return Expanded() ? m_ChildNodes.begin() : m_ChildNodes.end();  }
    TNodeList_I SubNodeEndEx()      { return m_ChildNodes.end(); }
};

END_NCBI_SCOPE

#endif  //GUI_WIDGETS_PHY_TREE___PHYLO_TREE_NODE__HPP
