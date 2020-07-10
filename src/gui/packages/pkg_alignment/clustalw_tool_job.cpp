/*  $Id: clustalw_tool_job.cpp 44601 2020-01-30 19:41:27Z filippov $
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

#include <gui/packages/pkg_alignment/clustalw_tool_job.hpp>

#include <wx/filename.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/utils.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CClustalwToolJob
CClustalwToolJob::CClustalwToolJob(const CClustalwToolParams& params) 
    : CMSAToolJob("ClustalW")
    , m_Params(params)
{
     m_Descr = "ClustalW Job";
}

wxString CClustalwToolJob::x_GetCommandLine(const wxString &input, const wxString &output, bool is_nucleotide)
{
    wxString clustalwExe = m_Params.GetClustalWExecutable();
    if (!wxFileName::FileExists(clustalwExe)) {
        NCBI_THROW(CException, eUnknown, "Path to ClustalW executable does not exist!");
    }

    if (m_Params.GetGenerateTree())
        m_TmpTreeOut = wxFileName::CreateTempFileName(wxT("tree"));
    
    ///
    /// compose the ClustalW command line
    ///

    wxStringOutputStream strstr;
    wxTextOutputStream os(strstr);

#if defined(__WINDOWS__)
    os << wxT("\"") << clustalwExe << wxT("\"")
#else
    os  << wxT("'") << clustalwExe << wxT("'")
#endif
        << wxT(" -INFILE=") << input
        << wxT(" -OUTFILE=") << output
        << wxT(" -OUTPUT=FASTA")
        << wxT(" -ALIGN");
    
    if (!m_TmpTreeOut.empty()) {
        os  << wxT(" -OUTPUTTREE=nexus")
            << wxT(" -NEWTREE=") << m_TmpTreeOut
            << wxT(" -TREE");
    }

    wxString moreParams = m_Params.GetCommandLine();
    moreParams.Trim(true);
    moreParams.Trim(false);
    if (!moreParams.empty())
        os << wxT(" ") << moreParams;

    wxString cmdline = strstr.GetString();

#if defined(__WINDOWS__)
    return strstr.GetString();
#else
    return "sh -c \"" + strstr.GetString() + "\"";
#endif    
}

END_NCBI_SCOPE

