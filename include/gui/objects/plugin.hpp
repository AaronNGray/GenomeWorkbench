#ifndef GUI_PLUGIN_PLUGIN_HPP
#define GUI_PLUGIN_PLUGIN_HPP

/*  $Id: plugin.hpp 14666 2007-07-09 13:40:22Z dicuccio $
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
 * Authors:  Mati Shomrat
 *
 * File Description:
 *      CPlugin - defines a base interface for all plugins
 */
#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <gui/objects/PluginMessage.hpp>
#include <gui/core/resolver.hpp>
#include <gui/utils/reporter.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CPluginInfoSet;
    class CPluginArg;
    class CPluginArgSet;
END_SCOPE(objects)

typedef void (*FPluginArgCallback)( objects::CPluginArg& arg, objects::CPluginArgSet& set );

class NCBI_GUIOBJECTS_EXPORT CPluginBase : public CObject
{
public:
    /// destructor
    virtual ~CPluginBase(void);

    /// @name Message-passing interface
    /// @{

    /// pass a message to the plugin
    /// This function will perform validation on the message and delegate
    /// actual handling to user-overridable methods.
    void Execute(objects::CPluginMessage& msg);

    /// Initialize the plugin.  This maps to the plugin command
    /// CPluginCommand::eCommand_init, and takes no arguments.  No specific
    /// registration is needed to obtain this command; however, if you want
    /// to register 'init' as an auto-initializable command (i.e., a command
    /// launched at application start-up), then you will need to register the
    /// command.  This command is implicitly supported by all plugins.  The
    /// provided plugin message is largely for future extension.
    virtual void Init(objects::CPluginMessage& msg);

    virtual void CreateInterface(objects::CPluginMessage& msg);

    /// Execute the 'load' command.  This maps to CPluginCommand::eCommand_load.
    /// Use of this command requires registration in the plugin's information
    /// settings, and explicit override from the user.  Calling this command
    /// on a plugin that does not support it will result in an exception.
    virtual void Load(objects::CPluginMessage& msg);

    /// Execute the 'save' command.  This maps to CPluginCommand::eCommand_save.
    /// Use of this command requires registration in the plugin's information
    /// settings, and explicit override from the user.  Calling this command
    /// on a plugin that does not support it will result in an exception.
    virtual void Save(objects::CPluginMessage& msg);

    /// Execute the 'import' command.  This maps to CPluginCommand::eCommand_import.
    /// Use of this command requires registration in the plugin's information
    /// settings, and explicit override from the user.  Calling this command
    /// on a plugin that does not support it will result in an exception.
    virtual void Import(objects::CPluginMessage& msg);

    /// Execute the 'load-project' command.  This maps to
    /// CPluginCommand::eCommand_load_project.  Use of this command requires
    /// registration in the plugin's information settings, and explicit override
    /// from the user.  Calling this command on a plugin that does not support it
    /// will result in an exception.  This command is implicitly supported by all
    /// data plugins so that the framework can generically call LoadProject on
    /// any data plugin.
    virtual void LoadProject(objects::CPluginMessage& msg);

    /// Execute the 'save-project' command.  This maps to
    /// CPluginCommand::eCommand_save_project.  Use of this command requires
    /// registration in the plugin's information settings, and explicit override
    /// from the user.  Calling this command on a plugin that does not support it
    /// will result in an exception.  This command is implicitly supported by all
    /// data plugins so that the framework can generically call LoadProject on any
    /// data plugin.
    virtual void SaveProject(objects::CPluginMessage& msg);

    /// Execute the 'search' command.  This maps to CPluginCommand::eCommand_search.
    /// Use of this command requires registration in the plugin's information
    /// settings, and explicit override from the user.  Calling this command on a
    /// plugin that does not support it will result in an exception.
    virtual void Search(objects::CPluginMessage& msg);

    /// Execute the 'manage' command.  This maps to CPluginCommand::eCommand_manage.
    /// Use of this command requires registration in the plugin's information
    /// settings, and explicit override from the user.  Calling this command on a
    /// plugin that does not support it will result in an exception.
    virtual void Manage(objects::CPluginMessage& msg);

    /// Execute the 'run' command.  This maps to CPluginCommand::eCommand_run.
    /// Use of this command requires registration in the plugin's information
    /// settings, and explicit override from the user.  Calling this command on a
    /// plugin that does not support it will result in an exception.
    virtual void Run(objects::CPluginMessage& msg);

