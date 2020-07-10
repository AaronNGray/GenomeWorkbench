/*  $Id: single_assemblymethod.cpp 43420 2019-06-27 14:26:24Z filippov $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/srcmod_panel.hpp>
#include <gui/widgets/edit/single_assemblymethod.hpp>

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CSingleAssemblyMethod type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSingleAssemblyMethod, wxPanel )


/*
 * CSingleAssemblyMethod event table definition
 */

BEGIN_EVENT_TABLE( CSingleAssemblyMethod, wxPanel )

////@begin CSingleAssemblyMethod event table entries
////@end CSingleAssemblyMethod event table entries

END_EVENT_TABLE()


/*
 * CSingleAssemblyMethod constructors
 */

CSingleAssemblyMethod::CSingleAssemblyMethod()
{
    Init();
}

CSingleAssemblyMethod::CSingleAssemblyMethod( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSingleAssemblyMethod creator
 */

bool CSingleAssemblyMethod::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSingleAssemblyMethod creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleAssemblyMethod creation
    return true;
}


/*
 * CSingleAssemblyMethod destructor
 */

CSingleAssemblyMethod::~CSingleAssemblyMethod()
{
////@begin CSingleAssemblyMethod destruction
////@end CSingleAssemblyMethod destruction
}


/*
 * Member initialisation
 */

void CSingleAssemblyMethod::Init()
{
////@begin CSingleAssemblyMethod member initialisation
    m_Choice = NULL;
    m_Value = NULL;
////@end CSingleAssemblyMethod member initialisation
}

/*
 * Control creation for CSingleAssemblyMethod
 */

void CSingleAssemblyMethod::CreateControls()
{
    ////@begin CSingleAssemblyMethod content construction
    CSingleAssemblyMethod* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer1);

    wxArrayString m_ChoiceStrings;
    m_ChoiceStrings.Add(_("ABySS"));
    m_ChoiceStrings.Add(_("AllPaths"));
    m_ChoiceStrings.Add(_("Arachne"));
    m_ChoiceStrings.Add(_("Celera Assembler"));
    m_ChoiceStrings.Add(_("CLC NGS Cell"));
    m_ChoiceStrings.Add(_("FALCON"));
    m_ChoiceStrings.Add(_("FALCON-Unzip"));
    m_ChoiceStrings.Add(_("GS De Novo Assembler"));
    m_ChoiceStrings.Add(_("HGAP"));
    m_ChoiceStrings.Add(_("JAZZ"));
    m_ChoiceStrings.Add(_("MaSuRCA"));
    m_ChoiceStrings.Add(_("MIRA"));
    m_ChoiceStrings.Add(_("Newbler"));
    m_ChoiceStrings.Add(_("SOAPdenovo"));
    m_ChoiceStrings.Add(_("SPAdes"));
    m_ChoiceStrings.Add(_("Velvet"));

    m_Choice = new wxComboBox( itemPanel1, ID_GASSEMBLYMETHOD, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ChoiceStrings, wxCB_DROPDOWN );
#ifdef __WXOSX_COCOA__
     m_Choice->SetSelection(-1);
#endif
    if (CSingleAssemblyMethod::ShowToolTips())
        m_Choice->SetToolTip(_("Name of algorithm, eg Newbler OR SOAPdenovo. Select the assembly program from the pulldown list, or type the name of the program directly in the box."));
    itemBoxSizer1->Add(m_Choice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

#ifdef __WXMSW__
    m_Value = new wxTextCtrl( itemPanel1, ID_GASSEMBLYVERSION, wxEmptyString, wxDefaultPosition, wxSize(120, -1), 0 );
#else
    m_Value = new wxTextCtrl(itemPanel1, ID_GASSEMBLYVERSION, wxEmptyString, wxDefaultPosition, wxSize(140, -1), 0);
#endif
    if (CSingleAssemblyMethod::ShowToolTips())
        m_Value->SetToolTip(_("Example: 2.3 OR MAY-2011"));
    itemBoxSizer1->Add(m_Value, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CSingleAssemblyMethod content construction
}


/*
 * Should we show tooltips?
 */

bool CSingleAssemblyMethod::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSingleAssemblyMethod::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSingleAssemblyMethod bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleAssemblyMethod bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSingleAssemblyMethod::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSingleAssemblyMethod icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleAssemblyMethod icon retrieval
}

/*
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_GASSEMBLYMETHOD
 */


const string kAssemblyMethodDelimiter = " v. ";
void CSingleAssemblyMethod::SetValue(const string& val) {
    m_Choice->SetValue(wxT(""));
    m_Value->SetValue(wxT(""));
    if (!val.empty()) {
        auto pos = NStr::Find(val, kAssemblyMethodDelimiter);
        if (pos == NPOS) {
            // only program
            m_Choice->SetValue(ToWxString(val));
        } else if (pos == 0) {
            // only version
            m_Value->SetValue(ToWxString(val.substr(kAssemblyMethodDelimiter.length())));
        } else {
            m_Choice->SetValue(ToWxString(val.substr(0, pos)));
            m_Value->SetValue(ToWxString(val.substr(pos + kAssemblyMethodDelimiter.length())));
        }
    }
}

string CSingleAssemblyMethod::GetValue() const
{
    string val;
    string program = ToStdString(m_Choice->GetValue());
    string version = ToStdString(m_Value->GetValue());
    if (!program.empty())
        val = program;    
    if (!version.empty()) 
        val += kAssemblyMethodDelimiter + version;
    return val;
}


END_NCBI_SCOPE

