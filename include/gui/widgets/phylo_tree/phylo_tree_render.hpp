#ifndef GUI_WIDGETS_PHY_TREE___PHYLO_TREE_RENDER__HPP
#define GUI_WIDGETS_PHY_TREE___PHYLO_TREE_RENDER__HPP

/*  $Id: phylo_tree_render.hpp 43030 2019-05-08 15:37:18Z katargir $
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
#include <corelib/ncbitime.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>
#include <gui/widgets/gl/ievent_handler.hpp>
#include <gui/widgets/wx/sticky_tooltip_handler.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_lod.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_label.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_boundary_shapes.hpp>

#include <gui/print/pdf.hpp>

#include <gui/utils/curve_bezier.hpp>
#include <gui/utils/rgba_color.hpp>

#include <gui/opengl/glcurve.hpp>
#include <gui/opengl/i3dframebuffer.hpp>
#include <gui/opengl/i3dtexture.hpp>

BEGIN_NCBI_SCOPE

class CMouseZoomHandler;
class CPhyloTreeDataSource;
class CGlTextureFont;
class CGlVboNode;
class IRender;


// rendering interface
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT IPhyloTreeRender
    : public CObjectEx,
      public wxEvtHandler,
      public IGlEventHandler,
      public IPhyloTreeLOD
{
    DECLARE_EVENT_TABLE()
public:
    typedef enum {
        eDistancesRendering,
        eAdaptiveMargins,
        eSimplification,
        eSplinesRendering,
        eRotatedLabels,
        eGenerateTextures,
        eAutofitLabels
    } TRenderingOption;

    enum ELayoutStatus {
        eValidLayout,       // Layout is up-to-date with topology
        eNeedLayout,        // Layout is out-of-date but renderer has not changed
        eNeedLayoutAndSize, // Layout is out-of-date and renderer has changed    
    };

    typedef CPhyloTree TTreeType;
    typedef CPhyloTree::TTreeIdx TTreeIdx;
    typedef CPhyloTree::TNodeType TNodeType;

    IPhyloTreeRender();
    IPhyloTreeRender(double width, double height);
    virtual ~IPhyloTreeRender();

    /// @name Forwarded messages from to support IStickyTooltipHandlerHost
    /// @{
    string  TTHH_NeedTooltip(const wxPoint & pt);
    CTooltipInfo  TTHH_GetTooltip(const wxRect & rect);
   /// @}
    
    /// id is the tip id of the tip that the users mouse is currently on, or -1
    void SetActiveTooltipNode(int id);

    /// point to a particular node from a tooltip to help user spot it
    /// rectangle should be the tooltip screen rectangle in screen (not model)
    /// coordinates where the lower-left corner of the panel is (0,0)
    void PointToNode(int id, wxRect tip_rect, float sec);

    void    SetHost(IPhyloTreeRenderHost* pHost);

    void    SetFont(CGlTextureFont * font) {m_pLabelFont = font;}

    void    SetZoomHandler(CMouseZoomHandler* mz) { m_pMouseZoomHandler = mz; }

    void    SetDistRendering(bool flag) {m_DistMode = flag;}
    bool    GetDistRendering(void) const {return m_DistMode;}
    virtual bool SupportsDistanceRendering() const { return true; }

    bool    GetSplinesRendering(void) const {return m_UseSplines;}
    void    SetSplinesRendering(bool flag) {m_UseSplines = flag;}

    bool    GetRotatedLabels(void) const {return m_RotatedLabels;}
    void    SetRotatedLabels(bool flag) {m_RotatedLabels = flag;}
    virtual bool SupportsRotatedLabels() const { return false; }
    
    bool    GetForceSquare(void) const { return m_ForceSquare; }
    void    SetForceSquare(bool flag) {m_ForceSquare= flag;}

    bool    GetRegenerateTexture(void) const { return m_RegenerateTexture; }
    void    SetRegenerateTexture(bool flag){m_RegenerateTexture = flag;}

    void    SetLabelViewPct(float pct) { m_LabelViewPct = pct; }
    virtual float   GetLabelViewPct() const { return m_LabelViewPct; }

    bool    GetRenderScale() const { return m_RenderScale; }
    void    SetRenderScale(bool s) { m_RenderScale = s; }

    /// Get sum of pixel size of all leaf nodes
    float GetTreeHeight() const { return m_TreePixels; }

    CPhyloTree::TSelState& GetSingleSelection() { return m_NodeSingleSelection; }
    /// Predicate to find location of a node in above selection set
    struct FindSelState {
        FindSelState(CPhyloTree::TTreeIdx node_idx) : m_Idx(node_idx) {}
        bool operator()(const CPhyloTree::TSelState::value_type& v) {
            return (v.first == m_Idx);
        }

        CPhyloTree::TTreeIdx m_Idx;
    };

    /// Allow caller to indicate layout has changed from one type to another
    /// (e.g. rect clad. to force)
    virtual void    StartRendering(bool b) { if (b) m_SwitchedLayout = true; }

    // return true if the tree has been laid out at least once
    virtual ELayoutStatus ValidLayout() const { return m_ValidLayout; }
    virtual void NeedsLayout(ELayoutStatus s) { m_ValidLayout = s; }


    void    OnLeftDown(wxMouseEvent& event);
    void    OnRightDown(wxMouseEvent& event);
    void    OnLeftDblClick(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnTimer(wxTimerEvent& event);
    void    OnTimerLeftUp(wxTimerEvent& event);

    IGenericHandlerHost* GetGenericHost();

    // IPhyloTreeLOD
    virtual TVPUnit DistanceBetweenNodes(void) const;
    float GetNodeSize(const CPhyloTreeNode* node) const;

    /// nodes, edges and labels can be colored for default, trace, shared, 
    /// select or cluster
    float  SetNodeColoration(CPhyloTreeNode* node,
                             TTreeIdx node_idx);
    short GetNodeColorIdx(CPhyloTreeNode* node);
    short GetLabelColorIdx(CPhyloTreeNode* node);
    /// Returns true if color is from selection (sel, shared, trace)
    bool GetEdgeColor(CPhyloTreeNode* child_node,
                      CPhyloTreeNode* parent_node,
                      CRgbaColor& c,
                      CRgbaColor& parent_color);

    CPhyloTreeNode* GetHoverNode(CVect2<float> pt);
    CPhyloTree::TTreeIdx GetHoverNodeIdx(CVect2<float> pt);

    I3DTexture * GetTexture(float& xcoord_limit, float& ycoord_limit);

    virtual string  GetDescription(void) = 0;
    virtual void    BufferedRender(CGlPane& pane, 
                                   CPhyloTreeDataSource& ds, 
                                   bool init_collision_info=true,
                                   bool select_edge=false);
    void InitCollisionInfo();
    
    /// Return true if lables are visible according to rendering scheme and at current scale
    bool LabelsVisible();
    /// Return default node size according to scheme or 0 if not visible at current scale
    virtual float DefaultNodeSize();
	/// Initialize (for current layout) label text and extent (label rectangle)
	void InitLabel(CPhyloTree* tree,
				   CPhyloTree::TNodeType& node);

    /// Returns false if rendering does not occur (e.g. due to active data
    /// sychronization).
    virtual bool    Render(CGlPane& pane, CPhyloTreeDataSource& ds);
    virtual void    RenderForHardcopy(CGlPane& pane, CPhyloTreeDataSource& ds);
    virtual void    RenderPdf(CGlPane& pane, CPhyloTreeDataSource& ds, CRef<CPdf> pdf);
    virtual void    Layout(CPhyloTreeDataSource& ds, CGlPane& p);
    virtual void    Redraw(void);
    virtual void    RemoveCurrentDataSource();
    virtual void    UpdateDataSource(CPhyloTreeDataSource& ds, CGlPane& p);

    /// @name IGlEventHandler implementaion
    /// @{
    virtual void    SetPane(CGlPane* pane);
    virtual wxEvtHandler*    GetEvtHandler();
    /// @}

    //const TModelRect & GetRasterRect(void);

    void  SetScheme(CPhyloTreeScheme & sl);

    CPhyloTreeScheme & GetScheme(void) { return m_SL.GetObject(); }

    void SetModelDimensions(double width, double height)
    {
        m_DimX = width;
        m_DimY = height;
    }

	// Projection extents are needed to computer viewing limits
	void CalculateExtents(CPhyloTree* tree, CPhyloTree::TNodeType& node);
	/// Set extents to default values so CalculateExtents can be called
	void InitExtents();

	/// Compute the optimal viewing pane size so that the tree occupies the entire
	/// viewing area, and labels do not go offscreen
    void ComputeViewingLimits(CGlPane& pane, 
                              bool force_square = false,
                              bool init_collision_info = true);

    void SetRenderingOption(TRenderingOption opt, bool bVal);

    // For most views scale glyph should be in LL (if not LR).
    virtual bool IsDistanceBarLowerLeft() const { return m_DistanceBarLowerLeft; }
    void SetDistanceBarLowerLeft(bool b) { m_DistanceBarLowerLeft = b; }

    // metrics
    const double GetDimX(void) {return m_DimX;}
    const double GetDimY(void) {return m_DimY;}

    virtual double ComputeDistFromPixels(CGlPane& pane,
                                         const CVect2<TModelUnit>& pixpos1, 
                                         const CVect2<TModelUnit>& pixpos2) const;
    double ComputeScale(double default_scale, int& precision) const;

    // Setup parameters for highlighting longer edges when view is zoomed
    // out to the level that many edges overlap the same pixels
    void SetHighlightEdges(bool enable);
    bool GetHighlightEdges() const { return m_HighlightLongerEdges; }

    // minimum dimensions preserving full visibility
    TVPRect GetMinDimensions(CGlPane& pane,
                             const CPhyloTreeDataSource& ds,
                             const CPhyloTreeScheme& sc,
                             int& label_height,
                             TModelRect& node_rect,
                             TModelRect& text_rect,
                             float width_factor,
                             float height_factor = 1.0f,
                             float wh_ratio = -1.0f);
    // The number of nodes that will fit (with labels visible) in current viewport
    int GetMaxLeavesVisible();

    virtual float GetDefaultNodeSize(const CPhyloTreeNode* node) const;
    virtual float GetNodeLayoutSize(const CPhyloTreeNode* node) const;
    virtual float GetNodeLabelDist(const CPhyloTreeNode* node) const;

protected:

    // data
    CPhyloTreeDataSource* m_DS;

    //style
    CRef<CPhyloTreeScheme> m_SL;

    double  m_DimX,  m_DimY;
    double  m_xStep, m_yStep;
    double  m_NormDistance;

    // If true distance marker is put on lower-left of screen
    bool m_DistanceBarLowerLeft;

    CGlTextureFont   *m_pLabelFont;

    IPhyloTreeRenderHost *m_pHost;
    CMouseZoomHandler*  m_pMouseZoomHandler;

    // scene texture
    CIRef<I3DTexture> m_Texture;

    CGlCurve<CCurveBezier> m_Curve;

    CPhyloTreeLabel     m_Label;
    CPhyloTreeLabel     m_LabelExt;

    TModelRect          m_RasterRect;

    /// Graphical representation of currently selected edge, if any,
    /// and data members needed for the selection process
    std::vector<CVect2<float> > m_SelectedEdge;
    CVect2<float> m_RightClick;
    bool m_SelEdge;
    float m_ClosestEdgeDistSq;
    CVect2<float> m_ClosestEdgeDir;


    /// event handling state
    enum    EState {
        eIdle,
        eSelPoint, /// selecting a single glyph by point
        eSelRect,  /// selecting multiple glyphs by rectangle
        eDragNode
    };

    /// label alignment
    enum    ELabelAlign {
        eLeft,
        eRight
    };

    /// event handling
    CGlPane         *m_pPane;
    EState           m_State;
    wxPoint          m_StartPoint;
    wxPoint          m_LastPos;
    wxPoint          m_DragPoint;
    TTreeIdx         m_ActiveTooltipNode;
    bool             m_DistMode;
    bool             m_MoveDuringSelection;
    bool             m_UseSplines;
    bool             m_RotatedLabels;
    bool             m_ForceSquare;
    bool             m_RegenerateTexture;
    wxTimer          m_EffectsTimer;
    bool             m_SwitchedLayout;
    /// If user is double clicking, want to avoid mouse single-click actions
    wxTimer          m_MouseSingleTimer;
    /// event info from last mouse click - used for single click processing
    wxMouseEvent     m_LastMouseEvent;
    
    /// When sizing the tree to window, this is the maximum % of screen
    /// area (width) that will be allocated to labels on either side
    /// of the screen (some views, e.g. radial, have labels on both sides).
    float            m_LabelViewPct;
    
    /// If true render a scale legend at the bottom of the screen.
    bool             m_RenderScale;

    /// Sum of size (height) of all leaf nodes in pixels, without considering labels
    /// (accomodates nodes that are not  equally sized)
    float            m_TreePixels;
    /// Height of tree in pixels when labels are displayed.
    float            m_LabelPixels;

    ELayoutStatus   m_ValidLayout;

    wxStockCursor   m_CursorId;

    /// When user only wants to display a single, current selection from
    /// the selection set (including traced and commmon nodes) those
    /// nodes and selection states are stored in this vector.
    CPhyloTree::TSelState m_NodeSingleSelection;

    /// Structure holds a 'pointer' from an active tooltip to its
    /// corresponding node
    struct NodePointer {
        NodePointer() 
            : m_NodeID(0)
            , m_NodeIdx(CPhyloTreeNode::Null())
            , m_TipCenter((TModelUnit)0, (TModelUnit)0)
            , m_Duration(0.0f)
        {}

        /// ID of the node to point to
        CPhyloTree::TID m_NodeID;

        /// Pointer to the node with id m_NodeID
        CPhyloTreeNode::TTreeIdx m_NodeIdx;

        /// Rectangle defining tooltip in screen coords
        wxRect m_TipRect;
        /// Center of tooltip widget, in screen coords with (0,0) in lower left
        TModelPoint  m_TipCenter;

        /// Timer to track duration of arrow display
        CStopWatch m_Timer;
        double m_Duration;
    };
    std::vector<NodePointer> m_NodePointers;   

    CIRef<I3DFrameBuffer>  m_MinimapBuffer;

    /// layout-spefic constant for label vertical sepatation needed for visibility
    virtual TModelUnit x_GetVerticalSeparationFactor() const { return TModelUnit(1.0); }
    /// Returns true if lines are separated enough to use the wider form of lines (more
    /// separation is required for circular views, since interior lines overlap more easily)
    virtual bool x_WideLines();

public:
    /// A coord (in one of +x,-x,+y,-y) and pixel offset in the same direction
    struct ProjectionElement {
        ProjectionElement() : m_ProjectionCood(0.0f) {}
        ProjectionElement(float p, float o) 
            : m_NodeCoord(p)
            , m_PixelOffset(o)
            , m_ProjectionCood(0.0f) {}

        /// Node x or y position depending on whether this extent is vert. or horiz.
        float m_NodeCoord;

        /// Pixel offset (direction +x,-x,+y,-y) relative to the x or y position
        /// above. The owning class tracks according to offset direction.
        float m_PixelOffset;

        /// Given a particular visible area and viewport size, position to which
        /// this node projects inside the viewport
        float m_ProjectionCood;
    };

    /// This is a set of positions (for nodes) and associated x or y pixel offset
    /// values.  The owning class determines whether this refers to x or y
    /// coordninates.
    class CExtentDimension
    {
    public:
        /// Compare the new coordinate and pixel offset with the contents
        /// of m_Extents. If node_coord is larger than any node coordinate
        /// in the array, or if pixel_offset is longer than any element
        /// in the array with the same or larger node coord, add it.
        void  UpdateExtent(float node_coord,
            float pixel_offset,
            bool has_label);

		void Clear() { m_Extents.clear(); }

        /// write out values for debugging purposes
        void DumpExtent(const string& dim);

        /// Sorted (in decreasing order) set of coordinates and pixel offsets
        vector<ProjectionElement> m_Extents;
    };

    /// This class extracts for each of the 4 directions (+x,-x,+y,-y) the maximum
    /// coordinates to which any element in the graph projects.  For node positions 
    /// this is easy since it is simply max/min x/y values.  But since text (labels)
    /// do not scale with the graph, they may extend beyond graph edges at some
    /// zoom levels but not others. We therefore also find the longest labels
    /// in each of the 4 directions. (in each of the 4 directions we add any node-
    /// label combo where the label is longer in that direction than labels
    /// of all other nodes further (left/right/up/down) in the same direction)
    class CProjectionExtents {
    public:
        CProjectionExtents() {}


        /// Combine together m_MaxX and m_MinX and m_MaxY and m_MinY
        void ConsolidateExtents(vector<ProjectionElement>& xexts,
            vector<ProjectionElement>& yexts);

        /// Write output to log for debugging
        void DumpExtents();
		void Clear() { m_MaxX.Clear(); m_MaxY.Clear(); m_MinX.Clear(); m_MinY.Clear(); }

        /// the 'rightmost' nodes and labels (includes the node with the largest
        /// x value, and any nodes to the left that have labels longer than any
        /// node to the right of them
        CExtentDimension m_MaxX;
        /// Same as for m_MaxX values, but in y dimension
        CExtentDimension m_MaxY;
        /// the 'leftmost' nodes, except that signs are flipped in order to allow
        /// uniform processing in all dimensions (ConsolidateExtents() unflips)
        CExtentDimension m_MinX;
        /// the 'bottommost' nodes, with signs flipped.
        CExtentDimension m_MinY;
    };

	CProjectionExtents  m_ProjectionExtents;
	TModelRect m_ZoomableExtent;

    //
    //Internal functions used by tree layout and rendering algorithms:
    //

    bool x_GetEdgeDist(const CVect2<float>& from,
                       const CVect2<float>& to);
    void    x_RenderLineVbo(TTreeIdx child_node_idx,
                            CPhyloTreeNode* child_node,
                            CPhyloTreeNode* parent_node, 
                            vector<CVect2<float> >& line_coords,
                            vector<CVect4<unsigned char> >& line_colors,
                            double x1,
                            double y1,
                            double x2,
                            double y2);
    // separate buffer for selected lines. Not needed for non-distance-based layouts
    // where edges do not overlap.
    void    x_RenderLineVbo(TTreeIdx child_node_idx,
                            CPhyloTreeNode* child_node,
                            CPhyloTreeNode* parent_node,
                            vector<CVect2<float> >& line_coords,
                            vector<CVect4<unsigned char> >& line_colors,
                            vector<CVect2<float> >& sel_line_coords,
                            vector<CVect4<unsigned char> >& sel_line_colors,
                            double x1,
                            double y1,
                            double x2,
                            double y2);
    void    x_RenderSplineVbo(TTreeIdx child_node_idx,
                              CPhyloTreeNode* child_node,
                              CPhyloTreeNode* parent_node,
                              vector<CVect2<float> >& edge_node_coords,
                              vector<CVect4<unsigned char> >& edge_node_colors,
                              vector<CVect2<float> >& sel_edge_node_coords,
                              vector<CVect4<unsigned char> >& sel_edge_node_colors,
                              const CVect3<float>& pt1,
                              const CVect3<float>& pt2,
                              const CVect3<float>& pt3,
                              const CVect3<float>& pt4);
    void    x_RenderCircularVbo(TTreeIdx child_node_idx,
                                CPhyloTreeNode* child_node,
                                CPhyloTreeNode* parent_node,
                                vector<CVect2<float> >* trace_line_coords,
                                vector<CVect4<unsigned char> >* trace_line_colors,
                                vector<CVect2<float> >* trace_point_coords,
                                vector<CVect4<unsigned char> >* trace_point_colors,
                                vector<CVect2<float> >& edge_node_coords,
                                vector<CVect4<unsigned char> >& edge_node_colors,
                                vector<CVect2<float> >& sel_edge_node_coords,
                                vector<CVect4<unsigned char> >& sel_edge_node_colors,
                                float radius,
                                const CVect2<float>& center,
                                const CVect2<float>& offset,
                                float resolution);
    void    x_RenderPseudoSplineVbo(TTreeIdx child_node_idx,
                                    CPhyloTreeNode* child_node,
                                    CPhyloTreeNode* parent_node,
                                    vector<CVect2<float> >& edge_node_coords,
                                    vector<CVect4<unsigned char> >& edge_node_colors,
                                    vector<CVect2<float> >& sel_edge_node_coords,
                                    vector<CVect4<unsigned char> >& sel_edge_node_colors,
                                    const CVect2<float>& pt1,
                                    const CVect2<float>&  mid_point,
                                    const CVect2<float>&  pt2);

    void x_AddCollapsedGeomNode(CPhyloTreeNode* node,
                                const CVect2<float> delta,                            
                                CRgbaColor color,
                                float alpha,
                                size_t circleSegments,
                                bool skip_pdf);
    bool x_AddCollapsedTextureNode(CPhyloTreeNode* node,
                                   const CVect2<float> delta,
                                   CRgbaColor color,
                                   float alpha);

    void    x_RenderNodeVbo(CPhyloTreeNode* node, 
                            TTreeIdx node_idx, 
                            vector<float>& color_coords,
                            const CVect2<float> delta=CVect2<float>(0.0f, 0.0f));

    virtual void x_ComputeNodeBoundary(CPhyloTree* tree,
								       CPhyloTree::TNodeType& node,
                                       CBoundaryPoints& boundary_pts,
                                       const string& layout_type);

    virtual void x_RenderVbo(CPhyloTreeDataSource& /*ds*/) {}

    /// Set OpenGL rendering options that depend on layout chosen.
    /// Do not do this during layout because the CGI may create multiple
    /// rendering contexts but layout is not called for each context
    virtual void x_SetGlRenderOptions(CPhyloTreeDataSource& ds) {}

