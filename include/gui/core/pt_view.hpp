#ifndef GUI_CORE___PT_VIEW__HPP
#define GUI_CORE___PT_VIEW__HPP

/*  $Id: pt_view.hpp 33078 2015-05-21 15:06:52Z katargir $
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

#include <gui/core/project_view.hpp>

BEGIN_NCBI_SCOPE

namespace PT
{
    class CView : public TPTItem<CIRef<IProjectView>, eView>
    {
        typedef TPTItem<CIRef<IProjectView>, eView>  TParent;
        public:
            CView(const TParent::TDataType& data) : TParent(data) {}

        void Initialize(wxTreeCtrl& treeCtrl, IProjectView& view, bool local);
    };
}

END_NCBI_SCOPE

#endif  /// GUI_CORE___PT_VIEW__HPP
