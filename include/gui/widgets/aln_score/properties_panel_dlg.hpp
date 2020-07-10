#ifndef GUI_WIDGETS___DATA___METHOD_PROPERTIES_DLG__HPP
#define GUI_WIDGETS___DATA___METHOD_PROPERTIES_DLG__HPP
/*  $Id: properties_panel_dlg.hpp 25739 2012-04-27 18:55:44Z voronov $
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
 */

#include <gui/gui.hpp>

#include <wx/dialog.h>

BEGIN_NCBI_SCOPE

class IUIToolWithGUI;

class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CPropertiesPanelDlg: public wxDialog
{
    DECLARE_EVENT_TABLE()
public:
    /// Constructors
    CPropertiesPanelDlg(wxWindow* parent, IUIToolWithGUI* metod);

    /// Should we show tooltips?
    static bool ShowToolTips();

protected:
    /// Creates the controls and sizers
    void CreateControls(IUIToolWithGUI* metod);

    void OnOkClick ( wxCommandEvent& event );
    wxWindow* m_PropPanel;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS___DATA___METHOD_PROPERTIES_DLG__HPP
