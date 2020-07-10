/*  $Id: retranslate_cds.hpp 43629 2019-08-09 19:00:33Z filippov $
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
#ifndef _RETRANSLATE_CDS_H_
#define _RETRANSLATE_CDS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/scope.hpp>
#include <objects/misc/sequence_macros.hpp>


BEGIN_NCBI_SCOPE
using namespace objects;

class CRetranslateCDS
{
public:
    enum ERetranslateMethod
    {
        eRetranslateCDSIgnoreStopExceptEnd,
        eRetranslateCDSObeyStop,
        eRetranslateCDSChooseFrame,
    };
    void apply( objects::CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor, string title, ERetranslateMethod method );
    static CRef<CCmdComposite> RetranslateCDSCommand(CScope& scope, const CSeq_feat& old_cds, CSeq_feat& cds, bool& cds_change, int& offset, bool create_general_only);
    static CRef<CCmdComposite> GetCommand( objects::CSeq_entry_Handle seh, string title, ERetranslateMethod method, string &error);
};

END_NCBI_SCOPE

#endif
   
