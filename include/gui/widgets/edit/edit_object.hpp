#ifndef GUI_WIDGETS_EDIT___EDIT_OBJECT__HPP
#define GUI_WIDGETS_EDIT___EDIT_OBJECT__HPP

/*  $Id: edit_object.hpp 38357 2017-04-27 20:57:00Z filippov $
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
 *
 */

#include <corelib/ncbiobj.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/framework/view_manager_service.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>

class wxWindow;

BEGIN_NCBI_SCOPE

class IEditCommand;

/// GUI editing interface
///
/// Implementations of this interface needs to register with CInterfaceRegistry
/// (Example: edit_object_seq_feat.cpp implements CEditObjectSeq_feat for CSeq_feat)
/// Declarations if objects used in Interface Registry are in file interface_registry.hpp
///
/// When we want to use the interface and we have C++Toolkit object and CScope we 
/// request interface via CreateObjectInterface<IEditObject>( TConstScopedObject obj, NULL). 
/// (Example: CProjectView::OnEditSelection(), file project_view_impl.cpp). 
/// If the interface is registered for this C++Toolkit class we get it and use it.
///
/// @sa IEditCommand, IUndoManager, CInterfaceRegistry
///
///
class IEditObject 
{
public:
    virtual ~IEditObject() {}

    /// creates a child (not top level) windows that can contain whatever 
    /// controls/subwindows etc. needed to edit an object.
    /// This window is placed in a placeholder of Edit Object Dialog (top level window).
    /// In the future we can present this window somewhere else (GBench view for example).
    ///
    virtual wxWindow* CreateWindow(wxWindow* parent) = 0;

    /// After placing the window in the Edit Object Dialog ShowModal() method of the dialog 
    /// is called. If OK button is called the second method of IEditObject dialog is called 
    /// which returns object derived from IEditCommand.  
    /// This object implements actual modifications of original object in the 
    /// Project/Document via Execute() method.
    virtual IEditCommand* GetEditCommand() = 0;
    virtual bool CanClose() {return true;}
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___EDIT_OBJECT__HPP
