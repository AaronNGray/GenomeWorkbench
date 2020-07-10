/*  $Id: phylo_tree_force.cpp 40280 2018-01-19 17:54:35Z katargir $
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

#include <ncbi_pch.hpp>
#include <corelib/ncbitime.hpp>
#include <corelib/ncbistl.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_force.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_pane.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_ps.hpp>
#include <gui/opengl/glhelpers.hpp>

#include <gui/widgets/gl/attrib_menu.hpp>

#include <gui/utils/event_translator.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/utils/view_event.hpp>


#include <math.h>

BEGIN_NCBI_SCOPE

#define PHYLO_RENDER_TIMER_ID 63001

BEGIN_EVENT_MAP(CPhyloForce, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CPhyloForce::OnAppJobNotification)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eProgress,
             &CPhyloForce::OnAppJobProgress)
END_EVENT_MAP()

BEGIN_EVENT_TABLE(CPhyloForce, CPhyloRadial)
    EVT_TIMER(PHYLO_RENDER_TIMER_ID, CPhyloForce::OnTimer)
END_EVENT_TABLE()


CPhyloForce::CPhyloForce()
: m_Timer(this, PHYLO_RENDER_TIMER_ID)
, m_MainThreadPS(NULL)
, m_UseMainThreadPhysics(true)
{
	m_JobID = CAppJobDispatcher::eInvalidJobID;

    // If rendering within gbench, use thread for force calculations,
    // otherwise we are probably cgi and should use main thread.
    if (CGuiRegistry::GetInstance().HasField("GBENCH")) {
        m_UseMainThreadPhysics = false;
    }
}

CPhyloForce::CPhyloForce(double w, double h) 
: CPhyloRadial(w, h)
, m_Timer(this, PHYLO_RENDER_TIMER_ID)
, m_MainThreadPS(NULL)
, m_UseMainThreadPhysics(true)
{
	m_JobID					= CAppJobDispatcher::eInvalidJobID;
    m_DistMode = false;

    // If rendering within gbench, use thread for force calculations,
    // otherwise we are probably cgi and should use main thread.
    if (CGuiRegistry::GetInstance().HasField("GBENCH")) {
        m_UseMainThreadPhysics = false;      
    }

#ifdef ATTRIB_MENU_SUPPORT
   // CAttribMenu& m = CAttribMenuInstance::GetInstance();

   // CAttribMenu* sub_menu = m.AddSubMenuUnique("Force ext", this);
   
   // sub_menu->AddFloatReadOnly("Vis Left", &vis_left);
   // sub_menu->AddFloatReadOnly("Vis Right", &vis_right);
   // sub_menu->AddFloatReadOnly("Vis Bottom", &vis_bottom);
   // sub_menu->AddFloatReadOnly("Vis Top", &vis_top);

   // sub_menu->AddFloatReadOnly("Ext Left", &ext_left);
   // sub_menu->AddFloatReadOnly("Ext Right", &ext_right);
   // sub_menu->AddFloatReadOnly("Ext Bottom", &ext_bottom);
   // sub_menu->AddFloatReadOnly("Ext top", &ext_top);
#endif  
   
}


CPhyloForce::~CPhyloForce()
{	
    if (m_JobID != CAppJobDispatcher::eInvalidJobID) {
        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        disp.DeleteJob(m_JobID);
    }

    m_Timer.Stop();
}

void  CPhyloForce::x_Layout(CPhyloTreeDataSource& ds)
{
    m_DistMode = false;

    // It is more efficient not to re-run the radial layout each time
    // if, for example, the user simply collapsed some nodes.  Unfortuately
    // some user operations wind up setting all node position values to 0,
    // so be be sure we have to compute the radial tree first each time.
    //if (m_SwitchedLayout)
    CPhyloRadial::x_Layout(ds);   

    Int4 leafs = ds.GetSize();
    Int4 width = ds.GetWidth();

    m_xStep = m_DimX / (double)width;
    // ystep is used to compute distance between nodes.  Since nodes are forced apart,
    // about 4 times size gives a better approximation of average distance between nodes.
    m_yStep = 4.0*m_DimY / (double)((leafs>1)?(leafs-1):2);

    if (ds.GetNormDistance() > 0){      
         m_NormDistance = m_DimX / ds.GetNormDistance();
    }

    m_Timer.Stop();

    CPhyloTreePS::PhysicsParms p;
    bool retain_parms = false;

    // Set drawing options that are specific to the force layout
    CGlVboNode* edge_node = ds.GetModel().FindGeomNode("TreeEdges");
    CGlVboNode* narrow_edge_node = ds.GetModel().FindGeomNode("NarrowTreeEdges");

    if (edge_node != NULL && narrow_edge_node != NULL) {   
        edge_node->GetState().Enable(GL_LINE_SMOOTH);
        edge_node->GetState().Enable(GL_BLEND);
        edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);        

        narrow_edge_node->GetState().Enable(GL_LINE_SMOOTH);
        narrow_edge_node->GetState().Enable(GL_BLEND);
        narrow_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        narrow_edge_node->GetState().ColorC(CRgbaColor(1.0f, 1.0f, 1.0f, 0.7f));
    }

    CGlVboNode* filler_points_node = m_DS->GetModel().FindGeomNode("FillerPoints");
    if (filler_points_node != NULL)
        filler_points_node->SetVisible(false);

    // Special case: no need for particle system if there is only
    // one node.
    if (ds.GetTree()->GetRoot().NumChildren() == 0) {
        TModelRect newRect = ds.GetBoundRect();

        newRect.Inflate(1.0f, 1.0f);
        m_RasterRect = newRect;

        // This handles single node case too.
        ComputeViewingLimits(*m_pPane, m_ForceSquare);

        return;
    }

    if (!m_UseMainThreadPhysics) {

        try {
            CForceJob* j = m_Job.GetPointerOrNull();

            // for debugging, copy the particle system parameters from the current 
            // system to the new one (parameters may be updated via attribute
            // menus).
            if (j != NULL) {
                p = j->GetPS()->getPhysicParms();
                retain_parms = true;
            }

            if (m_JobID != CAppJobDispatcher::eInvalidJobID) {
                CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
                IAppJob::EJobState state = disp.GetJobState(m_JobID);
                if (state == IAppJob::eRunning)
                    disp.CancelJob(m_JobID);
                else
                    disp.DeleteJob(m_JobID);

                m_Job.Reset();
            }

            j = m_Job.GetPointerOrNull();

            if (j == NULL) {
                // This is for debugging - if you run the force update several times,
                // you may change parameters each time and this retains those updates,
                // which probably occured through attribute menus, between updates.
                CForceJob *fj = new CForceJob(ds);

                if (retain_parms) {
                    fj->GetPS()->SetPhysicsParms(p);


#ifdef ATTRIB_MENU_SUPPORT
                    CAttribMenuInstance::GetInstance().SynchToUserValue();
#endif

                }

                m_Job.Reset(fj);        
                m_JobID = CAppJobDispatcher::GetInstance().StartJob(*m_Job, "ThreadPool", *this, 1, true);

                m_Timer.Start(200);
            }
        }
        // Unable to start a job - try to do physics in foreground
        catch (CAppJobException&) {
            m_UseMainThreadPhysics = true;
        }
    }
    else {
        if (!m_MainThreadPS) {
            m_MainThreadPS = new CPhyloTreePS(ds);
            // This makes the system end more quickly since the velocity threshold
            // below which we can stop is raised.
            m_MainThreadPS->SetVelocityThresholdK(0.2f);
        } else {
            m_MainThreadPS->Init(ds);
        }

        //m_Timer.Start(50); // no good for cgi...
        
        // This has to work for a batch, CGI version too - so there are
        // no progressive updates - we just want to iterate until it stabilizes
        // or we reach a maximum iteration count.
        for (int i=0; i<200 && !m_MainThreadPS->IsDone(); ++i)    
            x_SingleThreadPhysics();

        x_CalculateBoundary();
        ComputeViewingLimits(*m_pPane, m_ForceSquare);

        // This is the rect for viewing limits used by cgi.
        m_RasterRect = m_pPane->GetModelLimitsRect();

        return;
    }
}

void CPhyloForce::RemoveCurrentDataSource()
{
    IPhyloTreeRender::RemoveCurrentDataSource();

    StopLayout();
}

bool CPhyloForce::Render(CGlPane& pane, CPhyloTreeDataSource& ds)
{
    m_pPane     = &pane;
    m_DS        = &ds;

    // Is a job running? If so make sure we don't draw while the 
    // particle system is syncing with the graphics model.  Otherwise
    // just draw.
    if (m_JobID != CAppJobDispatcher::eInvalidJobID) {               

        CForceJob* j = m_Job.GetPointerOrNull();
        if (j != NULL ) {

            if (j->DataSynched() == CForceJob::eRenderInProgress) {
                j->SetDataSynched(CForceJob::eRenderInProgress);

                /// Need only to drop scale marker for force layout...
                IPhyloTreeRender::Render(pane,ds);

                j->SetDataSynched(CForceJob::eDataSynched);
            }
            // Unable to render since force thread is busy synchrnizing 
            // the particle system data with the graphics data.
            else {
                //_TRACE("No render. Data synched: " << j->DataSynched() << " instead of: " <<  CForceJob::eDataSynched);
                return false;
            }
        }
    }
    else {
        IPhyloTreeRender::Render(pane,ds);
    }

    return true;
}

void CPhyloForce::StopLayout()
{
    try {       
        m_Timer.Stop();

        if (m_JobID != CAppJobDispatcher::eInvalidJobID) {
            CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
            IAppJob::EJobState state = disp.GetJobState(m_JobID);

            if (state == IAppJob::eRunning) {
                disp.CancelJob(m_JobID);

                // Wait until job is really done (current iteration is finished)
                // to move on:
                int count = 0;
                while (count++<20 && disp.GetJobState(m_JobID) == IAppJob::eRunning) {
                    wxMilliSleep(100);
                }
            }
            else {
                disp.DeleteJob(m_JobID);
            }

            m_JobID = CAppJobDispatcher::eInvalidJobID;
            m_Job.Reset();
        }
    }
    catch (CAppJobException&) {    
        _TRACE("Error deleting force layout job");
    }

    m_JobID = CAppJobDispatcher::eInvalidJobID;            
    m_Job.Reset();
}

void  CPhyloForce::x_SingleThreadPhysics() 
{
    if (!m_MainThreadPS)
        return;

    if (!m_MainThreadPS->IsDone()) {    
        for (int j=0; j<10; ++j) {        
            m_MainThreadPS->Update();      
        }

        m_MainThreadPS->UpdateAndSynch();     
    }
    else {
        m_Timer.Stop();
    }
}


// Function to calculate node positions in tree. Essentially a recursive
// function (implemented as a loop) using TreeDepthFirstEx. This prevents
// very deep trees from the risk of stack overflow.
class CCalcBoundary
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    CCalcBoundary(CPhyloForce* clad)
    : m_Clad(clad)
    , m_ComputeBoundary(0)
    {
        m_BoundaryPoints.push(CBoundaryPoints());
    }

    ETreeTraverseCode operator()(CPhyloTree& tree, 
                                 TTreeIdx node_idx, int delta)
    {   
        CPhyloTree::TNodeType& node = tree[node_idx];

        if (delta==1)  {
            m_BoundaryPoints.push(CBoundaryPoints());
        }
        
        if (delta==1 || delta==0) {
            if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded)
                m_ComputeBoundary += 1;

            m_Clad->InitLabel(&tree, node);
            if (node.IsLeafEx())  {
                CBoundaryPoints node_boundary_pts;
                m_Clad->x_ComputeNodeBoundary(&tree, node, node_boundary_pts, "ForceLayout");
                if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded)
                    m_ComputeBoundary -= 1;
            
                if (m_ComputeBoundary>0) {
                    m_BoundaryPoints.top().AddBoundedPoints(node_boundary_pts);
                }
            }
            m_Clad->CalculateExtents(&tree, node);
        }
        else if (delta == -1) {          
            // Each node that computes a boundary has to add in the points (and text rectangles)
            // for all nodes below it in the tree
            CBoundaryPoints pts = m_BoundaryPoints.top();
            m_BoundaryPoints.pop();
            m_Clad->x_ComputeNodeBoundary(&tree, node, pts, "ForceLayout");

            if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded) {
                m_ComputeBoundary -= 1;
            }

            if (m_ComputeBoundary>0) {
                m_BoundaryPoints.top().AddBoundedPoints(pts);
            }
        }

        return eTreeTraverse;
    }

private:
    CPhyloForce* m_Clad;
    int m_ComputeBoundary;
    stack<CBoundaryPoints> m_BoundaryPoints;
};

void CPhyloForce::x_CalculateBoundary()
{
    CCalcBoundary  calc_tree(this);
    TreeDepthFirstEx(*m_DS->GetTree(), calc_tree);
}

void CPhyloForce::StartRendering(bool b) 
{ 
    IPhyloTreeRender::StartRendering(b);
    if (!b) {
        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();      
        disp.DeleteJob(m_JobID);
        m_Timer.Stop();
    }
}


/// handles "state changed" notification from CAppJobDispatcher
void CPhyloForce::OnAppJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);


    IAppJob::EJobState state = IAppJob::eInvalid;
    int job_id = 0;

    if(notn) {
        job_id = notn->GetJobID();
        if(job_id) {
            state = notn->GetState();
        }
    }
   
    if (state != IAppJob::eRunning) {        
        // Only redraw if job completed normally - not if it was cancellled.
        if (state == IAppJob::eCompleted) {
	    // If job is Not NULL, we will only render if the status is eRenderInProgress
	    if (!m_Job.IsNull())
                m_Job->SetDataSynched(CForceJob::eRenderInProgress);
            x_CalculateBoundary();
            ComputeViewingLimits(*m_pPane, m_ForceSquare);
            if (m_pHost != NULL)
                m_pHost->HMGH_UpdateLimits(true);
        }

        // Stop timer if job is not running.  Can't just use job status since
        // when layout it called it stops the current job but then immediately
        // starts a new one, so this can be called from the cancelled job
        // even though the timer is needed for the newly created job.
        if (job_id == m_JobID) {
            m_Timer.Stop();
            m_JobID = CAppJobDispatcher::eInvalidJobID;
            m_Job.Reset();
        }           
    }
}

void CPhyloForce::OnAppJobProgress(CEvent* evt)
{
    CAppJobNotification* notn =
        dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if(notn) {		
        // Could redraw on this with code below, but we use a separate timer
        // since this is called to infrequently to be helpful 
        // (only called around 1/sec)
        /*       
        CForceJob* j = m_Job.GetPointerOrNull();
        if (j != NULL) {

        CMutexGuard guard(j->GetMutex());

        x_CalculateBoundary();
        ComputeViewingLimits(*m_pPane, m_ForceSquare);
        m_pHost->HMGH_UpdateLimits(true);
        //x_GetParent()->GetPort().SetModelLimitsRect(m_MatrixPane.GetModelLimitsRect());
        //x_GetParent()->GetPort().SetVisibleRect(m_MatrixPane.GetVisibleRect());
        m_pPane->OpenOrtho();

        x_DrawBoundingAreas(m_DS->GetTree());
        x_DrawTree( m_DS->GetTree() );
        x_DrawNodes( m_DS->GetTree() );

        m_pPane->Close();
        }
        */
    }
}

