/*  $Id: submission_page_interface.cpp 42902 2019-04-25 15:23:56Z filippov $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/edit/submission_page_interface.hpp>

BEGIN_NCBI_SCOPE

bool ISubmissionPage::x_DoStepForward(wxListbook* book)
{
    if (!book) return false;
    int page = book->GetSelection();
    int max_pages = book->GetPageCount() - 1;
    if (page == max_pages) {
        // stepping out
        return false;
    }

    if (page != wxNOT_FOUND) {
        book->ChangeSelection(++page);
        return true;
    }
    return false;
}

bool ISubmissionPage::x_DoStepBackward(wxListbook* book)
{
    if (!book) return false;
    int page = book->GetSelection();
    if (page == 0) {
        // this should never be true
        return false;
    }
    if (page != wxNOT_FOUND) {
        book->ChangeSelection(--page);
        return true;
    }
    return false;

}

CSubmissionPageChangeListener* ISubmissionPage::GetParentWizard(wxWindow* wnd) const
{
    CSubmissionPageChangeListener* wizard = nullptr;
    if (wnd) {
        wxWindow* parent = wnd->GetParent();
        while (parent && !wizard) {
            wizard = dynamic_cast<CSubmissionPageChangeListener*>(parent);
            parent = parent->GetParent();
        }
    }
    return wizard;
}

END_NCBI_SCOPE
