/*  $Id: update_align.cpp 38984 2017-07-14 19:48:54Z filippov $
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
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqset/seqset_macros.hpp>
#include <util/line_reader.hpp>
#include <objmgr/bioseq_ci.hpp>

#include <objtools/readers/aln_reader.hpp>
#include <objtools/readers/reader_exception.hpp>

#include <gui/objutils/cmd_add_seq_annot.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/loaders/text_align_load_params.hpp>

#include <gui/packages/pkg_sequence_edit/import_alignparams_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/update_align.hpp>

#include <wx/filedlg.h>
#include <wx/choicdlg.h> 
#include <wx/msgdlg.h> 

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CUpdateAlign::apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, wxWindow *parent)
{
    wxFileDialog file(parent, wxT("Import file"), wxEmptyString, wxEmptyString,
        CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (file.ShowModal() != wxID_OK) {
        return;
    }

    m_FilePath = file.GetPath();
    CTextAlignParams params;
    params.SetUnknown(wxT("?Nn"));
    params.SetMatch(wxT(":"));
    params.SetBegin(wxT("-.Nn?"));
    params.SetMiddle(wxT("-."));  // for interpreting gaps
    params.SetEnd(wxT("-.Nn?"));
    params.SetSeqType(1);

    CImportAlignParamsDlg dlg(NULL);
    dlg.SetData(params);
    if (dlg.ShowModal() == wxID_OK) {
        params = dlg.GetData();
        x_ReadAlign(params);

        if (m_SeqAlign) {
            x_FindNotPresentIDs(tse);
            if (x_ProcessNotPresentIDs(parent)) {
                CRef<CSeq_annot> new_annot(new CSeq_annot);
                new_annot->SetData().SetAlign().push_back(m_SeqAlign);
                CRef<CCmdCreateSeq_annot> cmd(new CCmdCreateSeq_annot(tse, *new_annot));
                cmdProcessor->Execute(cmd);
            }
        }
    }
}

void CUpdateAlign::x_FindNotPresentIDs(CSeq_entry_Handle tse)
{
    m_NonPresentIDs.clear();
    _ASSERT(m_NonPresentIDs.empty());

    for (size_t index = 0; index < m_Ids.size(); ++index) {
        CRef<CSeq_id> id1;
        try {
            id1.Reset(new CSeq_id(m_Ids[index], CSeq_id::fParse_Default));
        } catch (exception &) {
            id1.Reset();
        }

        bool found = false;
        if (id1) {
            for (CBioseq_CI bi(tse); bi && !found; ++bi) {
                for (auto&& id_it : bi->GetId()) {
                    CConstRef<CSeq_id> id2 = id_it.GetSeqId();
                    if (id2 && id1->Match(*id2)) {
                        found = true;
                        break;
                    }
                }
                
            }
        }
        if (!found) {
            m_NonPresentIDs.emplace_back(m_Ids[index], (int)index);
        }
    }
}

 // TODO ask user if not-present id should be a far pointer, deleted, etc.
/*
 "This is a far pointer"// id = "acc"+id;
 "Remove this sequence from the alignment"
 "Use this ID for this sequence"
*/
bool CUpdateAlign::x_ProcessNotPresentIDs(wxWindow *parent)
{
    if (m_NonPresentIDs.empty()) return true;

    wxArrayString choices;
    choices.Add(_("All unmatched sequences are far pointers"));
    choices.Add(_("Remove all unmatched sequences from the alignment"));
    choices.Add(_("Read in a file that maps alignment IDs to sequence IDs"));

    wxString msg;
    msg << "Unable to find " << m_NonPresentIDs.size() << " sequence";
    if (m_NonPresentIDs.size() > 1) {
        msg << "s";
    }
    msg << " from alignment in set";

    int answer = wxGetSingleChoiceIndex(msg, _("Unmatched sequences found"),  choices);
    if (answer == 0) {
        x_ChangeIDsToFarPointers();
    }
    else if (answer == 1) {
        x_RemoveIDsFromAlign();
    }
    else if (answer == 2) {
        x_MapNonPresentIDs(parent);
    }

    return (answer != -1);  // -1 corresponds to pressing the Cancel button
}

