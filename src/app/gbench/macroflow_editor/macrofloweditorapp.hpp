/*  $Id:  macrofloweditorapp.h 38070 06/07/2017 15:12:46 17:48:35Z Igor Filippov$
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */
#ifndef _MACROFLOWEDITORAPP_H_
#define _MACROFLOWEDITORAPP_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/macrofloweditor.hpp>

//BEGIN_NCBI_SCOPE
/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
////@end includes

BEGIN_NCBI_SCOPE


/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

/*!
 * CMacroFlowEditorApp class declaration
 */

class CMacroFlowEditorApp: public wxApp
{    
    DECLARE_CLASS( CMacroFlowEditorApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    CMacroFlowEditorApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin CMacroFlowEditorApp event handler declarations
////@end CMacroFlowEditorApp event handler declarations

////@begin CMacroFlowEditorApp member function declarations
////@end CMacroFlowEditorApp member function declarations

private:
////@begin CMacroFlowEditorApp member variables
    CMacroFlowEditor *m_frame;
////@end CMacroFlowEditorApp member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(CMacroFlowEditorApp)
////@end declare app

END_NCBI_SCOPE
#endif
    // _MACROFLOWEDITORAPP_H_
