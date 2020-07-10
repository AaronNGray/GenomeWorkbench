#ifndef GUI_WX_DEMO___DOCK_WINDOW__HPP
#define GUI_WX_DEMO___DOCK_WINDOW__HPP

/*  $Id: dock_window.hpp 43813 2019-09-05 16:03:49Z katargir $
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


#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

class wxWindow;
class wxPoint;

BEGIN_NCBI_SCOPE

class CDockContainer;

//////////////////////////////////////////////////////////////////////////////
/// IDockableWindow - repersents a window that can be docked in Dock Manager.
/// IDockableWindow lives in a Dock Container and can be dragged between
/// different Dock Containers.

class IDockableWindow
{
public:
    virtual     CDockContainer* GetDockContainer() = 0;
    virtual void    SetDockContainer(CDockContainer* cont) = 0;

    virtual ~IDockableWindow()  {};
};


///////////////////////////////////////////////////////////////////////////////
/// EDockEffect

enum  EDockEffect {
    eNoEffect = -1,
    eSplitLeft,
    eSplitRight,
    eSplitTop,
    eSplitBottom,
    eSplitTargetLeft,
    eSplitTargetRight,
    eSplitTargetTop,
    eSplitTargetBottom,
    ePutInTab
};


///////////////////////////////////////////////////////////////////////////////
/// IDockDropTarget - interface representing a component that can serve as
/// a drop target during D&D docking.
class NCBI_GUIWIDGETS_WX_EXPORT     IDockDropTarget
{
public:
    // perfroms hit testing for the given position (screen coords)
    // returns a Docking Effect and a target window that shall be modified
    // if dropping occurs
    virtual EDockEffect DropTest(const wxPoint& screen_pt, wxWindow*& target) = 0;

    virtual ~IDockDropTarget()   {};
};


END_NCBI_SCOPE


#endif  // GUI_WX_DEMO___DOCK_WINDOW__HPP
