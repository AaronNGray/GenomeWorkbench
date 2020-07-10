/*  $Id: exon_from_cds.cpp 41526 2018-08-15 15:23:45Z filippov $
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

////@begin includes
////@end includes

#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/statbox.h>
#include <wx/valtext.h>

#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <gui/objutils/cmd_add_cds.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/packages/pkg_sequence_edit/exon_from_cds.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CExonFromCds type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CExonFromCds, CBulkCmdDlg )


/*!
 * CExonFromCds event table definition
 */

BEGIN_EVENT_TABLE( CExonFromCds, CBulkCmdDlg )

////@begin CExonFromCds event table entries
   
////@end CExonFromCds event table entries

END_EVENT_TABLE()


/*!
 * CExonFromCds constructors
 */

CExonFromCds::CExonFromCds()
{
    Init();
}

CExonFromCds::CExonFromCds( wxWindow* parent, IWorkbench* wb, CSeqFeatData::ESubtype subtype, const string &title, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : CBulkCmdDlg(wb), m_ExonNum(0), m_Type(subtype), m_Title(title)
{
    Init();
    Create(parent, id, m_Title, pos, size, style);
}


/*!
 * CExonFromCds creator
 */

bool CExonFromCds::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CExonFromCds creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CExonFromCds creation
    return true;
}


/*!
 * CExonFromCds destructor
 */

CExonFromCds::~CExonFromCds()
{
////@begin CExonFromCds destruction
////@end CExonFromCds destruction
}


/*!
 * Member initialisation
 */

void CExonFromCds::Init()
{
////@begin CExonFromCds member initialisation
    m_OkCancel = NULL;
////@end CExonFromCds member initialisation
    m_ErrorMessage = "";
    m_StringConstraintPanel = NULL;
}


/*!
 * Control creation for CExonFromCds
 */

