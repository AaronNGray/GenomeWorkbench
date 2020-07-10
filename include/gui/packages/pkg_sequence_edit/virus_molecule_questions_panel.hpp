/*  $Id: virus_molecule_questions_panel.hpp 27907 2013-04-23 19:22:51Z bollin $
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
#ifndef _VIRUS_MOLECULE_QUESTIONS_PANEL_H_
#define _VIRUS_MOLECULE_QUESTIONS_PANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/radiobut.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

class CSubPrep_panel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CVIRUSMOLECULEQUESTIONSPANEL 10114
#define ID_CDNA_FROM_MRNA 10115
#define ID_CDNA_FROM_GENOMIC 10116
#define ID_PURIFIED 10117
#define ID_WHOLE_CELL 10118
#define SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_TITLE _("VirusMoleculeQuestions")
#define SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_IDNAME ID_CVIRUSMOLECULEQUESTIONSPANEL
#define SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CVirusMoleculeQuestionsPanel class declaration
 */

class CVirusMoleculeQuestionsPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CVirusMoleculeQuestionsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CVirusMoleculeQuestionsPanel();
    CVirusMoleculeQuestionsPanel( wxWindow* parent, 
                                  objects::CSeq_entry_Handle seh, 
                                  ICommandProccessor* proccessor,
                                  wxWindowID id = SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_POSITION, const wxSize& size = SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_SIZE, long style = SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_POSITION, const wxSize& size = SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_SIZE, long style = SYMBOL_CVIRUSMOLECULEQUESTIONSPANEL_STYLE );

    /// Destructor
    ~CVirusMoleculeQuestionsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CVirusMoleculeQuestionsPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_CDNA_FROM_MRNA
    void OnCdnaFromMrnaSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_CDNA_FROM_GENOMIC
    void OnCdnaFromGenomicSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_PURIFIED
    void OnPurifiedSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_WHOLE_CELL
    void OnWholeCellSelected( wxCommandEvent& event );

////@end CVirusMoleculeQuestionsPanel event handler declarations

////@begin CVirusMoleculeQuestionsPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CVirusMoleculeQuestionsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CVirusMoleculeQuestionsPanel member variables
    wxRadioButton* m_CdnaFromMrna;
    wxRadioButton* m_CdnaFromGenomic;
    wxRadioButton* m_Purified;
    wxRadioButton* m_WholeCell;
////@end CVirusMoleculeQuestionsPanel member variables
    void OnDataChange ();

private:
    objects::CSeq_entry_Handle m_Seh;
    ICommandProccessor* m_CmdProcessor;

};

END_NCBI_SCOPE

#endif
    // _VIRUS_MOLECULE_QUESTIONS_PANEL_H_
