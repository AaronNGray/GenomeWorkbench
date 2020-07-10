/*  $Id: create_protein_id.hpp 33664 2015-09-02 15:51:36Z filippov $
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
#ifndef _CREATE_PROTEIN_ID_H_
#define _CREATE_PROTEIN_ID_H_

#include <corelib/ncbistd.hpp>

#include <wx/window.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CCreateProteinId
{
public:
    static void apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, wxWindow *parent);
    static void create_protein_ids(CSeq_feat_Handle fh, const string &dbname, const string &locus_tag, CScope &scope, CRef<CCmdComposite> cmd);
};

class CCreateLocusTagGene
{
public:
    static void apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, wxWindow *parent);
};


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCREATELOCUSTAGGENESDLG 10000
#define ID_CREATE_LOCUSTAG_GENE_PREFIX 10001
#define ID_CREATE_LOCUSTAG_GENES_PROT_ID 10002
#define ID_CREATE_LOCUSTAG_GENE_DATABASE 10003
#define SYMBOL_CCREATELOCUSTAGGENESDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCREATELOCUSTAGGENESDLG_TITLE _("Create locus-tag genes")
#define SYMBOL_CCREATELOCUSTAGGENESDLG_IDNAME ID_CCREATELOCUSTAGGENESDLG
#define SYMBOL_CCREATELOCUSTAGGENESDLG_SIZE wxSize(400, 300)
#define SYMBOL_CCREATELOCUSTAGGENESDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CCreateLocusTagGenesDlg class declaration
 */

class CCreateLocusTagGenesDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CCreateLocusTagGenesDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCreateLocusTagGenesDlg();
    CCreateLocusTagGenesDlg( wxWindow* parent, wxWindowID id = SYMBOL_CCREATELOCUSTAGGENESDLG_IDNAME, const wxString& caption = SYMBOL_CCREATELOCUSTAGGENESDLG_TITLE, const wxPoint& pos = SYMBOL_CCREATELOCUSTAGGENESDLG_POSITION, const wxSize& size = SYMBOL_CCREATELOCUSTAGGENESDLG_SIZE, long style = SYMBOL_CCREATELOCUSTAGGENESDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCREATELOCUSTAGGENESDLG_IDNAME, const wxString& caption = SYMBOL_CCREATELOCUSTAGGENESDLG_TITLE, const wxPoint& pos = SYMBOL_CCREATELOCUSTAGGENESDLG_POSITION, const wxSize& size = SYMBOL_CCREATELOCUSTAGGENESDLG_SIZE, long style = SYMBOL_CCREATELOCUSTAGGENESDLG_STYLE );

    /// Destructor
    ~CCreateLocusTagGenesDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCreateLocusTagGenesDlg event handler declarations

////@end CCreateLocusTagGenesDlg event handler declarations

////@begin CCreateLocusTagGenesDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCreateLocusTagGenesDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    string GetPrefix() {return m_Prefix->GetValue().ToStdString();}
    bool GetCreateProtIds() {return m_CreateProtIds->GetValue();}
    string GetDatabase() {return m_Database->GetValue().ToStdString();}
private:
////@begin CCreateLocusTagGenesDlg member variables
    wxTextCtrl* m_Prefix;
    wxCheckBox* m_CreateProtIds;
    wxTextCtrl* m_Database;
////@end CCreateLocusTagGenesDlg member variables
};


END_NCBI_SCOPE

#endif
    // _CREATE_PROTEIN_ID_H_
