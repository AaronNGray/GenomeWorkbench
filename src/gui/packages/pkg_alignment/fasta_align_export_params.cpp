/*  $Id: fasta_align_export_params.cpp 38543 2017-05-24 21:37:01Z evgeniev $
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
 * Authors: Vladislav Evgeniev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/packages/pkg_alignment/fasta_align_export_params.hpp>

BEGIN_NCBI_SCOPE

CFastaAlignExportParams::CFastaAlignExportParams()
{
    Init();
}

CFastaAlignExportParams::CFastaAlignExportParams(const CFastaAlignExportParams& data)
{
    Init();
    Copy(data);
}

CFastaAlignExportParams::~CFastaAlignExportParams()
{
}

void CFastaAlignExportParams::operator=(const CFastaAlignExportParams& data)
{
    Copy(data);
}

bool CFastaAlignExportParams::operator==(const CFastaAlignExportParams& data) const
{
////@begin CFastaAlignExportParams equality operator
     if (!(m_FileName == data.m_FileName)) return false;
////@end CFastaAlignExportParams equality operator
    return true;
}

void CFastaAlignExportParams::Copy(const CFastaAlignExportParams& data)
{
    m_Object = data.m_Object;
////@begin CFastaAlignExportParams copy function
    m_FileName = data.m_FileName;
////@end CFastaAlignExportParams copy function
}

void CFastaAlignExportParams::Init()
{
////@begin CFastaAlignExportParams member initialisation
////@end CFastaAlignExportParams member initialisation
}

END_NCBI_SCOPE
