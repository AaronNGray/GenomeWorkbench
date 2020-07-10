/*  $Id: modify_feature_id.hpp 43338 2019-06-17 14:54:52Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 *
 * Authors:  Igor Filippov
 */
#ifndef _MODIFY_FEATURE_ID_H_
#define _MODIFY_FEATURE_ID_H_

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>

BEGIN_NCBI_SCOPE

class CAssignFeatureId
{
public: 
    bool apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title);
private:
    bool AssignHighestFeatureId(const objects::CSeq_entry_Handle& entry, objects::CObject_id::TId& feat_id, CCmdComposite* composite);
};

class CClearFeatureId
{
public: 
    bool apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title);
private:
    void ClearFeatureIds(const objects::CSeq_entry_Handle& entry, CCmdComposite* composite);
};

class  CReassignFeatureId
{
public:
    bool apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title);
};

class CUniqifyFeatureId
{
public:
    bool apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title);
};

END_NCBI_SCOPE

#endif
    // _MODIFY_FEATURE_ID_H_
