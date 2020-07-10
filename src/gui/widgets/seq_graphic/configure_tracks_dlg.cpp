/*  $Id: configure_tracks_dlg.cpp 44121 2019-10-31 15:23:32Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>

#include "configure_tracks_dlg.hpp"
#include "track_list_ctrl.hpp"

#include <gui/widgets/seq_graphic/feature_panel.hpp>

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CConfigureTracksDlg, CDialog )

BEGIN_EVENT_TABLE( CConfigureTracksDlg, CDialog )

////@begin CConfigureTracksDlg event table entries
    EVT_TREE_SEL_CHANGED( ID_TREECTRL, CConfigureTracksDlg::OnTreectrlSelChanged )
    EVT_SEARCHCTRL_SEARCH_BTN( ID_SEARCHCTRL, CConfigureTracksDlg::OnSearchButtonClick )
    EVT_SEARCHCTRL_CANCEL_BTN( ID_SEARCHCTRL, CConfigureTracksDlg::OnSearchCancelClick )
    EVT_TEXT_ENTER( ID_SEARCHCTRL, CConfigureTracksDlg::OnSearchEnter )
    EVT_UPDATE_UI( ID_SEARCHCTRL, CConfigureTracksDlg::OnSearchCtrlUpdate )
    EVT_BUTTON( wxID_OK, CConfigureTracksDlg::OnOkClick )
////@end CConfigureTracksDlg event table entries

END_EVENT_TABLE()

CConfigureTracksDlg::CConfigureTracksDlg()
: m_ImageList(16, 16, true, 0), m_FeaturePanel()
{
    Init();
}

CConfigureTracksDlg::CConfigureTracksDlg(wxWindow* parent,
                                         CFeaturePanel* featurePanel,
                                         wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
 : m_ImageList(16, 16, true, 0), m_FeaturePanel(featurePanel)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CConfigureTracksDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CConfigureTracksDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CConfigureTracksDlg creation

    SetSize(ConvertDialogToPixels(wxSize(375, 200)));
    Centre();

    return true;
}

CConfigureTracksDlg::~CConfigureTracksDlg()
{
////@begin CConfigureTracksDlg destruction
////@end CConfigureTracksDlg destruction

    for (auto i : m_AllTracks)
        delete i;
}

void CConfigureTracksDlg::Init()
{
////@begin CConfigureTracksDlg member initialisation
    m_Splitter = NULL;
    m_TreeCtrl = NULL;
    m_SearchCtrl = NULL;
    m_TrackCountCtrl = NULL;
    m_TrackListCtrl = NULL;
////@end CConfigureTracksDlg member initialisation

    static bool sImagesRegistered = false;
    if (!sImagesRegistered) {
        wxFileArtProvider* provider = GetDefaultFileArtProvider();
        provider->RegisterFileAlias(wxT("tracks::active"), wxT("sequence_dna_symbol.png"));
        provider->RegisterFileAlias(wxT("tracks::search"), wxT("search.png"));
        sImagesRegistered = true;
    }

    wxBitmap bmp = wxArtProvider::GetBitmap(wxT("tracks::active"));
    if (bmp.IsOk()) m_ImageList.Add(bmp);
    bmp = wxArtProvider::GetBitmap(wxT("tracks::search"));
    if (bmp.IsOk()) m_ImageList.Add(bmp);
}


namespace
{
    class  CTreeItemTrackData : public wxTreeItemData
    {
    public:
        CTreeItemTrackData() {}
        CTreeItemTrackData(const CTrackListCtrl::TTracks& tracks) : m_Tracks(tracks) {}
        const CTrackListCtrl::TTracks& GetTracks() const { return m_Tracks; }
        CTrackListCtrl::TTracks& GetTracks() { return m_Tracks; }

    private:
        CTrackListCtrl::TTracks m_Tracks;
    };

    class CTrackDlgProxyInfo : public CTrackListCtrl::ITrackInfo
    {
    public:
        CTrackDlgProxyInfo(const CTempTrackProxy& paneTrack, bool visible) :
            m_PaneTrack(CConstRef<CTempTrackProxy>(&paneTrack)), m_Visible(visible) {}

        virtual string GetDisplayName() const
        {
            string trackName;
            if (m_PaneTrack->GetKey().empty() && !m_PaneTrack->IsNA() && m_PaneTrack->GetTrack())
                trackName = m_PaneTrack->GetTrack()->GetFullTitle();
            if (trackName.empty()) trackName = m_PaneTrack->GetDisplayName();
            if (trackName.empty()) trackName = m_PaneTrack->GetName();
            return trackName;
        }

        virtual string GetAccession() const
        {
            return m_PaneTrack->GetSource();
        }

        virtual string GetCategory() const
        {
            string category = m_PaneTrack->GetCategory();
            if (category.empty()) {
                const CAnnotMetaData* na = m_PaneTrack->GetNAdata();
                if (na)
                    category = na->m_xClass;
            }
            return category;
        }
        virtual string GetSubCategory() const { return m_PaneTrack->GetSubcategory(); }
        virtual string GetComments() const{ return m_PaneTrack->GetComments(); }
        virtual bool   GetVisible() const { return m_Toggled ? !m_Visible : m_Visible; }
        virtual void   SetVisible(bool visible) { m_Toggled = (m_Visible != visible); }

        const CTempTrackProxy& GetPaneTrack() { return *m_PaneTrack; }
        bool GetToggled() const { return m_Toggled; }

    private:
        CConstRef<CTempTrackProxy> m_PaneTrack;
        bool m_Visible;
        bool m_Toggled = false;
    };


    class CTreeBuilder
    {
    public:
        CTreeBuilder(wxTreeCtrl& treeCtrl, wxTreeItemId rootItem, CTrackListCtrl::TTracks& allTracks)
            : m_TreeCtrl(treeCtrl)
            , m_VisibleTracksItemData(new CTreeItemTrackData())
            , m_AllTracksItemData()
            , m_AllTracks(allTracks)
            , m_RootId(rootItem)
        {
            if (!m_RootId.IsOk())
                m_RootId = m_TreeCtrl.AddRoot(wxT(""), -1, -1);

            m_VisibleTracksId = m_TreeCtrl.AppendItem(m_RootId, wxT("Visible Tracks"), -1, -1, m_VisibleTracksItemData);

            m_AllTracksItemData = dynamic_cast<CTreeItemTrackData*>(m_TreeCtrl.GetItemData(m_RootId));
            if (!m_AllTracksItemData)
                m_AllTracksId = m_TreeCtrl.AppendItem(m_RootId, wxT("All Tracks"), -1, -1, (m_AllTracksItemData = new CTreeItemTrackData()));
        }

        wxTreeItemId GetAllTracksItem() const { return m_AllTracksId; }

        bool ContainerBegin(const CTempTrackProxy*, const CTrackContainer*, bool) const { return true; }
        void ContainerEnd(const CTempTrackProxy*, const CTrackContainer*, bool) const { ;; }
        bool Track(const CTempTrackProxy* proxy, const CLayoutTrack* track, bool visible);

    private:
        wxTreeCtrl& m_TreeCtrl;

        CTreeItemTrackData* m_VisibleTracksItemData;
        CTreeItemTrackData* m_AllTracksItemData;

        CTrackListCtrl::TTracks& m_AllTracks;

        wxTreeItemId m_RootId;
        wxTreeItemId m_VisibleTracksId;
        wxTreeItemId m_AllTracksId;
    };

    class CTopLevelContainers
    {
    public:
        bool ContainerBegin(CTempTrackProxy* proxy, CTrackContainer*, bool)
        {
            if (!proxy)
                return true; // CFeaturePanel
            m_Proxies.push_back(proxy);
            return false;
        }
        void ContainerEnd(CTempTrackProxy* proxy, CTrackContainer*, bool)
        {
            ;;
        }
        bool Track(const CTempTrackProxy*, const CLayoutTrack*, bool) { return true; }

        const vector<CTempTrackProxy*>& GetProxies() const { return m_Proxies; }
    private:
        vector<CTempTrackProxy*> m_Proxies;
    };

    bool CTreeBuilder::Track(const CTempTrackProxy* proxy, const CLayoutTrack* track, bool visible)
    {
        CTrackDlgProxyInfo* i = new CTrackDlgProxyInfo(*proxy, visible);
        m_AllTracks.push_back(i);

        m_AllTracksItemData->GetTracks().push_back(i);
        if (i->GetVisible())
            m_VisibleTracksItemData->GetTracks().push_back(i);

        wxString category = wxString::FromUTF8(i->GetCategory().c_str());
        if (category.empty()) category = wxT("[Unknown]");

        wxString subCategory = wxString::FromUTF8(i->GetSubCategory().c_str());
        if (subCategory.empty()) subCategory = wxT("Uncategorized");

        wxTreeItemIdValue cookie;
        wxTreeItemId categoryItem;

        wxTreeItemId child = m_TreeCtrl.GetFirstChild(m_RootId, cookie);
        while (child.IsOk())
        {
            if (child != m_VisibleTracksId && child != m_AllTracksId) {
                if (category == m_TreeCtrl.GetItemText(child)) {
                    categoryItem = child;
                    break;
                }
            }
            child = m_TreeCtrl.GetNextChild(m_RootId, cookie);
        }

        if (!categoryItem.IsOk())
            categoryItem = m_TreeCtrl.AppendItem(m_RootId, category, -1, -1, new CTreeItemTrackData());

        CTreeItemTrackData* data = dynamic_cast<CTreeItemTrackData*>(m_TreeCtrl.GetItemData(categoryItem));
        data->GetTracks().push_back(i);

        wxTreeItemId subCategoryItem;
        child = m_TreeCtrl.GetFirstChild(categoryItem, cookie);
        while (child.IsOk())
        {
            if (subCategory == m_TreeCtrl.GetItemText(child)) {
                subCategoryItem = child;
                break;
            }
            child = m_TreeCtrl.GetNextChild(categoryItem, cookie);
        }

        if (!subCategoryItem.IsOk())
            subCategoryItem = m_TreeCtrl.AppendItem(categoryItem, subCategory, -1, -1, new CTreeItemTrackData());

        data = dynamic_cast<CTreeItemTrackData*>(m_TreeCtrl.GetItemData(subCategoryItem));
        data->GetTracks().push_back(i);

        return true;
    }

    void s_CleanSingleChild(wxTreeCtrl& treeCtrl, wxTreeItemId item) {
        wxTreeItemIdValue cookie;
        wxTreeItemId child = treeCtrl.GetFirstChild(item, cookie);
        while (child.IsOk())
        {
            if (treeCtrl.GetChildrenCount(child) == 1) {
                wxTreeItemIdValue cookie2;
                wxTreeItemId sub = treeCtrl.GetFirstChild(child, cookie2);
                if (treeCtrl.GetItemText(sub) == "Uncategorized")
                    treeCtrl.DeleteChildren(child);
            }
            child = treeCtrl.GetNextChild(item, cookie);
        }
    }
} // namespace


void CConfigureTracksDlg::CreateControls()
{    
////@begin CConfigureTracksDlg content construction
    CConfigureTracksDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    m_Splitter = new wxSplitterWindow( itemCDialog1, ID_SPLITTERWINDOW, wxDefaultPosition, wxDLG_UNIT(itemCDialog1, wxSize(100, 100)), wxSP_3DBORDER|wxSP_3DSASH|wxNO_BORDER );
    m_Splitter->SetMinimumPaneSize(wxDLG_UNIT(itemCDialog1, wxSize(150, -1)).x);

    m_TreeCtrl = new wxTreeCtrl( m_Splitter, ID_TREECTRL, wxDefaultPosition, wxDLG_UNIT(m_Splitter, wxSize(100, 100)), wxTR_HAS_BUTTONS |wxTR_HIDE_ROOT|wxTR_SINGLE|wxTR_GBENCH_LINES );

    wxPanel* itemPanel5 = new wxPanel( m_Splitter, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemPanel5->SetSizer(itemBoxSizer6);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer6->Add(itemBoxSizer7, 0, wxGROW|wxALL, 0);
    m_SearchCtrl = new wxSearchCtrl( itemPanel5, ID_SEARCHCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    itemBoxSizer7->Add(m_SearchCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel5, wxSize(5, -1)).x);

    itemBoxSizer7->Add(wxDLG_UNIT(itemPanel5, wxSize(5, -1)).x, wxDLG_UNIT(itemPanel5, wxSize(-1, 5)).y, 1, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel5, wxSize(5, -1)).x);

    m_TrackCountCtrl = new wxStaticText( itemPanel5, wxID_STATIC, _("0 track(s)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(m_TrackCountCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel5, wxSize(5, -1)).x);

    m_TrackListCtrl = new CTrackListCtrl( itemPanel5, ID_LISTCTRL, wxDefaultPosition, wxDLG_UNIT(itemPanel5, wxSize(100, 100)), wxLC_REPORT );
    itemBoxSizer6->Add(m_TrackListCtrl, 1, wxGROW|wxALL, 0);

    m_Splitter->SplitVertically(m_TreeCtrl, itemPanel5, wxDLG_UNIT(itemCDialog1, wxSize(150, -1)).x);
    itemBoxSizer2->Add(m_Splitter, 1, wxGROW|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxStaticLine* itemStaticLine12 = new wxStaticLine( itemCDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine12, 0, wxGROW|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_RIGHT|wxALL, 0);

    wxButton* itemButton14 = new wxButton( itemCDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxButton* itemButton15 = new wxButton( itemCDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

////@end CConfigureTracksDlg content construction

    if (m_FeaturePanel->m_IsMultiLevel) {
        wxTreeItemId rootItem  = m_TreeCtrl->AddRoot(wxT(""), -1, -1);
        wxTreeItemId level1;

        CTopLevelContainers topCont;
        m_FeaturePanel->Traverse(topCont, true);

        for (auto p : topCont.GetProxies()) {
            CTrackContainer* cont = static_cast<CTrackContainer*>(p->GetTrack());

            wxString title = wxString::FromUTF8(cont->GetTitle().c_str());
            wxTreeItemId level = m_TreeCtrl->AppendItem(rootItem, title, -1, -1, new CTreeItemTrackData());
            if (!level1.IsOk())
                level1 = level;
            CTreeBuilder builder(*m_TreeCtrl, level, m_AllTracks);
            cont->Traverse(builder, p->GetShown());
            s_CleanSingleChild(*m_TreeCtrl, level);

            m_TreeCtrl->ExpandAll();
            if (level1.IsOk())
                m_TreeCtrl->ScrollTo(level1);
        }
    } else {
        CTreeBuilder builder(*m_TreeCtrl, wxTreeItemId(), m_AllTracks);
        m_FeaturePanel->Traverse(builder, true);
        s_CleanSingleChild(*m_TreeCtrl, m_TreeCtrl->GetRootItem());
        wxTreeItemId allTracksItem = builder.GetAllTracksItem();
        m_TreeCtrl->ExpandAll();
        if (allTracksItem.IsOk())
            m_TreeCtrl->SelectItem(allTracksItem);
    }
}

void CConfigureTracksDlg::OnOkClick(wxCommandEvent& event)
{
    bool update = false;

    for (auto t : m_AllTracks) {
        CTrackDlgProxyInfo* tinfo = (CTrackDlgProxyInfo*)t;
        CTempTrackProxy& paneTrack = const_cast<CTempTrackProxy&>(tinfo->GetPaneTrack());
        if (tinfo->GetToggled()) {
            paneTrack.SetAction(t->GetVisible() ? CTempTrackProxy::eMakeVisible : CTempTrackProxy::eHide);
            update = true;
        }
    }

    if (update) {
        vector<string> naAccessions;
        m_FeaturePanel->ToggleTracks(naAccessions);
    }

    EndModal(wxID_OK);
}


void CConfigureTracksDlg::OnTreectrlSelChanged(wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    CTreeItemTrackData* data = 0;
    if (item.IsOk())
        data = dynamic_cast<CTreeItemTrackData*>(m_TreeCtrl->GetItemData(item));

    if (data)
        m_TrackListCtrl->SetTracks(data->GetTracks());
    else {
        CTrackListCtrl::TTracks empty;
        m_TrackListCtrl->SetTracks(empty);
    }

    m_SearchCtrl->SetValue(wxEmptyString);
    m_SearchCtrl->ShowCancelButton(false);

    x_UpdateTrackCount();
}

void CConfigureTracksDlg::SetCategory(const string& category)
{
    wxTreeItemId rootId = m_TreeCtrl->GetRootItem();
    if (!rootId.IsOk())
        return;

    wxString text = wxString::FromUTF8(category.c_str());

    wxTreeItemIdValue cookie;
    wxTreeItemId item = m_TreeCtrl->GetFirstChild(rootId, cookie);
    while (item.IsOk())
    {
        wxString itemText = m_TreeCtrl->GetItemText(item);
        if (itemText == text) {
            m_TreeCtrl->SelectItem(item);
            break;
        }
        item = m_TreeCtrl->GetNextChild(rootId, cookie);
    }
}

void CConfigureTracksDlg::x_UpdateTrackCount()
{
    wxString label = wxString::Format(wxT("%u track(s)"), (unsigned)m_TrackListCtrl->GetItemCount());
    m_TrackCountCtrl->SetLabelText(label);
}

void CConfigureTracksDlg::OnSearchButtonClick(wxCommandEvent& event)
{
    x_OnSearch(event.GetString());
}

void CConfigureTracksDlg::OnSearchCancelClick(wxCommandEvent&)
{
    x_OnSearch(wxEmptyString);
}

void CConfigureTracksDlg::OnSearchEnter(wxCommandEvent&)
{
    x_OnSearch(m_SearchCtrl->GetValue());
}

void CConfigureTracksDlg::OnSearchCtrlUpdate(wxUpdateUIEvent& event)
{
    m_SearchCtrl->ShowCancelButton(!m_CurrentFilter.empty());
}

void CConfigureTracksDlg::x_OnSearch(const wxString& value)
{
    m_CurrentFilter = value.ToUTF8();
    NStr::TruncateSpacesInPlace(m_CurrentFilter);
    m_TrackListCtrl->FilterValues(m_CurrentFilter);
    x_UpdateTrackCount();
}

bool CConfigureTracksDlg::ShowToolTips()
{
    return true;
}

wxBitmap CConfigureTracksDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CConfigureTracksDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CConfigureTracksDlg bitmap retrieval
}

wxIcon CConfigureTracksDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CConfigureTracksDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CConfigureTracksDlg icon retrieval
}

static const char* kTrackListTag = "TrackList";
static const char* kSplitterPosTag = "SplitterPos";

void CConfigureTracksDlg::x_LoadSettings(const CRegistryReadView& view)
{
    if (m_TrackListCtrl)
        m_TrackListCtrl->LoadSettings(view.GetReadView(kTrackListTag));

    if (m_Splitter) {
        m_Splitter->SetSashPosition(
            view.GetInt(kSplitterPosTag, m_Splitter->GetSashPosition()));
    }
}

void CConfigureTracksDlg::x_SaveSettings(CRegistryWriteView view) const
{
    if (m_TrackListCtrl)
        m_TrackListCtrl->SaveSettings(view.GetWriteView(kTrackListTag));

    if (m_Splitter)
        view.Set(kSplitterPosTag, m_Splitter->GetSashPosition());
}

END_NCBI_SCOPE

