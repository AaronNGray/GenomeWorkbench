/*  $Id: country_conflict.cpp 43676 2019-08-14 14:28:05Z asztalos $
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
#include <ncbi_pch.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objects/seqtable/seq_table_exception.hpp>
#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <objtools/edit/source_edit.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <wx/msgdlg.h>

#include <gui/packages/pkg_sequence_edit/country_conflict.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);



IMPLEMENT_DYNAMIC_CLASS( CCountryConflict, wxDialog )

BEGIN_EVENT_TABLE( CCountryConflict, wxDialog )

    EVT_BUTTON( wxID_OK, CCountryConflict::OnClickOk )

    EVT_BUTTON( wxID_CANCEL, CCountryConflict::OnClickCancel )


END_EVENT_TABLE()

CCountryConflict::CCountryConflict()
{
    Init();
}

CCountryConflict::CCountryConflict( wxWindow* parent, CSeq_entry_Handle seh, 
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style ) : m_TopSeqEntry(seh)
{
    CBioseq_CI b_iter(m_TopSeqEntry,CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) 
    {
        m_Found = false;
        x_FindBioSource(b_iter->GetSeq_entry_Handle());
        if (!m_Found && b_iter->GetSeq_entry_Handle().HasParentEntry())
            x_FindBioSource(b_iter->GetSeq_entry_Handle().GetParentEntry());
    }
   
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CCountryConflict::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}

CCountryConflict::~CCountryConflict()
{
}


/*!
 * Member initialisation
 */

void CCountryConflict::Init()
{
    m_Grid=NULL;
}


