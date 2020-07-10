#ifndef GUI_CORE___SELECTION_CLIENT__HPP
#define GUI_CORE___SELECTION_CLIENT__HPP

/*  $Id: selection_client.hpp 38835 2017-06-23 18:15:53Z katargir $
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
#include <gui/gui_export.h>

#include <gui/core/selection_service.hpp>

BEGIN_NCBI_SCOPE

class IWorkbench;

class NCBI_GUICORE_EXPORT CSelectionClient : public CObjectEx, public ISelectionClient
{
public:
    CSelectionClient(const string& clientName)
        : m_ClientName(clientName), m_SelectionService() {}

    void SetSelectedObjects(TConstScopedObjects& objs);

    /// @name ISelectionClient interface implementation
    /// @{
    virtual void    GetSelection(TConstScopedObjects& objects) const;
    virtual void    SetSelectionService(ISelectionService* service);
    virtual string  GetSelClientLabel() { return m_ClientName; }
    virtual void    GetSelection(CSelectionEvent& evt) const;
    virtual void    SetSelection(CSelectionEvent&) {}   // do nothing, no support for selection broadcast
    virtual void    GetMainObject(TConstScopedObjects&) const {}
    virtual void    BroadcastSelection();
    /// @}

private:

    string m_ClientName;
    TConstScopedObjects m_Selection;
    ISelectionService*  m_SelectionService;
};

END_NCBI_SCOPE


#endif  // GUI_CORE___SELECTION_CLIENT__HPP

