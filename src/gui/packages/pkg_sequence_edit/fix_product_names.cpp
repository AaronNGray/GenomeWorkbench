/*  $Id: fix_product_names.cpp 41872 2018-10-31 15:16:50Z asztalos $
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
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <util/xregexp/regexp.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <gui/packages/pkg_sequence_edit/fix_product_names.hpp>

#include <wx/button.h>
#include <wx/sizer.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*!
 * CFixProductNames type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFixProductNames, wxDialog )


/*!
 * CFixProductNames event table definition
 */

BEGIN_EVENT_TABLE( CFixProductNames, wxDialog )

////@begin CFixProductNames event table entries
////@end CFixProductNames event table entries

END_EVENT_TABLE()


/*!
 * CFixProductNames constructors
 */

CFixProductNames::CFixProductNames()
{
    Init();
}

CFixProductNames::CFixProductNames( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CVectorTrimDlg creator
 */

bool CFixProductNames::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFixProductNames creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFixProductNames creation

    return true;
}


/*!
 * CFixProductNames destructor
 */

CFixProductNames::~CFixProductNames()
{
////@begin CFixProductNames destruction
////@end CFixProductNames destruction
}


/*!
 * Member initialisation
 */

void CFixProductNames::Init()
{
    m_BeforePunct = NULL;
    m_BeforeCap = NULL;
    m_BeforeNum = NULL;
    m_RmCommas = NULL;
    m_KeepLast = NULL;
}


/*!
 * Control creation for CFixProductNames
 */

