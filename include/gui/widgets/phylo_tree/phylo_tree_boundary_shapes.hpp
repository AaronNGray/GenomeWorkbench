#ifndef GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_BOUNDARY_SHAPES__HPP
#define GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_BOUNDARY_SHAPES__HPP 

/*  $Id: phylo_tree_boundary_shapes.hpp 35987 2016-07-25 13:06:57Z falkrb $
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
#include <corelib/ncbiobj.hpp>
#include <gui/utils/vect2.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/glrect.hpp>
#include <gui/opengl/glvbonode.hpp>

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
///
/// CBoundaryParms
///
/// Parameters for boundaries parsed from node features
///
/// A simple class to hold the parameter values for boundary shapes that
/// were parsed from the node features.  Makes it easier to have several
/// different shapes with same parameters and to pass the parameters around.
class CBoundaryParms
{
public:
    CBoundaryParms() 
    : m_BorderWidth(5.0f)
    , m_CornerWidth(5.0f) 
    , m_BoundaryEdge(false)
    , m_BoundaryEdgeColor(0.0f, 0.0f, 0.0f, 1.0f)
    , m_IncludeTextArea(false)
    , m_AxisAligned(true)
    , m_TextBox(true)
    , m_TriOffset(0.0f) {}

    /// Set/get color of boundary region
    void SetColor(const CRgbaColor& c) { m_Color = c; }
    CRgbaColor& GetColor() { return m_Color; }

    /// Set/get size of border region around subtree (in pixels)
    void SetBorderWidth(float w) { m_BorderWidth = w; }
    float GetBorderWidth() const { return m_BorderWidth; }

    /// Get/set size of rounded corners used for region (may 
    /// not apply to all typs of regions) (in pixels)
    void SetCornerWidth(float w) { m_CornerWidth = w; }
    float GetCornerWidth() const { return m_CornerWidth; }

    /// Enable/disable drawing of a single-pixel width colored edge around 
    /// boundary   
    void SetDrawBoundaryEdge(bool b) { m_BoundaryEdge = b; }
    bool GetDrawBoundaryEdge() const { return m_BoundaryEdge; }

    /// Set boundary edge color
    void SetBoundaryEdgeColor(const CRgbaColor& r) { m_BoundaryEdgeColor = r; }
    CRgbaColor& GetBoundaryEdgeColor() { return m_BoundaryEdgeColor; }

    /// Enable/disable inclusion of text boxes into boundary area
    void SetIncludeTextArea(bool b) { m_IncludeTextArea = b; }
    bool GetIncludeTextArea() const { return m_IncludeTextArea; }

    /// Enable/disable inclusion of text boxes into boundary area
    void SetAxisAligned(bool b) { m_AxisAligned = b; }
    bool GetAxisAligned() const { return m_AxisAligned; }

    /// Enable/disable using a separate bounding box for text (meant for triangles)
    void SetTextBox(bool b) { m_TextBox = b; }
    bool GetTextBox() const { return m_TextBox; }

    /// Set/Get distance of a triangle's starting vertex behind its base node
    void SetTriOffset(float o) { m_TriOffset = o; }
    float GetTriOffset() const { return m_TriOffset; }

protected:
    /// Number (width) of empty pixels on boundary edge
    float m_BorderWidth;

    // Color for the bounding region, including alpha
    CRgbaColor m_Color;

    /// Width, in pixels, of the rounding area at each corner.  This effectivly
    /// adds to a boundaries border width because the border width has to be
    /// at least the size of the corner width to guarantee all graphical 
    /// elements remain inside the boundary.
    float m_CornerWidth;

    /// If true a single pixel-width edge is drawn around the boundary using the
    /// color m_BoundaryEdgeColor
    bool m_BoundaryEdge;
    CRgbaColor m_BoundaryEdgeColor;

    /// If true, boundary is sized (enlarged) to include all text boxes.
    bool m_IncludeTextArea;

    /// If true shape will be aligned with x and y axes
    bool m_AxisAligned;

    /// If true a separate box will be created for text data. (esp. for triangles)
    bool m_TextBox;

    /// For triangles the toplevel node is always at one of the 3 corners
    /// and this is the offset behind that node, relative to the triangles
    /// orientation
    float m_TriOffset;
};


/////////////////////////////////////////////////////////////////////////////
///
/// IBoundaryShape --
///
/// Base class for boundaries (ovarlay regions) to be drawn over subtrees.
///
/// The base class defines the interface for initializing the shape with a 
/// set of points and drawing the boundary.  Also defined are common 
/// properties including border width, color, and whether the boundary is
/// currently visible.
class IBoundaryShape : public CObject
{
public:
    IBoundaryShape() 
    : m_Hidden(false) {}

    virtual ~IBoundaryShape() {}

    /// Given points which scale (pts) and rectangles for text (pixel_pts) which
    /// do not scale, compute the minimal area for the boundary
    virtual void ComputeShape(const CVect2<float>& scale,
                              const CVect2<float>& base_node_pos,                            
                              const std::vector<CVect2<float> >& pts,
                              const vector<std::pair<CVect2<float>, CGlRect<float> > >& pixel_pts) = 0;
    /// Render the boundary if !hidden
    virtual void Render(const CVect2<float>& scale, 
                        float alpha_mod = 1.0f,
                        bool include_labels = true);
    virtual void RenderVbo(CRef<CGlVboNode>& tri_node,
                           CRef<CGlVboNode>& edge_node,                           
                           const CVect2<float>& scale, 
                           float alpha_mod = 1.0f,
                           bool include_labels = true);

    /// Enable/disable rendering of the boundary
    void Hide(bool b) { m_Hidden = b; }

    void SetBoundaryParms(const CBoundaryParms& p) { m_Parms = p; }
    CBoundaryParms& GetBoundaryParms() { return m_Parms; }

    /// Create a new boundary object given it's name
    static IBoundaryShape* CreateBoundary(const string& boundary_type);

    virtual void ComputeShapeWithLabels(const CVect2<float>& scale, 
                                        bool labels_visible = true) {}

    const TModelRect& GetExtent() const { return m_Extent; }

protected:
    virtual void x_GetTris(vector<CVect2<float> >& tris, 
                           const CVect2<float>& scale) {}
    virtual void x_GetEdges(vector<CVect2<float> >& edges, 
                            const CVect2<float>& scale) {}

    /// If true, do not render
    bool m_Hidden;

    /// Paramters for boundary parsed from features string
    CBoundaryParms m_Parms;

    /// These pairs define the (scaled) location and associated text
    /// rectangle for the labels inside the boundary. The CGlRect
    /// objects are defined in pixel dimensions and the rect is assumed
    /// to have its lower left corner at the CVect2 object
    vector<std::pair<CVect2<float>, CGlRect<float> > > m_PixelPoints;

    /// Nodes inside the boundary
    std::vector<CVect2<float> > m_NodePoints;

    /// Parent/root node for nodes in the bounded area
    CVect2<float> m_BaseNodePos;

    /// Extent of boundery (min/max xy)
    TModelRect m_Extent;
};


/////////////////////////////////////////////////////////////////////////////
///
/// CBoundaryShapeRectBase --
///
/// Base class for rectangular boundaries
///
/// Defineds are of rectangle and computes size of rectangle.  Rendering
/// is left to subclasses which may or may not have rounded corners
///
class CBoundaryShapeRectBase : public IBoundaryShape
{
public:
    CBoundaryShapeRectBase();
    virtual ~CBoundaryShapeRectBase() {}

    virtual void ComputeShape(const CVect2<float>& scale,
                              const CVect2<float>& base_node_pos,                           
                              const std::vector<CVect2<float> >& pts,                            
                              const vector<std::pair<CVect2<float>, CGlRect<float> > >& pixel_pts);
    virtual void ComputeShapeWithLabels(const CVect2<float>& scale,
                                        bool labels_visible = true);

protected:
    /// Corners of the rectangle
    CVect2<float> m_Points[4];

    /// Delta values applied to corners to accomodate labels which do not scale
    /// with the tree as a whole
    float m_PixelDeltaNegX;
    float m_PixelDeltaNegY;
    float m_PixelDeltaPosX;
    float m_PixelDeltaPosY;  
};

/////////////////////////////////////////////////////////////////////////////
///
/// CBoundaryShapeRect --
///
/// Simple rectangular boundary
///
class CBoundaryShapeRect : public CBoundaryShapeRectBase
{
public:
    CBoundaryShapeRect() {}
    virtual ~CBoundaryShapeRect() {}

protected:
    void x_GetTris(vector<CVect2<float> >& tris, 
                   const CVect2<float>& scale);
    void x_GetEdges(vector<CVect2<float> >& edges, 
                    const CVect2<float>& scale);
};

/////////////////////////////////////////////////////////////////////////////
///
/// CBoundaryShapeRoundedRect --
///
/// Simple rectangular boundary with rounded corners.
///
class CBoundaryShapeRoundedRect : public CBoundaryShapeRectBase
{
public:
    CBoundaryShapeRoundedRect() {}
    virtual ~CBoundaryShapeRoundedRect() {}

protected:

    void x_GetTris(vector<CVect2<float> >& tris, 
                   const CVect2<float>& scale);
    void x_GetEdges(vector<CVect2<float> >& edges, 
                    const CVect2<float>& scale);
    void x_GetRoudedCornerTris(vector<CVect2<float> >& tris,
                               const CVect2<float>& pos,
                               float corner_width_x,
                               float corner_width_y,
                               float start_angle);  
    void x_GetRoudedCornerEdges(vector<CVect2<float> >& edges,
                                const CVect2<float>& pos,
                                float corner_width_x,
                                float corner_width_y,
                                float start_angle); 
};

/////////////////////////////////////////////////////////////////////////////
///
/// CBoundaryShapeTri --
///
/// Triangular boundary region
///
class CBoundaryShapeTri : public IBoundaryShape
{
public:
    CBoundaryShapeTri();
    virtual ~CBoundaryShapeTri() {}

    virtual void ComputeShape(const CVect2<float>& scale,
                              const CVect2<float>& base_node_pos,                             
                              const std::vector<CVect2<float> >& pts,                    
                              const vector<std::pair<CVect2<float>, CGlRect<float> > >& pixel_pts);
    virtual void ComputeShapeWithLabels(const CVect2<float>& scale,
                                        bool labels_visible = true);

protected:
    
    /// Rounded corners are drawn as circles that replace the corner that would
    /// extend from intersection[1,2] to initial_vertex. The circle is drawn
    /// at position 'pos' with initial scaled radius corner_width_[x,y]
    struct RoundedCorner {
        CVect2<float> pos;
        CVect2<float> initial_vertex;
        CVect2<float> intersection1;
        CVect2<float> intersection2;      
        float corner_width_x;
        float corner_width_y;    
    };

    void x_GetTris(vector<CVect2<float> >& tris, 
                   const CVect2<float>& scale);
    void x_GetEdges(vector<CVect2<float> >& edges, 
                    const CVect2<float>& scale);

    void x_ComputeTriParms(const std::vector<CVect2<float> >& pts,  
                           const CVect2<float>& base_pos,
                           const CVect2<float>& dir,
                           const CVect2<float>& perp_dir,
                           float& max_len,
                           float& max_angle_top,
                           float& max_angle_bottom);

    RoundedCorner x_ComputeRoundedCorner(const CVect2<float>& pt_in,
                                         const CVect2<float>& prev_pt_in,
                                         const CVect2<float>& next_pt_in,
                                         const CVect2<float>& scale,
                                         CVect2<float>& pt_out1,
                                         CVect2<float>& pt_out2);

    /// Get the points that make up the roundec
    void x_GetArc(const RoundedCorner& c,
                  const CVect2<float>& scale,
                  vector<CVect2<float> >& edges);

    /// Corners of the rectangles that make up the triangle with clipped
    /// corners (to allow rounding) and the (optional) text box.
    CVect2<float> m_Rect1[4];
    CVect2<float> m_Rect2[4];
    CVect2<float> m_Rect3[4];

    /// Used for debugging text box
    CVect2<float> m_TextBox[4];

    /// The rounded corners
    std::vector<RoundedCorner> m_RoundedCorners;

    /// Center of the shape, used for scaling if a border is applied
    CVect2<float> m_Center;
    /// Width and height (maximum extents) used if scaling is applied
    float m_Width;
    float m_Height;

    /// Allows multiple passes with different border colors if desired
    CRgbaColor m_CurrentBorderColor;
};

/////////////////////////////////////////////////////////////////////////////
///
/// BoundaryShapeEllipse --
///
/// Elliptical boundary region
///
class BoundaryShapeEllipse : public IBoundaryShape
{
public:
    virtual ~BoundaryShapeEllipse() {}

    virtual void ComputeShape(const CVect2<float>& /*scale*/,
                              const CVect2<float>& /*base_node_pos*/,
                              const std::vector<CVect2<float> >& /*pts*/,                          
                              const vector<std::pair<CVect2<float>, CGlRect<float> > >& /*pixel_pts*/) {}
    //virtual void Render(const CVect2<float>& /*scale*/, float /*alpha_mod = 1.0f*/) {}
};


