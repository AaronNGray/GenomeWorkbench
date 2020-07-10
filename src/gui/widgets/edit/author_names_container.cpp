/*  $Id: author_names_container.cpp 41446 2018-07-31 15:24:14Z katargir $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <util/line_reader.hpp>

#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/macro_fn_pubfields.hpp>
#include <gui/widgets/edit/singleauthor_panel.hpp>
#include <gui/widgets/edit/author_names_container.hpp>
#include <wx/scrolwin.h>
#include <wx/filedlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_ABSTRACT_CLASS(CAuthorNamesContainer, wxPanel )


BEGIN_EVENT_TABLE(CAuthorNamesContainer, wxPanel )
    
END_EVENT_TABLE()


void CAuthorNamesContainer::DeleteRow(wxWindow* wnd)
{
    if (wnd == NULL)  return;

    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxSizerItem* auth = x_FindSingleAuthor(wnd, itemList);

    wxSizerItemList::iterator node = itemList.begin();
    wxSizerItemList::iterator last_node = itemList.end();
    last_node--;
    if ((**last_node).GetWindow() == wnd)
        return;
    while (node != last_node)
    {
        if ((**node).GetWindow() == wnd)
        {
            (**node).DeleteWindows();
            delete (*node);
            node = itemList.erase(node);
        }
        else
            node++;
    }

    x_DeleteAuthor(auth);
}

void CAuthorNamesContainer::x_DeleteAuthor(wxSizerItem* link)
{
    if (!link) return;
    Freeze();
    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = itemList.begin();
    wxSizerItemList::iterator last_node = itemList.end();
    last_node--;
    if ((*last_node) == link)
        return;
    while (node != last_node)
    {
        if ((*node) == link)
        {
            (**node).DeleteWindows();
            delete (*node);
            node = itemList.erase(node);
        }
        else
            node++;
    }
    if (itemList.empty()) {
        CRef<CAuthor> auth(new CAuthor());
        wxWindow* row = new CSingleAuthorPanel(m_ScrolledWindow, *auth);
        x_AddRowToWindow(row);
    }
    else {
        m_ScrolledWindow->FitInside();
        m_Sizer->Layout();
        m_ScrolledWindow->Refresh();
    }
    Thaw();
}

void CAuthorNamesContainer::ShiftAuthorUp(wxWindow* link)
{
    // find the row that holds the author being changed
    int row_num = x_FindRow(link, m_Sizer->GetChildren());
    x_MoveRowUp(row_num);
}


void CAuthorNamesContainer::ShiftAuthorDown(wxWindow* link)
{
    // find the row that holds the author being changed
    int row_num = x_FindRow(link, m_Sizer->GetChildren());
    if (row_num > -1) {
        x_MoveRowUp(row_num + 1);
    }
}

static void s_GetAuthorNames(const string& line, CAuth_list& author_list)
{
    vector<string> row_values;
    NStr::Split(line, " ", row_values, NStr::fSplit_Tokenize);
    if (row_values.size() == 2) {
        CRef<CAuthor> author(new CAuthor);
        author->SetName().SetName().SetFirst(row_values[0]);
        author->SetName().SetName().SetLast(row_values[1]);
        author_list.SetNames().SetStd().push_back(author);
    }
    else if (row_values.size() == 3) {
        CRef<CAuthor> author(new CAuthor);
        CName_std& std_name = author->SetName().SetName();
        std_name.SetFirst(row_values[0]);
        std_name.SetLast(row_values[2]);
        const string middle_init = row_values[1];

        string initials = macro::CMacroFunction_AuthorFix::s_GetFirstNameInitials(std_name.GetFirst());
        if (!NStr::IsBlank(middle_init)) {
            initials.append(middle_init);
        }
        // don't add another period if one is already there
        if (!NStr::EndsWith(initials, ".")) {
            initials.append(".");
        }
        std_name.SetInitials(initials);
        author_list.SetNames().SetStd().push_back(author);
    }
}

void CAuthorNamesContainer::ImportAuthorsFromFile()
{
    wxFileDialog file(this, wxT("Import author names from file"), m_WorkDir, wxEmptyString,
        CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (file.ShowModal() == wxID_OK)
    {
        wxString path = file.GetPath();
        if (!path.IsEmpty())
        {
            CNcbiIfstream istr(path.fn_str());
            CRef<CAuth_list> new_auth_list(new CAuth_list);
            bool list_of_names = true;
            try {
                istr >> MSerial_AsnText >> *new_auth_list;
                list_of_names = false;
            }
            catch (const CException&) {}

            if (list_of_names) {
                CNcbiIfstream fstream(path.fn_str(), ios_base::binary);
                CRef<ILineReader> line_reader(ILineReader::New(fstream));
                if (line_reader.Empty()) {
                    NcbiMessageBox("Could not open file.");
                    return;
                }
                while (!line_reader->AtEOF()) {
                    line_reader->ReadLine();
                    string str = line_reader->GetCurrentLine();
                    NStr::TruncateSpacesInPlace(str);
                    if (str.empty())
                        continue;

                    NStr::ReplaceInPlace(str, "\r", kEmptyStr);
                    if (NStr::FindNoCase(str, ",") != NPOS) {
                        // a comma separated author list
                        vector<string> author_names;
                        NStr::Split(str, ",", author_names, NStr::fSplit_Tokenize);
                        if (!author_names.empty()) {
                            for (auto&& it : author_names) {
                                s_GetAuthorNames(it, new_auth_list.GetObject());
                            }
                        }
                    }
                    else {
                        // each line contains one author's name
                        s_GetAuthorNames(str, new_auth_list.GetObject());
                    }
                }
            }

            if (!new_auth_list->SetNames().SetStd().empty()) {
                SetAuthors(*new_auth_list);
            }
        }
    }
}


END_NCBI_SCOPE
