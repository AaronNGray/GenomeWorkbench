#ifndef GUI_UTILS___USER_TYPE_IDS__HPP
#define GUI_UTILS___USER_TYPE_IDS__HPP

/*  $Id: user_type_ids.hpp 17899 2008-09-25 18:02:17Z katargir $
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

#define UT_Unknown                  "/Unknown"

#define UT_Sequence                 "/Sequence"
#define UT_Sequence_DNA             "/Sequence/DNA"
#define UT_Sequence_Protein         "/Sequence/Protein"

#define UT_SequenceID               "/SequenceID"
#define UT_SequenceID_DNA           "/SequenceID/DNA"
#define UT_SequenceID_Protein       "/SequenceID/Protein"

#define UT_Location                 "/Location"

#define UT_Sequence_Set             "/SequenceSet"
#define UT_Sequence_Set_Nuc_Prot    "/SequenceSet/Nuc-Prot"
#define UT_Sequence_Set_Seg_Seq     "/SequenceSet/Seg_Seq"
#define UT_Sequence_Set_GenProdSet  "/SequenceSet/GenProdSet"
#define UT_Sequence_Set_GenProdSet  "/SequenceSet/GenProdSet"
#define UT_Sequence_Set_PopSet      "/SequenceSet/PopSet"
#define UT_Sequence_Set_PopSetAln   "/SequenceSet/PopSetAln"
#define UT_Sequence_Set_PhySet      "/SequenceSet/PhySet"
#define UT_Sequence_Set_PhySetAln   "/SequenceSet/PhySetAln"
#define UT_Sequence_Set_EcoSet      "/SequenceSet/EcoSet"
#define UT_Sequence_Set_EcoSetAln   "/SequenceSet/EcoSetAln"
#define UT_Sequence_Set_MutSet      "/SequenceSet/MutSet"
#define UT_Sequence_Set_MutSetAln   "/SequenceSet/MutSetAln"

#define UT_Alignment_Set            "/AlignmentSet"

#define UT_Annotation               "/Annotation"
#define UT_Annotation_Alignments    "/Annotation/Alignments"
#define UT_Annotation_Features      "/Annotation/Features"
#define UT_Annotation_Graphs        "/Annotation/Graphs"
#define UT_Annotation_SequenceIDs   "/Annotation/SequenceIDs"
#define UT_Annotation_Locations     "/Annotation/Locations"
#define UT_Annotation_Table         "/Annotation/Table"

#define UT_Alignment                "/Alignment"
#define UT_Feature                  "/Feature"
#define UT_Feature_SNP              "/Feature/SNP"
#define UT_Feature_exon             "/Feature/exon"
#define UT_Feature_mRNA             "/Feature/mRNA"
#define UT_Feature_otherRNA         "/Feature/otherRNA"

#define UT_Feature_gene             "/Feature/gene"
#define UT_Feature_STS              "/Feature/STS"
#define UT_Feature_CDS              "/Feature/CDS"

#define UT_EntrezRecord             "/EntrezRecord"
#define UT_ValidationResult         "/ValidationResult"
#define UT_BioTree                  "/BioTree"
#define UT_DistanceMatrix           "/DistanceMatrix"
#define UT_EntrezGeneRecord         "/EntrezGeneRecord"

#endif  // GUI_UTILS___USER_TYPE_IDS__HPP