void CCountryConflict::CreateControls()
{    
    CCountryConflict* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
   
    wxArrayString itemChoiceStrings, itemChoiceStringsWritable;

    if (m_LatLon.empty()) 
    {
        wxMessageBox(wxT("No LatLon records found"), wxT("Error"), wxOK | wxICON_ERROR);
        NCBI_THROW( CException, eUnknown, "No LatLon records found" );
    }

    CRef<CSeq_table> values_table = GetValuesTableFromSeqEntry();
    if (values_table->GetNum_rows() < 1)
    {
        wxMessageBox(wxT("No LatLon records found or all records correctly formatted"), wxT("Error"), wxOK | wxICON_ERROR);
        NCBI_THROW( CException, eUnknown, "No LatLon records found or all records correctly formatted" );
    }
    CRef<CSeq_table> choices = GetChoices(values_table);
    int glyph_col =   GetCollapsible();
    m_GridPanel = new CSeqTableGridPanel(this, values_table, choices, glyph_col);
    itemBoxSizer3->Add(m_GridPanel, 0, wxALIGN_TOP|wxALL, 5);
    m_Grid = m_GridPanel->GetGrid();
    CSeqTableGrid *gridAdapter = new CSeqTableGrid(values_table);
    m_Grid->SetTable(gridAdapter, true);
    m_Grid->AutoSizeColumns();
    int l_height = m_Grid->GetColLabelSize();
    m_Grid->SetColLabelSize( 2 * l_height );
    
    int pos = 0;
    ITERATE (CSeq_table::TColumns, it, values_table->GetColumns()) 
    {
        if (pos > 0) 
        {
            if ((*it)->IsSetHeader() && (*it)->GetHeader().IsSetTitle() )
            {
                string title = (*it)->GetHeader().GetTitle();
                if (!title.empty())
                {
                    itemChoiceStrings.Add(wxString(title));
                    if (!IsReadOnlyColumn(title)) 
                        itemChoiceStringsWritable.Add(wxString(title));
                }
                if (IsReadOnlyColumn(title)) 
                    m_GridPanel->MakeColumnReadOnly(pos - 1, true);
            } 
        }
        pos++;
    }
    
    if (glyph_col >= 0 && glyph_col+1 < m_Grid->GetNumberCols())
    {
        m_GridPanel->InitColumnCollapse(glyph_col+1);  
    }
  

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_StringConstraintPanel = new CStringConstraintSelect( itemDialog1, m_GridPanel, itemChoiceStrings, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_StringConstraintPanel, 0, wxALIGN_TOP|wxALL|wxFIXED_MINSIZE, 0);    

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

CRef<CSeq_table> CCountryConflict::GetValuesTableFromSeqEntry() 
{

    CRef<CSeqTable_column> id_col(new CSeqTable_column());
    id_col->SetHeader().SetField_id(CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);

    CRef<CSeqTable_column> expand_col(new CSeqTable_column());
    expand_col->SetHeader().SetTitle("");
    expand_col->SetHeader().SetField_name("expand");
    expand_col->SetData().SetString();

    CRef<CSeqTable_column> current_col(new CSeqTable_column());
    current_col->SetHeader().SetTitle("Lat-Lon");
    current_col->SetHeader().SetField_name("current");
    current_col->SetData().SetString();

    CRef<CSeqTable_column> country_col(new CSeqTable_column());
    country_col->SetHeader().SetTitle("Country");
    country_col->SetHeader().SetField_name("country");
    country_col->SetData().SetString();

    CRef<CSeqTable_column> suggested_col(new CSeqTable_column());
    suggested_col->SetHeader().SetTitle("Suggested Correction");
    suggested_col->SetHeader().SetField_name("suggested");
    suggested_col->SetData().SetString();

   
    // bogus column to include last, otherwise deletion of the previous column will not work
    CRef<CSeqTable_column> bogus_col(new CSeqTable_column());
    bogus_col->SetHeader().SetTitle("");
    bogus_col->SetHeader().SetField_name("");
    bogus_col->SetData().SetString();

    CRef<CSeq_table> table(new CSeq_table());
    table->SetColumns().push_back(id_col);
    table->SetColumns().push_back(expand_col);
    table->SetColumns().push_back(current_col);
    table->SetColumns().push_back(country_col);
    table->SetColumns().push_back(suggested_col);   
    table->SetColumns().push_back(bogus_col);

    size_t row = 0;
    m_ReplaceLatLon.clear();
    for ( vector<string>::iterator fi = m_LatLon.begin(); fi != m_LatLon.end() ; ++fi ) 
    {
        string country = m_Country[row];
        string latlon = *fi;
        CSubSource::ELatLonCountryErr errcode;
        string msg = CSubSource::ValidateLatLonCountry (country, latlon, false, errcode);
        if (errcode == CSubSource::eLatLonCountryErr_None)
            continue;
        CRef<CSeq_id> id(new CSeq_id());
        id->SetLocal().SetId(row);
        id_col->SetData().SetId().push_back(id);
        expand_col->SetData().SetString().push_back("");
        current_col->SetData().SetString().push_back(*fi);
        country_col->SetData().SetString().push_back(country);
        suggested_col->SetData().SetString().push_back(msg);
        bogus_col->SetData().SetString().push_back("");
    m_ReplaceLatLon.push_back(latlon);
        row++;
    }    
   
    table->SetNum_rows(row); 

    return table;
}


CRef<CCmdComposite> CCountryConflict::GetCommandFromValuesTable(CRef<CSeq_table> values_table) 
{
    bool select_all = false;
    if (!m_Grid->IsSelection() && values_table->GetNum_rows() > 0)
    {
        if (wxMessageBox(ToWxString("Apply to all?"), wxT("Nothing is selected"), wxOK | wxCANCEL, NULL) == wxOK)
        {
            select_all = true;
        }
    }

    m_LatLonCountry.clear();
    CRef<CCmdComposite> cmd(new CCmdComposite("Country Conflict Tool"));
    for (size_t row = 0; row < values_table->GetColumn("current").GetData().GetString().size(); ++row)
    {
    string old_latlon = values_table->GetColumn("current").GetData().GetString()[row]; 
    string old_country = m_Country[row];
    string new_latlon = m_ReplaceLatLon[row];
    string new_country = values_table->GetColumn("country").GetData().GetString()[row];
    if ((old_latlon != new_latlon || old_country != new_country) 
        && (m_Grid->IsInSelection(row,0) || select_all))
        m_LatLonCountry[make_pair(old_latlon, old_country)] = make_pair(new_latlon, new_country);
    }
    
    
    CBioseq_CI b_iter(m_TopSeqEntry,CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) 
    {
        m_Found = false;
        x_FindBioSource(b_iter->GetSeq_entry_Handle(), cmd);
        if (!m_Found && b_iter->GetSeq_entry_Handle().HasParentEntry())
            x_FindBioSource(b_iter->GetSeq_entry_Handle().GetParentEntry(),cmd);
    }
 
    return cmd;  
}

CRef<CCmdComposite> CCountryConflict::GetCommand()
{
    CRef<CSeq_table> values_table = m_GridPanel->GetValuesTable();
    CRef<CCmdComposite> cmd = GetCommandFromValuesTable(values_table);
    return cmd;
}

string CCountryConflict::GetErrorMessage()
{
    return "Invalid operation in Country Conflict Tool";
}


/*!
 * Should we show tooltips?
 */

bool CCountryConflict::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCountryConflict::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCountryConflict bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCountryConflict bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCountryConflict::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCountryConflict icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCountryConflict icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void CCountryConflict::OnClickOk( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in CCountryConflict.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in CCountryConflict. 
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void CCountryConflict::OnClickCancel( wxCommandEvent& event )
{    
    bool modified = m_GridPanel->GetModified();
    if (modified)
    {
        wxMessageDialog dlg(this,_("Discard modifications?"), _("Attention"),wxOK|wxCANCEL|wxCENTRE);
        if (dlg.ShowModal() == wxID_OK)
        {
            event.Skip();
        }
    }
    else
        event.Skip();
}

void CCountryConflict::x_GatherLatLon(const CBioSource& biosource)
{
    string latlon;
    FOR_EACH_SUBSOURCE_ON_BIOSOURCE(subsource, biosource)
    {
        if ((*subsource)->IsSetSubtype() &&
            (*subsource)->GetSubtype() == CSubSource::eSubtype_lat_lon
            && (*subsource)->IsSetName())
        {
            latlon = (*subsource)->GetName();
            break;
        }
    }

    string country;
    FOR_EACH_SUBSOURCE_ON_BIOSOURCE(subsource, biosource)
    {
        if ((*subsource)->IsSetSubtype() &&
            (*subsource)->GetSubtype() == CSubSource::eSubtype_country
            && (*subsource)->IsSetName())
        {
            country = (*subsource)->GetName();
            break;
        }
    }
    if (!latlon.empty() && !country.empty())
    {
        m_LatLon.push_back(latlon);
        m_Country.push_back(country);
        m_Found = true;
    }
}

bool CCountryConflict::x_ApplyToBioSource (CBioSource& biosource)
{
    bool modified = false;

    string latlon;
    FOR_EACH_SUBSOURCE_ON_BIOSOURCE(subsource,biosource)
    {
        if ((*subsource)->IsSetSubtype() &&
            (*subsource)->GetSubtype() == CSubSource::eSubtype_lat_lon && 
            (*subsource)->IsSetName())
        {
            latlon = (*subsource)->GetName();    
            break;
        }
    }

    string country;
    FOR_EACH_SUBSOURCE_ON_BIOSOURCE(subsource,biosource)
    {
        if ((*subsource)->IsSetSubtype() &&
            (*subsource)->GetSubtype() == CSubSource::eSubtype_country && 
            (*subsource)->IsSetName())
        {
            country = (*subsource)->GetName();              
            break;
     
        }
    }
    if (latlon.empty() || country.empty())
    {
        return modified;
    }

    m_Found = true;
    auto it = m_LatLonCountry.find(make_pair(latlon, country));
    if (it != m_LatLonCountry.end()) {
        string new_latlon = it->second.first;
        string new_country = it->second.second;

        EDIT_EACH_SUBSOURCE_ON_BIOSOURCE(subsource, biosource)
        {
            if ((*subsource)->IsSetSubtype() &&
                (*subsource)->GetSubtype() == CSubSource::eSubtype_lat_lon &&
                (*subsource)->IsSetName())
            {
                (*subsource)->SetName(new_latlon);               
                modified = true;
                break;
            }
        }
        EDIT_EACH_SUBSOURCE_ON_BIOSOURCE(subsource, biosource)
        {
            if ((*subsource)->IsSetSubtype() &&
                (*subsource)->GetSubtype() == CSubSource::eSubtype_country
                && (*subsource)->IsSetName())
            {
                (*subsource)->SetName(new_country);
                modified = true;
                break;
            }
        }
    }
    return modified;
}

void CCountryConflict::x_ApplyToDescriptors(const CSeq_entry& se, CCmdComposite* composite)
{
    if (!composite) {
        FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
            if ((*it)->IsSource()) {
                x_GatherLatLon((*it)->GetSource());
            }
        }
    }
    else {
        FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
            if ((*it)->IsSource()) {
                const CSeqdesc& orig_desc = **it;
                CRef<CSeqdesc> new_desc(new CSeqdesc);
                new_desc->Assign(orig_desc);
                if (x_ApplyToBioSource(new_desc->SetSource())) {
                    CRef<CCmdChangeSeqdesc> cmd(new
                        CCmdChangeSeqdesc(m_TopSeqEntry.GetScope().GetSeq_entryHandle(se), orig_desc, *new_desc));
                    composite->AddCommand(*cmd);
                }
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet()) {
            x_ApplyToDescriptors(**it, composite);
        }
    }
}

void CCountryConflict::x_FindBioSource(CSeq_entry_Handle tse, CCmdComposite* composite)
{
    x_ApplyToDescriptors(*(tse.GetCompleteSeq_entry()), composite);

    if (!composite) {
        for (CFeat_CI feat_it(tse, SAnnotSelector(CSeqFeatData::e_Biosrc)); feat_it; ++feat_it) {
            x_GatherLatLon(feat_it->GetData().GetBiosrc());
        }
    }
    else {
        for (CFeat_CI feat_it(tse, SAnnotSelector(CSeqFeatData::e_Biosrc)); feat_it; ++feat_it) {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(feat_it->GetOriginalFeature());
            if (x_ApplyToBioSource(new_feat->SetData().SetBiosrc())) {
                CRef<CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(*feat_it, *new_feat));
                composite->AddCommand(*cmd);
            }
        }
    }
}


END_NCBI_SCOPE
