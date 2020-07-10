/*  $Id: pt_utils.cpp 33078 2015-05-21 15:06:52Z katargir $
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

#include <objects/general/Date.hpp>
#include <objects/gbproj/FolderInfo.hpp>
#include <objects/seq/Annotdesc.hpp>

#include <objects/gbproj/ProjectDescr.hpp>

#include <gui/objects/GBWorkspace.hpp>

#include <gui/core/pt_utils.hpp>
#include <gui/core/pt_project.hpp>
#include <gui/core/document.hpp>
#include <gui/core/fname_validator_imp.hpp>
#include <gui/core/folder_edit_dlg.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/treectrl.h>

BEGIN_NCBI_SCOPE

using namespace PT;

bool PT::sm_HideDisabledItems = false;

int PT::GetItemTypes(const TItems& items)
{
    int types = 0;
    for (size_t i = 0;  i < items.size(); i++) 
        types |= items[i]->GetType();
    return types;
}

int PT::GetProjectId(wxTreeCtrl& treeCtrl, const CItem& item)
{
    const CItem* pitem = &item;

    while(pitem && pitem->GetType() != eProject)
          pitem = pitem->GetParent(treeCtrl);

    return pitem ? static_cast<const CProject*>(pitem)->GetData()->GetId() : -1;
}

CProject* PT::GetProject (wxTreeCtrl& treeCtrl, const CItem& item)
{
    const CItem* pitem = &item;
    while (pitem && pitem->GetType() != eProject)
          pitem = pitem->GetParent(treeCtrl);

    return static_cast<CProject*>(const_cast<CItem*>(pitem));
}

END_NCBI_SCOPE
