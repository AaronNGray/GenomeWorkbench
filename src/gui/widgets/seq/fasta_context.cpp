/*  $Id: fasta_context.cpp 39785 2017-11-03 16:40:10Z katargir $
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
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/text_widget/expand_traverser.hpp>

#include "fasta_context.hpp"
#include "text_commands.hpp"

#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

BEGIN_EVENT_TABLE(CFastaViewContext, CTextPanelContext)
    EVT_MENU(kExpandAll, CFastaViewContext::OnExpandAll)
END_EVENT_TABLE()

CFastaViewContext::CFastaViewContext(int leftMargin, objects::CScope* scope, const CSerialObject* so)
    : CTextPanelContext(leftMargin, scope, so)
{
}

CFastaViewContext::~CFastaViewContext()
{
}

static
WX_DEFINE_MENU(kFastaContextMenu)
    WX_MENU_SEPARATOR_L("Edit Selection")
    WX_MENU_ITEM(kExpandAll)
WX_END_MENU()

wxMenu* CFastaViewContext::CreateMenu() const
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    wxMenu* menu = cmd_reg.CreateMenu(kFastaContextMenu);
    return menu;
}

void CFastaViewContext::OnExpandAll(wxCommandEvent& event)
{
    CCompositeTextItem* root = dynamic_cast<CCompositeTextItem*>(m_Panel->GetRootItem());
    if (!root) return;

    CExpandTraverser expander(*this, true);
    root->Traverse(expander);
    root->UpdatePositions();
    m_Panel->Layout();
    m_Panel->Refresh();

    m_Panel->UpdateCaretPos();
}

END_NCBI_SCOPE
