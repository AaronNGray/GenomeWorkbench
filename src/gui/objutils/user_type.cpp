/*  $Id: user_type.cpp 26143 2012-07-25 19:53:48Z falkrb $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/user_type.hpp>

BEGIN_NCBI_SCOPE


const string CGUIUserType::sm_Tp_Undefined        = "";
const string CGUIUserType::sm_Tp_Alignment        = "Alignment";
const string CGUIUserType::sm_Tp_Alignment_Set    = "Alignment Set";
const string CGUIUserType::sm_Tp_Annotation       = "Annotation";
const string CGUIUserType::sm_Tp_EntrezGeneRecord = "Entrez Gene Record";
const string CGUIUserType::sm_Tp_EntrezRecord     = "Enrez Record";
const string CGUIUserType::sm_Tp_Feature          = "Feature";
const string CGUIUserType::sm_Tp_Location         = "Location";
const string CGUIUserType::sm_Tp_PhylogeneticTree = "Phylogenetic Tree";
const string CGUIUserType::sm_Tp_TaxId_Set        = "Taxonomic ID Set";
const string CGUIUserType::sm_Tp_Sequence         = "Sequence";
const string CGUIUserType::sm_Tp_Sequence_ID      = "Sequence ID";
const string CGUIUserType::sm_Tp_Sequence_Set     = "Sequence Set";
const string CGUIUserType::sm_Tp_Submission       = "Submission";
const string CGUIUserType::sm_Tp_TaxPlotData      = "Tax Plot Data";
const string CGUIUserType::sm_Tp_ValidationResult = "Validation Result";
const string CGUIUserType::sm_Tp_SeqTable         = "SEQ TABLE (Beta)"; // TODO
const string CGUIUserType::sm_Tp_Variation		  = "Variation";

const string CGUIUserType::sm_Tp_Project          = "Project";
const string CGUIUserType::sm_Tp_ProjectItem      = "Project Item";
const string CGUIUserType::sm_Tp_Workspace        = "Workspace";


const string CGUIUserType::sm_SbTp_Undefined      = "";

const string CGUIUserType::sm_SbTp_DNA            = "DNA";
const string CGUIUserType::sm_SbTp_Protein        = "Protein";

const string CGUIUserType::sm_SbTp_Features       = "Features";
const string CGUIUserType::sm_SbTp_Alignments     = "Alignments";
const string CGUIUserType::sm_SbTp_Graphs         = "Graphs";
const string CGUIUserType::sm_SbTp_Sequence_IDs   = "Sequence IDs";
const string CGUIUserType::sm_SbTp_Locations      = "Locations";
const string CGUIUserType::sm_SbTp_Table          = "Table";

const string CGUIUserType::sm_SbTp_NucProt        = "DNA + Protein Translation (Nuc-Prot)";
const string CGUIUserType::sm_SbTp_GenProdSet     = "Genomic Products";
const string CGUIUserType::sm_SbTp_SegSeq         = "Segmented Sequence";

const string CGUIUserType::sm_SbTp_PopSet         = "Population Study";
const string CGUIUserType::sm_SbTp_PhySet         = "Phylogenetic Study";
const string CGUIUserType::sm_SbTp_EcoSet         = "Ecological Study";
const string CGUIUserType::sm_SbTp_MutSet         = "Mutation Study";

const string CGUIUserType::sm_SbTp_PopSet_Aln     = "Population Study (Aligned)";
const string CGUIUserType::sm_SbTp_PhySet_Aln     = "Phylogenetic Study (Aligned)";
const string CGUIUserType::sm_SbTp_EcoSet_Aln     = "Ecological Study (Aligned)";
const string CGUIUserType::sm_SbTp_MutSet_Aln     = "Mutation Study (Aligned)";

const string CGUIUserType::sm_SbTp_ProjectVer1    = "Ver 1";
const string CGUIUserType::sm_SbTp_ProjectVer2    = "Ver 2";


CGUIUserType::CGUIUserType()
{
}

CGUIUserType::CGUIUserType(const string& type, const string& subtype)
:   m_Type(type),
    m_Subtype(subtype)
{
}

bool CGUIUserType::operator<(const CGUIUserType& other) const
{
    return m_Type < other.m_Type ||
           (m_Type == other.m_Type  &&  m_Subtype < other.m_Subtype);
}


END_NCBI_SCOPE
