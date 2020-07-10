/*  $Id: apply_gcode.cpp 43146 2019-05-21 17:51:17Z filippov $
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
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <objects/seqset/seqset_macros.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <gui/packages/pkg_sequence_edit/apply_gcode.hpp>

#include <wx/button.h>
#include <wx/sizer.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*!
 * CApplyGCode type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CApplyGCode, wxDialog )


/*!
 * CApplyGCode event table definition
 */

BEGIN_EVENT_TABLE( CApplyGCode, wxDialog )

////@begin CApplyGCode event table entries
////@end CApplyGCode event table entries

END_EVENT_TABLE()


/*!
 * CApplyGCode constructors
 */

CApplyGCode::CApplyGCode()
{
    Init();
}

CApplyGCode::CApplyGCode( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CVectorTrimDlg creator
 */

bool CApplyGCode::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CApplyGCode creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CApplyGCode creation

    return true;
}


/*!
 * CApplyGCode destructor
 */

CApplyGCode::~CApplyGCode()
{
////@begin CApplyGCode destruction
////@end CApplyGCode destruction
}


/*!
 * Member initialisation
 */

void CApplyGCode::Init()
{
    const CGenetic_code_table& tbl = CGen_code_table::GetCodeTable();
    ITERATE(CGenetic_code_table::Tdata, it, tbl.Get()) {
        m_GenCodes.push_back(pair<string,int>((*it)->GetName(), (*it)->GetId()));
    }


/*    m_GenCodes.push_back(pair<string,int>("Standard", 1 ));
    m_GenCodes.push_back(pair<string,int>("Vertebrate Mitochondrial", 2 ));
    m_GenCodes.push_back(pair<string,int>("Yeast Mitochondrial", 3 ));
    m_GenCodes.push_back(pair<string,int>("Mold Mitochondrial, etc.", 4 ));
    m_GenCodes.push_back(pair<string,int>("Invertebrate Mitochondrial", 5 ));
    m_GenCodes.push_back(pair<string,int>("Ciliate Nuclear, etc.", 6 ));
    m_GenCodes.push_back(pair<string,int>("Echinoderm Mitochondrial, etc.", 9 ));
    m_GenCodes.push_back(pair<string,int>("Euplotid Nuclear", 10 ));
    m_GenCodes.push_back(pair<string,int>("Bacterial, Archaeal and Plant Plastid", 11 ));
    m_GenCodes.push_back(pair<string,int>("Alternative Yeast Nuclear", 12 ));
    m_GenCodes.push_back(pair<string,int>("Ascidian Mitochondrial", 13 ));
    m_GenCodes.push_back(pair<string,int>("Alternative Flatworm Mitochondrial", 14 ));
    m_GenCodes.push_back(pair<string,int>("Blepharisma Macronuclear", 15 ));
    m_GenCodes.push_back(pair<string,int>("Chlorophycean Mitochondrial", 16 ));
    m_GenCodes.push_back(pair<string,int>("Trematode Mitochondrial", 21 ));
    m_GenCodes.push_back(pair<string,int>("Scenedesmus obliquus Mitochondrial", 22 ));
    m_GenCodes.push_back(pair<string,int>("Thraustochytrium Mitochondrial", 23 ));
    m_GenCodes.push_back(pair<string,int>("Pterobranchia Mitochondrial", 24 ));
    m_GenCodes.push_back(pair<string,int>("Candidate Division SR1 and Gracilibacteria", 25 ));
*/
}


/*!
 * Control creation for CApplyGCode
 */

void CApplyGCode::CreateControls()
{    
    CApplyGCode* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);
  
    wxArrayString codes;
    codes.Add(wxEmptyString);
    for (size_t i = 0; i < m_GenCodes.size(); i++)
    {
        wxString str;
        str << m_GenCodes[i].second << " " <<  m_GenCodes[i].first;
        codes.Add(str);
    }
    
    m_Code = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, codes, 0 );
    m_Code->SetSelection(0);
    itemBoxSizer2->Add(m_Code, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CApplyGCode::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CApplyGCode::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CVectorTrimDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CVectorTrimDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CApplyGCode::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CVectorTrimDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CVectorTrimDlg icon retrieval
}

CRef<CCmdComposite> CApplyGCode::GetCommand()
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Batch Apply Genetic Code"));
    x_FindBioSource(cmd);   
    return cmd;
}

void CApplyGCode::x_FindBioSource(CRef<CCmdComposite> composite)
{
    x_ApplyToDescriptors(*(m_TopSeqEntry.GetCompleteSeq_entry()), composite);
    for (CFeat_CI feat_it(m_TopSeqEntry, SAnnotSelector(CSeqFeatData::e_Biosrc)); feat_it; ++feat_it) {
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat_it->GetOriginalFeature());
        x_ApplyToBioSource(new_feat->SetData().SetBiosrc());
        CRef<CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(*feat_it, *new_feat));
        composite->AddCommand(*cmd);
    }
}

void CApplyGCode::x_ApplyToDescriptors(const CSeq_entry& se, CCmdComposite* composite)
{
    CScope& scope = m_TopSeqEntry.GetScope();
    FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
        if ((*it)->IsSource()) {
            const CSeqdesc& orig_desc = **it;
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(orig_desc);
            x_ApplyToBioSource(new_desc->SetSource());
            CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(scope.GetSeq_entryHandle(se), orig_desc, *new_desc));
            composite->AddCommand(*cmd);
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet()) {
            x_ApplyToDescriptors(**it, composite);
        }
    }
}

void CApplyGCode::x_ApplyToBioSource(CBioSource& biosource)
{   
    int id = 0;
    if (m_Code->GetSelection() > 0)
    {
        id = m_GenCodes[m_Code->GetSelection() - 1].second;
    }

    if (biosource.IsSetGenome())
    {
        switch (biosource.GetGenome())
        {
        case CBioSource::eGenome_mitochondrion:
        case CBioSource::eGenome_kinetoplast:
        {
            biosource.SetOrg().SetOrgname().SetMgcode() = id;
            if (id == 0)
                biosource.SetOrg().SetOrgname().ResetMgcode();
            break;
        }
        case CBioSource::eGenome_chloroplast:
        case CBioSource::eGenome_chromoplast:
        case CBioSource::eGenome_plastid:
        case CBioSource::eGenome_cyanelle:
        case CBioSource::eGenome_apicoplast:
        case CBioSource::eGenome_leucoplast:
        case CBioSource::eGenome_proplastid:
        case CBioSource::eGenome_chromatophore:
        {
            biosource.SetOrg().SetOrgname().SetPgcode() = id;
            if (id == 0)
                biosource.SetOrg().SetOrgname().ResetPgcode();
            break;
        }
        default: {
            biosource.SetOrg().SetOrgname().SetGcode() = id;
            if (id == 0)
                biosource.SetOrg().SetOrgname().ResetGcode();
            break;
        }
        }
    }
    else
    {
        biosource.SetOrg().SetOrgname().SetGcode() = id;
        if (id == 0)
            biosource.SetOrg().SetOrgname().ResetGcode();
    }
}

END_NCBI_SCOPE
