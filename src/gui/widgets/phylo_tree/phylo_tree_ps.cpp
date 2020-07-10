/*  $Id: phylo_tree_ps.cpp 33766 2015-09-15 19:26:48Z falkrb $
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
 *      Force tree layout
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbitime.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_ps.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>

#include <gui/widgets/gl/attrib_menu.hpp>

#include <corelib/ncbitime.hpp>
#include <corelib/ncbistl.hpp>

#include <cmath>

//#define NUMERIC_ERROR_CHECK  
template<typename T> bool isFinite(T arg)
{
    return arg == arg && 
           arg != std::numeric_limits<T>::infinity() &&
           arg != -std::numeric_limits<T>::infinity();
}

BEGIN_NCBI_SCOPE

//#define VALIDATE_NODE_NODE_FORCES 1

CPhyloTreePS::PhysicsParms::PhysicsParms()
: m_ElectricalRepulsion(100.0f)
, m_Step(0.015f)
, m_Damping(0.89f)
, m_EdgeK(250.0f)
, m_RepulsionDist(350.0f)
, m_VelocityThresholdK(0.002f)
{
}

CPhyloTreePS::CPhyloTreePS(CPhyloTreeDataSource& ds)
: m_AdaptiveStep(1.0f)
, m_MaxVelocity(1.0f)
, m_PrevMaxVelocity(1.0)
, m_IsDone(false)
, m_Tree(NULL)
{  
    // Set up interactive menus for debugging.  
#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenu& m = CAttribMenuInstance::GetInstance();

    CAttribMenu* sub_menu = m.AddSubMenuUnique("PhyloTreePS", this);
   
    sub_menu->AddFloat("Step Size", 
        &m_PhysicsParmsVolatile.m_Step, 0.02f, 0.000001f, 1.0f, 0.01f);
    sub_menu->AddFloat("Damping", 
        &m_PhysicsParmsVolatile.m_Damping, 0.88f, 0.0f, 1.0f, 0.01f);
    sub_menu->AddFloat("Edge K", 
        &m_PhysicsParmsVolatile.m_EdgeK, 300.0f, 0.0f, 1000.0f, 10.0f);
    sub_menu->AddFloat("Repulsion", 
        &m_PhysicsParmsVolatile.m_ElectricalRepulsion, 100.0f, 0.0f, 500.0f, 5.0f);
    sub_menu->AddFloat("Rep Dist", 
        &m_PhysicsParmsVolatile.m_RepulsionDist, 300.0f, 0.0f, 10000.0f, 20.0f);
    sub_menu->AddFloat("Vel Threshold K", 
        &m_PhysicsParmsVolatile.m_VelocityThresholdK, 0.003f, 0.0f, 1.0f, 0.001f);  

    sub_menu->AddFloatReadOnly("Time Edge F", &m_edge_forces_safe_t);
    sub_menu->AddFloatReadOnly("Time Node F", &m_node_forces_safe_t);
    sub_menu->AddFloatReadOnly("Time Int", &m_integrate_safe_t);
    sub_menu->AddFloatReadOnly("Time Vox Update", &m_bound_update_safe_t);
    sub_menu->AddIntReadOnly("Node-Node Int", &m_node_node_interactions_safe_t);
    sub_menu->AddFloatReadOnly("Maximum Velocity", &m_MaxVelocity);
    sub_menu->AddFloatReadOnly("Adaptive Step", &m_AdaptiveStep);
#endif   

    Init(ds);
}


CPhyloTreePS::~CPhyloTreePS()
{
#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenu& m = CAttribMenuInstance::GetInstance();
    m.RemoveMenuR("PhyloTreePS", this);
#endif

    Clear();
}

void CPhyloTreePS::Clear()
{
    m_Nodes.clear();
    m_Edges.clear();
    m_MaxVelocity = 1.0f;
    m_PrevMaxVelocity = 1.0f;
    m_AdaptiveStep = 1.0f;
    m_IsDone = false;

    // This is taken care of in Init()
    //m_NodeGrid.Clear();

#ifdef VALIDATE_NODE_NODE_FORCES
    m_ValidateGrid.clear();
    m_ValidateNsq.clear();
#endif
}

void CPhyloTreePS::Init(CPhyloTreeDataSource& ds)
{    
    Clear();

    m_Tree = ds.GetTree();

    m_DefaultEdgeLen = 0.0f;

    m_MinPos.X() = std::numeric_limits<float>::max();
    m_MinPos.Y() = std::numeric_limits<float>::max();
    m_MaxPos.X() = -std::numeric_limits<float>::max();
    m_MaxPos.Y() = -std::numeric_limits<float>::max();

    x_Init(ds.GetTree());

    x_UpdateVoxels();

    m_DefaultEdgeLen /= (float)m_Edges.size();

    // Compute edge forces between nodes
    std::vector<Edge>::iterator eiter;
    
    for (eiter=m_Edges.begin(); eiter!=m_Edges.end(); ++eiter) {
        (*eiter).rest_len_inv = 1.0f/m_DefaultEdgeLen;
    }
}

void CPhyloTreePS::x_ApplyNeighborCellForces(std::vector<int>& cell_nodes, 
                                             const CVect2<int>& adjacent_idx)
{
    std::vector<int>& neighbor_nodes = m_NodeGrid.Get(adjacent_idx);

    // For all nodes in a pair of cells, compute the forces between them
    for (size_t i=0; i<cell_nodes.size(); ++i) {
        for (size_t j=0; j<neighbor_nodes.size(); ++j) {
            int idx1 = cell_nodes[i];
            int idx2 = neighbor_nodes[j];

            TVec offset(m_Nodes[idx1].pos - m_Nodes[idx2].pos);

            float dist2 = offset.Length2();

            if (dist2 < m_RepulsionDist2 && dist2 > FLT_EPSILON) { 

                if (dist2 > FLT_EPSILON) {
                    float dist = sqrtf(dist2);

                    offset *= (m_PhysicsParmsSafe.m_ElectricalRepulsion-
                        m_RepulsionInv_x_ElectricalRepulsion*dist)/dist2;
                    m_Nodes[idx1].accel += offset;
                    m_Nodes[idx2].accel -= offset;

#ifdef NUMERIC_ERROR_CHECK                      
                    if (!isFinite( m_Nodes[idx1].accel.X()) || !isFinite( m_Nodes[idx1].accel.Y())) {
                        _TRACE("Bad float! " <<  m_Nodes[idx1].accel.X() << "  " <<  m_Nodes[idx1].accel.Y());
                    }
                    if (!isFinite( m_Nodes[idx2].accel.X()) || !isFinite( m_Nodes[idx2].accel.Y())) {
                        _TRACE("Bad float! " <<  m_Nodes[idx2].accel.X() << "  " <<  m_Nodes[idx2].accel.Y());
                    }
#endif                    


                    ++m_node_node_interactions_t;

#ifdef VALIDATE_NODE_NODE_FORCES
                    m_ValidateGrid.push_back( CVect2<int>(std::min(idx1, idx2), std::max(idx1, idx2) );
#endif
                }
                // for debugging
                //m_Nodes[idx1].force_nodes.push_back(idx2); 
                //m_Nodes[idx2].force_nodes.push_back(idx1);
            }
        }
    }
}

 void CPhyloTreePS::x_ApplyRepulsiveForcesHashed()
 {
    CVect2<int> min_pos = 0; //m_NodeGrid.GetMin();
    CVect2<int> max_pos = m_NodeGrid.GetMax() - m_NodeGrid.GetMin();
    
    // Iterate over the whole grid which subdivides the space occupied
    // by the tree into individual cells. For each cell, compute the 
    // forces between all the nodes in that cell, and then compute
    // the forces between nodes in that cell and nodes in adjacent
    // cells.  Node forces will not travel more than one cell since
    // we base cell size on the maximum distance for the the repulsion
    // force between cells.
    for (int x = min_pos.X(); x <= max_pos.X(); ++x) {
        for (int y = min_pos.Y(); y <= max_pos.Y(); ++y) {
            CVect2<int> idx(x,y);
            std::vector<int>& cell_nodes = m_NodeGrid.Get(idx);
            if (cell_nodes.size() > 0) {
                // Find collisions between nodes hashed in this entry
                for (size_t i=0; i<cell_nodes.size(); ++i) {
                    for (size_t j=i+1; j<cell_nodes.size(); ++j) {
                        
                        int idx1 = cell_nodes[i];
                        int idx2 = cell_nodes[j];

                        TVec offset(m_Nodes[idx1].pos - m_Nodes[idx2].pos);
        
                        float dist2 = offset.Length2();

                        // Compute forces between one pair of nodes
                        if (dist2 < m_RepulsionDist2 && dist2 > FLT_EPSILON) {
                            float dist = sqrtf(dist2);

                            offset *= (m_PhysicsParmsSafe.m_ElectricalRepulsion-
                                       m_RepulsionInv_x_ElectricalRepulsion*dist)/dist2;
                            m_Nodes[idx1].accel += offset;
                            m_Nodes[idx2].accel -= offset;

#ifdef NUMERIC_ERROR_CHECK                            
                            if (!isFinite( m_Nodes[idx1].accel.X()) || !isFinite( m_Nodes[idx1].accel.Y())) {
                                _TRACE("Bad float! " <<  m_Nodes[idx1].accel.X() << "  " <<  m_Nodes[idx1].accel.Y());
                            }

                            if (!isFinite( m_Nodes[idx2].accel.X()) || !isFinite( m_Nodes[idx2].accel.Y())) {
                                _TRACE("Bad float! " <<  m_Nodes[idx2].accel.X() << "  " <<  m_Nodes[idx2].accel.Y());
                            }
#endif


                            ++m_node_node_interactions_t;

#ifdef VALIDATE_NODE_NODE_FORCES
                              m_ValidateGrid.push_back( CVect2<int>(std::min(idx1, idx2), std::max(idx1, idx2) );
#endif

                            //m_Nodes[idx1].force_nodes.push_back(idx2);  // for debugging
                            //m_Nodes[idx2].force_nodes.push_back(idx1);
                        }                    
                    }
                }


                // Find all collisions between nodes in this entry and nodes
                // in the adjacent cells idx(x+1,y), idx(x+1,y+1), idx(x,y+1), 
                // idx(x-1,y+1). Only look at nodes in positive direction so that
                // you don't compute forces between adjacent cells twice (once 
                // when you visit each cell)
                if (x+1 < m_NodeGrid.GetWidth()) {
                    x_ApplyNeighborCellForces(cell_nodes, CVect2<int>(x+1,y));

                    if (y+1 < m_NodeGrid.GetHeight())
                        x_ApplyNeighborCellForces(cell_nodes, CVect2<int>(x+1,y+1));
                }

                if (y+1 < m_NodeGrid.GetHeight()) {
                    x_ApplyNeighborCellForces(cell_nodes, CVect2<int>(x,y+1));

                    if (x > 0)
                        x_ApplyNeighborCellForces(cell_nodes, CVect2<int>(x-1,y+1));
                }

            }
        }
    }
 }

// Simple global form of repulsive forces.  Has better visual result still,
// but is quite a bit slower.
void CPhyloTreePS::x_ApplyRepulsiveForces()
{
    // Iterate over all nodes and compute forces between them
    for (size_t i=0; i<m_Nodes.size(); ++i) {             
        for (size_t j=i+1; j<m_Nodes.size(); ++j) {                        
            TVec offset(m_Nodes[i].pos - m_Nodes[j].pos);

            float dist2 = offset.Length2();
            if (dist2 > FLT_EPSILON) {              
                //offset *= (m_ElectricalRepulsion-m_RepulsionInv_x_ElectricalRepulsion*dist)/(dist2+ + FLT_EPSILON);
                offset *=  m_PhysicsParmsSafe.m_ElectricalRepulsion/dist2;
                m_Nodes[i].accel += offset;
                m_Nodes[j].accel -= offset;

#ifdef NUMERIC_ERROR_CHECK  
                if (!isFinite( m_Nodes[i].accel.X()) || !isFinite( m_Nodes[i].accel.Y())) {
                    _TRACE("Bad float! " <<  m_Nodes[i].accel.X() << "  " <<  m_Nodes[i].accel.Y());
                }
                if (!isFinite( m_Nodes[j].accel.X()) || !isFinite( m_Nodes[j].accel.Y())) {
                    _TRACE("Bad float! " <<  m_Nodes[j].accel.X() << "  " <<  m_Nodes[j].accel.Y());
                }
#endif

                ++m_node_node_interactions_t;

#ifdef VALIDATE_NODE_NODE_FORCES
                m_ValidateGrid.push_back( CVect2<int>(std::min(idx1, idx2), std::max(idx1, idx2) );
#endif
            }
        }
     }
}


void CPhyloTreePS::CalcForces()
{
#ifdef VALIDATE_NODE_NODE_FORCES
    m_ValidateGrid.clear();
    m_ValidateNsq.clear();
#endif


    m_RepulsionDist2 = m_PhysicsParmsSafe.m_RepulsionDist * 
                       m_PhysicsParmsSafe.m_RepulsionDist;
    m_RepulsionInv_x_ElectricalRepulsion = (1.0f/m_PhysicsParmsSafe.m_RepulsionDist) *
                                           m_PhysicsParmsSafe.m_ElectricalRepulsion;

    // Compute edge forces between nodes
    std::vector<Edge>::iterator eiter;

    m_LogDistMax = -500.0f;
    m_LogDistMin = 500.0f;

    CStopWatch sw;
    m_node_node_interactions_t = 0;

    sw.Start();
    
    // Compute forces from all edges
    for (eiter=m_Edges.begin(); eiter!=m_Edges.end(); ++eiter) {
        Node& n1 = m_Nodes[(*eiter).from_idx];
        Node& n2 = m_Nodes[(*eiter).to_idx];

        TVec offset(n1.pos - n2.pos);
        float dist = offset.Length();
        (*eiter).len = dist;

        if (dist > FLT_EPSILON) {
            float ldist = logf(dist*(*eiter).rest_len_inv);
            //m_LogDistMax = std::max(ldist, m_LogDistMax);
            //m_LogDistMin = std::min(ldist, m_LogDistMin);
            offset *= (1.0f/dist)*ldist*m_PhysicsParmsSafe.m_EdgeK;

            n1.accel -= offset;
            n2.accel += offset;

#ifdef NUMERIC_ERROR_CHECK  
            if (!isFinite( n1.accel.X()) || !isFinite( n1.accel.Y())) {
                _TRACE("Bad float! " <<  n1.accel.X() << "  " <<  n1.accel.Y());
            }
            if (!isFinite( n2.accel.X()) || !isFinite( n2.accel.Y())) {
                _TRACE("Bad float! " <<  n2.accel.X() << "  " <<  n2.accel.Y());
            }
#endif
        }
 
    }

    sw.Stop();
    m_edge_forces_t = sw.Elapsed()*1000.0f;
    

    // Compute repulsion forces between all nodes  (n^2 version)
#ifdef VALIDATE_NODE_NODE_FORCES                             
    std::vector<Node>::iterator niter1, niter2;
    for (niter1=m_Nodes.begin(); niter1!=m_Nodes.end(); ++niter1) {
        TVec p1 = (*niter1).pos;

        for (niter2=niter1+1; niter2!=m_Nodes.end(); ++niter2) {
            TVec p2 = (*niter2).pos;
            TVec offset(p1-p2);

            //float dist = offset.Length() + 0.00001f;

            float dist2 = offset.Length2();
            if (dist2 < m_RepulsionDist2) {      
                int idx1 = niter1-m_Nodes.begin();
                int idx2 = niter2-m_Nodes.begin();
                m_ValidateNsq.push_back( CVect2<int>(std::min(idx1, idx2), std::max(idx1, idx2) );
                /*
                float dist = sqrtf(dist2);
                float mult_factor = (1.0f/dist)* (m_ElectricalRepulsion/dist);

                offset *= mult_factor;

                (*niter1).accel += offset;
                (*niter2).accel -= offset;
            */
            }
        }
    }    
