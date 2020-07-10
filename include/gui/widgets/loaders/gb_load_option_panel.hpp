#ifndef PKG_SEQUENCE___GB_LOAD_OPTION_PANEL__HPP
#define PKG_SEQUENCE___GB_LOAD_OPTION_PANEL__HPP

/*  $Id: gb_load_option_panel.hpp 40018 2017-12-11 21:37:08Z katargir $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <wx/panel.h>
#include <gui/widgets/wx/richtextctrl.hpp>
#include <gui/widgets/wx/htmlwin.hpp>

#include <gui/utils/mru_list.hpp>
#include <gui/objutils/registry.hpp>

#include <util/icanceled.hpp>

////@begin control identifiers
#define SYMBOL_CGENBANKLOADOPTIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CGENBANKLOADOPTIONPANEL_TITLE _("GenBank Load Option Panel")
#define SYMBOL_CGENBANKLOADOPTIONPANEL_IDNAME ID_CGENBANKLOADOPTIONPANEL
#define SYMBOL_CGENBANKLOADOPTIONPANEL_SIZE wxSize(266, 184)
#define SYMBOL_CGENBANKLOADOPTIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

BEGIN_SCOPE(objects);
    class CSeq_id;
END_SCOPE(objects);

class CwxHtmlWindow;

///////////////////////////////////////////////////////////////////////////////
/// CGenBankLoadOptionPanel

class NCBI_GUIWIDGETS_LOADERS_EXPORT CGenBankLoadOptionPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CGenBankLoadOptionPanel )
    DECLARE_EVENT_TABLE()

public:
    class CAccDescriptor
    {
    public:
        CAccDescriptor(const string& accession)
                        : m_Accession(accession) {}

        bool operator==(const CAccDescriptor& ad) 
        { 
            return (m_Accession == ad.m_Accession);
        }

        string GetAccession() const { return m_Accession; }
    private:
        string m_Accession;
    };

    typedef CTimeMRUList<CAccDescriptor> TMRUAccList;

    CGenBankLoadOptionPanel();
    CGenBankLoadOptionPanel( wxWindow* parent, wxWindowID id = SYMBOL_CGENBANKLOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CGENBANKLOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CGENBANKLOADOPTIONPANEL_SIZE, long style = SYMBOL_CGENBANKLOADOPTIONPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGENBANKLOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CGENBANKLOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CGENBANKLOADOPTIONPANEL_SIZE, long style = SYMBOL_CGENBANKLOADOPTIONPANEL_STYLE );

    ~CGenBankLoadOptionPanel();

    void Init();

    void CreateControls();

////@begin CGenBankLoadOptionPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_ACC_INPUT
    void OnAccInputUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_HTML_LINK_CLICKED event handler for ID_HTMLWINDOW2
    void OnRecentAccClicked( wxHtmlLinkEvent& event );

////@end CGenBankLoadOptionPanel event handler declarations

////@begin CGenBankLoadOptionPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGenBankLoadOptionPanel member function declarations

    static bool ShowToolTips();

////@begin CGenBankLoadOptionPanel member variables
    CRichTextCtrl* m_AccInput;
    CwxHtmlWindow* m_MRUWindow;
    /// Control identifiers
    enum {
        ID_CGENBANKLOADOPTIONPANEL = 10013,
        ID_ACC_INPUT = 10022,
        ID_HTMLWINDOW2 = 10002
    };
////@end CGenBankLoadOptionPanel member variables

public:
    typedef vector< CRef<CObject> > TIdsVec;

    void SaveSettings(const string& regPath);
    void LoadSettings(const string& regPath);

    void SaveMruAccessions(const string& regPath);

    string GetInput() const;
    void SetInput(const string& input);

    bool IsInputValid();
    TIdsVec& GetSeqIds();
    const vector<string>& GetNAs() const { return m_NAs; }
    const vector<string>& GetGenomicAccessions() const { return m_GenomicAccessions; }

private:
    void x_ValidateInput();
    void x_FillMRUList();

    struct Token
    {
        Token() : m_Pos(0), m_Size(0), m_Valid(false), m_GenomicAccession(false) {}
        int m_Pos;
        int m_Size;
        bool m_Valid;
        bool m_GenomicAccession;
    };

    int x_ProccessText(const string& text, vector<Token>& tokens, ICanceled& canceled);

    wxTextAttr  m_DefStyle;
    wxTextAttr  m_ErrStyle;
    int     m_TokensNum; // total number of tokens
    int     m_ErrNum; // number of token with errors

    TIdsVec        m_Ids;
    vector<string> m_NAs;
    vector<string> m_GenomicAccessions;

    vector<string> m_ValidTokens;
    
    TMRUAccList m_AccMRUList;
};


/* @} */

END_NCBI_SCOPE

#endif
    // PKG_SEQUENCE___GB_LOAD_OPTION_PANEL__HPP
