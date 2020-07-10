/*  $Id: edit_object_submit_block.cpp 42099 2018-12-19 18:25:33Z asztalos $
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
 * Authors: Colleen Bollin, based on a file by Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/edit/edit_object_submit_block.hpp>

#include <gui/utils/command_processor.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_create_desc.hpp>

#include <objects/submit/Submit_block.hpp>
#include <objects/submit/Contact_info.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/misc/sequence_macros.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>

#include <wx/panel.h>
#include <wx/treebook.h>
#include <wx/toplevel.h>

#ifdef _DEBUG
#include <serial/objostr.hpp>
#include <serial/serial.hpp>
#endif

#include "submit_remarks_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


wxWindow* CEditObjectSubmit_block::CreateWindow(wxWindow* parent)
{
    if (m_Window) {
        NCBI_THROW(CException, eUnknown, 
                   "CEditObjectSubmit_block::CreateWindow - Window already created!");
    }
    wxTopLevelWindow* gui_widget = dynamic_cast<wxTopLevelWindow*>(parent);
    const CSubmit_block& submit_block = dynamic_cast<const CSubmit_block&>(*m_Object);

    m_EditedSubmitBlock.Reset((CSerialObject*)CSubmit_block::GetTypeInfo()->Create());
    m_EditedSubmitBlock->Assign(submit_block);

    CSubmit_block& edited_submit_block = dynamic_cast<CSubmit_block&>(*m_EditedSubmitBlock);

    m_Window = new wxPanel(parent);
        				
    wxBoxSizer* main_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_Window->SetSizer(main_sizer);

    m_Treebook = new wxTreebook(m_Window, wxID_ANY, wxDefaultPosition, wxSize(-1, 400), wxBK_DEFAULT );
    main_sizer->Add(m_Treebook, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SubmitBlockPanel = new CSubmitBlockPanel(m_Treebook, Ref(&edited_submit_block));
    m_Treebook->AddPage(m_SubmitBlockPanel, wxT("Submission"));

    m_ContactPanel = new CContactPanel (m_Treebook, edited_submit_block.SetContact());
		m_Treebook->AddPage(m_ContactPanel, wxT("Contact"));

    CAuth_list& auth_list = edited_submit_block.SetCit().SetAuthors();
    m_AuthorsPanel = new CAuthorNamesPanel(m_Treebook, auth_list, false);
    m_Treebook->AddPage (m_AuthorsPanel, wxT("Authors"));

    m_AffilPanel = new CAuthorAffiliationPanel(m_Treebook, auth_list.SetAffil());
    m_Treebook->AddPage (m_AffilPanel, wxT("Affiliation"));

    m_Remarks = new CSubmitRemarksPanel (m_Treebook, edited_submit_block);
    m_Treebook->AddPage (m_Remarks, wxT("Remarks"));     

    x_TransferDataToWindow();
    if (gui_widget)
        gui_widget->SetTitle(_("Submit block"));
    return m_Window;
}

void CEditObjectSubmit_block::x_TransferDataToWindow()
{ 
    m_SubmitBlockPanel->TransferDataToWindow();
    m_ContactPanel->TransferDataToWindow();
    m_AuthorsPanel->TransferDataToWindow();
    m_AuthorsPanel->TransferDataToWindow();
    m_AffilPanel->TransferDataToWindow();
    m_Remarks->TransferDataToWindow();

}


IEditCommand* CEditObjectSubmit_block::GetEditCommand()
{
    if (!m_Window)
        return 0;

    if (!m_EditAction) {
        CSubmit_block& edited_submit_block = dynamic_cast<CSubmit_block&>(*m_EditedSubmitBlock);
        m_SubmitBlockPanel->TransferDataFromWindow();
        m_ContactPanel->TransferDataFromWindow();
        m_AuthorsPanel->TransferDataFromWindow();
        CAuth_list& auth_list = edited_submit_block.SetCit().SetAuthors();
        m_AuthorsPanel->PopulateAuthors(auth_list);
        m_AffilPanel->TransferDataFromWindow();
        m_Remarks->TransferDataFromWindow();

        CRef<objects::CAffil> affil = m_AffilPanel->GetAffil();
        if (affil)
            auth_list.SetAffil(*affil);
        else
            auth_list.ResetAffil();
   

        try {
            if (m_CreateMode) {
#if 0
                    m_EditAction.Reset(new CCmdCreateDesc(m_SEH,
                        dynamic_cast<const CSeqdesc&>(m_EditedDescriptor.GetObject())));
#endif
            } else {
                CChangeSubmitBlockCommand* cmd = new CChangeSubmitBlockCommand();
                CObject* actual = (CObject*) m_Object.GetPointer();
                cmd->Add(actual, CConstRef<CObject>(m_EditedSubmitBlock));
                m_EditAction.Reset(cmd);
            }
        } catch (CException& e) {
            LOG_POST(Error << "CEditObjectSeq_desc::GetEditAction(): " << e.GetMsg());
        }
    }

    return m_EditAction.GetPointer();
}


END_NCBI_SCOPE