void CExonFromCds::CreateControls()
{    
////@begin CExonFromCds content construction
    CExonFromCds* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);


    GetTopLevelSeqEntryAndProcessor();

  
    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* stattext2 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("First Exon Number"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(stattext2, 0, wxALIGN_TOP|wxALL, 5);
    m_Number = new wxTextCtrl( itemCBulkCmdDlg1, wxID_ANY, _("1"), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer3->Add(m_Number, 0, wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);
    m_Number->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );

    m_Introns = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Make Introns"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Introns->SetValue(false);
    itemBoxSizer2->Add(m_Introns, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxPanel *container1 = new wxPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(400,60), 0 );
    itemBoxSizer2->Add(container1, 1, wxGROW|wxALL|wxFIXED_MINSIZE, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    container1->SetSizer(itemBoxSizer4);

    vector<CFieldNamePanel::EFieldType> field_types;
    field_types.push_back(CFieldNamePanel::eFieldType_CDSGeneProt);
    m_FieldType = new CFieldChoicePanel( container1, field_types, true, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_FieldType, 1, wxGROW|wxALL|wxFIXED_MINSIZE, 0);
    m_FieldType->SetFieldType(CFieldNamePanel::eFieldType_CDSGeneProt, false);

    wxPanel *container2 = new wxPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(400,200), 0 );
    itemBoxSizer2->Add(container2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    container2->SetSizer(itemBoxSizer6);

    m_StringConstraintPanel = new CStringConstraintPanel( container2, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(m_StringConstraintPanel, 0, wxALIGN_BOTTOM|wxALL|wxFIXED_MINSIZE, 0); 

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CExonFromCds content construction
}


/*!
 * Should we show tooltips?
 */

bool CExonFromCds::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CExonFromCds::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CExonFromCds bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CExonFromCds bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CExonFromCds::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CExonFromCds icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CExonFromCds icon retrieval
}



string CExonFromCds::GetErrorMessage()
{
    return m_ErrorMessage;
}


CRef<CCmdComposite> CExonFromCds::GetCommand()
{
    CRef<edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());
    string constraint_field = m_FieldType->GetFieldName(false);
    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(constraint_field));

    CRef<CCmdComposite> cmd(new CCmdComposite(m_Title));
    for (CFeat_CI fi(m_TopSeqEntry, m_Type); fi; ++fi)
    {
        const CSeq_loc &feat_loc = fi->GetLocation();
        CSeq_entry_Handle seh = fi->GetAnnot().GetParentEntry();
        CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(feat_loc);
        TSeqPos seq_length = bsh.GetBioseqLength();
        bool pass = false;
        if (!constraint || constraint_field.empty())
        {
            pass = true;
        }
        else
        {
            vector<CRef<edit::CApplyObject> > objects = col->GetApplyObjects(seh, constraint_field, constraint);
            if (!objects.empty())       
                pass = true;
        }

        if (pass)
        {
            CRef<CSeq_id> id(new CSeq_id);
            id->Assign(*feat_loc.GetId());
            int exon_number = wxAtoi(m_Number->GetValue());
            int intron_start = 0;
            bool minus_strand = false;
            for (CSeq_loc_CI loc_it(feat_loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological); loc_it; ++loc_it)
            {
                CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
                TSeqPos feat_start = feat_range.GetFrom();
                TSeqPos feat_stop = feat_range.GetTo();
                CRef<CSeq_loc> interval(new CSeq_loc);
                if (loc_it.IsSetStrand() && loc_it.GetStrand() == eNa_strand_minus) {
                    minus_strand = true;
                } else {
                    minus_strand = false;
                }
                interval->SetInt().SetFrom(feat_start);
                interval->SetInt().SetTo(feat_stop);               
                interval->SetInt().SetId(*id);
                if (loc_it.GetFuzzFrom() && feat_start == 0)
                    interval->SetPartialStart(true, objects::eExtreme_Positional);
                if (loc_it.GetFuzzTo() && feat_stop == seq_length - 1)
                    interval->SetPartialStop(true, objects::eExtreme_Positional);
                if (minus_strand) {
                    interval->SetStrand(eNa_strand_minus);
                }

                CRef<CSeq_feat> new_feat(new CSeq_feat());
                new_feat->SetLocation().Assign(*interval);       
                new_feat->SetData().SetImp().SetKey("exon");
                new_feat->AddQualifier("number", NStr::IntToString(exon_number));
                new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Positional) || new_feat->GetLocation().IsPartialStop(eExtreme_Positional));

                cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
                if (m_Introns->GetValue() && ((intron_start == 0 && loc_it.GetFuzzFrom() && feat_start > 0) || intron_start >0 ))
                {
                    int intron_stop = feat_start - 1;
                    bool intron_fuzz_start = (intron_start == 0 && loc_it.GetFuzzFrom() && feat_start > 0);
                    bool intron_fuzz_stop = false;
                    AddIntron(seh, cmd, intron_start, intron_stop, intron_fuzz_start, intron_fuzz_stop, id, seq_length, exon_number, minus_strand);
                }
                exon_number++;                
                intron_start = feat_stop + 1;
            }
            
              if (m_Introns->GetValue() && intron_start <= seq_length - 1 && feat_loc.IsPartialStop(eExtreme_Positional))
              {
                  int intron_stop = seq_length - 1;
                  bool intron_fuzz_start = false;
                  bool intron_fuzz_stop = true;
                  AddIntron(seh, cmd,intron_start, intron_stop, intron_fuzz_start, intron_fuzz_stop, id, seq_length, exon_number, minus_strand);
              }
        }

    }
 
    return cmd;
}

void CExonFromCds::AddIntron(CSeq_entry_Handle seh, CRef<CCmdComposite> cmd, int intron_start, int intron_stop, bool intron_fuzz_start, bool intron_fuzz_stop, CRef<CSeq_id> id, TSeqPos seq_length, int exon_number, bool minus_strand)
{
    CRef<CSeq_loc> intron_loc(new CSeq_loc);
    intron_loc->SetInt().SetFrom(intron_start);
    intron_loc->SetInt().SetTo(intron_stop);
    intron_loc->SetInt().SetId(*id);
    if (intron_start == 0)
        intron_loc->SetPartialStart(intron_fuzz_start, objects::eExtreme_Positional);
    if (intron_stop == seq_length - 1)
        intron_loc->SetPartialStop(intron_fuzz_stop, objects::eExtreme_Positional);
    if (minus_strand) {
        intron_loc->SetStrand(eNa_strand_minus);
    }
    CRef<CSeq_feat> new_intron(new CSeq_feat());
    new_intron->SetLocation().Assign(*intron_loc);       
    new_intron->SetData().SetImp().SetKey("intron");
    new_intron->AddQualifier("number", NStr::IntToString(exon_number-1));
    new_intron->SetPartial(new_intron->GetLocation().IsPartialStart(eExtreme_Positional) || new_intron->GetLocation().IsPartialStop(eExtreme_Positional));
    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_intron)));   
}

END_NCBI_SCOPE




