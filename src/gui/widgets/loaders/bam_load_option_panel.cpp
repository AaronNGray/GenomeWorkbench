/*  $Id: bam_load_option_panel.cpp 43976 2019-10-01 16:28:09Z katargir $
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
 * Authors: Liangshou Wu
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/bam_load_option_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/ind_progress_bar.hpp>

#include <corelib/ncbifile.hpp>
#include <sra/readers/bam/bamread.hpp>
#include <sra/readers/sra/vdbread.hpp>
#include <util/format_guess.hpp>

#include <wx/filename.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/filedlg.h>
#include <wx/arrstr.h>


////@begin includes
////@end includes


BEGIN_NCBI_SCOPE

CBamLoadOptionPanel::CDropTarget::CDropTarget(CBamLoadOptionPanel& panel)
: m_Panel(panel)
{
    SetDataObject(new wxFileDataObject);
}


wxDragResult CBamLoadOptionPanel::CDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    if ( !GetData() )
        return wxDragNone;

    wxFileDataObject *dobj = (wxFileDataObject *)m_dataObject;
    wxArrayString filenames = dobj->GetFilenames();

    // On windows, dropping the file comes as a wxDragMove, and on Mac (wxCocoa)
    // the same operation causes a wxDragLink
    if(def == wxDragMove || def == wxDragLink)   {
        m_Panel.SetFilenames(filenames);
        return wxDragMove;
    } else if(def == wxDragCopy) {
        m_Panel.AddFilenames(filenames);
        return wxDragCopy;
    }
    return wxDragError;
}

IMPLEMENT_DYNAMIC_CLASS( CBamLoadOptionPanel, wxPanel )

BEGIN_EVENT_TABLE( CBamLoadOptionPanel, wxPanel )
////@begin CBamLoadOptionPanel event table entries
    EVT_TEXT( ID_BAM_INPUT, CBamLoadOptionPanel::OnBamInputTextUpdated )
    EVT_BUTTON( ID_BUTTON, CBamLoadOptionPanel::OnButtonClick )
////@end CBamLoadOptionPanel event table entries

    EVT_TIMER(-1, CBamLoadOptionPanel::OnTimer)
END_EVENT_TABLE()


CBamLoadOptionPanel::CBamLoadOptionPanel() : m_Timer(this)
{
    Init();
}

CBamLoadOptionPanel::CBamLoadOptionPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_Timer(this)
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CBamLoadOptionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBamLoadOptionPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBamLoadOptionPanel creation
    return true;
}


CBamLoadOptionPanel::~CBamLoadOptionPanel()
{
////@begin CBamLoadOptionPanel destruction
////@end CBamLoadOptionPanel destruction
}


void CBamLoadOptionPanel::Init()
{
////@begin CBamLoadOptionPanel member initialisation
    m_MainSizer = NULL;
    m_BamInput = NULL;
    m_ParseProgressSizer = NULL;
////@end CBamLoadOptionPanel member initialisation
    m_InputNum = 0;
    m_ErrNum = 0;
}


void CBamLoadOptionPanel::CreateControls()
{
////@begin CBamLoadOptionPanel content construction
    CBamLoadOptionPanel* itemPanel1 = this;

    m_MainSizer = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(m_MainSizer);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, wxID_ANY, _("BAM/CSRA Files or SRZ Accessions"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    m_MainSizer->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer3->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    m_BamInput = new CRichTextCtrl( itemStaticBoxSizer3->GetStaticBox(), ID_BAM_INPUT, wxEmptyString, wxDefaultPosition, wxSize(200, 100), wxTE_MULTILINE|wxTE_RICH2|wxWANTS_CHARS );
    itemBoxSizer4->Add(m_BamInput, 1, wxGROW|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemStaticBoxSizer3->GetStaticBox(), ID_BUTTON, _("Add BAM/CSRA Files"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_RIGHT|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemStaticBoxSizer3->GetStaticBox(), wxID_STATIC, _("For viewing a BAM file, an index file is required residing in \nthe same directory as the BAM file.  The index file should \nbe named by appending \".bai\" to the bam file name.\n\nIn cases of multiple inputs, the inputs must be separated \nby one of these separators: commas(,), semicolons(;), or \nnew lines."), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
    itemStaticBoxSizer3->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALL, 5);

    itemStaticBoxSizer3->Add(5, 5, 1, wxGROW|wxALL, 5);

    m_ParseProgressSizer = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(m_ParseProgressSizer, 0, wxGROW|wxALL, 0);

////@end CBamLoadOptionPanel content construction

    wxStaticText* parsingText = new wxStaticText(this, ID_PARSING_TEXT, wxT("Validating accessions ..."), wxDefaultPosition, wxDefaultSize, 0);
    wxFont bold(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName());
    parsingText->SetFont(bold);
    parsingText->SetForegroundColour(*wxBLACK);
    m_ParseProgressSizer->Add(parsingText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_ParseProgressSizer->Hide(m_ParseProgressSizer);

    CIndProgressBar* progress = new CIndProgressBar(this, ID_PARSING_PROGRESS, wxDefaultPosition, 100);
    m_ParseProgressSizer->Add(progress, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_ParseProgressSizer->Hide(progress);

    m_DefStyle.SetBackgroundColour(m_BamInput->GetBackgroundColour());
    m_ErrStyle.SetBackgroundColour(wxColour(255, 200, 200));
    m_BamInput->SetDefaultStyle(m_DefStyle);

    SetDropTarget(new CBamLoadOptionPanel::CDropTarget(*this));
}


string CBamLoadOptionPanel::GetInput() const
{
    return string(m_BamInput->GetValue().ToUTF8());
}


void CBamLoadOptionPanel::SetInput(const string& input)
{
    _ASSERT(m_BamInput);
    m_BamInput->SetValue(wxString::FromUTF8(input.c_str()));
}


#define BAM_LOADER_ERR "Bam - Error in input"

#define INVALID_INPUT "Input contains no valid BAM file/SRZ accession or no corresponding \
index file."

#define EMPTY_INPUT "Please specify BAM files or SRZ accessions to load!"

bool CBamLoadOptionPanel::IsInputValid()
{
    switch (m_State) {
    case eParseRunning :
    case eParseCanceling :
        return false;
    default :
        break;
    };

    if(m_InputNum > 0  &&  m_ErrNum == 0)    {
        return true;
    } else {
        if(m_InputNum == 0)    {
            wxMessageBox(wxT(EMPTY_INPUT), wxT(BAM_LOADER_ERR),
                         wxOK | wxICON_ERROR, this);
            return false;
        } else if(m_InputNum == m_ErrNum) {
            wxMessageBox(wxT(INVALID_INPUT), wxT(BAM_LOADER_ERR),
                         wxOK | wxICON_ERROR, this);
            return false;
        } else {
            string err_msg = "Input contains invalid BAM files or SRZ accessions (";
            err_msg += NStr::SizetToString(m_ErrNum) + " error" + (m_ErrNum > 1 ? "s)" : ")");
            err_msg += "\nWould you like to proceed?";

            int res = wxMessageBox(ToWxString(err_msg), wxT(BAM_LOADER_ERR),
                                   wxYES_NO | wxICON_EXCLAMATION, this);
            return res == wxYES;
        }
    }
}


bool CBamLoadOptionPanel::HasBamFile() const
{
    return !m_BamFiles.empty();
}


bool CBamLoadOptionPanel::HasIndexFile() const
{
    ITERATE (TBamFiles, iter, m_BamFiles) {
        string path, file, ext;
        if ((NStr::StartsWith(*iter, "http:", NStr::eNocase) ||
            NStr::StartsWith(*iter, "https:", NStr::eNocase)))
            return true;
        CFile::SplitPath(*iter, &path, &file, &ext);
        if (!CFile(path + file + ext + ".bai").Exists() && 
            !CFile(path + file + ".bai").Exists())
            return false;
    }

    return true;
}


struct CAccValidator
{
    enum FAccType {
        fAccType_Unknown,
        fAccType_Srz,
        fAccType_Sra
    };

    FAccType operator()(const string& srz_acc)
    {
        FAccType acc_type = fAccType_Unknown;
        try {
            // try if it is a SRZ accession
            objects::CSrzPath srz_path;
            string path = srz_path.FindAccPath(srz_acc);
            acc_type = fAccType_Srz;
        } catch (CException&) {
        }

        if (acc_type == fAccType_Unknown) {
            // try if it is a SRA accession
            try {
                objects::CVDBMgr sra_path;
                string path = sra_path.FindAccPath(srz_acc);
                acc_type = fAccType_Sra;
            } catch (CException&) {
            }
        }

        return acc_type;
    }
};


namespace
{
    struct SParseResults
    {
        set<string> csraFiles;
        set<string> bamFiles;
        set<string> srzAccs;
        set<string> sraAccs;
        size_t inputNum = 0;
        size_t errNum = 0;
        vector<int> errPos;
    };

    SParseResults s_ValidateInput(const string& input, ICanceled& canceled)
    {
        static string sDelim(",;\n\r");

        SParseResults results;

        if (input.empty())
            return results;

        vector<string> tokens;
        vector<SIZE_TYPE> token_pos;
        NStr::Split(input, sDelim, tokens, NStr::fSplit_Tokenize, &token_pos);
        size_t token_num = tokens.size();

        for (size_t i = 0; i < token_num; ++i) {

            if (canceled.IsCanceled())
                return SParseResults();

            string input_str = tokens[i];
            NStr::TruncateSpacesInPlace(input_str);

            // first check if the bam file exists
            CFile file(input_str);
            if (file.Exists()) {
                CFormatGuess fmt_guess(input_str);
                CFormatGuess::EFormat format = fmt_guess.GuessFormat();
                if (format == CFormatGuess::eSra || NStr::EndsWith(input_str, ".csra")) {
                    results.csraFiles.insert(input_str);
                }
                else {
                    results.bamFiles.insert(input_str);
                }
                ++results.inputNum;
            }
            else if ((NStr::StartsWith(input_str, "http:", NStr::eNocase) ||
                NStr::StartsWith(input_str, "https:", NStr::eNocase)) &&
                (NStr::EndsWith(input_str, ".csra", NStr::eNocase) ||
                    NStr::EndsWith(input_str, ".sra", NStr::eNocase) ||
                    NStr::EndsWith(input_str, ".bam", NStr::eNocase)))
            {
                // Remote BAM or CSRA
                if (NStr::EndsWith(input_str, ".bam", NStr::eNocase)) {
                    results.bamFiles.insert(input_str);
                }
                else {
                    results.csraFiles.insert(input_str);
                }
                ++results.inputNum;
            }
            else {
                // check if this is a SRZ/SRA assession
                string acc = NStr::ToUpper(input_str);
                CAccValidator::FAccType acc_type = CAccValidator()(acc);

                if (acc_type == CAccValidator::fAccType_Srz) {
                    results.srzAccs.insert(acc);
                    ++results.inputNum;
                }
                else if (acc_type == CAccValidator::fAccType_Sra) {
                    results.sraAccs.insert(acc);
                    ++results.inputNum;
                }
                else {
                    results.errPos.push_back((int)token_pos[i]);
                    results.errPos.push_back((int)(token_pos[i] + tokens[i].size()));
                    ++results.errNum;
                }
            }
        }
        return results;
    }
} // namespace

void CBamLoadOptionPanel::x_ValidateInput()
{
    if (m_State == eParseRunning) {
        if (m_ValidateJob.get() && m_ValidateJob->running()) {
            m_ValidateJob->cancel();
            wxStaticText* parsingText = dynamic_cast<wxStaticText*>(FindWindow(ID_PARSING_TEXT));
            parsingText->SetLabelText("Canceling validation ...");
            parsingText->SetForegroundColour(*wxBLACK);
            m_ParseProgressSizer->Layout();
            m_State = eParseCanceling;
            return;
        }
    }
    else if (m_State == eParseCanceling) {
        if (m_ValidateJob.get() && m_ValidateJob->running())
            return;
    }

    m_ValidateJob.reset();

    string text(m_BamInput->GetValue().ToUTF8());

    {
        CBoolGuard guard(m_UpdatingStyle);
        m_BamInput->SetStyle(0, (int)text.size(), m_DefStyle);
    }

    m_BamFiles.clear();
    m_CSraFiles.clear();
    m_SrzAccs.clear();
    m_SraAccs.clear();
    m_InputNum = 0;
    m_ErrNum = 0;

    wxStaticText* parsingText = dynamic_cast<wxStaticText*>(FindWindow(ID_PARSING_TEXT));
    parsingText->SetLabelText("Validating accessions ...");
    parsingText->SetForegroundColour(*wxBLACK);
    m_ParseProgressSizer->Show(parsingText);
    m_ParseProgressSizer->Show(FindWindow(ID_PARSING_PROGRESS));
    m_MainSizer->Layout();
    m_TextChanged = false;
    m_State = eParseRunning;

    if (m_Timer.IsRunning())
        m_Timer.Stop();

    m_ValidateJob.reset(job_async(
        [text](ICanceled& canceled)
        {
            return s_ValidateInput(text, canceled);
        },
        [this](job_future<SParseResults>& future)
        {
            SParseResults parseResults;
            string errMsg;
            try {
                parseResults = future();

                m_BamFiles = parseResults.bamFiles;
                m_CSraFiles = parseResults.csraFiles;
                m_SrzAccs = parseResults.srzAccs;
                m_SraAccs = parseResults.sraAccs;
                m_InputNum = parseResults.inputNum;
                m_ErrNum = parseResults.errNum;

                if (parseResults.errPos.size() > 1) {
                    CBoolGuard guard(m_UpdatingStyle);
                    for (size_t i = 0; i < parseResults.errPos.size() - 1; ++i)
                        m_BamInput->SetStyle(parseResults.errPos[i], parseResults.errPos[i + 1], m_ErrStyle);
                }
            }
            catch(const CException& e) {
                errMsg = e.GetMsg();
            }
            catch (const exception& e) {
                errMsg = e.what();
            }

            m_State = eParseComplete;

            if (!errMsg.empty()) {
                wxStaticText* parsingText = dynamic_cast<wxStaticText*>(FindWindow(ID_PARSING_TEXT));
                parsingText->SetLabelText("Parse error: " + errMsg);
                parsingText->SetForegroundColour(*wxRED);
                m_ParseProgressSizer->Show(parsingText);
                m_ParseProgressSizer->Hide(FindWindow(ID_PARSING_PROGRESS));
            }
            else {
                m_ParseProgressSizer->Hide(FindWindow(ID_PARSING_TEXT));
                m_ParseProgressSizer->Hide(FindWindow(ID_PARSING_PROGRESS));
            }
            m_MainSizer->Layout();
        }, "Validating accessions...")
    );
}


string CBamLoadOptionPanel::GetBamFilePath() const
{
    string path;
    if ( !m_BamFiles.empty() ) {
        CDirEntry::SplitPath(*m_BamFiles.begin(), &path);
    }
    return path;
}


const CBamLoadOptionPanel::TBamFiles&
CBamLoadOptionPanel::GetBamFiles() const
{
    return m_BamFiles;
}


const CBamLoadOptionPanel::TBamFiles&
CBamLoadOptionPanel::GetCSraFiles() const
{
    return m_CSraFiles;
}


const CBamLoadOptionPanel::TBamFiles&
CBamLoadOptionPanel::GetSrzAccs() const
{
    return m_SrzAccs;
}


const CBamLoadOptionPanel::TBamFiles&
CBamLoadOptionPanel::GetSraAccs() const
{
    return m_SraAccs;
}


bool CBamLoadOptionPanel::ShowToolTips()
{
    return true;
}


wxBitmap CBamLoadOptionPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBamLoadOptionPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBamLoadOptionPanel bitmap retrieval
}


wxIcon CBamLoadOptionPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBamLoadOptionPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBamLoadOptionPanel icon retrieval
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void CBamLoadOptionPanel::OnButtonClick( wxCommandEvent& event )
{
    string old_path(m_BamInput->GetValue().ToUTF8());
    string def_path;
    CDirEntry::SplitPath(old_path, &def_path);

    wxFileDialog dlg(this, wxT("Select BAM files"), wxString::FromUTF8(def_path.c_str()), wxT(""),
        wxString(wxT("BAM files|*.bam|")) + wxALL_FILES,
        wxFD_MULTIPLE);

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxArrayString filenames;
    dlg.GetPaths(filenames);
    AddFilenames(filenames);
}

void CBamLoadOptionPanel::SetFilenames(const wxArrayString& filenames)
{
    wxString str;
    for (size_t i = 0; i < filenames.Count(); ++i) {
        if ( !str.IsEmpty() ) {
            str += wxT("\n");
        }
        str += filenames[i];
    }
    m_BamInput->SetValue(str);
}

void CBamLoadOptionPanel::AddFilenames(const wxArrayString& filenames)
{
    wxString str = m_BamInput->GetValue();
    for (size_t i = 0; i < filenames.Count(); ++i) {
        if ( !str.IsEmpty() ) {
            str += wxT("\n");
        }
        str += filenames[i];
    }
    m_BamInput->SetValue(str);
}


/*
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_BAM_INPUT
 */

void CBamLoadOptionPanel::OnBamInputTextUpdated( wxCommandEvent& event )
{
    if (m_UpdatingStyle)
        return;

    if (!m_Timer.IsRunning())
        m_Timer.Start(1000);

    {
        // Mac-specific handling to remove unwanted formatting
        // that can come from paste operations (and paste events
        // are not caught on mac wx 2.9.3)
        CBoolGuard guard(m_UpdatingStyle);
        m_BamInput->RemoveFormatting();
    }

    m_TextChanged = true;
}

void CBamLoadOptionPanel::OnTimer(wxTimerEvent& event)
{
    x_ValidateInput();
}

END_NCBI_SCOPE