/////////////////////////////////////////////////////////////////////////////
///
/// CBoundaryPoints --
///
/// Holds all points in a bounded region.  Points may be fixed (such as text-box
/// corners) or scaled such as node points.  Fixed-scale points will be attached to
/// scaled points (text box moves with node)
///
/// Improvement:  If very large trees are bounded can reduce number of points
/// by only considering convex hull for set (this becomes a bit more complex with
/// the fixed scaling though)
/// 
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CBoundaryPoints {
public:
    CBoundaryPoints() {}

    /// Add point to be included in bounded area
    void AddBoundedPoint(const CVect2<float>& pt); 
    /// Add point in bounded area along with a rectangle based at
    /// that point and defined in pixels, that also will be included
    /// in the bounded area (allows non-resizing text to be included)
    void AddPixelRect(const CVect2<float>& pt, 
                      const CGlRect<float>& rect);

    /// Add the boundary areas of the subtree to this boundary area
    void AddBoundedPoints(CBoundaryPoints& pts);
    /// Clear all accumulated bounding points
    void ClearBoundedPoints();

    const vector<CVect2<float> >& GetGraphPoints() const { return m_GraphPoints; }
    const vector<std::pair<CVect2<float>, CGlRect<float> > >& GetPixelPoints() const { return m_PixelPoints; }

protected:
    /// Points and text rectangles used to compute boundary area (and discarded
    /// afterwards)
    vector<CVect2<float> > m_GraphPoints;  
    vector<std::pair<CVect2<float>, CGlRect<float> > > m_PixelPoints;
};

