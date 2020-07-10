#ifndef GUI_OBJUTILS___CMD_PROPAGATE_FEAT__HPP
#define GUI_OBJUTILS___CMD_PROPAGATE_FEAT__HPP

/*  $Id: cmd_propagate_feat.hpp 42666 2019-04-01 14:27:25Z filippov $
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

#include <corelib/ncbiobj.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

#include <gui/gui_export.h>
#include <gui/objutils/cmd_composite.hpp>

BEGIN_NCBI_SCOPE


NCBI_GUIOBJUTILS_EXPORT void PropagateToTarget(CRef<CCmdComposite> composite, const vector<CConstRef<objects::CSeq_feat>>& feat_list,
                                               objects::CBioseq_Handle src, objects::CBioseq_Handle target, const objects::CSeq_align& align,
                                               bool stop_at_stop, bool cleanup_partials, bool merge_abutting, bool extend_over_gaps, bool create_general_only,
                                               CMessageListener_Basic* listener, objects::CObject_id::TId* feat_id = nullptr);


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_PROPAGATE_FEAT__HPP
