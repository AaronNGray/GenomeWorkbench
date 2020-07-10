/*  $Id: assemblyprogrampanel.hpp 26367 2012-09-05 12:56:41Z bollin $
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
 * Authors:  Colleen Bollin
 */
#ifndef _ASSEMBLYPROGRAMPANEL_H_
#define _ASSEMBLYPROGRAMPANEL_H_

#include <corelib/ncbistd.hpp>
#include <wx/panel.h>

#define ID_CASSEMBLYPROGRAMPANEL 10121
#define SYMBOL_CASSEMBLYPROGRAMPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CASSEMBLYPROGRAMPANEL_TITLE _("AssemblyProgramPanel")
#define SYMBOL_CASSEMBLYPROGRAMPANEL_IDNAME ID_CASSEMBLYPROGRAMPANEL
#define SYMBOL_CASSEMBLYPROGRAMPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CASSEMBLYPROGRAMPANEL_POSITION wxDefaultPosition

BEGIN_NCBI_SCOPE
class CAssemblyProgramPanel: public wxPanel
{    

public:
    /// Constructors
    CAssemblyProgramPanel() {};
    CAssemblyProgramPanel( wxWindow* parent, wxWindowID id = SYMBOL_CASSEMBLYPROGRAMPANEL_IDNAME, const wxPoint& pos = SYMBOL_CASSEMBLYPROGRAMPANEL_POSITION, const wxSize& size = SYMBOL_CASSEMBLYPROGRAMPANEL_SIZE, long style = SYMBOL_CASSEMBLYPROGRAMPANEL_STYLE ) {};


    /// Destructor
    ~CAssemblyProgramPanel() {};

    virtual void SetValue( const string& val){};
    virtual string GetValue () {return "";};
};
END_NCBI_SCOPE

#endif
    // _ASSEMBLYPROGRAMPANEL_H_

