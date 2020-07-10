#ifndef PKG_ALIGNMENT___CLI_TOOL_JOB__HPP
#define PKG_ALIGNMENT___CLI_TOOL_JOB__HPP

/*  $Id: msa_tool_job.hpp 38490 2017-05-16 18:21:11Z evgeniev $
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
* Authors:  Roman Katargin, Vladislav Evgeniev
*
* File Description:
*      Defines a base class for integration of third-party multiple sequence alignment tools into GBench
*/

#include <corelib/ncbistd.hpp>
#include <corelib/ncbifile.hpp>
#include <gui/core/loading_app_job.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include <wx/string.h>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CScope;
    class CSeq_loc;
    class CSeq_id_Handle;
END_SCOPE(objects)

/** @addtogroup GUI_PKG_ALIGNMENT
*
* @{
*/

///////////////////////////////////////////////////////////////////////////////
/// CMSAToolJob
/// Provides basic functionality for the integration of third-party multiple sequence alignment tools:
/// - Creation/destruction of temporary files;
/// - Ids re-mapping;
/// - Execution of the third-party tool;
/// - Creation of project items.
class  CMSAToolJob : public CDataLoadingAppJob
{
public:
    typedef pair< CConstRef<objects::CSeq_loc>, CRef<objects::CScope> > TLocPair;
    typedef map<string, TLocPair> TIdMap;
    typedef map<objects::CSeq_id_Handle, bool> TSeqTypeMap;    

public:
    CMSAToolJob(const wxString &tool_name);
    virtual ~CMSAToolJob();

protected:
    virtual bool BeforeRun();
    virtual void x_CreateProjectItems();
    virtual void x_AddTreeProjectItem(const string &title, const string &comment);
    /// Returns the command line, that will be used to execute the third-party tool
    virtual wxString x_GetCommandLine(const wxString &input, const wxString &output, bool is_nucleotide) = 0;
    /// Returns the sequences that will be aligned
    virtual TConstScopedObjects& x_GetObjects() = 0;
    /// Returns the directory where to execute the msa tool
    virtual wxString x_GetWorkingDirectory() const { return wxT(""); }

protected:
    wxString    m_TmpIn;
    wxString    m_TmpOut;
    wxString    m_TmpTreeOut;

    long m_PId;

    /// to make the alignment as robust as possible, we assign every
    /// sequence a unique identifer independeont of its existing
    /// identifiers
    /// this map allows us to convert back at the end
    TIdMap                  m_IdMap;
    TSeqTypeMap             m_SeqTypes;
    objects::CMappingRanges m_Ranges;
    wxString                m_CmdLine;
    wxString                m_ToolName;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___CLI_TOOL_JOB__HPP
