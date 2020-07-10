/*  $Id: country_fixup.cpp 43676 2019-08-14 14:28:05Z asztalos $
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

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <objmgr/util/sequence.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <objects/seqtable/seq_table_exception.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/packages/pkg_sequence_edit/country_fixup.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> CCountryFixup::GetCommand(CSeq_entry_Handle tse, bool capitalize_after_colon)
{
    if (!tse)
        return CRef<CCmdComposite>();

    m_Seh = tse;
    m_capitalize_after_colon = capitalize_after_colon;
    string title = "Country Fixup Do Not Fix Capitalization After Colon";
    if (m_capitalize_after_colon) {
        title = "Country Fixup Fix Capitalization After Colon";
    }

    m_invalid_countries.clear();
    m_fixed_countries.clear();
    CRef<CCmdComposite> composite(new CCmdComposite(title));
    x_ApplyToSeqAndFeat(composite);

    if (!m_invalid_countries.empty())
    {
        CBulkCountryEdit dlg(NULL, m_invalid_countries);
        if (dlg.ShowModal() == wxID_OK)
        {
            m_fixed_countries = dlg.GetValues();
            composite.Reset(new CCmdComposite(title));
            x_ApplyToSeqAndFeat(composite);
        }
    }
    return composite;
}

static string s_GetAccession(CBioseq_Handle bsh)
{
    string accession;
    if (bsh)
    {
        CSeq_id_Handle best = sequence::GetId(bsh, sequence::eGetId_Best);
        int version;
        best.GetSeqId()->GetLabel(&accession, &version, CSeq_id::eContent);
    }
    return accession;
}

void CCountryFixup::x_ApplyToSeqAndFeat(CCmdComposite* composite)
{
    x_ApplyToDescriptors(*(m_Seh.GetCompleteSeq_entry()), composite);

    CScope& scope = m_Seh.GetScope();
    for (CFeat_CI feat_it(m_Seh, SAnnotSelector(CSeqFeatData::e_Biosrc)); feat_it; ++feat_it) {
        m_accession = s_GetAccession(scope.GetBioseqHandle(feat_it->GetLocation()));
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat_it->GetOriginalFeature());
        if (x_ApplyToBioSource(new_feat->SetData().SetBiosrc())) {
            CRef<CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(*feat_it, *new_feat));
            composite->AddCommand(*cmd);
        }
    }
}


void CCountryFixup::x_ApplyToDescriptors(const CSeq_entry& se, CCmdComposite* composite)
{
    CScope& scope = m_Seh.GetScope();
    FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
        if ((*it)->IsSource()) {
            const CSeqdesc& orig_desc = **it;
            if (se.IsSeq()) {
                m_accession = s_GetAccession(scope.GetBioseqHandle(se.GetSeq()));
            }
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(orig_desc);
            if (x_ApplyToBioSource(new_desc->SetSource())) {
                CRef<CCmdChangeSeqdesc> cmd(new 
                    CCmdChangeSeqdesc(scope.GetSeq_entryHandle(se), orig_desc, *new_desc));
                composite->AddCommand(*cmd);
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet()) {
            x_ApplyToDescriptors(**it, composite);
        }
    }
}


bool CCountryFixup::x_ApplyToBioSource(CBioSource& biosource)
{
    bool modified = false;

    EDIT_EACH_SUBSOURCE_ON_BIOSOURCE(subsource, biosource)
        if ((*subsource)->IsSetSubtype() && 
            (*subsource)->IsSetName() && 
            (*subsource)->GetSubtype() == CSubSource::eSubtype_country)
        {
            string country = (*subsource)->GetName();
            string new_country = CCountries::CountryFixupItem(country, m_capitalize_after_colon);
            if (!new_country.empty() && new_country != country)
            {
                (*subsource)->SetName(new_country);
                modified = true;
            }
            else if (m_fixed_countries.find(country) != m_fixed_countries.end())
            {
                (*subsource)->SetName(m_fixed_countries[country]);
                modified = true;
            }
            else if (!CCountries::IsValid(country))
            {
                m_invalid_countries.push_back(pair<string, string>(country, m_accession));
            }
        }
    
    return modified;
}


IMPLEMENT_DYNAMIC_CLASS( CBulkCountryEdit, wxDialog )

BEGIN_EVENT_TABLE( CBulkCountryEdit, wxDialog )

    EVT_BUTTON( wxID_CANCEL, CBulkCountryEdit::OnClickCancel )

END_EVENT_TABLE()

CBulkCountryEdit::CBulkCountryEdit()
{
    Init();
}


CBulkCountryEdit::CBulkCountryEdit( wxWindow* parent,  const vector<pair<string,string> > &invalid_countries, 
                    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_invalid_countries(invalid_countries)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CBulkCountryEdit::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
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


CBulkCountryEdit::~CBulkCountryEdit()
{
}


/*!
 * Member initialisation
 */

