#ifndef GUI_CORE___OPEN_VIEW_MANAGER__HPP
#define GUI_CORE___OPEN_VIEW_MANAGER__HPP

/*  $Id: open_view_manager.hpp 38460 2017-05-11 18:58:47Z evgeniev $
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

#include <gui/framework/view.hpp>
#include <gui/objutils/objects.hpp>

class wxPanel;
class wxWindow;


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// IOpenViewManager - interface dealing with a series of open view operations.
/// Issues: For a given view, one or more user selected objects might not
/// be able to be displayed directly.  A series of steps
/// might be required to convert the input objects into the view-specific
/// data types.  The data conversion process will take into account of
/// the user inputs and various view-specific rules/options. The main open
/// view process includes object extraction, object conversion, and object
/// aggregation.
/// IOpenViewManager works as a state machine similar to IUIToolManager.
/// But, it is simplified and more specific for open view operation.
/// It performs transition between states and in each state, it can display
/// a panel containing view-specific options that will be shown in a dialog.

class IOpenViewManager
{
public:
    enum EAction {
        eBack = 0,
        eNext,
    };

    virtual void    SetParentWindow( wxWindow* parent ) = 0;

    /// initialize the Manager before using in UI (set it to Initial state)
    virtual void    InitUI() = 0;

    /// clean UI objects associated with the Manager
    virtual void    CleanUI() = 0;

    /// Set input objects that serve as original user input data. They
    /// will be converted to the target object type if necessary.
    /// the manager can save the pointer, it will remain valid until
    /// CleanUI() is called.
    virtual void    SetInputObjects( const TConstScopedObjects& input ) = 0;

    /// Return the final input objects for views after possible object conversions
    /// and object aggregation.
    virtual const TConstScopedObjects& GetOutputObjects() const = 0;

    virtual const vector<int> GetOutputIndices() const = 0;

    /// return the panel corresponding to the current state
    /// the function may create a new panel or return a pointer to existing panel
    /// all panels are owned and destroyed by the Open dialog.
    virtual wxPanel*    GetCurrentPanel() = 0;

    /// true if state is Final (i.e. "Finish" button shall be shown instead of "Next")
    virtual bool        AtFinalStep() = 0;

    /// true if data gathering has finished and Open dialog shall be closed
    virtual bool        IsCompletedState() = 0;

    /// indicates whether a transition (action) is possible
    virtual bool        CanDo(EAction action) = 0;

    /// perform transition if possible and returns true, otherwise the function
    /// shall warn the user about the problem preventing the transition and return false.
    /// this function should be used to perfrom argument validation
    virtual bool    DoTransition(EAction action) = 0;

    virtual ~IOpenViewManager()    {};
};

END_NCBI_SCOPE


#endif  // GUI_CORE___OPEN_VIEW_MANAGER__HPP

