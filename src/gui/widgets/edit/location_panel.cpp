/*  $Id: location_panel.cpp 43145 2019-05-21 17:08:04Z filippov $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <serial/serialbase.hpp>
#include <serial/typeinfo.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_point.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_loc_mix.hpp>
#include <objmgr/objmgr_exception.hpp>
#include <gui/objutils/label.hpp>

#include <gui/widgets/edit/locationl_list_ctrl.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>

////@begin includes
////@end includes

#include "location_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*!
 * CLocationPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLocationPanel, wxPanel )


/*!
 * CLocationPanel event table definition
 */

BEGIN_EVENT_TABLE( CLocationPanel, wxPanel )

////@begin CLocationPanel event table entries
////@end CLocationPanel event table entries

END_EVENT_TABLE()


/*!
 * CLocationPanel constructors
 */

CLocationPanel::CLocationPanel() : m_Object(NULL)
{
    Init();
}

static void s_Load_Seq_interval(CPacked_seqint& packed_seqint, const CSeq_interval& src)
{
    CRef<CSeq_interval> interval(new CSeq_interval());
    interval->Assign(src);
    packed_seqint.Set().push_back(interval);
}

static void s_Load_Packed_seqint(CPacked_seqint& packed_seqint, const CPacked_seqint& src)
{
    ITERATE(CPacked_seqint::Tdata, it, src.Get()) {
        CRef<CSeq_interval> tmp(new CSeq_interval());
        tmp->Assign(**it);
        packed_seqint.Set().push_back(tmp);
    }
}

static void s_Load_Seq_point(CPacked_seqint& packed_seqint, const CSeq_point& src)
{
    CRef<CSeq_id> seq_id(new CSeq_id());
    if (src.IsSetId())
        seq_id->Assign(src.GetId());
    CRef<CSeq_interval> interval(new CSeq_interval(*seq_id, src.GetPoint(), src.GetPoint()));
    if (src.IsSetStrand())
        interval->SetStrand(src.GetStrand());
    if (src.IsSetFuzz())
    {
        CRef<CInt_fuzz> fuzz(new CInt_fuzz);
        fuzz->Assign(src.GetFuzz());
        interval->SetFuzz_from(*fuzz);
    }

    packed_seqint.Set().push_back(interval);
}


static void s_ConvertWholeToInt(CSeq_loc& loc, CRef<CScope> scope)
{
    if (loc.IsWhole()) {
        CRef<CSeq_id> new_id(new CSeq_id());
        new_id->Assign(loc.GetWhole());
        loc.SetInt().SetId(*new_id);
        loc.SetInt().SetFrom(0);
        loc.SetInt().SetTo(0);

        if (scope)
        {
            CBioseq_Handle bsh = scope->GetBioseqHandle(*new_id);
            if (bsh) {
                loc.SetInt().SetTo(bsh.GetInst_Length() - 1);
            } 
        }
    }    
}