namespace {

    struct SFind_ID
    {
        SFind_ID(const string& id_name) : m_IdName(id_name) {}

        bool operator() (const pair<string, string>& str_pair) const
        {
            return NStr::FindNoCase(str_pair.first, m_IdName) != NPOS;
        }
    private:
        string m_IdName;
    };

    CUpdateAlign::TIdToRowVec::iterator s_IsIDNonPresent(CUpdateAlign::TIdToRowVec& nonPresentIDs, int index)
    {
        for (CUpdateAlign::TIdToRowVec::iterator it = nonPresentIDs.begin();
            it != nonPresentIDs.end(); ++it) {
            if (it->second == index) {
                return it;
            }
        }
        return nonPresentIDs.end();
    }

    string s_GetIDLabel(const string& initial_val)
    {
        string id_label = initial_val;
        size_t pos = id_label.find("|");
        if (pos != string::npos) {
            id_label = id_label.substr(pos + 1, string::npos);
        }
        return id_label;
    }
}

void CUpdateAlign::x_RemoveIDsFromAlign()
{
    _ASSERT(m_SeqAlign);
    if (!m_SeqAlign->IsSetSegs()
        || !m_SeqAlign->GetSegs().IsDenseg()
        || m_SeqAlign->GetDim() == 2) {
        return;
    }

    CDense_seg& dense_seg = m_SeqAlign->SetSegs().SetDenseg();
    int dim = dense_seg.GetIds().size();

    vector<int> rows;
    for (auto& it : m_NonPresentIDs) {
        rows.push_back(it.second);
    }

    if (dense_seg.IsSetNumseg()) {
        CDense_seg::TNumseg numseg = dense_seg.GetNumseg();

        // remove rows from starts
        if (dense_seg.IsSetStarts()) {
            CDense_seg::TStarts new_starts;
            auto starts_it = dense_seg.GetStarts().begin();

            for (int seg = 0; seg < numseg; ++seg) {
                for (int index = 0; index < dim; ++index) {
                    if (find(rows.begin(), rows.end(), index) == rows.end()) {
                        new_starts.push_back(*starts_it);
                    }
                    ++starts_it;
                }
            }
            dense_seg.SetStarts().swap(new_starts);
        }

        // remove rows from strands
        if (dense_seg.IsSetStrands()) {
            CDense_seg::TStrands new_strands;
            auto strand_it = dense_seg.GetStrands().begin();

            for (int seg = 0; seg < numseg; ++seg) {
                for (int index = 0; index < dim; ++index) {
                    if (find(rows.begin(), rows.end(), index) == rows.end()) {
                        new_strands.push_back(*strand_it);
                    }
                    ++strand_it;
                }
            }
            dense_seg.SetStrands().swap(new_strands);
        }
    }

    // remove ids that are not present
    CDense_seg::TIds new_ids;
    for (size_t index = 0; index < dense_seg.GetIds().size(); ++index) {
        if (find(rows.begin(), rows.end(), (int)index) == rows.end()) {
            new_ids.push_back(dense_seg.GetIds()[index]);
        }
    }

    dense_seg.SetDim(new_ids.size());
    m_SeqAlign->SetDim(new_ids.size());
    dense_seg.SetIds().swap(new_ids);
    dense_seg.RemovePureGapSegs(); // sets numsegs correctly
}


