#ifndef GUI_WIDGETS___LOADERS___TOOL_WIZARD__HPP
#define GUI_WIDGETS___LOADERS___TOOL_WIZARD__HPP

/*  $Id: tool_wizard.hpp 28944 2013-09-18 15:50:31Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/objutils/objects.hpp>

class wxPanel;
class wxWindow;


BEGIN_NCBI_SCOPE

class IServiceLocator;
class IWizardPage;
class IExecuteUnit;

class IToolWizard
{
public:
    virtual ~IToolWizard() {}

    virtual void	      SetParentWindow(wxWindow* parent) = 0;

    /// Returns first options page (if any) of the tool to be shown in wizard dialog
    virtual IWizardPage*  GetFirstPage() { return 0; }
    /// Returns page for tool options (long wizard path via "Options" button)
    virtual IWizardPage*  GetOptionsPage() { return 0; }
    /// Sets a wizard page which should show before the first page of the tool
    virtual void          SetPrevPage(IWizardPage*) {}
    /// Sets a wizard page which should show after the last page of the tool
    virtual void          SetNextPage(IWizardPage*) {}
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS___LOADERS___TOOL_WIZARD__HPP

