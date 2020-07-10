#ifndef GUI_WIDGETS_PHYLO_TREE_TREE_COLLISION_MODEL_2D_HPP
#define GUI_WIDGETS_PHYLO_TREE_TREE_COLLISION_MODEL_2D_HPP

/*  $Id: tree_collision_model2d.hpp 37891 2017-02-28 20:50:36Z falkrb $
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/opengl/spatialhash2d.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>

#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>

#include <float.h>

BEGIN_NCBI_SCOPE

class CTreeCollisionModel2D;
class CPhyloTreeNode;

////////////////////////////////////////////////////////////////////////////////
/// class CollisionEntry
/// This is the class that is stored in the collision data structure - each 
/// entry in the collision data structure has one of these which stores the
/// set of geometry that overlaps that (2d) spatial region.
struct CollisionEntry {
    void clear() { m_Labels.clear(); }
    
    vector<size_t> m_Labels;
};

class CTreeGraphicsModel;

////////////////////////////////////////////////////////////////////////////////
/// class CTreeCollisionModel2D
/// This is a subclass of the spatial collision detection datastructure
/// specialized to support data important to (phylogenetic) trees.  Currently
/// it is focused on the labels in the tree.
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT  CTreeCollisionModel2D : public CSpatialHash2D<CollisionEntry>
{
public:
    CTreeCollisionModel2D();
    ~CTreeCollisionModel2D();

    /// Update all geometry (needed when topology changes)
    void Update(const CGlPane& pane, const CVect2<float>& scale);
    /// Only update scaled geometry (needed when zoom changes)
    void UpdateScaled(const CGlPane& pane, const CVect2<float>& scale);

    /// Insert data from model (labels/nodes)
    void Sync(const CGlPane& pane, CTreeGraphicsModel* m);
    /// Set rendering scheme for tree (has info needed for node sizes)
    void  SetScheme(CPhyloTreeScheme& sl);

    /// Find collisions between labels in visible regions to determine visiblity
    virtual void UpdateVisibility(const TModelRect& r);

    /// Utility function returns true if rectangle 
    /// one (ll1-ur1) intersects rectangle 2 (ll2-ur2)
    static bool RectIntersect(const CVect2<float>& ll1,
                              const CVect2<float>& ur1,
                              const CVect2<float>& ll2,
                              const CVect2<float>& ur2);

    /// Return node at position (x,y), if any
    size_t TestForNode(float x, float y, bool labels_visible, bool rotated_labels=false);
    /// Return all nodes within the region (x1,y1) - (x2,y2)
    vector<size_t> SelectNodes(float x1, float y1, float x2, float y2);

protected:
    /// Underlying graphics model
    CTreeGraphicsModel* m_Model;
    CRef<CPhyloTreeScheme> m_SL;

    float m_VisUpdateTime;
    float m_ScaleUpdateTime;
};

END_NCBI_SCOPE




#endif // GUI_WIDGETS_PHYLO_TREE_TREE_COLLISION_MODEL_2D_HPP