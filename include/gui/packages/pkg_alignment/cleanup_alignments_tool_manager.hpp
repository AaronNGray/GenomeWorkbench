#ifndef PKG_ALIGNMENT___CLEANUP_ALIGNMENTS_TOOL_MANAGER__HPP
#define PKG_ALIGNMENT___CLEANUP_ALIGNMENTS_TOOL_MANAGER__HPP

/*  $Id: cleanup_alignments_tool_manager.hpp 38460 2017-05-11 18:58:47Z evgeniev $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/core/algo_tool_manager_base.hpp>

#include <gui/packages/pkg_alignment/cleanup_alignments_params_panel.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CCleanupAlignmentsToolManager
class  CCleanupAlignmentsToolManager :
    public CAlgoToolManagerBase
{
public:
    CCleanupAlignmentsToolManager();

    /// @name overriding IUIToolManager functions implemented in CAlgoToolManagerBase
    /// @{
    virtual void    InitUI();
    virtual void    CleanUI();
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

protected:
    /// @name overriding CAlgoToolManagerBase virtual functions
    /// @{
    virtual CAlgoToolManagerParamsPanel* x_GetParamsPanel();
    virtual IRegSettings* x_GetParamsAsRegSetting();
    virtual void x_CreateParamsPanelIfNeeded();
    virtual bool x_ValidateParams();
    virtual CDataLoadingAppJob* x_CreateLoadingJob();
    /// @}

    void x_SelectCompatibleInputObjects();

    bool x_AreSelfAlignmentsOnly() const;

protected:
    TConstScopedObjects     m_Alignments;

    SCleanupAlignmentsParams  m_Params;
    CCleanupAlignmentsParamsPanel*    m_ParamsPanel;
};


///////////////////////////////////////////////////////////////////////////////
/// CCleanupAlignmentsJob
class  CCleanupAlignmentsJob : public CDataLoadingAppJob
{
public:
    CCleanupAlignmentsJob(const SCleanupAlignmentsParams& params);

protected:
    virtual void    x_CreateProjectItems(); // overriding virtual function

protected:
    SCleanupAlignmentsParams  m_Params;
};





/* @} */

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___CLEANUP_ALIGNMENTS_TOOL_MANAGER__HPP
