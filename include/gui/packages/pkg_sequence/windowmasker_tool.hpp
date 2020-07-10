#ifndef PKG_SEQUENCE___WINDOWMASKER_TOOL__HPP
#define PKG_SEQUENCE___WINDOWMASKER_TOOL__HPP

/*  $Id: windowmasker_tool.hpp 38460 2017-05-11 18:58:47Z evgeniev $
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
* Authors:  Vladimir Tereshkov
*
* File Description:
*      Implementation of Window Masker tool for GenomeWorkbench.
*/

#include <corelib/ncbistd.hpp>

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_sequence/windowmasker_params.hpp>

BEGIN_NCBI_SCOPE

class CWindowMaskerPanel;

///////////////////////////////////////////////////////////////////////////////
/// CWindowMaskerTool
class  CWindowMaskerTool : public CAlgoToolManagerBase
{
public:
    CWindowMaskerTool();

    /// @name overriding IUIToolManager functions implemented in CAlgoToolManagerBase
    /// @{
    virtual void        InitUI();
    virtual void        CleanUI();
    virtual bool        UsesSingleOMScope() { return false; }
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
    virtual IRegSettings*                   x_GetParamsAsRegSetting();
    virtual void                            x_CreateParamsPanelIfNeeded();
    virtual bool                            x_ValidateParams();
    virtual CDataLoadingAppJob*             x_CreateLoadingJob();
    /// @}

    virtual void    x_SelectCompatibleInputObjects();

protected:
    TConstScopedObjects     m_Objects;
    CWindowMaskerParams     m_Params;
    CWindowMaskerPanel*     m_Panel;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___WINDOWMASKER_TOOL__HPP