void CBulkCountryEdit::Init()
{
    m_Grid=NULL;
}


void CBulkCountryEdit::CreateControls()
{    
    CBulkCountryEdit* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
   
    wxArrayString itemChoiceStrings, itemChoiceStringsWritable;

    CRef<CSeq_table> values_table = GetValuesTable();
    CRef<CSeq_table> choices = GetChoices(values_table);
    int glyph_col =   GetCollapsible();
    m_GridPanel = new CSeqTableGridPanel(this, values_table, choices, glyph_col);
    itemBoxSizer3->Add(m_GridPanel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
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
    itemBoxSizer4->Add(m_StringConstraintPanel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxFIXED_MINSIZE, 0);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AecrPanel = new CApplyEditconvertPanel( itemDialog1, m_GridPanel, itemChoiceStringsWritable, 0, true, wxID_ANY, wxDefaultPosition, wxSize(1128, 219));
    itemBoxSizer5->Add(m_AecrPanel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxFIXED_MINSIZE, 1);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

CRef<CSeq_table> CBulkCountryEdit::GetValuesTable() 
{

    CRef<CSeqTable_column> id_col(new CSeqTable_column());
    id_col->SetHeader().SetField_id(CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);

    CRef<CSeqTable_column> expand_col(new CSeqTable_column());
    expand_col->SetHeader().SetTitle("");
    expand_col->SetHeader().SetField_name("expand");
    expand_col->SetData().SetString();

    CRef<CSeqTable_column> country_col(new CSeqTable_column());
    country_col->SetHeader().SetTitle("Country");
    country_col->SetHeader().SetField_name("country");
    country_col->SetData().SetString();

    CRef<CSeqTable_column> accession_col(new CSeqTable_column());
    accession_col->SetHeader().SetTitle("Accession");
    accession_col->SetHeader().SetField_name("accession");
    accession_col->SetData().SetString();

    // bogus column to include last, otherwise deletion of the previous column will not work
    CRef<CSeqTable_column> bogus_col(new CSeqTable_column());
    bogus_col->SetHeader().SetTitle("");
    bogus_col->SetHeader().SetField_name("");
    bogus_col->SetData().SetString();

    CRef<CSeq_table> table(new CSeq_table());
    table->SetColumns().push_back(id_col);
    table->SetColumns().push_back(expand_col);
    table->SetColumns().push_back(country_col);
    table->SetColumns().push_back(accession_col);
    table->SetColumns().push_back(bogus_col);

    size_t row = 0;
    for( vector<pair<string,string> >::const_iterator it = m_invalid_countries.begin(); it != m_invalid_countries.end(); ++it)
    {
        string country = it->first;
        string accession = it->second;

        CRef<CSeq_id> id(new CSeq_id());
        id->SetLocal().SetId(row);

        id_col->SetData().SetId().push_back(id);
        expand_col->SetData().SetString().push_back("");
        country_col->SetData().SetString().push_back(country);
        accession_col->SetData().SetString().push_back(accession);
        bogus_col->SetData().SetString().push_back("");
        row++;
    }
   
    table->SetNum_rows(row); 

    return table;
}


map<string,string> CBulkCountryEdit::GetValuesFromValuesTable(CRef<CSeq_table> values_table) 
{
    map<string,string> result;
    size_t row = 0;
    for( vector<pair<string,string> >::const_iterator it = m_invalid_countries.begin(); it != m_invalid_countries.end(); ++it)
    {
        string country = it->first;
        string accession = it->second;
    
        string new_country;
        try
        {
            if (row < values_table->GetColumn("country").GetData().GetString().size())
                new_country = values_table->GetColumn("country").GetData().GetString()[row];
        } catch(CSeqTableException& ) {}
             
        if (new_country != country && !new_country.empty())
        {
            result[country] = new_country;
        }
        row++;
    }            
 
 return result;  
}

map<string,string> CBulkCountryEdit::GetValues()
{
    CRef<CSeq_table> values_table = m_GridPanel->GetValuesTable();
    map<string,string> fixed_countries = GetValuesFromValuesTable(values_table);
    return fixed_countries;
}

string CBulkCountryEdit::GetErrorMessage()
{
    return "Invalid operation in Bulk Country Edit";
}


/*!
 * Should we show tooltips?
 */

bool CBulkCountryEdit::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBulkCountryEdit::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBulkCountryEdit bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBulkCountryEdit bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBulkCountryEdit::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBulkCountryEdit icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBulkCountryEdit icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void CBulkCountryEdit::OnClickCancel( wxCommandEvent& event )
{    
    bool modified = m_GridPanel->GetModified() | m_AecrPanel->GetModified();
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

END_NCBI_SCOPE
