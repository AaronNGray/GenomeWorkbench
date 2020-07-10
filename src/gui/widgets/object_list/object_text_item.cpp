/*  $Id: object_text_item.cpp 32647 2015-04-07 16:12:16Z katargir $
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

#include <gui/widgets/object_list/object_text_item.hpp>

#include "text_item_seq_loc.hpp"
#include "text_item_seq_feat.hpp"
#include "text_item_seq_align.hpp"
#include "text_item_default.hpp"

BEGIN_NCBI_SCOPE

ITextItem* CObjectTextItem::CreateTextItem(SConstScopedObject& object, EMode mode, ICanceled* cancel)
{
    const CObject* pobj = object.object.GetPointerOrNull();
    const objects::CSeq_loc* seq_loc =  dynamic_cast<const objects::CSeq_loc*>(pobj);
    if (seq_loc)
        return CTextItemSeq_loc::CreateObject(object, mode, cancel);

    const objects::CSeq_feat* seq_feat =  dynamic_cast<const objects::CSeq_feat*>(pobj);
    if (seq_feat)
        return CTextItemSeq_feat::CreateObject(object, mode);

    const objects::CSeq_align* seq_align =  dynamic_cast<const objects::CSeq_align*>(pobj);
    if (seq_align)
        return CTextItemSeq_align::CreateObject(object, mode);

    return CTextItemDefault::CreateObject(object, mode);
}

END_NCBI_SCOPE
