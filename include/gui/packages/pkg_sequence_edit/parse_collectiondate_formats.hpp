/*  $Id: parse_collectiondate_formats.hpp 42254 2019-01-22 15:22:29Z asztalos $
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
#ifndef _PARSE_COLLECTIONDATE_FORMATS_H_
#define _PARSE_COLLECTIONDATE_FORMATS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/packages/pkg_sequence_edit/modify_bsrc_interface.hpp>

BEGIN_NCBI_SCOPE

class CParseCollectionDateFormats : public IModifyBiosourceQuals
{
public:
    CRef<CCmdComposite> GetCommand(objects::CSeq_entry_Handle tse, bool monthFirst);

private:
    virtual bool x_ApplyToBioSource(objects::CBioSource& biosource);
    bool m_MonthFirst{ false };
};

END_NCBI_SCOPE

#endif
    // _PARSE_COLLECTIONDATE_FORMATS_H_