#endif

   
    sw.Restart();

    // Compute repulsive forces between nodes
    //x_ApplyRepulsiveForces();
    x_ApplyRepulsiveForcesHashed();

    m_node_forces_t = sw.Elapsed()*1000.0f;
}



void CPhyloTreePS::Update()
{
    {
        // Throttle step size if velocity is too high.
        // Not really proper adaptive time stepping, but it should
        // keep things from exploding        
        if (m_MaxVelocity > 20.0f &&
            m_MaxVelocity > m_PrevMaxVelocity && 
            m_AdaptiveStep > 0.01f) {
            m_AdaptiveStep = m_AdaptiveStep*0.95f;
        }
        else if (m_AdaptiveStep < 1.0f &&
                 m_MaxVelocity < 20.0f &&
                 m_MaxVelocity < m_PrevMaxVelocity) {
            m_AdaptiveStep += (1.0f - m_AdaptiveStep)*0.01f;
        }        
    }

    // Compute all forces
    CalcForces(); 

    // Reset tree extent, so that we can recompute it as
    // we update the system.  Extent is neede for our spatial
    // collision grid each update.
    m_MinPos.X() = std::numeric_limits<float>::max();
    m_MinPos.Y() = std::numeric_limits<float>::max();
    m_MaxPos.X() = -std::numeric_limits<float>::max();
    m_MaxPos.Y() = -std::numeric_limits<float>::max();

    CStopWatch sw;
    sw.Start();

    // This will reduce step size if we seem unstable.
    float step = m_PhysicsParmsSafe.m_Step*m_AdaptiveStep;
    
    // Integrate forces into position updates
    std::vector<Node>::iterator niter;
    for (niter=m_Nodes.begin(); niter!=m_Nodes.end(); ++niter) {
        // pos = pos + (pos-prev_pos)*damping + accel*dt;
        TVec prev_offset((*niter).pos - (*niter).prev_pos);
        (*niter).prev_pos = (*niter).pos;
    
        (*niter).pos.X() += (prev_offset.X()*m_PhysicsParmsSafe.m_Damping + 
            (*niter).accel.X()*step)*(*niter).constrained;
        (*niter).pos.Y() += (prev_offset.Y()*m_PhysicsParmsSafe.m_Damping + 
            (*niter).accel.Y()*step)*(*niter).constrained;

#ifdef NUMERIC_ERROR_CHECK  
        if (!isFinite((*niter).pos.X()) || !isFinite((*niter).pos.Y())) {
            _TRACE("Bad float! " << (*niter).pos.X() << "  " << (*niter).pos.Y());
        }
#endif

        m_MinPos.X() = std::min(m_MinPos.X(), (*niter).pos.X());
        m_MinPos.Y() = std::min(m_MinPos.Y(), (*niter).pos.Y());
        m_MaxPos.X() = std::max(m_MaxPos.X(), (*niter).pos.X());
        m_MaxPos.Y() = std::max(m_MaxPos.Y(), (*niter).pos.Y());        

        (*niter).accel = TVec( TVec::TVecType(0) );       
        
        //(*niter).prev_force_nodes = (*niter).force_nodes;
        //(*niter).force_nodes.clear();     
    };

    sw.Stop();
    m_integrate_t = sw.Elapsed()*1000.0f;

    sw.Restart();
    // Update spatial data structure
    x_UpdateVoxels();
    m_bound_update_t  = sw.Elapsed()*1000.0f;
}

