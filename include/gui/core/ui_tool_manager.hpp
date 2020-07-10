#ifndef GUI_FRAMEWORK___UI_TOOL_MANAGER__HPP
#define GUI_FRAMEWORK___UI_TOOL_MANAGER__HPP

/*  $Id: ui_tool_manager.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include <gui/objutils/objects.hpp>

#include <gui/utils/ui_object.hpp>

class wxPanel;
class wxWindow;


BEGIN_NCBI_SCOPE

class IAppTask;
class IServiceLocator;
class IWizardPage;
class IExecuteUnit;

///////////////////////////////////////////////////////////////////////////////
// The purpose of IUIToolManager is to manage UI associated with a tool
// (algorithm, loader, exporter) in a specialized wizard-like dialog (or other
// similar UI component).
// The dialog serves as a host for the Tool Manager and provides basic support
// for creating wizard-like user interfaces. It is assumed that Tool Manager
// works as a state machine with states corresponding to different steps in the
// process of gathering user input. For every step a UI panel can be displayed
// in the dialog, the tool manager is responsible for providing the panel.
// Pressing “Back”, Forward” and “Finish” buttons leads to a transition from one
// state to another. When transition is peformed the data entered by the user is
// extracted from the current panel and validated; the destinaton state may
// depend on the user input.
// In the end the Tool Manager produces an IAppTask object representing the
// algorithm, this object shall contain all necessary input arguments gathered
// in the process. The task object then can be executed using Application Task Manager.

// Classes implementing IUIToolManager shall be CIRef-compatible. The easiest
// way to achieve this is to derive your implementation from CObject.

class  NCBI_GUICORE_EXPORT  IUIToolManager
{
public:
    // enumerates possible transitions, corresponding to "Back" and "Next" buttons
    enum EAction {
        eBack = 0,
        eNext,
        eSkip
    };

    /// Returns the object describing this tool (UI meta data).
    virtual const IUIObject&  GetDescriptor() const = 0;

    /// Sets / unsets Service Locator. Tool Manager can use the locator
    /// to obtain the services necessary services.
    virtual void    SetServiceLocator(IServiceLocator* srv_locator) = 0;

    virtual void    SetParentWindow(wxWindow* parent) = 0;

    /// Initializes the Manager before using it in UI. This function shall reset
    /// Internal state of the tool.
    virtual void    InitUI() = 0;

    /// CleanUI() is called after the host finished using the manager. This function
    /// shall free resources associated with the session and reset internal state.
    virtual void    CleanUI() = 0;

    /// Return the panel corresponding to the current state of Tool Manager.
    virtual wxPanel*    GetCurrentPanel() = 0;

    /// Return the panel that occupies max size on display, to calculate dialog window size
    virtual wxPanel*    GetMaxPanel() { return 0; }

    /// True if Tool Manager has reached its final state, i.e. the last page
    /// (panel) in the sequence is displayed and "Finish" button is shown. The
    /// framework calls this function to decide whether “Finish” or “Next”
    /// button needs to be shown.
    virtual bool        IsFinalState() = 0;

    /// Manager goes into "Complete" state when "Finish" button is pressed and
    /// all input data is gatherred and validated. Typically this is the last
    /// state after Final state. This state does not have an associated UI panel.
    /// When manager reaches the “Completed” state the framework hides the dialog
    /// and calls GetTask() function.
    virtual bool        IsCompletedState() = 0;

    /// Indicates whether given transition is possible in the current state.
    virtual bool        CanDo(EAction action) = 0;

    /// Performs transition if possible and returns true, otherwise the function
    /// shall warn the user about the problem preventing the transition
    /// (such as insufficient or invalid input) and return false.
    /// Typically this function would take user input from the current panel,
    /// validate the input and go to the next state creating new UI panel if needed.
    virtual bool    DoTransition(EAction action) = 0;

    /// If a transition fails, it may make sense to switch to a different tool manger
    /// When a tool manager identifies a more appropriate manager for a task, it
    /// returns it here, otherwise NULL
    virtual IUIToolManager*  GetAlternateToolManager() { return NULL; }

    /// Once parameters are gathered and validated this function is called
    /// to produce the final Task object that will be executed to do the actual
    /// computational work.
    virtual IAppTask*   GetTask() = 0;

    virtual IExecuteUnit* GetExecuteUnit()  { return nullptr; }

    /// Returns first options page (if any) of the tool to be shown in wizard dialog
    virtual IWizardPage* GetFirstPage() { return 0; }
    /// Returns page for tool options (long wizard path via "Options" button)
    virtual IWizardPage* GetOptionsPage() { return 0; }
    /// Sets a wizard page which should show before the first page of the tool
    virtual void         SetPrevPage(IWizardPage*) {}
    /// Sets a wizard page which should show after the last page of the tool
    virtual void         SetNextPage(IWizardPage*) {}

    /// Restores the default options for the current panel
    virtual void RestoreDefaults() {}

    virtual void ResetState() {}

    virtual bool UsesSingleOMScope() { return true; }

    virtual ~IUIToolManager()    {};
};


///////////////////////////////////////////////////////////////////////////////
/// IUIAlgoToolManager is a IUIToolManager that represents an algorithmic tool.
///
/// TODO - consider placing this interface in a separate file
class  NCBI_GUICORE_EXPORT  IUIAlgoToolManager : public IUIToolManager
{
public:
    /// returns the name for the ToolCategory to which the tool belongs.
    /// Tools are shown in in UI groupped by Categories.
    virtual string    GetCategory() = 0;

    /// sets input objects that can be used as arguments for the tool.
    /// the manager can save the pointer, it will remain valid until
    /// CleanUI() is called.
    virtual string SetInputObjects(const vector<TConstScopedObjects>& input) = 0;
    //virtual bool    IsMultiScopeInputAccepted() const { return false; }

    virtual bool      CanQuickLaunch() const = 0;
    virtual IAppTask* QuickLaunch() = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// This Extension Point allows external components to add new algorithmic
/// tools.
/// The Extensions shall implement IUIAlgoToolManager interface.

/// Extension Point ID for Tool Managers implementing IUIAlgoToolManager interface
#define EXT_POINT__UI_ALGO_TOOL_MANAGER         "ui_algo_tool_manager"


END_NCBI_SCOPE


#endif  // GUI_FRAMEWORK___UI_TOOL_MANAGER__HPP

