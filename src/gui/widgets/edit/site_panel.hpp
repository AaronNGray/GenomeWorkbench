#ifndef GUI_WIDGETS_EDIT___SITE_PANEL__HPP
#define GUI_WIDGETS_EDIT___SITE_PANEL__HPP

/*  $Id: site_panel.hpp 27154 2012-12-31 20:33:35Z filippov $
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
 * Authors:  Roman Katargin and Igor Filippov
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */


#include <wx/panel.h>
#include <wx/choice.h>

#include <objects/seqfeat/SeqFeatData.hpp>
/*!
 * Forward declarations
 */


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*!
 * Control identifiers
 */

#define ID_CSITEPANEL  10037
#define ID_SITEPANEL_CHOICE 10038
#define SYMBOL_CSITEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSITEPANEL_TITLE _("Site Panel")
#define SYMBOL_CSITEPANEL_IDNAME ID_CSITEPANEL
#define SYMBOL_CSITEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSITEPANEL_POSITION wxDefaultPosition


class CSitePanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CSitePanel )
    DECLARE_EVENT_TABLE()
    
public:
    /// Constructors
    CSitePanel();
    CSitePanel( wxWindow* parent, CSeqFeatData::ESite site,
                 wxWindowID id = SYMBOL_CSITEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSITEPANEL_POSITION, const wxSize& size = SYMBOL_CSITEPANEL_SIZE, long style = SYMBOL_CSITEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSITEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSITEPANEL_POSITION, const wxSize& size = SYMBOL_CSITEPANEL_SIZE, long style = SYMBOL_CSITEPANEL_STYLE );

    /// Destructor
    ~CSitePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );


    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual bool TransferDataFromWindow();

    CSeqFeatData::ESite GetSite();
    
private:
    CSeqFeatData::ESite m_Site;   
    wxChoice* m_Choice;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___SITE_PANEL__HPP
