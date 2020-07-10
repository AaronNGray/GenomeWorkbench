#ifndef PKG_ALIGNMENT___NGALIGN_TOOL__HPP
#define PKG_ALIGNMENT___NGALIGN_TOOL__HPP

/*  $Id: ngalign_tool.hpp 38720 2017-06-12 20:49:12Z evgeniev $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *      Implementation of "NGAlign" tool for GenomeWorkbench.
 */

#include <corelib/ncbistd.hpp>

#include <gui/core/algo_tool_manager_base.hpp>

#include <gui/packages/pkg_alignment/ngalign_params.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CNGAlignPanel;
class CNGAlignBLASTPanel;

///////////////////////////////////////////////////////////////////////////////
/// CNGAlignTool
class  CNGAlignTool : public CAlgoToolManagerBase
{
public:
    CNGAlignTool();

    /// @name overriding IUIToolManager functions implemented in CAlgoToolManagerBase
    /// @{
    virtual void    InitUI();
    virtual void    CleanUI();
    virtual bool    IsFinalState();
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

protected:
    /// @name overriding CAlgoToolManagerBase virtual functions
    /// @{
    virtual CAlgoToolManagerParamsPanel*    x_GetParamsPanel();
    virtual IRegSettings*   x_GetParamsAsRegSetting();
    virtual void    x_CreateParamsPanelIfNeeded();
    virtual bool    x_ValidateParams();
    virtual CDataLoadingAppJob*     x_CreateLoadingJob();
    virtual bool DoTransition(EAction action);
    virtual wxPanel*    GetCurrentPanel();

    virtual bool    IsMultiScopeInputAccepted() const { return false; }
    /// @}

    void    x_CreateBALSTPanelIfNeeded();

    virtual void    x_SelectCompatibleInputObjects();


// Managing this tool pages
    wxPanel* x_GetCurrentPage();
    bool x_IsFirstPage() const;
    bool x_IsLastPage() const;
    bool x_GoFirstPage();
    bool x_GoNextPage();
    bool x_GoPrevPage();
    bool x_GoLastPage();
    bool x_LoadPage();

protected:
    TConstScopedObjects m_SeqIds;
    TConstScopedObjects m_SeqLocs;
    CNGAlignParams      m_Params;
    CNGAlignPanel*      m_Panel;       // first page
    CNGAlignBLASTPanel* m_BLASTPanel;  // second page
    int                 m_CurrentPage;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___NGALIGN_TOOL__HPP