// Do the same thing as update, but also update the underlying
// data structure.
void CPhyloTreePS::UpdateAndSynch()
{   
    // This function is thread-safe, so update parms here:
    m_PhysicsParmsSafe = m_PhysicsParmsVolatile;

    CalcForces();    

    float max_velocity_squared = 0.0f;

    m_MinPos.X() = std::numeric_limits<float>::max();
    m_MinPos.Y() = std::numeric_limits<float>::max();
    m_MaxPos.X() = -std::numeric_limits<float>::max();
    m_MaxPos.Y() = -std::numeric_limits<float>::max();

    float step = m_PhysicsParmsSafe.m_Step*m_AdaptiveStep;

    // Integrate and reset 
    std::vector<Node>::iterator niter;
    for (niter=m_Nodes.begin(); niter!=m_Nodes.end(); ++niter) {
        // pos = pos + (pos-prev_pos)*damping + accel*dt; 
        TVec prev_offset((*niter).pos - (*niter).prev_pos);
        (*niter).prev_pos = (*niter).pos;       
            
        float xoff = (prev_offset.X()*m_PhysicsParmsSafe.m_Damping + 
            (*niter).accel.X()*step)*(*niter).constrained;
        float yoff = (prev_offset.Y()*m_PhysicsParmsSafe.m_Damping + 
            (*niter).accel.Y()*step)*(*niter).constrained;

        max_velocity_squared = std::max(max_velocity_squared, xoff*xoff + yoff*yoff);

        (*niter).pos.X() += xoff;
        (*niter).pos.Y() += yoff;

        m_MinPos.X() = std::min(m_MinPos.X(), (*niter).pos.X());
        m_MinPos.Y() = std::min(m_MinPos.Y(), (*niter).pos.Y());
        m_MaxPos.X() = std::max(m_MaxPos.X(), (*niter).pos.X());
        m_MaxPos.Y() = std::max(m_MaxPos.Y(), (*niter).pos.Y());

        // update node....

        (*m_Tree)[(*niter).tree_node_idx]->XY() = (*niter).pos;

        // Need angle to compute label positions
        CPhyloTree::TTreeIdx pidx = (*m_Tree)[(*niter).tree_node_idx].GetParent();
        if (pidx != CPhyloTree::Null()) {
            CVect2<float> ppos = (*m_Tree)[pidx]->XY();
            
            // For how its currently used we just set hard coded angles
            // based on whether x is positive or negative since we are only deciding
            // if label is on left or right side
            (*m_Tree)[(*niter).tree_node_idx]->SetAngle(
                ((*niter).pos.X()-ppos.X() >= 0.0f) ? 0.0f : 3.1415927f);
        }
		
        (*niter).accel = TVec( TVec::TVecType(0) );
    };

    m_PrevMaxVelocity = m_MaxVelocity;
    m_MaxVelocity = sqrtf(max_velocity_squared)/m_PhysicsParmsSafe.m_Step;

    /// It would be better to filter this result (probably filter x frames of
    /// velocity to search for outliers and make sure average is good)
    if (!m_IsDone) {
        float node_count = (float)m_Nodes.size();
        float threshold = m_PhysicsParmsSafe.m_VelocityThresholdK*((node_count+100.0f)/5.0f);     
        threshold = std::min(10.0f, threshold);
        //_TRACE("Threshold: " << threshold << " max_velocity: " << m_MaxVelocity << " velocity threshold: " << m_PhysicsParmsSafe.m_VelocityThresholdK);
        if (m_AdaptiveStep > 0.9f &&
            m_MaxVelocity < threshold) {
            m_IsDone = true;
        }
    }

    m_edge_forces_safe_t = m_edge_forces_t;
    m_node_forces_safe_t = m_node_forces_t;
    m_integrate_safe_t = m_integrate_t;
    m_bound_update_safe_t = m_bound_update_t;
    m_node_node_interactions_safe_t = m_node_node_interactions_t;

    x_UpdateVoxels();
}

