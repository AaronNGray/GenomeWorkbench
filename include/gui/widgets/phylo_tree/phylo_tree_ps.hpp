#ifndef __GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_PS__HPP
#define __GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_PS__HPP

/*  $Id: phylo_tree_ps.hpp 33847 2015-09-21 12:59:45Z falkrb $
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

#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>

#include <gui/opengl/spatialhash2d.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>

#include <float.h>

BEGIN_NCBI_SCOPE

class CPhyloTreeDataSource;

////////////////////////////////////////////////////////////////////////////////
/// class CPhyloTreePS
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT  CPhyloTreePS                        
{
public:
    typedef CVect2<float> TVec;

    //
    // Physics parameters - may be updated by another thread
    // 
    struct PhysicsParms { 
        PhysicsParms(); 

        float m_ElectricalRepulsion;
        float m_Step;
        float m_Damping;
        float m_EdgeK;
        float m_RepulsionDist;
        /// factor to scale final velocity threshold
        float m_VelocityThresholdK;
    };

public:
    CPhyloTreePS(CPhyloTreeDataSource& ds);
    ~CPhyloTreePS();

    void Clear();

    void Init(CPhyloTreeDataSource& ds);

    ///
    /// Get/Set particle system parameters.  Two sets of parms are specified
    /// (volatile and safe) so that other threads can access parms while
    /// particle system is running.
    ///

    /// Get/Set node-node repulsion force (between all nodes)
    void SetElectricalRepulsion(float r) 
        { m_PhysicsParmsVolatile.m_ElectricalRepulsion = r; }
    float GetElectricalRepulsion() const 
        { return m_PhysicsParmsVolatile.m_ElectricalRepulsion; }

    /// Get/Set timestep
    void SetStep(float s) { m_PhysicsParmsVolatile.m_Step = s; }
    float GetStep() const { return  m_PhysicsParmsVolatile.m_Step; }

    /// Get/Set damping parameter
    void SetDamping(float d) {  m_PhysicsParmsVolatile.m_Damping = d; }
    float GetDamping() const { return  m_PhysicsParmsVolatile.m_Damping; }

    /// Get/Set edge stiffness 
    void SetEdgeK(float k) {  m_PhysicsParmsVolatile.m_EdgeK = k; }
    float GetEdgeK() const { return  m_PhysicsParmsVolatile.m_EdgeK; }

    /// Get/Set maximum effective repulsion distance
    void SetRepulsionDist(float d) 
        {  m_PhysicsParmsVolatile.m_RepulsionDist = d; }
    float GetRepulsionDist() const 
        { return  m_PhysicsParmsVolatile.m_RepulsionDist; }

    /// Get/Set scaling factor for velocity threshold below which system stops
    void SetVelocityThresholdK(float k) 
        {  m_PhysicsParmsVolatile.m_VelocityThresholdK = k; }
    float GetVelocityThresholdK() const 
        { return  m_PhysicsParmsVolatile.m_VelocityThresholdK; }

    /// Get/Set all the current physics parms
    void SetPhysicsParms(const PhysicsParms& p) { m_PhysicsParmsVolatile = p; }
    PhysicsParms getPhysicParms() { return m_PhysicsParmsVolatile; }

    /// Get max velocity from last call to UpdateAndSynch
    float GetMaxVelocity() const { return m_MaxVelocity; }

    /// Return true if velocity has dropped below a threshold that
    /// indicates the system is stable (and doesn't need to keep running)
    bool IsDone() const { return m_IsDone; }

    /// Calculate forces for all nodes
    void CalcForces();

    /// Calculate force then update positions
    void Update();

    /// Calculate forces, update positions, and update underlying tree
    void UpdateAndSynch();

    /// Visualize graph - debug rendering
    void Draw();

    /// Data structure for an edge between two nodes
    struct Edge {
        Edge() : from_idx(-1), to_idx(-1), len(0.0f), rest_len_inv(0.0f), k(1.0f) {}
        Edge(int idx1, int idx2) 
            : from_idx(idx1), to_idx(idx2), len(0.0f), rest_len_inv(0.0f), k(1.0f) {}

        int from_idx;
        int to_idx;

        float len;
        float rest_len_inv;
        float k;
    };

    /// Data structure for a node in the particle system
    struct Node {
        Node() 
            : tree_node_idx(CPhyloTree::Null())            
            , constrained(0.0f) {}

        TVec pos;
        TVec prev_pos;
        TVec accel;
        CPhyloTree::TTreeIdx tree_node_idx;
        bool is_leaf;

        float constrained;
    };

    /// Default length for all edges in system
    float& GetDefaultEdgeLen() { return m_DefaultEdgeLen; }

    /// Tracks bounding rectangle for all nodes
    TVec& GetMinPos() { return m_MinPos; }
    TVec& GetMaxPos() { return m_MaxPos; }
    /// Set of all nodes
    std::vector<Node>& GetNodes() { return m_Nodes; }
    /// Set of all edges
    std::vector<Edge>& GetEdges() { return m_Edges; }

protected:

    /// Create particle system from tree - ignore collapsed
    void x_Init(CPhyloTree* tree);
    /// Update spatial subdivision of nodes 
    void x_UpdateVoxels();
    /// Apply forces between nodes based on defined neighborhood size
    void x_ApplyRepulsiveForcesHashed();
    /// Apply repulsive forces between all nodes
    void x_ApplyRepulsiveForces();
    /// Called compute forces between nodes in 2 cells
    void x_ApplyNeighborCellForces(std::vector<int>& cell_nodes, 
                                   const CVect2<int>& adjacent_idx);

    /// Set of all nodes
    std::vector<Node> m_Nodes;
    /// Set of all edges
    std::vector<Edge> m_Edges;

    /// Grid that keeps track of adjacent nodes
    CSpatialHash2D<std::vector<int> > m_NodeGrid;

    /// (potentially) updated from other thread(s)
    PhysicsParms m_PhysicsParmsVolatile;
    /// Updated from m_PhysicsPamsVolatile when safe to do so
    PhysicsParms m_PhysicsParmsSafe;

    /// Multiplier for m_PhysicsParmsSafe.m_Step to allow step size to be
    /// adaptively lowered if system appears unstable.
    float m_AdaptiveStep;

    /// Square of current effective repulsion distance
    float m_RepulsionDist2;
    /// Inverse of repulsion dist * electrical repulsion factor
    float m_RepulsionInv_x_ElectricalRepulsion;

    ///
    /// For timer values
    ///

    float m_edge_forces_t;
    float m_node_forces_t;
    float m_integrate_t;
    float m_bound_update_t;
    int   m_node_node_interactions_t;

    float m_edge_forces_safe_t;
    float m_node_forces_safe_t;
    float m_integrate_safe_t;
    float m_bound_update_safe_t;
    int   m_node_node_interactions_safe_t;

    /// The maximum velocity is the maximum node velocity during the last
    /// call to UpdateAndSynch()
    float m_MaxVelocity;
    /// Maximum velocity in previous update
    float m_PrevMaxVelocity;
    /// If true, system has slowed down to the point where continuing to
    /// update it is not necessary.  Based on max velocity and tree size
    bool  m_IsDone;


    /// Default length for all edges in system
    float m_DefaultEdgeLen;

    /// Root node of tree
    CPhyloTree* m_Tree;

    /// Tracks bounding rectangle for all nodes
    TVec m_MinPos;
    TVec m_MaxPos;

    /// Only used for debugging
    std::vector<CVect2<int> > m_ValidateGrid;
    std::vector<CVect2<int> > m_ValidateNsq;

    /// For debug-drawing
    float m_LogDistMax;
    float m_LogDistMin;

private:
    // default ctor not available
    CPhyloTreePS();
};


END_NCBI_SCOPE

#endif //__GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_PS__HPP