void CFixProductNames::CreateControls()
{    
    CFixProductNames* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);
  
    m_BeforePunct = new wxCheckBox( itemDialog1, wxID_ANY, _("Capital letters before punctuation should remain capitalized"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BeforePunct->SetValue(false);
    itemBoxSizer2->Add(m_BeforePunct, 0, wxALIGN_LEFT|wxALL, 5);
    
    m_BeforeCap = new wxCheckBox( itemDialog1, wxID_ANY, _("Capital letters before other capital letters should remain capitalized"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BeforeCap->SetValue(false);
    itemBoxSizer2->Add(m_BeforeCap, 0, wxALIGN_LEFT|wxALL, 5);

    m_BeforeNum = new wxCheckBox( itemDialog1, wxID_ANY, _("Capital letters before numbers should remain capitalized"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BeforeNum->SetValue(false);
    itemBoxSizer2->Add(m_BeforeNum, 0, wxALIGN_LEFT|wxALL, 5);

    m_RmCommas = new wxCheckBox( itemDialog1, wxID_ANY, _("Commas should be removed"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RmCommas->SetValue(false);
    itemBoxSizer2->Add(m_RmCommas, 0, wxALIGN_LEFT|wxALL, 5);

    m_KeepLast = new wxCheckBox( itemDialog1, wxID_ANY, _("Keep caps single letter at the end"), wxDefaultPosition, wxDefaultSize, 0 );
    m_KeepLast->SetValue(false);
    itemBoxSizer2->Add(m_KeepLast, 0, wxALIGN_LEFT|wxALL, 5);
    
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

bool CFixProductNames::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFixProductNames::GetBitmapResource( const wxString& name )
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

wxIcon CFixProductNames::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CVectorTrimDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CVectorTrimDlg icon retrieval
}

CRef<CCmdComposite> CFixProductNames::GetCommand()
{
    CRef<CCmdComposite> cmd = apply(m_TopSeqEntry, m_BeforePunct->GetValue(), m_BeforeCap->GetValue(), m_BeforeNum->GetValue(), m_RmCommas->GetValue(), m_KeepLast->GetValue());
    return cmd;
}

CRef<CCmdComposite> CFixProductNames::apply(objects::CSeq_entry_Handle tse, bool before_punct, bool before_cap, bool before_num, bool rm_commas, bool keep_last)
{
    CRef<CCmdComposite> composite( new CCmdComposite("Fix Product Names") );   

    for (CFeat_CI feat_ci(tse, SAnnotSelector(CSeqFeatData::e_Prot)); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);
        bool modified = false;
        if (new_feat->IsSetData() && new_feat->GetData().IsProt() && new_feat->GetData().GetProt().IsSetName())
        {
            EDIT_EACH_NAME_ON_PROTREF(name, new_feat->SetData().SetProt())
            {
                string new_name = *name;
                FixCaps(new_name, before_punct, before_cap, before_num, rm_commas, keep_last);
                modified |= new_name != *name;
                *name = new_name;
            }
        }
        if (modified)
            composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));    
    }
    for (CFeat_CI feat_ci(tse, SAnnotSelector(CSeqFeatData::e_Rna)); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);
        bool modified = false;
        if (new_feat->IsSetData() && new_feat->GetData().IsRna() && new_feat->GetData().GetRna().IsSetExt() && new_feat->GetData().GetRna().GetExt().IsName())
        {
            string new_name = new_feat->GetData().GetRna().GetExt().GetName();
            FixCaps(new_name, before_punct, before_cap, before_num, rm_commas, keep_last);
            modified |= new_name != new_feat->GetData().GetRna().GetExt().GetName();
            new_feat->SetData().SetRna().SetExt().SetName() = new_name;
        }
        if (new_feat->IsSetData() && new_feat->GetData().IsRna() && new_feat->GetData().GetRna().IsSetExt() && new_feat->GetData().GetRna().GetExt().IsGen() && new_feat->GetData().GetRna().GetExt().GetGen().IsSetProduct())
        {
            string new_name = new_feat->GetData().GetRna().GetExt().GetGen().GetProduct();
            FixCaps(new_name, before_punct, before_cap, before_num, rm_commas, keep_last);
            modified |= new_name != new_feat->GetData().GetRna().GetExt().GetGen().GetProduct();
            new_feat->SetData().SetRna().SetExt().SetGen().SetProduct() = new_name;
        }
        if (modified)
            composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));    
    }
    return composite;
}

static const char* capitalized_names[] = 
{
    "ABC",
    "AAA",
    "ATP",
    "ATPase",
    "A\\/G",
    "AMP",
    "CDP",
    "coproporphyrinogen III",
    "cytochrome BD",
    "cytochrome C",
    "cytochrome C2",
    "cytochrome C550",
    "cytochrome D",
    "cytochrome O",
    "cytochrome P450",
    "cytochrome P460",
    "D\\-alanine",
    "D\\-alanyl",
    "D\\-amino",
    "D\\-beta",
    "D\\-cysteine",
    "D\\-lactate",
    "D\\-ribulose",
    "D\\-xylulose",
    "endonuclease I",
    "endonuclease II",
    "endonuclease III",
    "endonuclease V",
    "EPS I",
    "Fe\\-S",
    "ferredoxin I",
    "ferredoxin II",
    "GTP",
    "GTPase",
    "H\\+",
    "hemolysin I",
    "hemolysin II",
    "hemolysin III",
    "L\\-allo",
    "L\\-arabinose",
    "L\\-asparaginase",
    "L\\-aspartate",
    "L\\-carnitine",
    "L\\-fuculose",
    "L\\-glutamine",
    "L\\-histidinol",
    "L\\-isoaspartate",
    "L\\-serine",
    "MFS",
    "FAD\\/NAD\\(P\\)",
    "MCP",
    "Mg\\+",
    "Mg chelatase",
    "Mg\\-protoporphyrin IX",
    "N\\(5\\)",
    "N\\,N\\-",
    "N\\-\\(",
    "N\\-acetyl",
    "N\\-acyl",
    "N\\-carb",
    "N\\-form",
    "N\\-iso",
    "N\\-succ",
    "NADP",
    "Na\\+\\/H\\+",
    "NAD",
    "NAD\\(P\\)",
    "NADPH",
    "O\\-sial",
    "O\\-succ",
    "pH",
    "ribonuclease BN",
    "ribonuclease D",
    "ribonuclease E",
    "ribonuclease G",
    "ribonuclease H",
    "ribonuclease I",
    "ribonuclease II",
    "ribonuclease III",
    "ribonuclease P",
    "ribonuclease PH",
    "ribonuclease R",
    "RNAse",
    "S\\-adeno",
    "type I",
    "type II",
    "type III",
    "type IV",
    "type V",
    "type VI",
    "UDP",
    "UDP\\-N",
    "Zn",
    "\0"
};

void CFixProductNames::FixCaps(string &name, bool before_punct, bool before_cap, bool before_num, bool rm_commas, bool keep_last)
{

    size_t len = name.size();
    for (size_t i = 0; i < len; i++)
    {
        if ( !(i > 0 && isdigit(name[i-1]) && tolower(name[i]) == 's') &&
             !(before_punct && i+1 < len && ispunct(name[i+1])) &&
             !(before_cap && i+1 < len && isalpha(name[i+1]) && isupper(name[i+1])) &&
             !(before_num && i+1 < len && isdigit(name[i+1])) &&
             !(keep_last && i+1 == len && isalpha(name[i]))
            )
            name[i] = tolower(name[i]);
    }
    if (rm_commas)
    {
        NStr::ReplaceInPlace(name,",",kEmptyStr);
    }
    FixAbbreviationsInElement(name, false);
    for(unsigned int p = 0; capitalized_names[p][0] != '\0'; ++p)
    {
        string cap_name = capitalized_names[p];
        CRegexpUtil replacer( name );
        replacer.Replace( "\\b"+cap_name+"\\b", cap_name, CRegexp::fCompile_ignore_case, CRegexp::fMatch_default, 0);
        replacer.GetResult().swap( name );
    }
}

END_NCBI_SCOPE
