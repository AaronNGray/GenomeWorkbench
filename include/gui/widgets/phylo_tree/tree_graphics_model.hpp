#ifndef __GUI_WIDGETS_PHYLO_TREE___GRAPHICS_MODEL__HPP
#define __GUI_WIDGETS_PHYLO_TREE___GRAPHICS_MODEL__HPP

/*  $Id: tree_graphics_model.hpp 43136 2019-05-20 18:37:04Z katargir $
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

#include <gui/gui_export.h>
#include <gui/opengl/glmodel2d.hpp>
#include <gui/opengl/glrect.hpp>
#include <gui/opengl/glcolortable.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/i3dtexture.hpp>
#include <gui/widgets/phylo_tree/tree_collision_model2d.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>
#include <gui/print/pdf.hpp>

BEGIN_NCBI_SCOPE 

class CPhyloTree;

////////////////////////////////////////////////////////////////////////////////
/// class CTreeBoundaryNode
/// 
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CTreeBoundaryNode : public  CGlRenderNode {
public:
    CTreeBoundaryNode(CPhyloTree* t) 
        : CGlRenderNode(), m_Tree(t), m_IncludeLabels(true) {}
    virtual ~CTreeBoundaryNode() {}

    void  SetScheme(CPhyloTreeScheme& sl) { m_SL.Reset(&sl); }
    void  SetPane(const CGlPane& p) { m_Pane = p; }

    void SetIncludeLabels(bool b) { m_IncludeLabels = b; }

    void Clear() { m_BoundaryNodes.clear(); }
    vector<size_t>& GetNodes() { return m_BoundaryNodes; }

    void RenderPdf(CRef<CPdf>& pdf);

protected:

    virtual void x_Render();

    /// Vector of all nodes that have a boundary defined

    vector<size_t> m_BoundaryNodes;

    CPhyloTree* m_Tree;

    CRef<CPhyloTreeScheme> m_SL;

    CGlPane  m_Pane;

    bool m_IncludeLabels;
};


////////////////////////////////////////////////////////////////////////////////
/// class CTreeTriFanNode
/// 
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CTreeTriFanNode : public CGlVboNode
{
public:
    /// 
    CTreeTriFanNode() : m_LabelVis(false) {}

    /// Delete all buffers
    virtual ~CTreeTriFanNode() {}

    void CreateVbo(size_t numSements);

    /// Set default label visibility (if false, no labels rendered, if true, labels
    /// which are turned on and visible are rendered)
    void SetLabelVisibility(bool b) { m_LabelVis = b; }
    bool GetLabelVisibility() const { return m_LabelVis; }
    
protected:

    /// If false, no labels are rendered, if true, designated labels rendered
    bool m_LabelVis;
};

////////////////////////////////////////////////////////////////////////////////
/// class CTreeGraphicsModel
/// Model for rendering tree graphics data.  Only holds data currently needed
/// to render the model.  Actual rendering is all done by objects derived from
/// CGlRenderNode.  Where there is a lot to render (nodes, edges) that is done
/// by vertex buffer objects.  Where there is less (marker nodes) that may be 
//  a simple class derived from CGlRenderNode for that purpose alone.
/// 
/// For nodes and edges in the model, they are currently first copied to arrays
/// (m_Nodes and m_Edges) prior to rendering.  This allows the data to be shared
/// with collision detection.
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT  CTreeGraphicsModel : public CGlModel2D
{
public:
    CTreeGraphicsModel();
    virtual ~CTreeGraphicsModel();

    /// Allocate any default buffers etc.
    void Init(CPhyloTree* t);

    /// Return a node that can be used to represent a collapsed tree
    CGlVboNode* AddTempCollapsedNode(const vector<CVect2<float> >& vb);

    /// Draw all the nodes
    virtual void Render(CGlPane& pane);

    /// Draw to PDF file
    virtual void RenderPDF(CGlPane& pane, CRef<CPdf>& pdf);

    /// Update buffers as needed.
    virtual void SyncBuffers();

    /// Clear arrays prior to re-synching with the model
    virtual void ClearArrays();

    /// Called for circular layout - enables trimming of non-rotated labels
    /// based on proximity to adjacent leaf nodes in the circle
    void EnableCircularLabelTrimming(const CVect2<float>& cladogram_center);

    /// Return node's label rotation angle for current zoom level. (Circular view
    /// rotates less as you zoom out and that is computed in CTreeGraphicsModel)
    float GetCurrentRotationAngle(const CGlPane& pane,
                                  const CPhyloTreeNode& n,
                                  bool& visible) const;

    /// Set rendering scheme for tree (some nodes need this to render)
    void  SetScheme(CPhyloTreeScheme& sl);

    /// Get the tree (to which the node indices belong)
    CPhyloTree* GetTree() { return m_Tree; }

    /// Get all the nodes (indicies point to nodes in phylo tree)
    vector<size_t>& GetNodeIndices() { return m_NodeIndices; }

    /// Get color table used a 1D texture map for edges and nodes
    CRgbaGradColorTable* GetColorTable() { return m_ColorTable.GetPointer(); }

    CTreeBoundaryNode* GetBoundaryNode() { return m_BoundaryNode; }
    CTreeTriFanNode* GetTriFanNode() { return m_TriFanNode; }

    CTreeCollisionModel2D& GetCollisionData() { return m_CollisionData; }
    const CTreeCollisionModel2D& GetCollisionData() const { return m_CollisionData; }

    void SetRotateLabels(bool b) { m_RotateLabels = b; }
    bool GetRotateLabels() const { return m_RotateLabels; }

    void SetRenderTooltips(bool b) { m_RenderPdfTooltips = b; }
    bool GetRenderTooltips() const { return m_RenderPdfTooltips; }

protected:
    void x_RenderNodes(CGlPane& pane,
                       const CVect3<float>& scale);
    void x_RenderNodeLabelsPdf(CGlPane& pane,
                               CRef<CPdf>& pdf,
                               CVect3<float> scale);
    void x_RenderNodesPdf(CGlPane& pane,
                          CRef<CPdf>& pdf,
                          CVect3<float> scale);

    void x_RenderBoundariesPdf(CGlPane& pane,
                          CRef<CPdf>& pdf,
                          const CVect3<float>& scale);

    CVect2<float> x_Project(CVect2<float>& p);

    void x_CircularLabelTrim(const CGlPane& pane,
                             const CVect3<float>& scale,
                             CPhyloTreeNode &n,
                             bool truncate_labels);

    void x_CircularLabelMinRotate(const CGlPane& pane,
                                  const CVect3<float>& scale,
                                  CPhyloTreeNode &n,
                                  bool truncate_labels);

    float x_CircularLabelMinAngle(const CGlPane& pane,
                                  const CVect3<float>& scale,
                                  const CPhyloTreeNode &n,
                                  bool& visible) const;

    //spatial 2d subdivision
    CTreeCollisionModel2D m_CollisionData;

    vector<size_t> m_NodeIndices;

    CTreeBoundaryNode* m_BoundaryNode;
    CTreeTriFanNode* m_TriFanNode;

    CPhyloTree* m_Tree;
    CRef<CPhyloTreeScheme> m_SL;
    CRef<CRgbaGradColorTable> m_ColorTable;

    bool m_RotateLabels;

    bool m_RenderPdfTooltips;

    // Triangle texture for collapsed nodes
    CIRef<I3DTexture> m_CollapseTexture;

    /// These are specialized for rendering circular cladograms.  It gives
    /// info to approximate positions of adjacent labels so that labels
    /// can be trimmed and rotation can be done in a minimal fashion
    /// This is automatically disabled when ClearArrays is called.
    bool m_EnableCircularLabelTrimming;
    CVect2<float> m_CladogramCenter;
    float m_DeltaTheta;
    
public:
    float m_RenderTime;
    float m_LineRenderTime;
    float m_NodesRenderTime;
    float m_BoundaryRenderTime;
    int m_NodeRenderCount;
    float m_TotalTime;
    float m_CDUpdateTime;
};

END_NCBI_SCOPE

#endif // __GUI_WIDGETS_PHYLO_TREE___GRAPHICS_MODEL__HPP
