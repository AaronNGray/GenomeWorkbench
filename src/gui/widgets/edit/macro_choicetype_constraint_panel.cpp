/*  $Id: macro_choicetype_constraint_panel.cpp 40627 2018-03-21 15:34:12Z asztalos $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
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

#include <objects/seqfeat/RNA_ref.hpp>
#include <gui/widgets/edit/macro_constraint_panel.hpp>
#include <gui/widgets/edit/macro_choicetype_constraint_panel.hpp>


BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*
 * CMacroChoiceTypePanel type definition
 */

IMPLEMENT_CLASS( CMacroChoiceTypePanel, wxPanel )


/*
 * CMacroChoiceTypePanel event table definition
 */

BEGIN_EVENT_TABLE( CMacroChoiceTypePanel, wxPanel )

////@begin CMacroChoiceTypePanel event table entries
////@end CMacroChoiceTypePanel event table entries

END_EVENT_TABLE()


/*
 * CMacroChoiceTypePanel constructors
 */

CMacroChoiceTypePanel::CMacroChoiceTypePanel()
{
    Init();
}

CMacroChoiceTypePanel::CMacroChoiceTypePanel( wxWindow* parent, const string& target, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
  : m_target(target)
{
    Init();
    Create( parent, id, pos, size, style );
}


/*
 * CMacroChoiceTypePanel creator
 */

bool CMacroChoiceTypePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroChoiceTypePanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    Centre();
////@end CMacroChoiceTypePanel creation
    return true;
}


/*
 * CMacroChoiceTypePanel destructor
 */

CMacroChoiceTypePanel::~CMacroChoiceTypePanel()
{
////@begin CMacroChoiceTypePanel destruction
////@end CMacroChoiceTypePanel destruction
}


/*
 * Member initialisation
 */

void CMacroChoiceTypePanel::Init()
{
////@begin CMacroChoiceTypePanel member initialisation
    m_List = NULL;
////@end CMacroChoiceTypePanel member initialisation
}


/*
 * Control creation for CMacroChoiceTypePanel
 */

void CMacroChoiceTypePanel::CreateControls()
{    
////@begin CMacroChoiceTypePanel content construction
    CMacroChoiceTypePanel* itemPanel2 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemPanel2->SetSizer(itemBoxSizer1);

    wxStaticText* itemStaticText18 = new wxStaticText( itemPanel2, wxID_STATIC, _("Select only"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(itemStaticText18, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString listStrings;
    m_List = new wxListBox( itemPanel2, wxID_ANY, wxDefaultPosition, wxSize(-1,109), listStrings, wxLB_SINGLE );
#ifdef __WXGTK__
    m_List->SetMinSize(wxSize(170,109));
#endif
    x_PopulateList();
    itemBoxSizer1->Add(m_List, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);    
   
////@end CMacroChoiceTypePanel content construction
}

void CMacroChoiceTypePanel::x_PopulateList()
{
  m_List->Clear();
  if (m_target == "SeqFeat")
    {
      m_field = "\"data\"";
      m_List->Append(_("gene"));
      m_List->Append(_("org"));
      m_List->Append(_("cdregion"));
      m_List->Append(_("prot"));
      m_List->Append(_("rna"));
      m_List->Append(_("pub"));              ///< publication applies to this seq
      m_List->Append(_("seq"));              ///< to annotate origin from another seq
      m_List->Append(_("imp"));
      m_List->Append(_("region"));           ///< named region (globin locus)
      m_List->Append(_("comment"));          ///< just a comment
      m_List->Append(_("bond"));
      m_List->Append(_("site"));
      m_List->Append(_("rsite"));            ///< restriction site  (for maps really)
      m_List->Append(_("user"));             ///< user defined structure
      m_List->Append(_("txinit"));           ///< transcription initiation
      m_List->Append(_("num"));              ///< a numbering system
      m_List->Append(_("psec-str"));
      m_List->Append(_("non-std-residue"));  ///< non-standard residue here in seq
      m_List->Append(_("het"));              ///< cofactor, prosthetic grp, etc, bound to seq
      m_List->Append(_("biosrc"));
      m_List->Append(_("clone"));
      m_List->Append(_("variation"));
    }
  if (m_target == "Seqdesc")
    {
      m_field = "";
      for (int i = CSeqdesc::e_not_set+1; i != CSeqdesc::e_MaxChoice; i++)
	{
	  m_List->Append(wxString(CSeqdesc::SelectionName(static_cast<CSeqdesc::E_Choice>(i))));
	}
    }
  if (m_target == "UserObject")
    {
      m_field = "\"type\"";
      m_List->Append(_("str"));
    }
  if (m_target == "ImpFeat")
    {
      m_field = "data.imp.key";
      for (int i = CSeqFeatData::eSubtype_bad + 1; i != CSeqFeatData::eSubtype_max; i++)
	{
	  if (CSeqFeatData::GetTypeFromSubtype(static_cast<CSeqFeatData::ESubtype>(i)) == CSeqFeatData::e_Imp)
	    {
	      m_List->Append(wxString(CSeqFeatData::SubtypeValueToName(static_cast<CSeqFeatData::ESubtype>(i))));
	    }
	}     
    }
  if (m_target == "RNA")
    {
      m_field = "data.rna.type";
      CEnumeratedTypeValues::TValues type_values = objects::CRNA_ref::ENUM_METHOD_NAME(EType)()->GetValues();
      for (CEnumeratedTypeValues::TValues::const_iterator i = type_values.begin(); i != type_values.end(); ++i)
	{
	  m_List->Append(wxString(i->first));
	}          
    }
  if (m_target == "SeqSet")
    {
      m_field = "class ";
      CEnumeratedTypeValues::TValues type_values = objects::CBioseq_set::ENUM_METHOD_NAME(EClass)()->GetValues();
      for (CEnumeratedTypeValues::TValues::const_iterator i = type_values.begin(); i != type_values.end(); ++i)
	{
	  m_List->Append(wxString(i->first));
	}             
    }
}

/*
 * Should we show tooltips?
 */

bool CMacroChoiceTypePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CMacroChoiceTypePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroChoiceTypePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroChoiceTypePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CMacroChoiceTypePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroChoiceTypePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroChoiceTypePanel icon retrieval
}



pair<string,string> CMacroChoiceTypePanel::GetMatcher(const pair<string, string> &target, size_t num)
{
  string choice = m_List->GetStringSelection().ToStdString();
  return make_pair(GetDescription(choice), GetLocationConstraint(choice)); 
}

void CMacroChoiceTypePanel::SetMatcher(const string &item)
{
// TODO
}

string CMacroChoiceTypePanel::GetLocationConstraint(const string &choice)
{
  string str;
  if (!choice.empty())
    {
      if (m_target == "ImpFeat"  ||  m_target == "RNA"   ||  m_target == "SeqSet")
	{
	  str = m_field + " = \"" + choice + "\"";
	}
      else
	{
	  str = "CHOICETYPE(" + m_field + ") = \"" + choice + "\"";
	}
    }
 
  return str;
}

string CMacroChoiceTypePanel::GetDescription(const string& choice) const
{
  string str;
  if (!choice.empty())
    {
      if (m_target == "SeqFeat")
	str = "Feature ";
      if (m_target == "Seqdesc")
	str = "Descriptor ";
      if (m_target == "UserObject")
	str = "User object ";
      if (m_target == "ImpFeat"  ||  m_target == "RNA"   ||  m_target == "SeqSet")
	{
	  str = m_target + " ";
	}
      str += "is of the type " + choice;
    }

  return str;      
}


END_NCBI_SCOPE