void CUpdateAlign::x_ChangeIDsToFarPointers()
{
    _ASSERT(m_SeqAlign);
    _ASSERT(!m_NonPresentIDs.empty());

    if (!m_SeqAlign->IsSetSegs()
        || !m_SeqAlign->GetSegs().IsDenseg()) {
        return;
    }

    CDense_seg& dense_seg = m_SeqAlign->SetSegs().SetDenseg();
    size_t previous_ids = dense_seg.GetIds().size();

    CDense_seg::TIds new_ids;
    for (size_t index = 0; index < dense_seg.GetIds().size(); ++index) {
        bool found = false;
        for (auto& it : m_NonPresentIDs) {
            if ((int)index == it.second) {
                string farptrID = "acc" + s_GetIDLabel(it.first);
                CRef<CSeq_id> new_id(new CSeq_id(CSeq_id::e_Local, farptrID));
                new_ids.push_back(new_id);
                found = true;
                break;
            }
        }

        if (!found) {
            new_ids.push_back(dense_seg.GetIds()[index]);
        }
    }

    dense_seg.SetIds().swap(new_ids);
    _ASSERT(previous_ids == dense_seg.GetIds().size());
}

void CUpdateAlign::x_MapNonPresentIDs(wxWindow *parent)
{
    _ASSERT(m_SeqAlign);
    _ASSERT(!m_NonPresentIDs.empty());

    if (!m_SeqAlign->IsSetSegs()
        || !m_SeqAlign->GetSegs().IsDenseg()) {
        return;
    }

    vector<pair<string, string>> oldId_toNewId;
    wxFileDialog file(parent, wxT("Import ID map from file"), wxEmptyString, wxEmptyString,
        CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (file.ShowModal() != wxID_OK) {
        return;
    }

    wxString path = file.GetPath();
    if (!path.IsEmpty()) {

        CNcbiIfstream istr(path.fn_str());
        CStreamLineReader line_reader(istr);
        do {
            string str = *++line_reader;
            NStr::TruncateSpacesInPlace(str);
            if (str.empty())
                continue;
            list<string> row_values;
            NStr::Split(str, "\t", row_values, NStr::fSplit_MergeDelimiters);
            if (row_values.size() == 2) {
                oldId_toNewId.emplace_back(*row_values.begin(), *row_values.rbegin());
            }
        } while (!line_reader.AtEOF());
    }

    CDense_seg& dense_seg = m_SeqAlign->SetSegs().SetDenseg();
    size_t previous_ids = dense_seg.GetIds().size();

    int unmapped = 0;
    string unmapped_ids = "(";
    CDense_seg::TIds new_ids;

    for (size_t index = 0; index < dense_seg.SetIds().size(); ++index) {
        auto id_it = s_IsIDNonPresent(m_NonPresentIDs, (int)index);
        if (id_it == m_NonPresentIDs.end()) {
            new_ids.push_back(dense_seg.GetIds()[index]);
        }
        else {
            std::function<bool(const pair<string, string>& str_pair)> tester = SFind_ID(id_it->first);
            auto map_it = find_if(begin(oldId_toNewId), end(oldId_toNewId), tester);
            if (map_it == oldId_toNewId.end()) {
                unmapped++;
                unmapped_ids += id_it->first;
                unmapped_ids += ", ";
                new_ids.push_back(dense_seg.GetIds()[index]);
            }
            else {
                CRef<CSeq_id> new_id(new CSeq_id(CSeq_id::e_Local, s_GetIDLabel(map_it->second)));
                new_ids.push_back(new_id);
                m_NonPresentIDs.erase(id_it);
            }
        }
    }
    dense_seg.SetIds().swap(new_ids);

    if (unmapped > 0) {
        unmapped_ids.pop_back();
        unmapped_ids.pop_back();
        unmapped_ids += ")";

        string msg = "Mapping not found for ";
        msg += NStr::NumericToString(unmapped) + unmapped_ids;
        msg += " sequence";
        if (unmapped > 1) {
            msg += "s";
        }
        
        msg += "\n";
        if (unmapped == 1) {
            msg += "Is this a far pointer? ";
        }
        else {
            msg += "Are these far pointers?";
        }

        if (wxYES == wxMessageBox(ToWxString(msg), "Info", wxYES_DEFAULT|wxYES_NO, parent)) {
            x_ChangeIDsToFarPointers();
        }
    }

    _ASSERT(previous_ids == dense_seg.GetIds().size());
}

/*
int CUpdateAlign::FindMostFrequentLength(const map<string,string> &id_to_seq)
{
    map<int,int> lengths;
    for (map<string, string>::const_iterator i = id_to_seq.begin(); i != id_to_seq.end(); ++i)
    {
        size_t length = i->second.length();
        lengths[length]++;
    }
    int length = 0;
    int max_occ = 0;
    for (map<int,int>::iterator c = lengths.begin(); c != lengths.end(); ++c)
    {
        if (c->second > max_occ)
        {
            max_occ = c->second;
            length = c->first;
        }
    }
    return length;
}

bool CUpdateAlign::ProcessDiffLengths(map<string,string> &id_to_seq, int length)
{
    set<string> diff_length;
    for (map<string, string>::const_iterator i = id_to_seq.begin(); i != id_to_seq.end(); ++i)
    {
        size_t length2 = i->second.length();
        if (length2 != length)
            diff_length.insert(i->first);
    }
    
    if (!diff_length.empty())
    {
        wxString msg;
        msg << "Remove " << diff_length.size() << " sequence";
        if (diff_length.size() > 1)
            msg << "s";
        msg << " with different length";
        if (diff_length.size() > 1)
            msg << "s";
        int answer = wxMessageBox (msg, _("Different length sequences detected"), wxYES_NO |  wxICON_QUESTION);
        if (answer == wxYES)
        {
            for (set<string>::const_iterator s = diff_length.begin(); s != diff_length.end(); ++s)
                id_to_seq.erase(*s);
        }
        else
            return true;
    }
    return false;
}
*/

static string s_FormatErrors(const CAlnReader::TErrorList& errors)
{
    string errmsg = "";

    ITERATE(CAlnReader::TErrorList, iter2, errors) {
        const string& id = (*iter2).GetID();
        int line_num = (*iter2).GetLineNum();

        if (!NStr::IsBlank(errmsg)) {
            errmsg += "\n";
        }
        if (line_num > -1) {
            errmsg += "At line ";
            errmsg += NStr::IntToString(line_num);
        }
        if (!NStr::IsBlank(id)) {
            errmsg += "(Sequence ID ";
            errmsg += id;
            errmsg += ") ";
        }
        errmsg += (*iter2).GetMsg();
    }
    return errmsg;
}

void CUpdateAlign::x_ReadAlign(const CTextAlignParams& params)
{
    string unknown = ToStdString(params.GetUnknown());
    string match = ToStdString(params.GetMatch());
    string gapbegin = ToStdString(params.GetBegin());
    string gapmiddle = ToStdString(params.GetMiddle());
    string gapend = ToStdString(params.GetEnd());

    vector<CAlnReader::EAlphabet> alphas;
    if (params.GetSeqType() == 1) {
        alphas.push_back(CAlnReader::eAlpha_Nucleotide);
    }
    else if (params.GetSeqType() == 2) {
        alphas.push_back(CAlnReader::eAlpha_Protein);
    }
    else {
        alphas.push_back(CAlnReader::eAlpha_Nucleotide);
        alphas.push_back(CAlnReader::eAlpha_Protein);
    }

    CCompressedFile file(m_FilePath);
    string errmsg;

    ITERATE(vector<CAlnReader::EAlphabet>, iter, alphas) {
        CAlnReader reader(file.GetIstream());
        try {
            reader.SetMissing(unknown);
            reader.SetMatch(match);
            reader.SetBeginningGap(gapbegin);
            reader.SetMiddleGap(gapmiddle);
            reader.SetEndGap(gapend);
            reader.SetAlphabet(*iter);

            reader.Read();
            m_SeqAlign = reader.GetSeqAlign();
            m_Ids = reader.GetIds();
            errmsg = s_FormatErrors(reader.GetErrorList());
            break;
        }
        catch (CObjReaderParseException& _DEBUG_ARG(e)) {
            _TRACE("reading text alignment failed: " << e.what());
            errmsg = s_FormatErrors(reader.GetErrorList());
            NcbiMessageBox(errmsg);
        }
    }
}

END_NCBI_SCOPE
