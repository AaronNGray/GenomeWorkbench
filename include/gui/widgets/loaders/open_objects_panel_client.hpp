#ifndef GUI_WIDGETS___LOADERS___OPEN_OBJECTS_PANEL_CLIENT__HPP
#define GUI_WIDGETS___LOADERS___OPEN_OBJECTS_PANEL_CLIENT__HPP

/*  $Id: open_objects_panel_client.hpp 28937 2013-09-17 21:26:20Z katargir $
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

class wxPanel;

BEGIN_NCBI_SCOPE

class IExecuteUnit;
class IWizardPage;

class IOpenObjectsPanelClient
{
public:
	virtual ~IOpenObjectsPanelClient() {}

	virtual string        GetLabel() const = 0;
    virtual IExecuteUnit* GetExecuteUnit() { return 0; }
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS___LOADERS___OPEN_OBJECTS_PANEL_CLIENT__HPP

