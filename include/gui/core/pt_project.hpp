#ifndef GUI_CORE___PT_PROJECT__HPP
#define GUI_CORE___PT_PROJECT__HPP

/*  $Id: pt_project.hpp 33078 2015-05-21 15:06:52Z katargir $
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
    class CProject : public TPTItem<CRef<CGBDocument>, eProject>
    {
        typedef TPTItem<CRef<CGBDocument>, eProject> TParent;
        public:
            CProject(const TParent::TDataType& data) : TParent(data) {}

        void Initialize(wxTreeCtrl& treeCtrl);

        size_t CountDisabledItems(wxTreeCtrl& treeCtrl);
        void UpdateDisabledItems(wxTreeCtrl& treeCtrl);

        void UpdateLabel(wxTreeCtrl& treeCtrl);
        void UpdateViews(wxTreeCtrl& treeCtrl);
        void UpdateProjectItems(wxTreeCtrl& treeCtrl);
        void UpdateViewLabel(wxTreeCtrl& treeCtrl, IProjectView& view);
        void ProjectStateChanged(wxTreeCtrl& treeCtrl);
       
        virtual bool CanDoProperties();
        virtual bool DoProperties(wxTreeCtrl& treeCtrl);

        virtual void BeginLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event);
        virtual bool EndLabelEdit(wxTreeCtrl& treeCtrl, wxTreeEvent& event);
    };
}

END_NCBI_SCOPE

#endif  /// GUI_CORE___PT_PROJECT__HPP