protected:
    /// Returns viewport projection size in x/y from last iteration
    /// and puts optimal viewing (to see whole tree) area in limits
    CVect2<float> x_FindBestSize(CGlPane& pane,
                           std::vector<ProjectionElement>& xexts,
                           std::vector<ProjectionElement>& yexts,
                           TModelRect& limits,
                           TModelRect& node_extents,
                           TModelRect& text_extents,
                           int max_iterations=500);
    void    x_InitCollisionInfo();
    // Based on zoom level, set visiblity for edges, nodes and labels
    virtual void x_SetElementVisibility();
    void    x_RenderSelection(CGlPane& pane);
    void    x_RenderScaleMarker(CGlPane& pane, IRender& gl);
    void    x_RenderTreeLabel(CGlPane& pane, IRender& gl);
    void    x_RenderTooltipHints(CGlPane& pane);
    void    x_RenderNodeHighlight(CGlPane& pane,
                                  CPhyloTree::TTreeIdx node_idx,
                                  const CRgbaColor& c);


    void    x_DrawBoundingAreas(CPhyloTreeNode * node);
    void    x_DrawBoundingAreasVbo(CPhyloTreeNode * node);
    
    void    x_GenerateTexture(void);

    int x_OnMouseMove(void);
    int x_OnKeyDown(void);
    int x_OnKeyUp(void);

    void    x_OnSelectCursor(void);

    /// Returns true if selection status changed (or may have changed..)
    bool    x_SelectByRect(bool toggle);

    virtual void x_Layout(CPhyloTreeDataSource& ds) {}
    virtual void x_Render(CGlPane& pane, CPhyloTreeDataSource& ds) {}

    /// compute coefficients for edge highlighting (if enabled)
    void x_ComputeEdgeHighlight();

    /// For performance analysis (time to update VBOs after topology change)
    float m_BufferdRenderTime;
	float m_VboSyncTime;
    float m_LayoutTime;

    /// If true, highlight longer edges when zoomed out
    bool m_HighlightLongerEdges;

    /// Parms for logistics function to highlight edges when zoomed out to the
    /// point that many edges overlap (en.wikipedia.org/wiki/Logistic_function)
    float m_HighlightFunctionK;
    float m_HighlightFunctionMidpoint;
    float m_HighlightIntensity;

    class ComputePixelHeight
    {
    public:
        ComputePixelHeight(IPhyloTreeLOD* c) 
        : m_Clad(c)
        , m_PixelHeight(0.0f)
        , m_TotalHeight(0.0f) 
        {
            m_FontHeight = (float)c->GetScheme().GetFont().GetFontSize();
        }

        ETreeTraverseCode operator()(TTreeType&  tree, TTreeType::TTreeIdx node_idx, int delta)
        {
            TNodeType& node = tree[node_idx];

            if (delta == 0 || delta == 1) {
                if (node.IsLeafEx()) {
                    float node_size = 2.0f*(float)m_Clad->GetNodeLayoutSize(&node);
                    m_PixelHeight += node_size;
                    if (m_FontHeight > node_size)
                        m_TotalHeight += m_FontHeight;
                    else
                        m_TotalHeight += node_size;
                }
            }

            if (!node.Expanded())
                return eTreeTraverseStepOver;

            return eTreeTraverse;
        }

        IPhyloTreeLOD* m_Clad;
        float m_PixelHeight;
        float m_TotalHeight;
        float m_FontHeight;

    };
};



END_NCBI_SCOPE

#endif //GUI_WIDGETS_PHY_TREE___PHYLO_TREE_RENDER__HPP

