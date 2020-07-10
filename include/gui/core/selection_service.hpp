#ifndef GUI_CORE___SELECTION_SERVICE__HPP
#define GUI_CORE___SELECTION_SERVICE__HPP

/*  $Id: selection_service.hpp 38835 2017-06-23 18:15:53Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE

class ISelectionClient;
class CSelectionEvent;

///////////////////////////////////////////////////////////////////////////////
/// ISelectionService - an interface
class ISelectionService
{
public:
    virtual ~ISelectionService() {}

    virtual void AttachClient(ISelectionClient* client) = 0;
    virtual void DetachClient(ISelectionClient* client) = 0;
    virtual bool IsAttached(ISelectionClient* client) const = 0;

    /// returns objects currently selected in the application, this function
    /// should be used for obtaining arguments for running tools, opening views and
    /// performing other application commands
    virtual void GetCurrentSelection(TConstScopedObjects& sel) = 0;

    /// returns objects currently active in the application, this function
    /// should be used for obtaining arguments for running tools, opening views and
    /// performing other application commands
    virtual void GetActiveObjects(TConstScopedObjects& sel) = 0;

    /// returns prioritized vector of vectors of objects currently active in the application,
    //  this function should be used for obtaining arguments for running tools, opening views and
    /// performing other application commands
    //  Supposed use - if the first vector is not of use to a tool proceed to the next vector
    virtual void GetActiveObjects(vector<TConstScopedObjects>& sel) = 0;


    /// request broadcasting of the given event, Broadcast() notifies observers
    /// about changes in selection (do not need to call OnSelectionChanged())
    virtual void Broadcast( CSelectionEvent& evt, ISelectionClient* source ) = 0;

    /// this function is called by GUI components to notify server
    virtual void OnActiveClientChanged( ISelectionClient* active ) = 0;

    /// notifies Service and its observers about changes in selection
    virtual void OnSelectionChanged( ISelectionClient* source ) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// ISelectionClient - represents an object that support a notion of selection.
/// ISelectionClient provides functions for setiing and getting selections
class ISelectionClient
{
public:
    virtual ~ISelectionClient() {}

    /// shall return data objects representing selected things, return one
    /// data object for every selected thing. Implementation shall not clear
    /// the container as it can be used for accumulating objects.
    virtual void GetSelection( TConstScopedObjects& objects ) const = 0;

    virtual void SetSelectionService( ISelectionService* service ) = 0;

    /// returns a string identifying the client in UI
    virtual string GetSelClientLabel() = 0;

    /// get selection for broadcasting
    virtual void GetSelection( CSelectionEvent& evt ) const = 0;

    /// set broadcasted selection
    virtual void SetSelection( CSelectionEvent& evt ) = 0;

    /// Adds the main data objects represented by the client to "objects".
    /// Implementation shall not clear the container as it can be used for
    /// accumulating objects.
    virtual void GetMainObject( TConstScopedObjects& objects ) const = 0;

    virtual void GetActiveObjects( TConstScopedObjects& objects ) const
    {
        GetSelection(objects);
        GetMainObject(objects);
    }

    /// Prioritized active objects
    virtual void GetActiveObjects(vector<TConstScopedObjects>& sel)
    {
        if (sel.size() < 2)
            sel.resize(2);
        GetSelection(sel[0]);
        GetMainObject(sel[1]);
    }

    virtual void BroadcastSelection() {}
};

END_NCBI_SCOPE


#endif  // GUI_CORE___SELECTION_SERVICE__HPP

