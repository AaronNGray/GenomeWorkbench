/*  $Id: label_rna.hpp 36273 2016-09-06 14:58:31Z filippov $
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
#ifndef _LABEL_RNA_H_
#define _LABEL_RNA_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/scope.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/framework/pkg_wb_connect.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/core/selection_service_impl.hpp>

#include <util/line_reader.hpp>


BEGIN_NCBI_SCOPE
using namespace objects;

class CLabelRna
{
public:
    bool apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title, wxWindow* parent);
private:
    string GetGenBankId(CBioseq_Handle bsh);
    CBioseq_Handle GetBioseqHandleFromIdGuesser(const string &id_str, objects::CSeq_entry_Handle tse);
    void AddNewMrnaCommand(CRef <CCmdComposite> composite, CBioseq_Handle bsh, CRef <CSeq_feat> new_mrna, bool negative);
    void PrepareInputFile(CSeq_entry_Handle tse, CNcbiOfstream &ostr);
};

END_NCBI_SCOPE

#endif
    // _LABEL_RNA_H_
