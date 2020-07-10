/*  $Id: convert_desc_to_feat.hpp 37346 2016-12-27 18:27:53Z filippov $
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
 * Authors:  Igor Filippov
 */
#ifndef _CONVERT_DESC_TO_FEAT_H_
#define _CONVERT_DESC_TO_FEAT_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/scope.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_create_desc.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CConvertDescToFeatComment
{
public:
static CRef<CCmdComposite> apply(CSeq_entry_Handle tse);
};

class CConvertDescToFeatSource
{
public:
static CRef<CCmdComposite> apply(CSeq_entry_Handle tse);
};

class CConvertDescToFeatPub
{
public:
static CRef<CCmdComposite> apply(CSeq_entry_Handle tse);
};

class CConvertDescToFeatPubConstraint
{
public:
static CRef<CCmdComposite> apply(CSeq_entry_Handle tse);
};

class CConvertFeatToDescComment
{
public:
static CRef<CCmdComposite> apply(CSeq_entry_Handle tse);
};

class CConvertFeatToDescSource
{
public:
static CRef<CCmdComposite> apply(CSeq_entry_Handle tse);
};

class CConvertFeatToDescPub
{
public:
static CRef<CCmdComposite> apply(CSeq_entry_Handle tse);
};

END_NCBI_SCOPE

#endif
    // _CONVERT_DESC_TO_FEAT_H_
