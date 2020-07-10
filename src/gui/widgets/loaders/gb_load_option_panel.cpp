/*  $Id: gb_load_option_panel.cpp 44285 2019-11-22 15:41:35Z katargir $
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
 * Authors:
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/gb_load_option_panel.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/util/sequence.hpp>
#include <util/xregexp/regexp.hpp>
#include <gui/objutils/utils.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>

#include <wx/txtstrm.h>
#include <wx/sstream.h>

////@begin includes
////@end includes


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


IMPLEMENT_DYNAMIC_CLASS( CGenBankLoadOptionPanel, wxPanel )

BEGIN_EVENT_TABLE( CGenBankLoadOptionPanel, wxPanel )
////@begin CGenBankLoadOptionPanel event table entries
    EVT_TEXT( ID_ACC_INPUT, CGenBankLoadOptionPanel::OnAccInputUpdated )

    EVT_HTML_LINK_CLICKED( ID_HTMLWINDOW2, CGenBankLoadOptionPanel::OnRecentAccClicked )

////@end CGenBankLoadOptionPanel event table entries
END_EVENT_TABLE()


CGenBankLoadOptionPanel::CGenBankLoadOptionPanel()
{
    Init();
}

CGenBankLoadOptionPanel::CGenBankLoadOptionPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CGenBankLoadOptionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGenBankLoadOptionPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGenBankLoadOptionPanel creation
    return true;
}


CGenBankLoadOptionPanel::~CGenBankLoadOptionPanel()
{
////@begin CGenBankLoadOptionPanel destruction
////@end CGenBankLoadOptionPanel destruction
}


void CGenBankLoadOptionPanel::Init()
{
////@begin CGenBankLoadOptionPanel member initialisation
    m_AccInput = NULL;
    m_MRUWindow = NULL;
////@end CGenBankLoadOptionPanel member initialisation
    m_TokensNum = 0;
    m_ErrNum = 0;
}


void CGenBankLoadOptionPanel::CreateControls()
{
////@begin CGenBankLoadOptionPanel content construction
    CGenBankLoadOptionPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Accessions to load:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Separate accessions with spaces, tabs, commas or semicolons.\nAdd a range to accession after colon (NT_029999:5k-10k)."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText4, 0, wxGROW|wxALL, 5);

    m_AccInput = new CRichTextCtrl( itemPanel1, ID_ACC_INPUT, wxEmptyString, wxDefaultPosition, itemPanel1->ConvertDialogToPixels(wxSize(250, 100)), wxWANTS_CHARS|wxTE_RICH2|wxTE_MULTILINE );
    itemBoxSizer2->Add(m_AccInput, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Recently loaded accessions:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALL, 5);

    m_MRUWindow = new CwxHtmlWindow( itemPanel1, ID_HTMLWINDOW2, wxDefaultPosition, itemPanel1->ConvertDialogToPixels(wxSize(250, 100)), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_MRUWindow, 1, wxGROW|wxALL, 5);

////@end CGenBankLoadOptionPanel content construction
    m_DefStyle.SetBackgroundColour(m_AccInput->GetBackgroundColour());
    m_ErrStyle.SetBackgroundColour(wxColour(255, 200, 200));
    m_AccInput->SetDefaultStyle(m_DefStyle);

    m_MRUWindow->SetBorders(2);
    m_MRUWindow->SetStandardFonts();
}


static string sFormatTimePeriod(CTime& t_now, CTime& t_before)
{
    CTimeFormat format("b D, H:m p");
    return t_before.ToLocalTime().AsString(format);
}

void  CGenBankLoadOptionPanel::x_FillMRUList()
{
    CTime now(CTime::eCurrent);

    typedef TMRUAccList::TTimeToTMap TMap;
    const TMap& map =  m_AccMRUList.GetMap();

    wxStringOutputStream strstrm;
    wxTextOutputStream os(strstrm);

    os << wxT("<html><body>");

    for( TMap::const_reverse_iterator it = map.rbegin();  it != map.rend();  ++it) {
        time_t t = it->first;
        CTime tm(t);
        string acc = it->second.GetAccession();

        os  << wxT("<b><a href=\"") << acc << wxT("\">") << acc
            << wxT("</a></b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;")
            << wxT("<font color=#808080>last&nbsp;loaded&nbsp;&nbsp;")
            << ToWxString(sFormatTimePeriod(now, tm)) << wxT("</font>")
            << wxT("<br/>");
    }

    os << wxT("</body></html>");

    m_MRUWindow->SetPage(strstrm.GetString());
    m_MRUWindow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
}


string CGenBankLoadOptionPanel::GetInput() const
{
    return ToStdString(m_AccInput->GetValue());
}


void CGenBankLoadOptionPanel::SetInput(const string& input)
{
    _ASSERT(m_AccInput);
    m_AccInput->SetValue(ToWxString(input));
}


#define GB_LOADER_ERR "GenBank - Error in input"

#define INVALID_ELEMS_MSG " elements are not valid sequence identifiers and so cannot be loaded.\
\nWould you like to proceed?"

#define INVALID_INPUT "Input does not contain valid sequence identifiers."

#define EMPTY_INPUT "Please specify one or more sequence identifiers to load!"


bool CGenBankLoadOptionPanel::IsInputValid()
{
    x_ValidateInput();
    if(m_TokensNum > 0  &&  m_ErrNum == 0)    {
        return true;
    } else {
        if(m_TokensNum == 0)    {
            wxMessageBox(wxT(EMPTY_INPUT), wxT(GB_LOADER_ERR),
                         wxOK | wxICON_ERROR, this);
            return false;
        } else if(m_TokensNum == m_ErrNum) {
            wxMessageBox(wxT(INVALID_INPUT), wxT(GB_LOADER_ERR),
                         wxOK | wxICON_ERROR, this);
            return false;
        } else {
            string text = NStr::IntToString(m_ErrNum) + INVALID_ELEMS_MSG;
            int res = wxMessageBox(ToWxString(text), wxT(GB_LOADER_ERR),
                                   wxYES_NO | wxICON_EXCLAMATION, this);
            return res == wxYES;
        }
    }
}


static string sDelim(" \t,;\n\r");
const char* sRangeRegexp = "[0-9,]+[kKmM]?([-:]|\\.\\.)[0-9,]+[kKmM]?";

static bool sParseRange(CSeq_interval& ival, const string& range)
{
    string tmp = range;
    NStr::ReplaceInPlace(tmp, ",", "");
    NStr::ReplaceInPlace(tmp, "k", "K");
    NStr::ReplaceInPlace(tmp, "m", "M");
    NStr::ReplaceInPlace(tmp, "K", "000");
    NStr::ReplaceInPlace(tmp, "M", "000000");

    list<string> pos;
    NStr::Split(tmp, "-:.", pos, NStr::fSplit_Tokenize);
    if (pos.size() != 2)
        return false;

    long from = 0, to = 0;
    try {
        from = NStr::StringToLong(pos.front()) - 1;
        to   = NStr::StringToLong(pos.back()) - 1;
    } catch (CException&) {
        return false;
    }

    ival.SetFrom(from);
    ival.SetTo(to);

    return true;
}

int CGenBankLoadOptionPanel::x_ProccessText(const string& text, vector<Token>& tokens, ICanceled& canceled)
{
    tokens.clear();
    m_Ids.clear();
    m_NAs.clear();
    m_GenomicAccessions.clear();

    if (canceled.IsCanceled())
        return 0;

    vector<string> splits;
    vector<SIZE_TYPE> split_pos;
    NStr::Split(text, sDelim, splits, NStr::fSplit_Tokenize, &split_pos);
    size_t size = splits.size();
    tokens.resize(size);

    for (size_t i = 0; i < size; i++) {
        tokens[i].m_Pos = (int)split_pos[i];
        tokens[i].m_Size = (int)splits[i].size();
        tokens[i].m_Valid = false;
    }

    CScope scope(*CObjectManager::GetInstance());
    scope.AddDefaults();

    CRegexp regexGenAcc("GC(A|F)_\\d{9}(.\\d+)");

    size_t processed = 0;

    for (; processed < size; processed++) {
        if (canceled.IsCanceled())
            break;

        string s_id = splits[processed];

        string na = s_id;
        NStr::ToUpper(na);

        if (CSeqUtils::IsNAA(na, true)) {
            if (na.find(".") == string::npos)
                na += ".1";
            m_NAs.push_back(na);
            tokens[processed].m_Valid = true;
            continue;
        }

        string range;
        size_t pos = s_id.find(':');

        bool seq_id_valid = true;

        if (pos != string::npos) {
            range = s_id.substr(pos + 1);
            s_id = s_id.substr(0, pos);

            CRegexp re(sRangeRegexp);
            seq_id_valid = re.IsMatch(range);
        }

        if (seq_id_valid) {
            CSeq_id::EAccessionInfo ainfo = CSeq_id::IdentifyAccession(s_id);
            CSeq_id::E_Choice acc_type = CSeq_id::GetAccType(ainfo);
            switch (acc_type){
            case CSeq_id::e_Local:
                seq_id_valid = false;
                break;

            default:
                try {
                    CSeq_id id(s_id);
                    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);
                    idh = sequence::GetId(idh, scope, sequence::eGetId_Best);

                    if (idh){
                        CRef<CSeq_id> id_copy(new CSeq_id());

                        id_copy->Assign(*idh.GetSeqId());

                        if (range.empty())
                            m_Ids.push_back(CRef<CObject>(id_copy.GetPointer()));
                        else {
                            CRef<CSeq_loc> loc(new CSeq_loc());
                            CSeq_interval& ival = loc->SetInt();
                            ival.SetId().Assign(*id_copy);

                            if (sParseRange(ival, range))
                                m_Ids.push_back(CRef<CObject>(loc.GetPointer()));
                            else
                                seq_id_valid = false;
                        }
                    }
                    else {
                        seq_id_valid = false;
                    }

                }
                catch (CSeqIdException&){
                    if (!regexGenAcc.IsMatch(s_id))
                        seq_id_valid = false;
                    else {
                        seq_id_valid = true;
                        tokens[processed].m_GenomicAccession = true;
                        m_GenomicAccessions.push_back(s_id);
                    }
                }
                break;
            }
        }

        tokens[processed].m_Valid = seq_id_valid;

        // Debug delay
        //::wxMilliSleep(100);
    }

    return (int)processed;
}

void CGenBankLoadOptionPanel::x_ValidateInput()
{
    static bool fReentry = false;
    if (fReentry)
        return;
    CBoolGuard _guard(fReentry);

    string text = ToStdString(m_AccInput->GetValue());

    m_ValidTokens.clear();

    vector<Token> tokens;
    int processed = GUI_AsyncExec([this, &text, &tokens](ICanceled& canceled)
    {
        return x_ProccessText(text, tokens, canceled);
    }, wxT("Validating accessions..."));

    m_TokensNum = (int)tokens.size();
    m_ErrNum = m_TokensNum - processed;

    {{
        // gtk control loses kbd focus after Freeze/Thaw
        // restore it afterwards
        bool restoreFocus = (wxWindow::FindFocus() == m_AccInput);

        m_AccInput->Freeze();
        int valid_start = 0;
        for( size_t i = 0;  i < (size_t)processed;  i++) {
            if (!tokens[i].m_Valid) {
                ++m_ErrNum;
                int start = tokens[i].m_Pos;
                int end = start + tokens[i].m_Size;
                if (valid_start < start)
                    m_AccInput->SetStyle(valid_start, start, m_DefStyle);
                m_AccInput->SetStyle(start, end, m_ErrStyle);
                valid_start = end;
            } else {
                string value = text.substr(tokens[i].m_Pos, tokens[i].m_Size);
                NStr::TruncateSpacesInPlace(value);
                m_ValidTokens.push_back(value);
            }
        }
        if (processed < m_TokensNum) {
            int start = tokens[processed].m_Pos;
            int end = (int)text.size();
            if (valid_start < start)
                m_AccInput->SetStyle(valid_start, start, m_DefStyle);
            m_AccInput->SetStyle(start, end, m_ErrStyle);
        }
        else {
            if (valid_start < (int)text.size())
                m_AccInput->SetStyle(valid_start, (int)text.size(), m_DefStyle);
        }
        m_AccInput->Thaw();

        if (restoreFocus)
            m_AccInput->SetFocus();
    }}
}

void CGenBankLoadOptionPanel::OnRecentAccClicked( wxHtmlLinkEvent& event )
{
    const wxHtmlLinkInfo info = event.GetLinkInfo();
    string acc = ToStdString(info.GetHref());
    NStr::TruncateSpacesInPlace(acc);
    string input = GetInput();
    NStr::TruncateSpacesInPlace(input);

    vector<string> tokens;
    NStr::Split(input, sDelim, tokens, NStr::fSplit_Tokenize);
        
    for (vector<string>::iterator it = tokens.begin();  it != tokens.end();  ++it) {
        if (acc == *it) {
            tokens.erase(it);
            tokens.push_back(acc);
            input = NStr::Join(tokens, " ");
            m_AccInput->SetValue(ToWxString(input));
            return;
        }
    }

    if (!input.empty())
        *m_AccInput << wxT(" ");

    *m_AccInput << ToWxString(acc);
}

static const char* kPanelTag = "GenBankLoadOptionPanel";
static const char* kIdsInputTag = "IdsInput";
static const char* kMRUTag = "AccMRU";

void CGenBankLoadOptionPanel::SaveSettings(const string& regPath)
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(regPath + "." + kPanelTag);

    string input = GetInput();
    string encoded = NStr::URLEncode(input);
    // do not save large texts
    if (encoded.size() < 10000) {
        view.Set(kIdsInputTag, encoded);
    }
}

void CGenBankLoadOptionPanel::SaveMruAccessions(const string& regPath)
{
    if (m_Ids.empty() && m_NAs.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(regPath + "." + kPanelTag);

    ITERATE(vector<string>, it, m_ValidTokens)
        m_AccMRUList.Add(*it);

    vector<string> values;
    CTimeFormat format =
        CTimeFormat::GetPredefined(CTimeFormat::eISO8601_DateTimeSec);

    const TMRUAccList::TTimeToTMap& map = m_AccMRUList.GetMap();
    ITERATE(TMRUAccList::TTimeToTMap, it, map)  {
        CTime time(it->first);
        time.ToLocalTime();
        string s_time = time.AsString(format);
        string accession = it->second.GetAccession();
        values.push_back(s_time);
        values.push_back(accession);
    }
    view.Set(kMRUTag, values);
}

void CGenBankLoadOptionPanel::LoadSettings(const string& regPath)
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(regPath + "." + kPanelTag);

    string encoded = view.GetString(kIdsInputTag, kEmptyStr);
    string input = NStr::URLDecode(encoded);
    SetInput(input);

    // load MRU accessions
    m_AccMRUList.Clear();
    
    try {
        vector<string> values;
        view.GetStringVec(kMRUTag, values);
        CTimeFormat format =
            CTimeFormat::GetPredefined(CTimeFormat::eISO8601_DateTimeSec);
        for(  size_t i = 0;  i + 1 < values.size() ;  )   {
            string s_time    = values[i++];
            string accession = values[i++];

            CTime time(s_time, format);

            time_t t = time.GetTimeT();
            m_AccMRUList.Add(TMRUAccList::value_type(accession), t);
        }
    } catch (const exception&) {
    }

    x_FillMRUList();
}



CGenBankLoadOptionPanel::TIdsVec& CGenBankLoadOptionPanel::GetSeqIds()
{
    return m_Ids;
}

void CGenBankLoadOptionPanel::OnAccInputUpdated(wxCommandEvent& event)
{
    static bool fReentry = false;
    if (fReentry)
        return;
    CBoolGuard _guard(fReentry);

    long pos = m_AccInput->GetInsertionPoint();
    m_AccInput->ChangeValue(m_AccInput->GetValue());
    m_AccInput->SetInsertionPoint(pos);
    x_ValidateInput();
}


bool CGenBankLoadOptionPanel::ShowToolTips()
{
    return true;
}


wxBitmap CGenBankLoadOptionPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGenBankLoadOptionPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGenBankLoadOptionPanel bitmap retrieval
}


wxIcon CGenBankLoadOptionPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGenBankLoadOptionPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGenBankLoadOptionPanel icon retrieval
}

END_NCBI_SCOPE