    /// Wait until the plugin has completed its execution.  Plugins that override
    /// this API should block until execution of their plugin has completed.  This
    /// maps to the plugin command CPluginCommand::eCommand_finish.  This command
    /// is implicitly supported by all plugins.  The provided plugin message is
    /// largely for future extension.
    virtual void Finish(objects::CPluginMessage& msg);

    /// Attempt to stop a plugin's operation immediately.  This maps to the plugin
    /// command CPluginCommand::eCommand_abort.  If a plugin manages a thread of
    /// its own, it must respond to this command and block until the thread has
    /// been stopped.  This command is implicitly supported by all plugins.  The
    /// provided plugin message is largely for future extension.
    virtual void Abort(objects::CPluginMessage& msg);

    /// Pause execution of a plugin.  This maps to the plugin command
    /// CPluginCommand::eCommand_suspend.  This command is implicitly supported by
    /// all plugins.  If a plugin manages a thread, it should overload this function
    /// and provide a blocking thread suspension.  The provided plugin message is
    /// largely for future extension.
    virtual void Suspend(objects::CPluginMessage& msg);

    /// Resume execution of a paused plugin.  This maps to the plugin command
    /// CPluginCommand::eCommand_resume.  This command is implicitly supported by
    /// all plugins.  If a plugin manages a thread, it should overload this function
    /// and provide a blocking thread resumption.  The provided plugin message is
    /// largely for future extension.
    virtual void Resume(objects::CPluginMessage& msg);

    /// Create a new view.  This maps to the plugin command
    /// CPluginCommand::eCommand_new_view.  Use of this command requires
    /// registration in the plugin's information settings, and explicit override
    /// from the user.  Calling this command on a plugin that does not support it
    /// will result in an exception.
    virtual void NewView(objects::CPluginMessage& msg);

    /// Create a new composite view.  This maps to the plugin command
    /// CPluginCommand::eCommand_new_composite_view.  A composite view differs from
    /// a regular view in that a composite view serves as a container for other
    /// views.  Composite views are derived from IViewComposite or CViewComposite.
    /// Use of this command requires registration in the plugin's information
    /// settings, and explicit override from the user.  Calling this command on a
    /// plugin that does not support it will result in an exception.
    virtual void NewCompositeView(objects::CPluginMessage& msg);

    /// Retrieve some status information about the current execution.  GetStatus()
    /// is passed a message with two arguments - 'pct_completed' and 'message' -
    /// so that a plugin can fill in some basic status information
    virtual void GetStatus(objects::CPluginMessage& msg);

    /// @}

    /// @name Message-free interface
    /// @{

    /// Status inquiry: Are we still running?  The default implementation returns
    /// true, and assumes that all tasks are running in a non-threaded, blocking
    /// fashion.  If an algorithm is managing a thread or interactive dialog,
    /// this function should be overridden to prevent the instance from being
    /// deleted until interaction or computation is completed.
    virtual bool IsCompleted(void) const;

    /// Retrieve information about a plugin's arguments, class names, and so forth.
    virtual void GetPluginInfo(objects::CPluginInfoSet& info) const = 0;

    /// Finalize the arguments (gets called just before argument dialog).
    /// Version defined here does nothing; subclasses should override
    /// if they want to do something.
    virtual void FinalizeArgs(objects::CPluginMessage& msg);

    /// Validate all the arguments (gets called just after argument dialog).
    /// If arguments are not valid, it thows an exception. Primarily it is
    /// to be catched in PluginArgDialog, so it is of CDDV_DDX_Exception type.
    /// Version defined here does nothing; subclasses should override
    /// if they want to do something.
    virtual objects::CPluginArg* ValidateArgs(objects::CPluginArgSet& set);

    /// This is the way to provide customized resolving dialog for particular
    /// plugin. By default, it returns NULL, thus standard global resolver will
    /// be used.
    virtual FResolvePluginArgs GetArgsResolver();

    /// This is the way to provide a dynamic behaviour of automatically generated
    /// PluginArgForm.
    virtual FPluginArgCallback GetPluginArgCallback();

    /// @}
};


template<typename PluginType>
class CPlugin : public CPluginBase
{
public:
    /// overloaded GetInfo() - we call a static function in our derived plugin
    /// type.  This static function is a requirement of all plugin
    /// implementations.
    virtual void GetPluginInfo(objects::CPluginInfoSet& info) const
    {
        PluginType::GetInfo(info);
    }
};


END_NCBI_SCOPE

#endif /// GUI_PLUGIN_PLUGIN_HPP