///////////////////////////////////////////////////////////////////////////////
void CPhyloForce::OnTimer(wxTimerEvent& /* evt */)
{
    // If there is a valid (deform) job running and it has 
    // synchronized the visual data with the physics data
    // since the last time called, recompute the tree boundaries
    // and redraw.
    if (!m_UseMainThreadPhysics &&
        m_JobID != CAppJobDispatcher::eInvalidJobID) {
        
        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        IAppJob::EJobState state = disp.GetJobState(m_JobID);

        CForceJob* j = m_Job.GetPointerOrNull();
        if (j != NULL ) {
            CMutexGuard guard(j->GetMutex());
            if (state == IAppJob::eRunning && 
                j->DataSynched() == CForceJob::eDataRefreshed) {
                    j->SetDataSynched(CForceJob::eRenderInProgress);
                    x_CalculateBoundary();
                    ComputeViewingLimits(*m_pPane, m_ForceSquare);
                    m_pHost->HMGH_UpdateLimits(true);
                    //_TRACE("Synched. " <<  j->DataSynched());
            }
        }
        else if (m_UseMainThreadPhysics) {
            x_SingleThreadPhysics();
    
            x_CalculateBoundary();
            ComputeViewingLimits(*m_pPane, m_ForceSquare);
            if (m_pHost != NULL)
                m_pHost->HMGH_UpdateLimits(true);
        }
    }
}