CLocationPanel::CLocationPanel( wxWindow* parent, CSerialObject& object, CScope* scope, bool allow_nuc, bool allow_prot, bool is_cds,
                                wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(&object), m_ToEdit(new CPacked_seqint()), m_AllowNuc(allow_nuc), m_AllowProt(allow_prot), m_is_cds(is_cds)
{
    if (scope)
        m_Scope.Reset(scope); 
    Init();
    Create(parent, id, pos, size, style);
    x_SetJoinOrder();
}

CLocationPanel::CLocationPanel( wxWindow* parent, CSerialObject& object, CScope& scope, bool allow_nuc, bool allow_prot, bool is_cds,
                                wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(&object), m_ToEdit(new CPacked_seqint()), m_AllowNuc(allow_nuc), m_AllowProt(allow_prot), m_is_cds(is_cds)
{
    m_Scope.Reset(&scope); 

    Init();
    Create(parent, id, pos, size, style);
    x_SetJoinOrder();
}

void CLocationPanel::x_LoadSeqLoc(const CSeq_loc& loc)
{
    switch(loc.Which()) {
    case CSeq_loc::e_Int :
        s_Load_Seq_interval(*m_ToEdit, loc.GetInt());
        break;
    case CSeq_loc::e_Packed_int :
        s_Load_Packed_seqint(*m_ToEdit, loc.GetPacked_int());
        break;
    case CSeq_loc::e_Pnt :
        s_Load_Seq_point(*m_ToEdit, loc.GetPnt());
        break;
    default:
        break;
    }
}



/*!
 * CLocationPanel creator
 */

bool CLocationPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLocationPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLocationPanel creation
    return true;
}


/*!
 * CLocationPanel destructor
 */

CLocationPanel::~CLocationPanel()
{
////@begin CLocationPanel destruction
////@end CLocationPanel destruction
}

// An ordered Seq-loc should altername between non-NULL and NULL locations
bool s_IsOrdered(const CSeq_loc& loc)
{
    bool rval = false;
    
    if (loc.IsMix() && loc.GetMix().Get().size() > 1) {
        rval = true;
        bool should_be_null = false;
        ITERATE(CSeq_loc_mix::Tdata, it, loc.GetMix().Get()) {
            if ((*it)->IsNull()) {
                if (should_be_null) {
                    should_be_null = false;
                } else {
                    rval = false;
                    break;
                }
            } else {
                if (should_be_null) {
                    rval = false;
                    break;
                } else {
                    should_be_null = true;
                }
            }
        }
        if (!should_be_null) {
            rval = false;
        }
    }

    return rval;
}


/*!
 * Member initialisation
 */

void CLocationPanel::Init()
{
    if (m_Object && m_ToEdit)
    {
        CSeq_loc& loc = dynamic_cast<CSeq_loc&>(*m_Object);
        s_ConvertWholeToInt(loc, m_Scope);
        CSeq_loc::E_Choice which = loc.Which();
        _ASSERT(which == CSeq_loc::e_Int ||
                which == CSeq_loc::e_Mix ||
                which == CSeq_loc::e_Packed_int ||
                which == CSeq_loc::e_Pnt ||
                which == CSeq_loc::e_not_set ||
                which == CSeq_loc::e_Null);
        
        if (which == CSeq_loc::e_Int) {
            s_Load_Seq_interval(*m_ToEdit, loc.GetInt());
        }
        else if (which == CSeq_loc::e_Packed_int) {
            s_Load_Packed_seqint(*m_ToEdit, loc.GetPacked_int());
        }
        else if (which == CSeq_loc::e_Pnt) {
            s_Load_Seq_point(*m_ToEdit, loc.GetPnt());
        }
        else if (which == CSeq_loc::e_Mix) {
            const CSeq_loc_mix& mix = loc.GetMix();
            ITERATE (CSeq_loc_mix::Tdata, it, mix.Get()) {
                x_LoadSeqLoc(**it);
            }
        }
        m_IsOrdered = s_IsOrdered(loc);
    }
////@begin CLocationPanel member initialisation
    m_LocationCtrl = NULL;
    m_UpdatemRNASpan = NULL;
////@end CLocationPanel member initialisation
}


/*!
 * Control creation for CLocationPanel
 */

void CLocationPanel::CreateControls()
{    
////@begin CLocationPanel content construction
    CLocationPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    CSeq_loc& loc = dynamic_cast<CSeq_loc&>(*m_Object);
    bool is_aa = false;
    CBioseq_Handle bsh;
    try
    {
        bsh = m_Scope->GetBioseqHandle(loc);
    }
    catch (const CObjMgrException& e) 
    {
        bsh.Reset();
    }
    if (bsh && bsh.IsAa()) {
        is_aa = true;
    }

    m_LocationCtrl = new CLocationListCtrl( itemPanel1, m_Scope, m_AllowNuc, m_AllowProt, is_aa, ID_WINDOW2, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_LocationCtrl, 1, wxGROW|wxALL, 0);

    if (m_is_cds)
    {
        m_UpdatemRNASpan = new wxCheckBox( itemPanel1, wxID_ANY, _("Update mRNA span"), wxDefaultPosition, wxDefaultSize, 0 );
        m_UpdatemRNASpan->SetValue(false);
        itemBoxSizer2->Add(m_UpdatemRNASpan, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    }
////@end CLocationPanel content construction

    x_FillList();
}

CRef<CSeq_loc> CLocationPanel::GetSeq_loc() const
{
    if (m_LocationCtrl) {
        CRef<CSeq_loc> loc = m_LocationCtrl->GetSeq_loc();
        return loc;
    }

    return CRef<CSeq_loc>();
}

void CLocationPanel::x_FillList()
{
    if (m_ToEdit->Get().empty()) {
        m_LocationCtrl->AddEmptyRow();
        m_LocationCtrl->SetPartials(false, false);
    } else {
        ITERATE(CPacked_seqint::Tdata, it, m_ToEdit->Get()) {
            TSeqPos from = (**it).GetFrom();
            TSeqPos to = (**it).GetTo();
            wxString seqId;

            if ((**it).IsSetId()) {
                seqId = m_LocationCtrl->GetListIdStrForSeqId((**it).GetId());
            }

            ENa_strand strand = eNa_strand_unknown;
            if ((**it).IsSetStrand())
                strand = (**it).GetStrand();

            bool fuzz = false;
            if ((**it).IsSetFuzz_from() && (**it).GetFuzz_from().IsLim())
                fuzz |= (**it).GetFuzz_from().GetLim() == CInt_fuzz::eLim_tr;
            if ((**it).IsSetFuzz_to() && (**it).GetFuzz_to().IsLim())
                fuzz |= (**it).GetFuzz_to().GetLim() == CInt_fuzz::eLim_tr;
            if (fuzz && from == to)
                to++;
            m_LocationCtrl->AddRow(from + 1, to + 1, strand, seqId, fuzz);
        }
        m_LocationCtrl->SetPartials(m_ToEdit->IsPartialStart(objects::eExtreme_Biological), m_ToEdit->IsPartialStop(objects::eExtreme_Biological));
    }

    m_LocationCtrl->FitInside();
}


void CLocationPanel::SetSeq_loc(CSerialObject& object)
{
    m_Object = &object;
    CSeq_loc& loc = dynamic_cast<CSeq_loc&>(*m_Object);
    s_ConvertWholeToInt(loc, m_Scope);
    TransferDataToWindow();
}


bool CLocationPanel::TransferDataToWindow()
{
    m_LocationCtrl->ClearRows();
    CSeq_loc& loc = dynamic_cast<CSeq_loc&>(*m_Object);
    s_ConvertWholeToInt(loc, m_Scope);
    m_ToEdit.Reset(new CPacked_seqint());
    CSeq_loc::E_Choice which = loc.Which();
    _ASSERT(which == CSeq_loc::e_Int ||
            which == CSeq_loc::e_Mix ||
            which == CSeq_loc::e_Packed_int ||
            which == CSeq_loc::e_Pnt ||
            which == CSeq_loc::e_Null ||
            which == CSeq_loc::e_not_set);

    if (which == CSeq_loc::e_Int) {
        s_Load_Seq_interval(*m_ToEdit, loc.GetInt());
    }
    else if (which == CSeq_loc::e_Packed_int) {
        s_Load_Packed_seqint(*m_ToEdit, loc.GetPacked_int());
    }
    else if (which == CSeq_loc::e_Pnt) {
        s_Load_Seq_point(*m_ToEdit, loc.GetPnt());
    }
    else if (which == CSeq_loc::e_Mix) {
        const CSeq_loc_mix& mix = loc.GetMix();
        ITERATE (CSeq_loc_mix::Tdata, it, mix.Get()) {
            x_LoadSeqLoc(**it);
        }
    }
    x_FillList();
    m_IsOrdered = s_IsOrdered(loc);
    x_SetJoinOrder();

    if (!wxPanel::TransferDataToWindow())
        return false;


    return true;
}


bool CLocationPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    return true;
}

/*!
 * Should we show tooltips?
 */

bool CLocationPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CLocationPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CLocationPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CLocationPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CLocationPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLocationPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLocationPanel icon retrieval
}

void CLocationPanel::x_SetJoinOrder()
{
    if (m_IsOrdered) {
        m_LocationCtrl->SetLocType(1);
    } else {
        m_LocationCtrl->SetLocType(0);
    }
}

bool CLocationPanel::IsImportEnabled()
{   
    return RunningInsideNCBI();
}

bool CLocationPanel::IsExportEnabled()
{
    return RunningInsideNCBI();
}

CRef<CSerialObject> CLocationPanel::OnExport()
{
    if (m_LocationCtrl) {
        CRef<CSeq_loc> loc = m_LocationCtrl->GetSeq_loc();
        CRef<CSerialObject> so((CSerialObject*)(new CSeq_loc));
        so->Assign(*loc);
        return so;
    }

    return CRef<CSerialObject>();
}

void CLocationPanel::OnImport( CNcbiIfstream &istr)
{
    CRef<objects::CSeq_loc> loc(new CSeq_loc);
    istr >> MSerial_AsnText >> *loc;
    SetSeq_loc(*loc);
}

END_NCBI_SCOPE