void CPhyloTreePS::Draw()
{
    std::vector<Edge>::iterator eiter;

    
    glLineWidth(2.0f);
    glBegin(GL_LINES);

    for (eiter=m_Edges.begin(); eiter!=m_Edges.end(); ++eiter) {
        Node& n1 = m_Nodes[(*eiter).from_idx];
        Node& n2 = m_Nodes[(*eiter).to_idx];

        TVec offset(n1.pos - n2.pos);
        float dist = offset.Length();      
        float ldist = logf(dist*(*eiter).rest_len_inv);       
        
        if (ldist >= 0.0f)
            glColor3f(std::max(0.0f, ldist/m_LogDistMax), 0.0f, 0.0f);
        else
            glColor3f(0.0f, 0.0f, std::max(0.0f, std::abs(ldist/m_LogDistMin)));

        glVertex2fv(n1.pos.GetData());
        glVertex2fv(n2.pos.GetData());
    }
    
    glEnd();
    
   
    glPointSize(5.0f);
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_POINTS);
    std::vector<Node>::iterator niter;
    /*
    for (niter=m_Nodes.begin(); niter!=m_Nodes.end(); ++niter) {        
        if ( (*m_Tree)[*niter]->GetSelectedState() == CPhyloNodeData::eSelected) {                     
            glBegin(GL_LINES);
            for (size_t j=0; j<(*niter).prev_force_nodes.size(); ++j) {
                int idx = (*niter).prev_force_nodes[j];
                TVec p1 = (*niter).pos;
                TVec p2 = m_Nodes[idx].pos;
                glVertex2fv(p1.GetData());
                glVertex2fv(p2.GetData());
            }
            glEnd();            

            glLineWidth(1.0f);
        }
        if ( !(*niter).constrained ) {
            glVertex2fv((*niter).pos.GetData());
        }
    }    
    */
    glEnd();

