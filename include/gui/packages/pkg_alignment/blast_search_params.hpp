#ifndef PKG_ALIGNMENT___BLAST_SEARCH_PARAMS__HPP
#define PKG_ALIGNMENT___BLAST_SEARCH_PARAMS__HPP

/*  $Id: blast_search_params.hpp 36454 2016-09-26 17:49:22Z katargir $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/utils/mru_list.hpp>

#include <gui/objutils/objects.hpp>

#include <algo/blast/api/blast_types.hpp>
#include <algo/blast/api/blast_options.hpp>
#include <algo/blast/api/blast_options_handle.hpp>


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

 BEGIN_SCOPE(objects)
    class CUser_object;
END_SCOPE(objects)

///////////////////////////////////////////////////////////////////////////////
/// CBLASTParams - save user-specified parameters and preferences
class CBLASTParams
{
public:
    typedef CMRUList<string>    TMRUDatabases;
    typedef CMRUList<string>    TMRUQueries;

    enum ESubjectType
    {
        eSequences,
        eNCBIDB,
        eLocalDB
    };

    /// describes static attributes of a BLAST program
    struct SProgInfo    {
        blast::EProgram m_Program;
        string  m_Label;
        bool    m_NucInput;
        bool    m_NucDB;
        bool    m_NeedsThreshold;
        bool    m_NeedsQueryGenCode;
        bool    m_NeedsDbGenCode;
        bool    m_UseDustFilter;
    };

    /// describes parameters for a BLAST program
    struct SProgParams
    {
        int     m_Threshold;
        int     m_WordSize;
        double  m_eValue;
        bool    m_FilterLowComplex;
        int     m_QueryGeneticCode;
        int     m_DbGeneticCode;
        bool    m_MaskLowercase;    ///< mask Lowercase features
        bool    m_MaskRepeats;      ///< mask Repeat features
        string  m_RepeatLib;
        int     m_WM_TaxId;         ///< winmask tax id

        SProgParams();
        SProgParams(int thresh, int word, double ev, bool low_c, int query_gen_code, int db_gen_code);
    };

public:
    CBLASTParams();

    bool&   IsNucInput();
    bool    IsNucInput() const;

    TConstScopedObjects& GetSeqLocs();
    const TConstScopedObjects&   GetSeqLocs() const;

    /// returns the current program for the currently selected input type
    blast::EProgram& GetCurrProgram();
    const blast::EProgram GetCurrProgram() const;

    /// returns the selected programs for the given input type
    blast::EProgram& GetProgram(bool nuc);
    const blast::EProgram GetProgram(bool nuc) const;

    /// returns the current database for the current program
    string&     GetCurrDatabase();
    const string& GetCurrDatabase() const;

    /// returns the database of the specified type
    string&     GetDatabase(bool nuc_db);
    const string& GetDatabase(bool nuc_db) const;

    /// returns MRU databases for the current program
    TMRUDatabases&  GetCurrMRUDatabases();

    TMRUDatabases&  GetMRUDatabases(bool nuc_db);

    string& GetEntrezQuery();
    string  GetEntrezQuery() const;

    bool    IsSubjNucInput() const;

    TConstScopedObjects& GetSubjSeqLocs();
    const TConstScopedObjects&   GetSubjSeqLocs() const;

    /// returns params for the current program
    SProgParams&    GetCurrParams();
    const SProgParams&    GetCurrParams() const;

    void ResetCurrParams();


    string& GetAdvancedParams();
    string  GetAdvancedParams() const;

    typedef set<string, PNocase> TRepeatLibs;
    static TRepeatLibs GetAllRepeatLibs();
    static string      GetRepeatLib(const string& repeat_db);
    static string      GetRepeatDb(const string& repeat_lib);

    string& GetJobTitle();
    string  GetJobTitle() const;

    bool IsStandaloneRequested() const { return mf_Standalone; }
    void StandaloneRequested( bool req ) { mf_Standalone = req; }

    bool IsCompartmentsRequested() const { return mf_FindComparts; }
    void FindCompartments( bool req ) { mf_FindComparts = req; }

    void SetWM_Dir(const string& wm_dir_path) { m_WM_Dir = wm_dir_path; }
    const string& GetWmDir() const { return m_WM_Dir; }

    vector<string>& ToArgVec( vector<string>& arg_vec ) const;
    CArgs* ToArgs( vector<string>& argv ) const;
    CRef<blast::CBlastOptionsHandle> ToBlastOptions( bool local = false ) const;
    CRef<blast::CBlastOptionsHandle> ToArgVecBlastOptions( vector<string>& arg_vec, bool local = true ) const;

    objects::CUser_object*  ToUserObject() const;
    void    FromUserObject(const objects::CUser_object& user_obj);

    static void     GetProgramLabels(bool nuc_input, vector<string>& labels);
    static string   GetProgramLabel(blast::EProgram program);
    static blast::EProgram  GetProgramByLabel(const string& str);

    static bool     IsNucInput(blast::EProgram program);
    static bool     IsNucDatabase(blast::EProgram program);

    static bool     UseDustFilter(blast::EProgram program);

    static bool     NeedsThreshold(blast::EProgram program);

    static bool     NeedsDbGeneticCode(blast::EProgram program);
    static bool     NeedsQueryGeneticCode(blast::EProgram program);

    static int      GetGeneticCodeByLabel(const string& label);
    static string   GetGeneticCodeLabel(int code_id);
    static void     GetGeneticCodeLabels(vector<string>& labels);


    /// LOG_POST params for forensic purposes
    void PrintParams(const string& log_title) const;

    string          GetLocalNucDB() const { return m_LocalNucDB; }
    void            SetLocalNucDB(const string& value) { m_LocalNucDB = value; }

    string          GetLocalProtDB() const { return m_LocalProtDB; }
    void            SetLocalProtDB(const string& value) { m_LocalProtDB = value; }

    ESubjectType    GetSubjectType() const { return m_SubjectType;  }
    void            SetSubjectType(ESubjectType subjectType) { m_SubjectType = subjectType; }

    bool            GetLocalDBLoader() const { return m_LocalDBLoader; }
    void            SetLocalDBLoader(bool value) { m_LocalDBLoader = value; }

protected:
    void    x_AddProgramParams(blast::EProgram program, int thresh, int word, int ev);

    void    x_ArgsToBlastOptions(const string& cmd_line, blast::CBlastOptions& opts) const;

    static objects::CUser_object*  x_ToUserObject(blast::EProgram prg,
                                                  const SProgParams& params);
    static void    x_FromUserObject(const objects::CUser_object& user_obj,
                                    blast::EProgram prg,
                                    SProgParams& params);

protected:
    typedef map<blast::EProgram, const SProgInfo*>     TProgramAttrMap;
    typedef map<blast::EProgram, SProgParams>   TProgramParamsMap;

    static TProgramAttrMap sm_ProgramToAttr;

    bool    m_NucInput;
    TConstScopedObjects m_SeqLocs;

    blast::EProgram m_NucProgram;
    blast::EProgram m_ProtProgram;

    string  m_NucDatabase;
    string  m_ProtDatabase;
    TMRUDatabases   m_NucMRUDatabases;
    TMRUDatabases   m_ProtMRUDatabases;

    /// Params applying only to BLAST Search
    string      m_EntrezQuery;
    TMRUQueries m_MRUQueries;

    /// Params applying only to BLAST Sequences
    TConstScopedObjects m_SubjSeqLocs;

    /// Common parameters
    TProgramParamsMap   m_ProgToParams;

    string  m_AdvancedParams;
    string  m_JobTitle;
    bool mf_FindComparts;
    bool mf_Standalone;
    string m_WM_Dir; ///< path to WM statistics databases

    string m_LocalNucDB;
    string m_LocalProtDB;

    ESubjectType m_SubjectType;
    bool m_LocalDBLoader;
};


/* @} */

END_NCBI_SCOPE;


#endif  // PKG_ALIGNMENT___BLAST_SEARCH_PARAMS__HPP

