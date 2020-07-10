#ifndef GUI_CORE___PT_PROJECT_FOLDER__HPP
#define GUI_CORE___PT_PROJECT_FOLDER__HPP

/*  $Id: pt_project_folder.hpp 33078 2015-05-21 15:06:52Z katargir $
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
#include <gui/core/document.hpp>

BEGIN_NCBI_SCOPE

namespace PT
{
    class CProjectFolder : public TPTItem<CRef<objects::CProjectFolder>, eProjectFolder>
    {
        typedef TPTItem<CRef<objects::CProjectFolder>, eProjectFolder> TParent;
        public:
            CProjectFolder(const TParent::TDataType& data) : TParent(data) {}

        size_t CountDisabledItems(wxTreeCtrl& treeCtrl);

        void UpdateDisabledItems(wxTreeCtrl& treeCtrl);
        void UpdateProjectItems(wxTreeCtrl& treeCtrl, CGBDocument& doc);

        void Initialize(wxTreeCtrl& treeCtrl, CGBDocument& doc);

        virtual void OnItemExpandedCollapsed(wxTreeCtrl& treeCtrl);

        virtual bool CanDoNewFolder() const { return true; }
        virtual void DoNewFolder(wxTreeCtrl& treeCtrl);

        virtual bool CanDoProperties() { return true; }
        virtual bool DoProperties(wxTreeCtrl& treeCtrl);

        virtual void BeginLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event);
        virtual bool EndLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event);

        virtual bool CanDoRemove(wxTreeCtrl& treeCtrl) const;
        virtual bool DoRemove(wxTreeCtrl& treeCtrl);

        virtual bool CanCopyToClipboard(wxTreeCtrl& treeCtrl) const;
        virtual bool CanCutToClipboard(wxTreeCtrl& treeCtrl) const;

        virtual bool CanPaste(wxTreeCtrl& treeCtrl) const;
        virtual bool Paste(wxTreeCtrl& treeCtrl, PT::TItems& items, bool move);
    };
}

END_NCBI_SCOPE

#endif  /// GUI_CORE___PT_PROJECT_FOLDER__HPP
