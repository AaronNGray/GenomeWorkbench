/*  $Id: muscle_tool_job.cpp 44601 2020-01-30 19:41:27Z filippov $
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
* Authors:  Roman Katargin
*/


#include <ncbi_pch.hpp>

#include <objmgr/util/sequence.hpp>

#include <objects/seq/Annotdesc.hpp>
#include <objects/gbproj/AbstractProjectItem.hpp>

#include <gui/objutils/label.hpp>
#include <gui/packages/pkg_alignment/muscle_tool_job.hpp>

#include <wx/filename.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CMuscleToolJob
CMuscleToolJob::CMuscleToolJob(const CMuscleToolParams& params)
    : CMSAToolJob("MUSCLE"),
    m_Params(params) 
{
    m_Descr = "MUSCLE Job";
}

wxString CMuscleToolJob::x_GetCommandLine(const wxString &input, const wxString &output, bool is_nucleotide)
{
    wxString muscleExe = m_Params.GetMuscleExecutable();
    if (!wxFileName::FileExists(muscleExe)) {
        NCBI_THROW(CException, eUnknown, "Path to MUSCLE executable does not exist!");
    }
       
    ///
    /// compose the MUSCLE command line
    ///

    wxStringOutputStream strstr;
    wxTextOutputStream os(strstr);

#if defined(__WINDOWS__)
    os << wxT("\"") << muscleExe << wxT("\"")
#else
    os  << wxT("'") << muscleExe << wxT("'")
#endif
        << wxT(" -in ") << input 
        << wxT(" -out ") << output
        << wxT(" -maxmb 2048");

    if (!is_nucleotide){
        os << wxT(" -seqtype protein");
    }

    switch (m_Params.GetScoringMethod()) {
    case 1 :
        os << wxT(" -sp");
        break;
    case 2 :
        os << wxT(" -sv");
        break;
    default :
        os << wxT(" -le");
        break;
    }

    /// do we create a tree?
    if (m_Params.GetGenerateTree()) {
        m_TmpTreeOut = wxFileName::CreateTempFileName(wxT("tree"));
        os << wxT(" -tree2 ") << m_TmpTreeOut;
    }

    wxString moreParams = m_Params.GetCommandLine();
    moreParams.Trim(true);
    moreParams.Trim(false);
    if (!moreParams.empty())
        os << wxT(" ") << moreParams;

#if defined(__WINDOWS__)
    return strstr.GetString();
#else
    return "sh -c \"" + strstr.GetString() + "\"";
#endif
}

END_NCBI_SCOPE