#ifdef VALIDATE_NODE_NODE_FORCES
    //m_ValidateGrid.clear();
    //m_ValidateNsq.clear();
#endif
}


void CPhyloTreePS::x_UpdateVoxels()
{      
    // Reset size of collision grid based on current  node extent
    CVect2<int> min_idx(int(m_MinPos[0]/m_PhysicsParmsSafe.m_RepulsionDist), 
                        int(m_MinPos[1]/m_PhysicsParmsSafe.m_RepulsionDist));
    CVect2<int> max_idx(int(m_MaxPos[0]/m_PhysicsParmsSafe.m_RepulsionDist), 
                        int(m_MaxPos[1]/m_PhysicsParmsSafe.m_RepulsionDist));

    m_NodeGrid.SetMax(max_idx);
    m_NodeGrid.SetMin(min_idx);

    // Resize and clear spatial grid
    m_NodeGrid.ResizeFast();
    m_NodeGrid.Clear();

    // Put all nodes into the correct places in the grid
    for (size_t j=0; j<m_Nodes.size(); ++j) {
        TVec pos = m_Nodes[j].pos;
        CVect2<int> posi(int(pos[0]/m_PhysicsParmsSafe.m_RepulsionDist) - min_idx.X(), 
                         int(pos[1]/m_PhysicsParmsSafe.m_RepulsionDist) - min_idx.Y());

        m_NodeGrid.Get(posi).push_back(j);
    }
}

