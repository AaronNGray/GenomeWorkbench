#ifndef GUI_FRAMEWORK___MENU_SERVICE__HPP
#define GUI_FRAMEWORK___MENU_SERVICE__HPP

/*  $Id: menu_service_impl.hpp 27492 2013-02-25 17:55:56Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/framework/menu_service.hpp>
#include <gui/framework/service.hpp>

class wxFrame;
class wxMenuBar;

BEGIN_NCBI_SCOPE

class CUICommandRegistry;

///////////////////////////////////////////////////////////////////////////////
/// CMenuService - standard implementation of IMenuService.
class NCBI_GUIFRAMEWORK_EXPORT CMenuService :
    public CObject,
    public IMenuService,
    public IService
{
public:
    CMenuService();
    virtual void    SetFrame(wxFrame* frame, CUICommandRegistry& cmd_reg);

    /// @name IService implementation
    /// @{
    virtual void    InitService();
    virtual void    ShutDownService();
    /// @}

    /// @name IMenuService implementation
    /// @{
    virtual void    ResetMenuBar();
    virtual void    AddContributor( IMenuContributor* contributor );
    virtual void    RemoveContributor( IMenuContributor* contributor );
    virtual void    AddPendingCommand( int cmd );
    /// @}

protected:
    typedef vector<IMenuContributor*>   TContributors;

    wxFrame* m_Frame;
    TContributors m_Contributors;

    CUICommandRegistry* m_CmdReg;

    /// Menu bar to add to m_Frame (but not until is is created).  Adding to
    /// the frame too early messes up help menu on Mac (Carbon)
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___MENU_SERVICE__HPP

