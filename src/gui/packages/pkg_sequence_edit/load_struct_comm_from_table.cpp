/*  $Id: load_struct_comm_from_table.cpp 42282 2019-01-24 22:16:33Z asztalos $
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
#include <util/line_reader.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objects/general/Object_id.hpp>
#include <objtools/readers/reader_exception.hpp>
#include <objtools/readers/struct_cmt_reader.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/load_struct_comm_from_table.hpp>

#include <wx/filedlg.h>
#include <wx/msgdlg.h> 
#include <wx/choicdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> CLoadStructComments::apply(wxWindow *parent, const wxString& cwd, CSeq_entry_Handle tse)
{
    wxFileDialog file(parent, wxT("Load structured comments from table"), cwd, wxEmptyString,
                      CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (file.ShowModal() != wxID_OK) 
    {
        return CRef<CCmdComposite>(NULL);
    }

    wxString file_path = file.GetPath();
    wxArrayString choices;
    choices.Add(_("Table includes accessions for structured comments, one per line"));
    choices.Add(_("Table is one structured comment, field names in the first column"));
    wxSingleChoiceDialog dlg(parent, wxEmptyString, _("Parse Structured Comment From File"), choices);
    if (dlg.ShowModal() != wxID_OK) 
    {
        return CRef<CCmdComposite>(NULL);
    }
    
    if (dlg.GetSelection() == 0)
        return LoadWithAccessions(tse, file_path);
    else
        return LoadSingle(tse, file_path);
}

CRef<CCmdComposite> CLoadStructComments::LoadWithAccessions(CSeq_entry_Handle tse, const wxString& file)
{
    CStructuredCommentsReader reader(NULL);
    std::list<CStructuredCommentsReader::CStructComment> comments;
    CNcbiIfstream fstream(file.fn_str(), ios_base::binary);
    CRef<ILineReader> reader1(ILineReader::New(fstream));
    reader.LoadComments(*reader1, comments); 
    bool modified = false;
    CScope& scope = tse.GetScope();
    CRef<CCmdComposite> cmd(new CCmdComposite("Load Structured Comments"));
    for (const CStructuredCommentsReader::CStructComment& cmt: comments)
    {
        CRef<objects::CSeq_id> id = cmt.m_id;
        if (id && id->IsLocal() && id->GetLocal().IsStr())
        {
            string id_str = id->GetLocal().GetStr();
            try 
            {
                id.Reset(new CSeq_id(id_str, CSeq_id::fParse_Default));
            }
            catch(exception &e)
            {
                id.Reset();
            }
        }
        if (!id)
            continue;
        const vector<CRef<objects::CSeqdesc> >& descs = cmt.m_descs;
        CBioseq_Handle bsh = scope.GetBioseqHandle(*id);
        if (bsh)
        {
            CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
            for (const auto& desc : descs)
            {
                cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(seh, *desc)) );
                modified = true;
            }
        }
    }
    if (!modified)
        cmd.Reset();
    return cmd;
}

CRef<CCmdComposite> CLoadStructComments::LoadSingle(CSeq_entry_Handle tse, const wxString& file)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Load Structured Comments"));

    CNcbiIfstream fstream(file.fn_str(), ios_base::binary);
    CRef<ILineReader> line_reader(ILineReader::New(fstream));
    if (line_reader.Empty()) 
    {
        NcbiMessageBox("Could not open file.");
        return CRef<CCmdComposite>(NULL);
    }

    CRef<CUser_object> user_obj(new CUser_object);
    user_obj->SetType().SetStr("StructuredComment");
    bool modified = false;
    string line;
    string prefix;
    string suffix;
    wxString msg;
    while (!line_reader->AtEOF()) 
    {
        line_reader->ReadLine();
        line = line_reader->GetCurrentLine();

        vector<string> fields;
        NStr::Split(line, "\t", fields, NStr::fSplit_MergeDelimiters);
        for (auto&& it : fields) 
        {
            NStr::TruncateSpacesInPlace(it);
        }

        if (fields.size() != 2) 
        {
            msg << "Cannot parse line: " << line << "\n";
            continue;
        }
        string name = fields.front();
        string value = fields.back();
        if (name == "StructuredCommentPrefix")
            prefix = value;
        if (name == "StructuredCommentSuffix")
            suffix = value;
 
        user_obj->AddField(name, value);
        modified = true;
    }
    if (!msg.IsEmpty())
    {
        CGenericReportDlg* report = new CGenericReportDlg(NULL);  
        report->SetTitle(wxT("Load Structured Comments From Table Errors")); 
        report->SetText(msg);
        report->Show(true);
    }
    if (suffix.empty() && !prefix.empty())
    {
        suffix = prefix;
        NStr::ReplaceInPlace(suffix, "START", "END");
        user_obj->AddField("StructuredCommentSuffix", suffix);
    }
    if (!modified)
        return CRef<CCmdComposite>(NULL);

    modified = false;
    for (objects::CBioseq_CI bi(tse, objects::CSeq_inst::eMol_na); bi; ++bi)
    {
        objects::CBioseq_Handle bsh = *bi;
        CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->SetUser(*user_obj);
        cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(seh, *new_desc)) );
        modified = true;
    }
    if (!modified)
        return CRef<CCmdComposite>(NULL);
    return cmd;
}

END_NCBI_SCOPE
