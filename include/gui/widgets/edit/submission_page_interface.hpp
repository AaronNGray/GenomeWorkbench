#ifndef GUI_WIDGETS_EDIT___SUBMISSION_PAGE_INTERFACE__HPP
#define GUI_WIDGETS_EDIT___SUBMISSION_PAGE_INTERFACE__HPP

/*  $Id: submission_page_interface.hpp 43203 2019-05-28 18:33:53Z filippov $
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

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <objects/submit/Submit_block.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <wx/listbook.h>
#include <wx/window.h>

BEGIN_NCBI_SCOPE

class CSubmissionPageChangeListener
{
public:
    virtual ~CSubmissionPageChangeListener() {}
    virtual void UpdateOnPageChange(wxWindow* current_page) = 0;
    virtual void ApplySubmitCommand() = 0;
};

class NCBI_GUIWIDGETS_EDIT_EXPORT ISubmissionPage
{
public:
    virtual ~ISubmissionPage() {}

    // return false if the page does not contain tabs
    virtual bool StepForward() { return false;  }
    // return false if the page does not contain tabs
    virtual bool StepBackward() { return false; }

    // implement this function for the first page of the submission wizard
    virtual bool IsFirstPage() const { return false; }
    // implement this function for the last page of the submission wizard
    virtual bool IsLastPage() const { return false; }

    // update page with information from provided submit block
    virtual void ApplySubmitBlock(objects::CSubmit_block& block) { }
    virtual void ApplyDescriptor(objects::CSeqdesc& desc) { }
    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh) {}

    CSubmissionPageChangeListener* GetParentWizard(wxWindow* wnd) const;

    virtual void ApplyCommand() {}
    virtual void ReportMissingFields(string &text) {}
    virtual wxString GetAnchor() {return wxEmptyString;}
protected:
    bool x_DoStepForward(wxListbook* book);
    bool x_DoStepBackward(wxListbook* book);
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___SUBMISSION_PAGE_INTERFACE__HPP
