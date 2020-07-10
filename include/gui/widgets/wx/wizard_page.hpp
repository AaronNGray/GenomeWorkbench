#ifndef GUI_WIDGETS_WX___WIZARD_PAGE__HPP
#define GUI_WIDGETS_WX___WIZARD_PAGE__HPP

/*  $Id: wizard_page.hpp 26914 2012-11-26 20:23:45Z katargir $
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

class wxPanel;

BEGIN_NCBI_SCOPE

class IWizardPage
{
public:
    virtual ~IWizardPage() {}

    virtual bool         CanLeavePage(bool forward) = 0;
    virtual wxPanel*     GetPanel() = 0;

    virtual IWizardPage* GetNextPage() = 0;
    virtual IWizardPage* GetPrevPage() = 0;
    virtual IWizardPage* GetOptionsPage() = 0;
    virtual void SetNextPage(IWizardPage*) = 0;
    virtual void SetPrevPage(IWizardPage*) = 0;
    virtual void SetOptionsPage(IWizardPage*) = 0;
};

END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___WIZARD_PAGE__HPP
