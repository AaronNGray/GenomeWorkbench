/*  
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
* Authors:  Vladislav Evgeniev
*
* File Description:
*    Command-line tool to run macros on annotated biological sequence
*	  data represented according to NCBI's ASN.1 specifications.
*
*/

#include <corelib/ncbiapp.hpp>
#include <serial/objhook.hpp>
#include <gui/objutils/macro_lib.hpp>
#include <gui/objutils/macro_rep.hpp>

BEGIN_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////
///  Main class of the command-line tool to run macros on annotated 
/// biological sequence data
class CGBenchMacroApplication :
    public CNcbiApplication
{
public:
    CGBenchMacroApplication();
    ~CGBenchMacroApplication();
private:
    /// @name CNcbiApplication overridables
    /// @{
    void	Init(void);
    int		Run(void);
    int		DryRun(void);
    /// @}

    /// Verifies the command-line arguments of the application.
    /// @param[in] args
    ///     The command-line arguments to validate.
    /// @return
    ///     0 if the arguments are valid, 1 otherwise.
    int Setup(const CArgs& args);

    /// Creates an input stream and detects the file format (text or binary).
    /// @param[in] fname
    ///     Reference to a string, holding the file name.
    /// @param[in] binary
    ///     Flag, indicationg if the input is binary (used only if the format cannot be detected).
    /// @return
    ///     Returns smart pointer to the input stream.
    auto_ptr<CObjectIStream> OpenFile(const string& fname);

    /// Reads a CSeq_submit structure from the input stream
    /// @throws CException
    ///     In case CSeq_submit contains no sequences.
    void ProcessSeqSubmit(void);
    /// Adds a CSeq_entry structure to the scope and executes a macro on it
    /// @param[in] se
    ///     Reference to the CSeq_entry 
    /// @sa
    ///     ExecuteMacro    
    void ProcessSeqEntry(objects::CSeq_entry& se);
    /// Reads a CSeq_entry structure from the input stream
    void ProcessSeqEntry(void);
    /// Reads a CBioseq structure from the input stream
    void ProcessBioseqset(void);
    /// Reads a CBioseq_set structure from the input stream
    void ProcessBioseq(void);

    /// Obtains the parent entry for the sequence with the specified id
    /// @param[in] scope
    ///     Reference to the scope to search 
    /// @param[in] seq_id
    ///     Reference to the Seq_id in question 
    objects::CSeq_entry_Handle GetParentEntry(objects::CScope &scope, const objects::CSeq_id &seq_id);

    /// Executes a macro on CSeq_entry_Handle
    /// @param[in] entry
    ///     Reference to the CSeq_entry_Handle 
    /// @throws CException
    ///     In case the macro has unresolved variables.
    void ExecuteMacro (const objects::CSeq_entry_Handle &entry, objects::CSeq_submit* submit = 0);
    /// Detects the top level ASN.1 element and invokes the proper ProcessXXX method
    /// @throws CException
    ///     In case the top level element can not be determined.
    void ProcessFile (void);
    /// Executes a macro on all files in a folder matching a pattern
    /// @param[in] dir_name
    ///     Reference to a string, holding the folder's path
    /// @param[in] recurse
    ///     Flag, indicating whether to recurse sub-folders
    void ExecuteMacroOnDirectory(const string &dir_name, bool recurse);
    /// Executes a macro on a file
    /// @param[in] fname
    ///     Reference to a string, holding the file's path
    void ExecuteMacroOnFile(const string &fname);
    //void ProcessReleaseFile(const CArgs& args);

    /// Creates a new scope
    /// @return
    ///     Returns a smart pointer to the newly created scope
    CRef<objects::CScope> BuildScope(void);
    /// Resolves path, relative to the program's executable
    /// @param[in,out] sPath
    ///     Reference to the path to be resolved
    void ResolvePath(string& sPath);
    /// Resolves the path to the synonyms.txt file
    /// @param[in,out] sPath
    ///     Reference to the path to be resolved
    void ResolveSynonymPath(string& sPath);

    /// Merges the messages of the exceptions, preceding the current one
    /// @param[in] error
    ///     The exception, who's predecessors messages are to be merged
    string MergeExceptionMessages(const CException &error) const;

private:
    typedef list<CRef<objects::CSeq_id> > TSeqIdsList;

    /// Input stream
    auto_ptr<CObjectIStream>    m_In;
    /// Flag indicatitng if the input is binary
    bool                        m_BinaryInput;
    /// Flag indicatitng if the output is binary
    bool                        m_BinaryOutput;
    /// Number of files, processed in this run
    size_t                      m_NumFiles;
    /// ASN.1 object type
    string                      m_ObjType;
    /// List of macros to execute in this run
    vector< CRef<macro::CMacroRep> >   m_MacrosToExecute;
    /// List of accessions to operate on
    TSeqIdsList                 m_IdentifierList;
    /// Output stream smart pointer
    unique_ptr<CObjectOStream>    m_OutputStream;
    /// Output stream for listing values from the input file
    CNcbiOstream*                 m_ListStream{ nullptr };
    /// Flag to use parallel execution
    bool m_Parallel{ false };
};

END_NCBI_SCOPE;
