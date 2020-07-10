#ifndef GUI_WIDGETS_WX___WIZARD_PAGE_IMPL__HPP
#define GUI_WIDGETS_WX___WIZARD_PAGE_IMPL__HPP

/*  $Id: wizard_page_impl.hpp 26914 2012-11-26 20:23:45Z katargir $
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
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/wizard_page.hpp>

BEGIN_NCBI_SCOPE

class CWizardPage : public IWizardPage
{
public:
    CWizardPage() : m_NextPage(), m_PrevPage(), m_OptionsPage() {}

    void Reset() { m_NextPage = m_PrevPage = m_OptionsPage = 0; }

    /// @name IWizardPage interface implementation
    /// @{
    virtual IWizardPage* GetNextPage() { return m_NextPage; }
    virtual IWizardPage* GetPrevPage() { return m_PrevPage; }
    virtual IWizardPage* GetOptionsPage() { return m_OptionsPage; }
    virtual void SetNextPage(IWizardPage* page) { m_NextPage = page; }
    virtual void SetPrevPage(IWizardPage* page) { m_PrevPage = page; }
    virtual void SetOptionsPage(IWizardPage* page) { m_OptionsPage = page; }
    /// @}

protected:
    IWizardPage* m_NextPage;
    IWizardPage* m_PrevPage;
    IWizardPage* m_OptionsPage;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___WIZARD_PAGE_IMPL__HPP
