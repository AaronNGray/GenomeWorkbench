/*  $Id: init.cpp 33488 2015-08-03 20:34:48Z evgeniev $
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
#include <gui/objutils/init.hpp>

BEGIN_NCBI_SCOPE

extern void initCGuiObjectInfoDefault();
extern void initCGuiObjectInfoSeq_id();
extern void initCGuiObjectInfoSeq_loc();
extern void initCGuiObjectInfoBioseq();
extern void initCGuiObjectInfoBioseq_set();
extern void initCGuiObjectInfoSeq_feat();
extern void initCGuiObjectInfoSeq_annot();
extern void initCGuiObjectInfoSeq_align();
extern void initCGuiObjectInfoSeq_align_set();
extern void initCGuiObjectInfoValidError();
extern void initCGuiObjectInfoBioTreeContainer();
extern void initCGuiObjectInfoGCAssembly();
extern void initCGuiObjectInfoDistanceMatrix();
extern void initCGuiObjectInfoEntrezgene();
extern void initCGuiObjectInfoSeq_entry();

extern void initCGuiSeqInfoDefault();
extern void initCGuiSeqInfoSeq_id();

extern void initCTableDataSeq_table();
extern void initCTableDataBioTreeContainer();
extern void initCTableDataGCAssembly();
extern void initCTableDataSeq_annot();

extern void init_basic_converters();

extern void init_Seq_idFP();
extern void init_Seq_locFP();
extern void init_Seq_FeatFP();
extern void init_Seq_alignFP();
extern void init_Seq_graphFP();

bool init_gui_objutils()
{
    initCGuiObjectInfoDefault();
    initCGuiObjectInfoSeq_id();
    initCGuiObjectInfoSeq_loc();
    initCGuiObjectInfoBioseq();
    initCGuiObjectInfoBioseq_set();
    initCGuiObjectInfoSeq_feat();
    initCGuiObjectInfoSeq_annot();
    initCGuiObjectInfoSeq_align();
    initCGuiObjectInfoSeq_align_set();
    initCGuiObjectInfoValidError();
    initCGuiObjectInfoBioTreeContainer();
    initCGuiObjectInfoGCAssembly();
    initCGuiObjectInfoDistanceMatrix();
    initCGuiObjectInfoEntrezgene();
    initCGuiObjectInfoSeq_entry();
    
    initCGuiSeqInfoDefault();
    initCGuiSeqInfoSeq_id();

    initCTableDataSeq_table();
    initCTableDataBioTreeContainer();
    initCTableDataGCAssembly();
    initCTableDataSeq_annot();

    init_basic_converters();

    init_Seq_idFP();
    init_Seq_locFP();
    init_Seq_FeatFP();
    init_Seq_alignFP();
    init_Seq_graphFP();

    return true;
}

END_NCBI_SCOPE
