/*  $Id: auto_complete_combo.cpp 27932 2013-04-25 20:41:35Z voronov $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/auto_complete_combo.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/textctrl.h>


BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE(CAutoCompleteCombo, wxComboBox)
    EVT_TEXT( wxID_ANY, CAutoCompleteCombo::OnTextChanged )
    EVT_COMBOBOX( wxID_ANY, CAutoCompleteCombo::OnItemSelected )
    EVT_KEY_DOWN( CAutoCompleteCombo::OnKeyDown )
    EVT_IDLE( CAutoCompleteCombo::OnIdle )
#   ifndef __WXMSW__
    EVT_TIMER(-1, CAutoCompleteCombo::x_OnTimer)
#   endif

END_EVENT_TABLE()


CAutoCompleteCombo::CAutoCompleteCombo( wxWindow* parent, wxWindowID id )
#   ifndef __WXMSW__
    : m_Timer( this )
#   endif
{
    Init();

    wxArrayString choices;
    Create(
        parent, id, wxT(""), wxDefaultPosition, wxDefaultSize,
        choices, wxCB_DROPDOWN
    );
}

CAutoCompleteCombo::CAutoCompleteCombo(
    wxWindow* parent,
    wxWindowID id,
    const wxString& value,
    const wxPoint& pos,
    const wxSize& size,
    const wxArrayString& choices,
    long style,
    const wxValidator& validator,
    const wxString& name
)
#   ifndef __WXMSW__
    : m_Timer( this )
#   endif
{
    Init();
    Create( parent, id, value, pos, size, choices, style | wxCB_DROPDOWN, validator, name );
}

CAutoCompleteCombo::~CAutoCompleteCombo()
{
}

void CAutoCompleteCombo::Init()
{
    m_IgnoreCase = false;
    m_IgnoreNextTextChangedEvent = false;
    m_BlockTextChangedEvents = false;
    m_DoBackspace = false;
    m_Filtered = false;
    m_MatchedOnly = false;
    m_AutoOff = false;

    m_PrevValue = wxT("");
    m_PrevCursor = 0;
}

void CAutoCompleteCombo::Create(
    wxWindow* parent,
    wxWindowID id,
    const wxString& value,
    const wxPoint& pos,
    const wxSize& size,
    const wxArrayString& choices,
    long style,
    const wxValidator& validator,
    const wxString& name
){
    m_BaseChoices = choices;
    m_PrevArray = choices;

    wxComboBox::Create( parent, id, value, pos, size, choices, style, validator, name );
}

void CAutoCompleteCombo::SetBaseItems( const wxArrayString& choices )
{
    if (m_BaseChoices == choices) 
        return;

    m_BaseChoices = choices;

    Clear();
    Append( choices );
}

void CAutoCompleteCombo::GetBaseItems( wxArrayString& choices )
{
    choices = m_BaseChoices;
}

void CAutoCompleteCombo::Push()
{
    wxString value = GetValue();
    if (m_BaseChoices.Index(value) != wxNOT_FOUND) {
        return;
    }

    while (GetCount() >= DICTIONARY_MAX_SIZE) {
        if (GetCount()) Delete(0);
        m_BaseChoices.RemoveAt(m_BaseChoices.GetCount() - 1);
    }
    m_BaseChoices.Insert(value, 0);
    Append(value);
}

void CAutoCompleteCombo::OnTextChanged( wxCommandEvent& event )
{
    _TRACE("CAutoCompleteCombo::OnTextChanged  " << ToStdString(event.GetString()));
    if( m_BlockTextChangedEvents ){
        event.Skip();
        return;
    }

    CBoolGuard _guard( m_BlockTextChangedEvents );

    if( !m_MatchedOnly && 
        (event.GetString().length() <= m_PrevValue.length()) ){
        m_PrevValue = event.GetString();
        m_PrevCursor = GetInsertionPoint();
        event.Skip();
        return;
    }


    if( m_MatchedOnly ){
        wxString cur_value = event.GetString();
        int cur_pt = GetInsertionPoint();

        if( m_DoBackspace ){
            cur_value = cur_value.Mid( 0, cur_value.Len() -1 );
            cur_pt -= 1;
            m_DoBackspace = false;
        }

        if( m_PrevValue == cur_value ){
            SetValue( m_PrevValue );
            x_AutoComplete();
            event.Skip();
            return;
        }

        wxArrayString new_base_choices = ProvideItems( cur_value );
        wxArrayString* compare_to = HasProvider() ? &new_base_choices : &m_BaseChoices;

        wxString cur_low = m_IgnoreCase ? cur_value.Lower() : cur_value;
        unsigned int matched_ix = 0;
        while( matched_ix < compare_to->GetCount() ){
            wxString cmp_str = m_IgnoreCase ? (*compare_to)[matched_ix].Lower() : (*compare_to)[matched_ix];

            if( cmp_str.StartsWith( cur_low ) ){
                break;
            }
            ++matched_ix;
        }
        if( matched_ix == compare_to->GetCount() ){
            SetValue( m_PrevValue );
            x_AutoComplete();
            return;
        }

        if( HasProvider() ){
            m_BaseChoices = new_base_choices;
        }

        wxArrayString new_choices = m_Filtered ? GetFilteredItems( cur_value ) : m_BaseChoices;

        if( HasProvider() || m_Filtered ){
            Clear();
            Append( m_BaseChoices );
        }

        SetValue( cur_value );
        SetInsertionPoint( cur_pt );

        m_PrevValue = cur_value;
        m_PrevCursor = cur_pt;

        x_AutoComplete();

    } else {
        if( m_PrevValue == event.GetString() ){
            x_AutoComplete();
            event.Skip();
            return;
        }

        wxString cur_value = event.GetString();
        int cur_pt = GetInsertionPoint();


        m_PrevValue = cur_value;
        m_PrevCursor = cur_pt;

        bool to_be_completed = cur_pt == GetLastPosition();

        // update base items
        if( HasProvider() ){
            m_BaseChoices = ProvideItems( cur_value );
        }

        wxArrayString new_choices = m_Filtered ? GetFilteredItems( cur_value ) : m_BaseChoices;

        if( HasProvider() || m_Filtered ){
            Clear();
            Append( m_BaseChoices );

            SetValue( m_PrevValue );
            SetInsertionPoint( m_PrevCursor );
        }


        if( to_be_completed && !m_AutoOff ){
            x_AutoComplete();
        }
    }
}


void CAutoCompleteCombo::OnItemSelected( wxCommandEvent& event )
{
    _TRACE("CAutoCompleteCombo::OnItemSelected index " << event.GetSelection() );// << " text " << str.c_str());

    wxString cur_value = m_IgnoreCase ? GetValue().Lower() : GetValue();
    wxString prev_value = m_IgnoreCase ? m_PrevValue.Lower() : m_PrevValue;

    if( cur_value.StartsWith( prev_value ) ){
        SetInsertionPoint( (long)prev_value.Length() );
        SetSelection( (int)prev_value.Length(), -1 );
    }

    if( cur_value.StartsWith( prev_value ) ){
        SetInsertionPoint( (long)prev_value.Length() );
        SetSelection( (int)prev_value.Length(), -1 );
    }

    m_IgnoreNextTextChangedEvent = true;

    event.Skip();
}

void CAutoCompleteCombo::OnKeyDown( wxKeyEvent &event )
{
    //***********************************************************************
    //Note: This currently doesn't get called in in wxWidgets 2.9.3 for Cocoa
    //      but since the matching feature is only used in Radar (windows only)
    //      that should not be a problem.
    //************************************************************************
    _TRACE( "CAutoCompleteCombo::OnKeyDown " << event.GetKeyCode() );

    int kc = event.GetKeyCode();

    if( m_MatchedOnly ){
        if( (kc == WXK_BACK || kc == WXK_CLEAR) ){
#if defined(__WXMAC__)
            if( GetSelection() == GetLastPosition() ){
                m_DoBackspace = true;
            }
#else
            long from, to;
            GetSelection( &from, &to );
            if( from != to && to == GetLastPosition() ){
                m_DoBackspace = true;
            }
#endif
        }
    }
    event.Skip();
}

void CAutoCompleteCombo::OnIdle( wxIdleEvent &event )
{
    if(m_IgnoreNextTextChangedEvent) {
        _TRACE( "CAutoCompleteCombo::OnIdle" );
    }

    m_IgnoreNextTextChangedEvent = false;
    event.Skip();
}


void CAutoCompleteCombo::x_AutoComplete()
{
#   ifndef __WXMSW__
    m_Timer.Start( 10, true );
}
void CAutoCompleteCombo::x_OnTimer( wxTimerEvent& )
{
#   endif

    wxString orig_value = GetValue();
    wxString prefix = m_IgnoreCase ? orig_value.Lower() : orig_value;

    for( unsigned int i = 0; i < GetCount(); i++ ){
        wxString value = GetString( i );
        wxString str_value = m_IgnoreCase ? value.Lower() : value;
        if( str_value.StartsWith( prefix ) ){
            // without this "if" application dead locks on Ubuntu/GTK and wxwidgets 2.9.3
            if (value != orig_value) {
                SetValue( value );
                SetSelection( (int)prefix.Len(), -1 );
            }
            break;
        }
    }
}

wxArrayString CAutoCompleteCombo::ProvideItems( const wxString& pattern )
{
    wxArrayString wx_items;

    if( !m_Provider.IsNull() ){
        vector<string> items;

        m_Provider->GetItems( ToStdString( pattern ), items );

        for( int i = 0; i < (int)items.size(); i++ ){
            wx_items.Add( ToWxString( items[i] ) );
        }
    }

    return wx_items;
}

wxArrayString CAutoCompleteCombo::GetFilteredItems( const wxString& pattern )
{
    wxArrayString new_choices;
    wxString pat_low = m_IgnoreCase ? pattern.Lower() : pattern;

    for( int i = 0; i < (int)m_BaseChoices.GetCount(); i++ ){
        wxString base_choice = m_IgnoreCase ? m_BaseChoices[i].Lower() : m_BaseChoices[i];
        if( base_choice.StartsWith( pat_low ) ){
            new_choices.Add( m_BaseChoices[i] );
        }
    }

    return new_choices;
}

/*
template<> void CMRUItemsProvider<string>::GetItems( const string& pattern, vector<string>& result )
{
    m_List.GetItems( result );
}
*/

END_NCBI_SCOPE
