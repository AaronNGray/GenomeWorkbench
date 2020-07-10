/*  $Id: align_project_item_extension.cpp 30731 2014-07-17 17:24:59Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include "align_project_item_extension.hpp"

#include <objects/gbproj/ProjectItem.hpp>
#include <objects/seq/Seq_annot.hpp>

#include <algo/align/util/score_builder.hpp>

#include <gui/core/document.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CAlignProjectItemExtension

string CAlignProjectItemExtension::GetExtensionIdentifier() const
{
    static string sid("align_project_item_extension");
    return sid;
}

string CAlignProjectItemExtension::GetExtensionLabel() const
{
    static string slabel("Alignment package Project item extension");
    return slabel;
}

static const string kBLASTProjectItemTag = "blast_project_item";

/// calculate scores, this is done in a separate function that is called before
/// inserting the Project Item into a Project because we need a Scope.

void CAlignProjectItemExtension::ProjectItemAttached(CProjectItem& item, CGBDocument& doc, IServiceLocator* service_locator)
{
    if (item.HasTag(kBLASTProjectItemTag)) {
        if (item.GetItem().IsAnnot()) {
                typedef CSeq_annot::TData::TAlign TAlign;

                /// add scores where needed
                CSeq_annot& annot = item.SetItem().SetAnnot();
                TAlign& an_align = annot.SetData().SetAlign();

                CScoreBuilder builder;
                builder.AddScore(*doc.GetScope(), an_align, CSeq_align::eScore_PercentIdentity);
        }
        item.RemoveTag(kBLASTProjectItemTag);
    }
}

void CAlignProjectItemExtension::ProjectItemDetached(CProjectItem& /*item*/, CGBDocument& /*doc*/, IServiceLocator* /*service_locator*/)
{
}

END_NCBI_SCOPE
