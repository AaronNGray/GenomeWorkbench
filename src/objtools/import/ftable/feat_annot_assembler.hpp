/*  $Id: feat_annot_assembler.hpp 585671 2019-05-02 15:04:11Z ludwigf $
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
* Author:  Frank Ludwig, NCBI
*
* File Description:
*   Test application for the CFormatGuess component
*
* ===========================================================================
*/

#ifndef FEAT_ANNOT_ASSEMBLER__HPP
#define FEAT_ANNOT_ASSEMBLER__HPP

#include <corelib/ncbifile.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

#include "feat_import_data.hpp"
#include "../annot_import_data.hpp"

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE

//  ============================================================================
class CFeatAnnotAssembler
//  ============================================================================
{
public:
    CFeatAnnotAssembler(
        CImportMessageHandler&);

    virtual ~CFeatAnnotAssembler();

    virtual void
    InitializeAnnot(
        CSeq_annot&);

    virtual void
    ProcessRecord(
        const CFeatImportData&,
        CSeq_annot&) =0;

    virtual void
    FinalizeAnnot(
        const CAnnotImportData&,
        CSeq_annot&);

protected:
    CImportMessageHandler& mErrorReporter;
};

END_objects_SCOPE
END_NCBI_SCOPE

#endif