/////////////////////////////////////////////////////////////////////////////
///
/// CSubtreeBoundary --
///
/// Primary boundary object for drawing boundaries around subtrees
///
/// This boundary object parses the boundary features requested and
/// creates the appropriate IBoundaryShape subclass with the parsed
/// parameters.  Based on the requested features, more than one
/// shape may be created so that different shapes can be shown
/// for different layouts (e.g. a rectangle for a rectangular cladogram
/// and a triangular boundary for a slanted cladogram).
/// 
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CSubtreeBoundary {
public:
    CSubtreeBoundary()
        : m_BoundaryShape(NULL)
        , m_LastScale(0.0f, 0.0f) {}

    ~CSubtreeBoundary();
    
    /// Render current shape
    void RenderBoundary(const CVect2<float>& scale, 
                        float alpha_mod = 1.0f,
                        bool include_labels = true);
    /// Render to vbo (create vbo but do not draw)
    void RenderBoundaryVbo(const CVect2<float>& scale, 
                           float alpha_mod = 1.0f,
                           bool include_labels=true);
    /// Render the vbo
    void RenderVbo();

    CRef<CGlVboNode> GetBoundaryTris() { return m_BoundaryTris; }
    CRef<CGlVboNode> GetBoundaryEdges() { return m_BoundaryEdges; }

    /// Parse the features to create the shapes and set their options   
    void CreateShapes(const std::string& features);
    /// Use the accumulated point data to compute shape dimensions
    void ComputeShapes(const CBoundaryPoints& boundary_pts,
                       const CVect2<float>& scale,
                       const CVect2<float>& base_node_pos,
                       const string& layout_type);

    /// Show/hide boundary area
    void Hide(bool b=true);

    /// Return true if boundary (in model coordinates) overlaps rectangle r
    bool Overlaps(TModelRect& r) const;

    /// Set get source string for boundary parms
    void SetBoundaryString(const string& str) { m_BoundaryStr = str; }
    string GetBoundaryString() const { return m_BoundaryStr; }

protected:
    /// All shapes created for boundary (may be different shapes for differnt
    /// layout algorighms).  Key is a string designating the layout algorithm.
    map<string, CRef<IBoundaryShape> > m_Shapes;


    /// Feature string from which boundary parameters were created
    string m_BoundaryStr;


    /// Current (active) boundary shape (for current layout)
    CRef<IBoundaryShape> m_BoundaryShape;

    /// For parsing boudary info - get a parameter
    string x_GetParameter(const string& features, const string& parm);
    /// For parsing a color parameter
    bool x_GetColorParameter(const string& features, 
                             const string& parm,
                             bool parm_required,
                             CRgbaColor& c);
    /// Add a boundary for the specified layout algorithm
    void x_AddBoundaryType(const CBoundaryParms& parms, 
                           const string& boundary_type,
                           const string& layout_type);

    CRef<CGlVboNode> m_BoundaryTris;
    CRef<CGlVboNode> m_BoundaryEdges;

    CVect2<float> m_LastScale;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_BOUNDARY_SHAPES__HPP
