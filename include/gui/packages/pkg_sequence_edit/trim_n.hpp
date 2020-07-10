/*  $Id: trim_n.hpp 41062 2018-05-16 18:48:10Z katargir $
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
#ifndef _TRIM_N_H_
#define _TRIM_N_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/scope.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/util/sequence.hpp>

#include <wx/window.h>

BEGIN_NCBI_SCOPE
using namespace objects;



class CTrimN
{
public:
    bool apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title, wxWindow *parent, const wxString& workDir,
               const CSequenceAmbigTrimmer::TTrimRuleVec &trim_rules = CSequenceAmbigTrimmer::GetDefaultTrimRules());
};

END_NCBI_SCOPE

#endif
    // _TRIM_N_H_
