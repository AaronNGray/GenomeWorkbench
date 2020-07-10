/*  $Id: init_w_aln_score.cpp 37435 2017-01-10 23:15:14Z shkeda $
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
#include <gui/widgets/aln_score/init_w_aln_score.hpp>

#include <gui/widgets/wx/ui_tool_registry.hpp>

#include <gui/widgets/aln_score/quality_methods.hpp>
#include <gui/widgets/aln_score/simple_methods.hpp>
#include <gui/widgets/aln_score/template_methods.hpp>

BEGIN_NCBI_SCOPE

bool init_w_aln_score()
{
    CUIToolRegistry* reg = CUIToolRegistry::GetInstance();

    reg->RegisterTool(new CQualityScoringMethodAA(), "cq_aa"); // Column Quality Score - Protein
    reg->RegisterTool(new CQualityScoringMethodNA(), "cq_na"); // Column Quality Score - DNA

    reg->RegisterTool(new CSimpleScoringMethod(), "fbd"); // Frequency-Based Difference
    reg->RegisterTool(new CSNPScoringMethod(), "diff");
    reg->RegisterTool(new CConservationScoringMethod(), "cons"); // Conservation Score, prot only

    reg->RegisterTemplateTool("ColumnScore", new CColumnScoringMethod());
    reg->RegisterTemplateTool("Coloration", new CColorTableMethod());
    reg->RegisterTemplateTool("MatrixScore", new CMatrixScoringMethod());

    return true;
}

END_NCBI_SCOPE
