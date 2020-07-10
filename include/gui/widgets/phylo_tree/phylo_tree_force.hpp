#ifndef GUI_WIDGETS_PHY_TREE___PHYLO_TREE_FORCE__HPP
#define GUI_WIDGETS_PHY_TREE___PHYLO_TREE_FORCE__HPP

/*  $Id: phylo_tree_force.hpp 32170 2015-01-13 16:11:35Z falkrb $
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


#include <gui/widgets/phylo_tree/phylo_tree_radial.hpp>
#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/utils/app_job_impl.hpp>

BEGIN_NCBI_SCOPE

class CPhyloTreeNode;
class CPhyloTreeDataSource;
class CPhyloTreePS;

///
/// Class used to run deformation on separate thread
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CForceJob : public CJobCancelable
{
public:
    enum TDataSynchStatus { eDataNotSynched, 
        eDataSynched, eRenderInProgress, eDataRefreshed };

public:
    CForceJob(CPhyloTreeDataSource& ds);
    ~CForceJob();

    /// Get the particle system
    CPhyloTreePS* GetPS() { return m_PS; }

    /// Return true if tree has been updated from
    /// particle system since last call
    TDataSynchStatus DataSynched();

    /// Update data sych state
    void SetDataSynched(TDataSynchStatus s);

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string GetDescr() const;
    /// @}

    /// Get mutex
    CMutex& GetMutex() { return m_Mutex; }

protected:
	void x_Calculate();
   
    /// Guard access to underlying tree
    mutable CMutex m_Mutex;

    /// Particle system used to deform tree
    CPhyloTreePS*  m_PS;
    /// Set to true each time particle system updates underlying tree
    TDataSynchStatus m_DataSynched;
    /// Counter to prevent synchronizing too often (that delays rendering)
    int m_UpdatesSinceLastSync;
};

///
/// Main layout class
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloForce : public CPhyloRadial, public CEventHandler
{
	DECLARE_EVENT_MAP();
    DECLARE_EVENT_TABLE()

    typedef CAppJobDispatcher::TJobID   TJobID;

public:
    CPhyloForce();
    CPhyloForce(double w, double h);
    ~CPhyloForce();

    /// Overridden to cancel any running (deform) job when rendering
    /// switches to a different layout
    virtual void StartRendering(bool b);

	/// functions below handle notifications from the IAppJob
    void OnAppJobNotification(CEvent* event);
    void OnAppJobProgress(CEvent* event);

    /// If datasource is deleted/recreated during layout, this will cancel
    /// current layout (to prevent from crashing on deleted data access)
    virtual void RemoveCurrentDataSource();
    
    /// While job is running, use a timer to redraw (updated) tree
    void OnTimer(wxTimerEvent& evt);

    /// Render in force overrides the pane size computation (adaptive margins)
    bool Render(CGlPane& pane, CPhyloTreeDataSource& ds);

    string GetDescription(void) {  return "Force Layout"; }   

    /// Stop active layout (particle system)
    void StopLayout();

    /// Force layout uses uniform branch lengths
    virtual bool SupportsDistanceRendering() const { return false; }

protected:
    void  x_SingleThreadPhysics();
    //void  x_DrawTree(CPhyloTree::TTreeIdx node_idx);
    //void  x_Render(CGlPane& pane, CPhyloTreeDataSource& ds);
    void  x_Layout(CPhyloTreeDataSource& ds);
    void  x_CalculateBoundary();


    /// run during force calculations to re-display graph during upates
    wxTimer m_Timer;
    /// Guard access to tree during force updates
    CFastMutex m_Mutex;
    
    /// Current forced-based graph layout job
    CIRef<CForceJob> m_Job;
    /// Job ID of current force-based graph layout job
    TJobID           m_JobID;

    /// Particle system to use if physics is to be done in main thread
    CPhyloTreePS  *m_MainThreadPS;
    /// If true, we update the physics in the main thread.
    bool m_UseMainThreadPhysics;


    /*
    // Debug vis. area
    float vis_left;
    float vis_right;
    float vis_bottom;
    float vis_top;

    float ext_left;
    float ext_right;
    float ext_bottom;
    float ext_top;
    */
};


END_NCBI_SCOPE

#endif //GUI_WIDGETS_PHY_TREE___PHYLO_TREE_FORCE__HPP