class CInitPSNodes
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    CInitPSNodes(CPhyloTreePS* ps)
    : m_PS(ps)
    {
    }

    ETreeTraverseCode operator()(CPhyloTree& tree, 
                                 TTreeIdx node_idx, int delta)
    {
        if (delta==1 || delta==0) {
            CPhyloTree::TNodeType& tnode = tree[node_idx];
            CPhyloTreePS::Node n;
            n.pos = tnode->XY();
            n.prev_pos = n.pos;    
            n.accel = 0.0f;       
            n.constrained = 1.0f;
            n.tree_node_idx = node_idx;
            n.is_leaf = tnode.IsLeaf();

            m_PS->GetMinPos().X() = std::min(m_PS->GetMinPos().X(), n.pos.X());
            m_PS->GetMinPos().Y() = std::min(m_PS->GetMinPos().Y(), n.pos.Y());
            m_PS->GetMaxPos().X() = std::max(m_PS->GetMaxPos().X(), n.pos.X());
            m_PS->GetMaxPos().Y() = std::max(m_PS->GetMaxPos().Y(), n.pos.Y());

            // Parent index is index in new (force) array, not tree
            // since tree may have collapsed nodes or unused nodes.
            if (delta == 1) {
                int pt_idx = (int) m_PS->GetNodes().size()-1;
                m_ParentIdx.push(pt_idx);
            }

            /// Add the edge from the current node to its parent
            if (tnode.HasParent()) {
                n.constrained = 1.0f;
                CVect2<float> ppos(tree.GetParent(tnode)->XY());

                float len = (n.pos-ppos).Length();
                m_PS->GetDefaultEdgeLen() += len;

                CPhyloTreePS::Edge e(m_ParentIdx.top(), m_PS->GetNodes().size());
                e.len = len;
                e.rest_len_inv = 1.0f/100.0f;
                m_PS->GetEdges().push_back(e);
            }

             m_PS->GetNodes().push_back(n);
        }
        else if (delta == -1) {
            m_ParentIdx.pop();
        }
        return eTreeTraverse;
    }

private:
    CPhyloTreePS* m_PS;
    stack<int> m_ParentIdx;
};

void CPhyloTreePS::x_Init(CPhyloTree* tree)
{    
    CInitPSNodes  draw_tree(this);
    TreeDepthFirstEx(*tree, draw_tree);
}


END_NCBI_SCOPE
