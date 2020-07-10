#ifndef PKG_ALIGNMENT___NET_BLAST_LOAD_OPTION_PANEL__HPP
#define PKG_ALIGNMENT___NET_BLAST_LOAD_OPTION_PANEL__HPP

/*  $Id: net_blast_load_option_panel.hpp 36498 2016-09-29 19:56:56Z katargir $
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
#include <gui/widgets/wx/richtextctrl.hpp>

////@begin includes
////@end includes

////@begin forward declarations
////@end forward declarations

#include <wx/panel.h>

////@begin control identifiers
#define SYMBOL_CNETBLASTLOADOPTIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CNETBLASTLOADOPTIONPANEL_TITLE _("Net BLAST Load Option Panel")
#define SYMBOL_CNETBLASTLOADOPTIONPANEL_IDNAME ID_NETBLASTLOADOPTIONPANEL
#define SYMBOL_CNETBLASTLOADOPTIONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CNETBLASTLOADOPTIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */


class CNetBLASTLoadOptionPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CNetBLASTLoadOptionPanel )
    DECLARE_EVENT_TABLE()

public:
    CNetBLASTLoadOptionPanel();
    CNetBLASTLoadOptionPanel( wxWindow* parent, wxWindowID id = SYMBOL_CNETBLASTLOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CNETBLASTLOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CNETBLASTLOADOPTIONPANEL_SIZE, long style = SYMBOL_CNETBLASTLOADOPTIONPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CNETBLASTLOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CNETBLASTLOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CNETBLASTLOADOPTIONPANEL_SIZE, long style = SYMBOL_CNETBLASTLOADOPTIONPANEL_STYLE );

    ~CNetBLASTLoadOptionPanel();

    void Init();

    void CreateControls();

////@begin CNetBLASTLoadOptionPanel event handler declarations

////@end CNetBLASTLoadOptionPanel event handler declarations

////@begin CNetBLASTLoadOptionPanel member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CNetBLASTLoadOptionPanel member function declarations

    static bool ShowToolTips();

////@begin CNetBLASTLoadOptionPanel member variables
    enum {
        ID_NETBLASTLOADOPTIONPANEL = 10013,
        ID_TEXTCTRL1 = 10044
    };
////@end CNetBLASTLoadOptionPanel member variables

public:
    string  GetInput() const;
    void    SetInput(const string& input);

    bool    IsInputValid();

    void GetRIDs(vector<string>& rids);
};


/* @} */

END_NCBI_SCOPE

#endif
    // PKG_ALIGNMENT___NET_BLAST_LOAD_OPTION_PANEL__HPP
