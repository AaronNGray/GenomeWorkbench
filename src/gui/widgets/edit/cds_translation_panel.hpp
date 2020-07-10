/*  $Id: cds_translation_panel.hpp 42778 2019-04-11 16:12:41Z filippov $
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
#ifndef _CDSTRANSLATION_PANEL_H_
#define _CDSTRANSLATION_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seq/Bioseq.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/statline.h"
#include "wx/hyperlink.h"
////@end includes

#include "wx/combobox.h"
#include <wx/odcombo.h>
#include "wx/checkbox.h"
#include <wx/toplevel.h>
#include <wx/stattext.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxHyperlinkCtrl;
////@end forward declarations

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects) // namespace ncbi::objects::
class CScope;
END_SCOPE(objects)

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CCDSTRANSLATIONPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCDSTRANSLATIONPANEL_TITLE _("CDS TranslationPanel")
#define SYMBOL_CCDSTRANSLATIONPANEL_IDNAME ID_CCDSTRANSLATIONPANEL
#define SYMBOL_CCDSTRANSLATIONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCDSTRANSLATIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CCDSTranslationPanel class declaration
 */

class CCDSTranslationPanel: public wxPanel,  public IRegSettings
{
    DECLARE_DYNAMIC_CLASS( CCDSTranslationPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCDSTranslationPanel();
    CCDSTranslationPanel( wxWindow* parent, objects::CSeq_feat& feat, objects::CScope& scope,
        wxWindowID id = SYMBOL_CCDSTRANSLATIONPANEL_IDNAME,
        const wxString& caption = SYMBOL_CCDSTRANSLATIONPANEL_TITLE,
        const wxPoint& pos = SYMBOL_CCDSTRANSLATIONPANEL_POSITION,
        const wxSize& size = SYMBOL_CCDSTRANSLATIONPANEL_SIZE,
        long style = SYMBOL_CCDSTRANSLATIONPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent,
        wxWindowID id = SYMBOL_CCDSTRANSLATIONPANEL_IDNAME,
        const wxString& caption = SYMBOL_CCDSTRANSLATIONPANEL_TITLE,
        const wxPoint& pos = SYMBOL_CCDSTRANSLATIONPANEL_POSITION,
        const wxSize& size = SYMBOL_CCDSTRANSLATIONPANEL_SIZE,
        long style = SYMBOL_CCDSTRANSLATIONPANEL_STYLE );

    /// Destructor
    ~CCDSTranslationPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void SetProduct(objects::CSeq_feat& feat);
    void SetProtein(objects::CBioseq& bioseq);

////@begin CCDSTranslationPanel event handler declarations

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_PREDICT_INTERVAL
    void OnPredictIntervalHyperlinkClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_RETRANSLATE_LINK
    void OnRetranslateLinkHyperlinkClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL2
    void AdjustForStopCodonHyperlinkClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_IMPORT_CDS_PROTEIN
    void OnImportCdsProteinHyperlinkClicked( wxHyperlinkEvent& event );

    void OnProductChanged(wxCommandEvent& event);

    void OnRetranslateOnOkChanged(wxCommandEvent& event);
////@end CCDSTranslationPanel event handler declarations

////@begin CCDSTranslationPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCDSTranslationPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCDSTranslationPanel member variables
    wxChoice* m_GeneticCode;
    wxChoice* m_Frame;
    wxCheckBox* m_ConflictCtrl;
    wxStaticText* m_ProteinLengthTxt;
    wxOwnerDrawnComboBox* m_ProductCtrl;
    wxCheckBox* m_ForceNear;
    wxTextCtrl* m_TranslationCtrl;
    wxHyperlinkCtrl* m_RetranslateLink;
    wxCheckBox* m_RetranslateOnOkBtn;
    wxCheckBox* m_UpdatemRNASpan;
    wxCheckBox* m_UpdateGeneSpan;
    /// Control identifiers
    enum {
        ID_CCDSTRANSLATIONPANEL = 10079,
        ID_CHOICE1,
        ID_CHOICE2,
        ID_CHECKBOX,
        ID_COMBOBOX,
        ID_FORCE_NEAR_BTN,
        ID_TEXTCTRL27,
        ID_PREDICT_INTERVAL,
        ID_RETRANSLATE_LINK,
        ID_HYPERLINKCTRL2,
        ID_IMPORT_CDS_PROTEIN,
        ID_RETRANSLATE_ON_OK_BTN,
        ID_CHECKBOX3,
        ID_UPDATE_GENE_SPAN
    };
////@end CCDSTranslationPanel member variables
    CRef<objects::CSeq_feat> m_EditedFeat;
    objects::CScope *m_Scope;
    CRef<objects::CBioseq> m_EditedBioseq;

    bool ShouldUpdatemRNALocation() { return m_UpdatemRNASpan->GetValue(); }
    bool ShouldUpdateGeneLocation() { return m_UpdateGeneSpan->GetValue(); }
    bool ShouldRetranslate() { return (m_RetranslateOnOkBtn->GetValue()); }
    void Retranslate();
    bool ShouldCreateNewProductSeq();
    void SetProductId(const string& val);
    CRef<objects::CSeq_id> GetProductId();

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    void DisableRetranslateOnOk(bool exception, bool pseudo);
    void UpdateForceNear();
    static bool IsValidException(const string& text);

private:
    wxTreebook* x_GetTree();
    bool x_IsPseudo();
    void x_SetLocationForProtein(CRef<objects::CSeq_feat> cds, objects::CSeq_entry_Handle seh);
    void x_SetProtein(const objects::CBioseq& protein, CRef<objects::CSeq_feat> cds, objects::CSeq_entry_Handle seh);
    bool x_CollectData();
    void x_NormalizeGeneticCodeName(string& code_name);
    void x_Translate(void);
    void x_UpdateForceNear(const string &val);
    bool m_add_stop_codon;
    string m_RegPath;
    map<string, CConstRef<objects::CSeq_id> > m_SeqIds;
};

END_NCBI_SCOPE

#endif
    // _CDSTRANSLATION_PANEL_H_
