#ifndef GUI_CORE___PT_WORKSPACE__HPP
#define GUI_CORE___PT_WORKSPACE__HPP

/*  $Id: pt_workspace.hpp 40149 2017-12-26 17:31:04Z katargir $
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

#include <corelib/ncbiobj.hpp>

#include <gui/core/pt_item.hpp>

#include <gui/objects/GBWorkspace.hpp>

BEGIN_NCBI_SCOPE

namespace PT
{
    class CWorkspace : public TPTItem<CRef<objects::CGBWorkspace>, eWorkspace>
    {
        typedef TPTItem<CRef<objects::CGBWorkspace>, eWorkspace> TParent;
        public:
            CWorkspace(const TParent::TDataType& data) : TParent(data) {}

        void UpdateDisabledItems(wxTreeCtrl& treeCtrl);

        void ProjectStateChanged(wxTreeCtrl& treeCtrl, CGBDocument& doc);
        void ProjectAdded(wxTreeCtrl& treeCtrl, CGBDocument& doc);
        void ProjectRemoved(wxTreeCtrl& treeCtrl, size_t id);
        void UpdateProjectItems(wxTreeCtrl& treeCtrl, CGBDocument& doc);
        void UpdateProjectLabel(wxTreeCtrl& treeCtrl, CGBDocument& doc);
        void UpdateViewLabel(wxTreeCtrl& treeCtrl, CGBDocument& doc, IProjectView& view);

        void UpdateLabel(wxTreeCtrl& treeCtrl);
        void UpdateViews(wxTreeCtrl& treeCtrl, CGBDocument* doc = 0);

        void UpdateHiddenItems(wxTreeCtrl& treeCtrl);

        void Initialize(wxTreeCtrl& treeCtrl);

        virtual void BeginLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event);
        virtual bool EndLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event);
    };

    typedef TPTItem<int, eHiddenItems> CHiddenItems;
}

END_NCBI_SCOPE

#endif  /// GUI_CORE___PT_WORKSPACE__HPP