//
// Background job for running force update on a separate thread
//
///////////////////////////////////////////////////////////////////////////////
CForceJob::CForceJob(CPhyloTreeDataSource& ds)
: m_PS(NULL)
, m_DataSynched(eDataSynched)
, m_UpdatesSinceLastSync(0)
{
    m_PS = new CPhyloTreePS(ds);
}

CForceJob::~CForceJob()
{
    delete m_PS;
    m_PS =  NULL;
}

IAppJob::EJobState CForceJob::Run()
{
    // Update data for rendering before starting so initial
    // image can show
    m_PS->UpdateAndSynch();
    m_DataSynched = eRenderInProgress;

    // Loop a large number of times.  Ideally, system will stop when
    // velocity drops (checked in x_Calculate)
    for (int i=0; i<20000 && !IsCanceled(); i++) 
        x_Calculate();

    return eCompleted;    
}

void CForceJob::x_Calculate()
{
    if (IsCanceled()) {
        return;
    }

    // updates should be timer based, but make sure that
    // a minimum number happen (otherwise it will slow down
    // progress of the particle system)
    //double i = 0.0f;

    CStopWatch sw;
    sw.Start();

    // Do not update the system until at least 0.4 seconds has passed.  
    while (sw.Elapsed() < 0.2) {        
        m_PS->Update(); 
        ++m_UpdatesSinceLastSync;
    }

    // Do a special update that synchronizes the tree data with the particle
    // system.  To keep rendering (kind of ) smooth during updates of larger
    // trees, don't do this if there were less than a (low) number of iterations
    // since synch blocks rendering.
    if (!IsCanceled() && 
        m_DataSynched != eRenderInProgress &&
        m_UpdatesSinceLastSync > 5) {
            m_UpdatesSinceLastSync = 0;
            CMutexGuard guard(m_Mutex);
            m_DataSynched = eDataNotSynched;
            m_PS->UpdateAndSynch();
            m_DataSynched = eDataRefreshed;

            /// If velocity has dropped low enough, stop system.
            if (m_PS->IsDone())
                RequestCancel();
    }                  
}

CForceJob::TDataSynchStatus CForceJob::DataSynched()
{
    return m_DataSynched;
}

void CForceJob::SetDataSynched(TDataSynchStatus s)
{
    CMutexGuard guard(m_Mutex);
    m_DataSynched = s;
}

CConstIRef<IAppJobProgress> CForceJob::GetProgress()
{
    return CConstIRef<IAppJobProgress>(new CAppJobProgress(1.0, "Finished"));
}

CRef<CObject> CForceJob::GetResult()
{
    CMutexGuard guard(m_Mutex);
    CRef<CObject> result(new CObject());
    return result;
}

CConstIRef<IAppJobError> CForceJob::GetError()
{
    return CConstIRef<IAppJobError>(new CAppJobError("No errors"));
}

string CForceJob::GetDescr() const
{
    return "Building Force Layout";
}





END_NCBI_SCOPE

