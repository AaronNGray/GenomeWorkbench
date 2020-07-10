#ifndef GUI_WIDGETS_WX___AUTO_COMPLETE_COMBO__HPP
#define GUI_WIDGETS_WX___AUTO_COMPLETE_COMBO__HPP

/*  $Id: auto_complete_combo.hpp 27933 2013-04-25 20:56:17Z voronov $
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


#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/utils/mru_list.hpp>
#include <gui/objutils/filter_items.hpp>

#include <wx/combobox.h>
#ifndef __WXMSW__
#include <wx/timer.h>
#endif


BEGIN_NCBI_SCOPE

#define DICTIONARY_MAX_SIZE 50

///////////////////////////////////////////////////////////////////////////////
/// CAutoCompleteComboBox
class NCBI_GUIWIDGETS_WX_EXPORT CAutoCompleteCombo :
    public wxComboBox
{
    DECLARE_EVENT_TABLE()
public:
    CAutoCompleteCombo(wxWindow* parent, wxWindowID id = wxID_ANY);
    CAutoCompleteCombo(
        wxWindow* parent,
        wxWindowID id,
        const wxString& value,
        const wxPoint& pos,
        const wxSize& size,
        const wxArrayString& choices,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxT("auto complete combo")
    );

    virtual ~CAutoCompleteCombo();

    void Init();
    void Create(
        wxWindow* parent,
        wxWindowID id,
        const wxString& value,
        const wxPoint& pos,
        const wxSize& size,
        const wxArrayString& choices,
        long style = wxCB_DROPDOWN,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxT("auto complete combo")
    );

    void SetItemsProvider( CIRef<IFilterItems>& provider )
    {
        m_Provider = provider;
    }
    bool HasProvider() const { return !m_Provider.IsNull(); }

    void SetIgnoreCase( bool fIgnore ){ m_IgnoreCase = fIgnore; }

    void SetFiltered( bool fFiltered ){ m_Filtered = fFiltered; }
    void SetMatchedOnly( bool fMatchedOnly ){ m_MatchedOnly = fMatchedOnly; }
    void SetAutoOff( bool fAutoOff = true ){ m_AutoOff = fAutoOff; }

    void SetBaseItems( const wxArrayString& choices );
    void GetBaseItems( wxArrayString& choices );
    void Push();

protected:
    void OnTextChanged( wxCommandEvent& event );
    void OnItemSelected( wxCommandEvent& event );
    void OnKeyDown( wxKeyEvent &event );
    void OnIdle( wxIdleEvent &event );

    /// Completes value from available choices
    void x_AutoComplete();
#   ifndef __WXMSW__
    void x_OnTimer( wxTimerEvent& event );
    wxTimer m_Timer;
#   endif

    virtual wxArrayString ProvideItems( const wxString& pattern );
    wxArrayString GetFilteredItems( const wxString& pattern );

private:
    wxArrayString m_BaseChoices;
    CIRef<IFilterItems> m_Provider;
    bool m_IgnoreCase;
    bool m_Filtered;
    bool m_MatchedOnly;
    bool m_AutoOff;

    bool m_IgnoreNextTextChangedEvent;
    bool m_BlockTextChangedEvents;
    bool m_DoBackspace;

    wxArrayString m_PrevArray; // previous wxChoice string array
    wxString m_PrevValue; // last processed input value
    int m_PrevCursor;
};

template<class T = string> class CMRUItemsProvider
    : public CObject, public IFilterItems
{
public:
    CMRUItemsProvider( CMRUList<T>& aList ) : m_List( aList ) {}
    virtual void GetItems( const string& pattern, vector<string>& result );

protected:
    const string ToString( const T& item );

private:

    CMRUList<T>& m_List;
};

// template<> void CMRUItemsProvider<string>::GetItems( const string& pattern, vector<string>& result );

template<> class CMRUItemsProvider<string>
: public CObject, public IFilterItems
{
public:
    CMRUItemsProvider( CMRUList<string>& aList ) : m_List( aList ) {}
    virtual void GetItems( const string& /*pattern*/, vector<string>& result )
    {
        m_List.GetItems( result );
    }

private:
    CMRUList<string>& m_List;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_WX___AUTO_COMPLETE_COMBO__HPP
