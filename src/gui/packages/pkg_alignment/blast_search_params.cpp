/*  $Id: blast_search_params.cpp 43823 2019-09-05 20:13:56Z katargir $
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
 * Authors:  Andrey Yazhuk, Mike DiCuccio
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/blast_search_params.hpp>

#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>

#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objects/seqfeat/Genetic_code.hpp>

#include <algo/blast/api/blast_options.hpp>
#include <algo/blast/api/blast_nucl_options.hpp>
#include <algo/blast/api/blast_prot_options.hpp>
#include <algo/blast/api/blastx_options.hpp>
#include <algo/blast/api/tblastx_options.hpp>
#include <algo/blast/api/tblastn_options.hpp>

#include <algo/blast/blastinput/blastn_args.hpp>
#include <algo/blast/blastinput/blastp_args.hpp>
#include <algo/blast/blastinput/blastx_args.hpp>
#include <algo/blast/blastinput/tblastn_args.hpp>
#include <algo/blast/blastinput/tblastx_args.hpp>

#include <corelib/ncbi_system.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(blast);

static const CBLASTParams::SProgInfo sc_ProgPairs[] = {
    { blast::eBlastn,        "Nucleotide-Nucleotide (BLASTn)",
      true,  true,  false, false, false, true },
    { blast::eMegablast,     "Nucleotide-Nucleotide (MegaBLAST)",
      true,  true,  false, false, false, true },
    { blast::eDiscMegablast, "Nucleotide-Nucleotide (Discontiguous MegaBLAST)",
      true,  true,  false, false, false, true },
    { blast::eBlastp,        "Protein-Protein (BLASTp)",
      false, false, true, false, false, false },
    { blast::eBlastx,        "Translated Nucleotide-Protein (BLASTx)",
      true,  false, true,  true, false, false },
    { blast::eTblastn,       "Protein-Translated Nucleotide (tBLASTn)",
      false, true,  true,  false, true, false },
    { blast::eTblastx,       "Translated Nucleotide-Translated Nucleotide (tBLASTx)",
      true,  true,  true,  true, true, false }
};

CBLASTParams::SProgParams::SProgParams()
	: m_Threshold(0)
	, m_WordSize(0)
	, m_eValue(0.0)
	, m_FilterLowComplex(true)
	, m_QueryGeneticCode(1)
	, m_DbGeneticCode(1)
	, m_MaskLowercase(false)
	, m_MaskRepeats(false)
    , m_WM_TaxId(0)
{
}


CBLASTParams::SProgParams::SProgParams(
	int thresh, int word, double ev,
    bool low_c, int query_gen_code, int db_gen_code
)   
	: m_Threshold(thresh)
	, m_WordSize(word)
	, m_eValue(ev)
	, m_FilterLowComplex(low_c)
	, m_QueryGeneticCode(query_gen_code)
	, m_DbGeneticCode(db_gen_code)
	, m_MaskLowercase(false)
	, m_MaskRepeats(false)
    , m_WM_TaxId(0)
{
}


CBLASTParams::TProgramAttrMap CBLASTParams::sm_ProgramToAttr;

bool CBLASTParams::IsNucInput(blast::EProgram program)
{
    TProgramAttrMap::const_iterator it = sm_ProgramToAttr.find(program);
    if(it != sm_ProgramToAttr.end())    {
        return it->second->m_NucInput;
    } else {
        _ASSERT(false);
        return false;
    }
}


bool CBLASTParams::UseDustFilter(blast::EProgram program)
{
    TProgramAttrMap::const_iterator it = sm_ProgramToAttr.find(program);
    if (it != sm_ProgramToAttr.end())    {
        return it->second->m_UseDustFilter;
    }
    else {
        _ASSERT(false);
        return false;
    }
}

bool CBLASTParams::IsNucDatabase(blast::EProgram program)
{
    TProgramAttrMap::const_iterator it = sm_ProgramToAttr.find(program);
    if(it != sm_ProgramToAttr.end())    {
        return it->second->m_NucDB;
    } else {
        _ASSERT(false);
        return false;
    }
}


bool CBLASTParams::NeedsThreshold(blast::EProgram program)
{
    TProgramAttrMap::const_iterator it = sm_ProgramToAttr.find(program);
    if(it != sm_ProgramToAttr.end())    {
        return it->second->m_NeedsThreshold;
    } else {
        _ASSERT(false);
        return false;
    }
}


bool CBLASTParams::NeedsQueryGeneticCode(blast::EProgram program)
{
    TProgramAttrMap::const_iterator it = sm_ProgramToAttr.find(program);
    if(it != sm_ProgramToAttr.end())    {
        return it->second->m_NeedsQueryGenCode;
    } else {
        _ASSERT(false);
        return false;
    }
}

bool CBLASTParams::NeedsDbGeneticCode(blast::EProgram program)
{
    TProgramAttrMap::const_iterator it = sm_ProgramToAttr.find(program);
    if(it != sm_ProgramToAttr.end())    {
        return it->second->m_NeedsDbGenCode;
    } else {
        _ASSERT(false);
        return false;
    }
}


int CBLASTParams::GetGeneticCodeByLabel(const string& label)
{
    const CGenetic_code_table& code_table = CGen_code_table::GetCodeTable();
    const CGenetic_code_table::Tdata& codes = code_table.Get();
    ITERATE (CGenetic_code_table::Tdata, it, codes) {
        const CGenetic_code& code = **it;
        if (code.GetName() == label) {
            return code.GetId();
        }
    }
    NCBI_THROW(CException, eUnknown,
        "CBLASTParams::GetGeneticCodeByLabel() - invalid argument" + label);
}


string CBLASTParams::GetGeneticCodeLabel(int code_id)
{
    const CGenetic_code_table& code_table = CGen_code_table::GetCodeTable();
    const CGenetic_code_table::Tdata& codes = code_table.Get();
    ITERATE (CGenetic_code_table::Tdata, it, codes) {
        const CGenetic_code& code = **it;
        if (code.GetId() == code_id) {
            return code.GetName();
        }
    }
    NCBI_THROW(CException, eUnknown,
               "CBLASTParams::GetGeneticCodeLabel() - invalid argument: " +
               NStr::IntToString(code_id));
    return "";
}


void CBLASTParams::GetGeneticCodeLabels(vector<string>& labels)
{
    const CGenetic_code_table& code_table = CGen_code_table::GetCodeTable();
    const list<CRef<CGenetic_code> >& codes = code_table.Get();

    ITERATE (list<CRef<CGenetic_code> >, it, codes) {
        string name = (*it)->GetName();
        labels.push_back(name);
    }
}


static CBLASTParams::SProgParams s_DefaultParams(blast::EProgram prg)
{
    switch(prg) {
    case blast::eBlastn :
        return CBLASTParams::SProgParams(-1, 11, 10.0, false, 1, 1);
    case blast::eMegablast :
        return CBLASTParams::SProgParams(-1, 28, 0.05, false, 1, 1);
    case blast::eDiscMegablast :
        return CBLASTParams::SProgParams(-1, 11, 0.05, false, 1, 1);
    case blast::eBlastp :
        return CBLASTParams::SProgParams(11, 3, 10.0, false, 1, 1);
    case blast::eBlastx :
        return CBLASTParams::SProgParams(12, 3, 10.0, false, 1, 1);
    case blast::eTblastn :
        return CBLASTParams::SProgParams(13, 3, 10.0, false, 1, 1);
    case blast::eTblastx :
        return CBLASTParams::SProgParams(13, 3, 10.0, false, 1, 1);
    default:
        return CBLASTParams::SProgParams(13, 3, 10.0, false, 1, 1);
    }
}


CBLASTParams::CBLASTParams()
    : m_NucInput( true )
    , m_NucProgram( blast::eMegablast )
    , m_ProtProgram( blast::eBlastp )
    , mf_Standalone( true )
    , m_SubjectType(eSequences)
    , m_LocalDBLoader(true)
{
    //TODO we may need to synhronize this
    if(sm_ProgramToAttr.empty())    {
        size_t n = sizeof(sc_ProgPairs) / sizeof(SProgInfo);
        for(  size_t i = 0;  i < n; i++)    {
            const SProgInfo& info = sc_ProgPairs[i];
            sm_ProgramToAttr.insert(TProgramAttrMap::value_type(info.m_Program, &info));
        }
    }

    m_ProgToParams[blast::eBlastn] = s_DefaultParams(blast::eBlastn);
    m_ProgToParams[blast::eMegablast] = s_DefaultParams(blast::eMegablast);
    m_ProgToParams[blast::eDiscMegablast] = s_DefaultParams(blast::eDiscMegablast);
    m_ProgToParams[blast::eBlastp] = s_DefaultParams(blast::eBlastp);
    m_ProgToParams[blast::eBlastx] = s_DefaultParams(blast::eBlastx);
    m_ProgToParams[blast::eTblastn] = s_DefaultParams(blast::eTblastn);
    m_ProgToParams[blast::eTblastx] = s_DefaultParams(blast::eTblastx);
}


bool& CBLASTParams::IsNucInput()
{
    return m_NucInput;
}


bool CBLASTParams::IsNucInput() const
{
    return m_NucInput;
}


TConstScopedObjects& CBLASTParams::GetSeqLocs()
{
    return m_SeqLocs;
}


const TConstScopedObjects& CBLASTParams::GetSeqLocs() const
{
    return m_SeqLocs;
}


blast::EProgram& CBLASTParams::GetCurrProgram()
{
    return GetProgram(m_NucInput);
}


const blast::EProgram CBLASTParams::GetCurrProgram() const
{
    return GetProgram(m_NucInput);
}


blast::EProgram& CBLASTParams::GetProgram(bool nuc)
{
    return nuc ? m_NucProgram : m_ProtProgram;
}


const blast::EProgram CBLASTParams::GetProgram(bool nuc) const
{
    return nuc ? m_NucProgram : m_ProtProgram;
}


string& CBLASTParams::GetCurrDatabase()
{
    bool nuc_db = IsNucDatabase(GetCurrProgram());
    return GetDatabase(nuc_db);
}


const string& CBLASTParams::GetCurrDatabase() const
{
    bool nuc_db = IsNucDatabase(GetCurrProgram());
    return GetDatabase(nuc_db);
}


string& CBLASTParams::GetDatabase(bool nuc_db)
{
    return nuc_db ? m_NucDatabase : m_ProtDatabase;
}


const string& CBLASTParams::GetDatabase(bool nuc_db) const
{
    return nuc_db ? m_NucDatabase : m_ProtDatabase;
}


CBLASTParams::TMRUDatabases& CBLASTParams::GetCurrMRUDatabases()
{
    bool nuc_db = IsNucDatabase(GetCurrProgram());
    return GetMRUDatabases(nuc_db);
}


CBLASTParams::TMRUDatabases& CBLASTParams::GetMRUDatabases(bool nuc_db)
{
    return nuc_db ? m_NucMRUDatabases : m_ProtMRUDatabases;
}

typedef SStaticPair<const char*, const char*> TRepeatName;
static TRepeatName sc_RepeatLibs[] = {
    { "Anopheles gambiae",         "repeat/repeat_7165" },
    { "Arabidopsis thaliana",      "repeat/repeat_3702" },
    { "bony fishes",               "repeat/repeat_31032" },
    { "Caenorhabditis briggsae",   "repeat/repeat_6238" },
    { "Caenorhabditis elegans",    "repeat/repeat_6239" },
    { "Chlamydomonas reinhardtii", "repeat/repeat_3055" },
    { "Ciona intestinalis",        "repeat/repeat_7719" },
    { "Danio rerio",               "repeat/repeat_7955" },
    { "Drosophila melanogaster",   "repeat/repeat_7227" },
    { "fungi",                     "repeat/repeat_4751" },
    { "human",                     "repeat/repeat_9606" },
    { "mammals",                   "repeat/repeat_40674" },
    { "Oryza sativa",              "repeat/repeat_4530" },
    { "rodents",                   "repeat/repeat_9989" },
    { "Thalassiosira pseudonana",  "repeat/repeat_35128" }
};

typedef CStaticPairArrayMap<const char*, const char*, PNocase_CStr> TRepeatMap;
DEFINE_STATIC_ARRAY_MAP(TRepeatMap, sc_RepeatMap, sc_RepeatLibs);

CBLASTParams::TRepeatLibs CBLASTParams::GetAllRepeatLibs()
{
    TRepeatLibs libs;
    ITERATE (TRepeatMap, iter, sc_RepeatMap) {
        libs.insert(iter->first);
    }
    return libs;
}

string CBLASTParams::GetRepeatLib(const string& repeat_db)
{
    ITERATE (TRepeatMap, iter, sc_RepeatMap) {
        if (NStr::EqualNocase(iter->second, repeat_db.c_str())) {
            return iter->first;
        }
    }
    return kEmptyStr;
}

string CBLASTParams::GetRepeatDb(const string& repeat_name)
{
    TRepeatMap::const_iterator iter = sc_RepeatMap.find(repeat_name.c_str());
    if (iter != sc_RepeatMap.end()) {
        return iter->second;
    }
    return kEmptyStr;
}

string& CBLASTParams::GetEntrezQuery()
{
    return m_EntrezQuery;
}


string CBLASTParams::GetEntrezQuery() const
{
    return m_EntrezQuery;
}


bool CBLASTParams::IsSubjNucInput() const
{
    return IsNucDatabase(GetCurrProgram());
}


TConstScopedObjects& CBLASTParams::GetSubjSeqLocs()
{
    return m_SubjSeqLocs;
}


const TConstScopedObjects& CBLASTParams::GetSubjSeqLocs() const
{
    return m_SubjSeqLocs;
}


CBLASTParams::SProgParams& CBLASTParams::GetCurrParams()
{
    blast::EProgram prg = GetCurrProgram();
    TProgramParamsMap::iterator it = m_ProgToParams.find(prg);
    _ASSERT(it != m_ProgToParams.end());
    return it->second;
}


const CBLASTParams::SProgParams& CBLASTParams::GetCurrParams() const
{
    CBLASTParams* nc_this = const_cast<CBLASTParams*>(this);
    return nc_this->GetCurrParams();
}


void CBLASTParams::ResetCurrParams()
{
    blast::EProgram prg = GetCurrProgram();
    TProgramParamsMap::iterator it = m_ProgToParams.find(prg);
    _ASSERT(it != m_ProgToParams.end());
    it->second = s_DefaultParams(prg);
}


string& CBLASTParams::GetAdvancedParams()
{
    return m_AdvancedParams;
}


string CBLASTParams::GetAdvancedParams() const
{
    return m_AdvancedParams;
}

string& CBLASTParams::GetJobTitle()
{
    return m_JobTitle;
}

string  CBLASTParams::GetJobTitle() const
{
    return m_JobTitle;
}

static const char* kBLASTParamsTag = "BLAST Params";
static const char* kNucProgTag = "Nuc Program";
static const char* kProtProgTag = "Prot Program";
static const char* kNucDBTag = "Nuc DB";
static const char* kProtDBTag = "Prot DB";
static const char* kNucMRUDBsTag = "Nuc MRU DBs";
static const char* kProtMRUDBsTag = "Prot MRU DBs";
static const char* kEntrezQueryTag = "Entrez Query";
static const char* kAdvParamsTag = "Advanced Params";
static const char* kJobTitleTag = "Job Title";

static const char* kLocalNucDB= "LocalNucDB";
static const char* kLocalProtDB = "LocalProtDB";
static const char* kSubjectType = "SubjectType";
static const char* kLocalDBLoader = "LocalDBLoader";

CUser_object* CBLASTParams::ToUserObject() const
{
    CRef<CUser_object> obj(new CUser_object());
    obj->SetType().SetStr(kBLASTParamsTag);

    // Programs
    string val = GetProgramLabel(m_NucProgram);
    obj->AddField(kNucProgTag, val);
    val = GetProgramLabel(m_ProtProgram);
    obj->AddField(kProtProgTag, val);

    // Databases
    obj->AddField(kNucDBTag, m_NucDatabase);
    obj->AddField(kProtDBTag, m_ProtDatabase);

    vector<string>  databases;
    m_NucMRUDatabases.GetItems(databases);
    obj->AddField(kNucMRUDBsTag, databases);

    databases.clear();
    m_ProtMRUDatabases.GetItems(databases);
    obj->AddField(kProtMRUDBsTag, databases);

    // Queries
    obj->AddField(kEntrezQueryTag, m_EntrezQuery);

    // Parameters
    ITERATE(TProgramParamsMap, it, m_ProgToParams)  {
        EProgram prg = it->first;
        val = GetProgramLabel(prg);
        const SProgParams& params = it->second;
        CUser_object* p_obj = x_ToUserObject(prg, params);
        string name = "Params for " + val;
        obj->AddField(name, *p_obj);
    }

    obj->AddField(kAdvParamsTag, m_AdvancedParams);
    obj->AddField(kJobTitleTag, m_JobTitle);

    obj->AddField(kLocalNucDB, m_LocalNucDB);
    obj->AddField(kLocalProtDB, m_LocalProtDB);
    obj->AddField(kSubjectType, (int)m_SubjectType);
    obj->AddField(kLocalDBLoader, m_LocalDBLoader);

    return obj.Release();
}


void CBLASTParams::FromUserObject(const objects::CUser_object& obj)
{
    string val = obj.GetField(kNucProgTag).GetData().GetStr();
    m_NucProgram = GetProgramByLabel(val);
    val = obj.GetField(kProtProgTag).GetData().GetStr();
    m_ProtProgram = GetProgramByLabel(val);

    // Databases
    m_NucDatabase = obj.GetField(kNucDBTag).GetData().GetStr();
    m_ProtDatabase = obj.GetField(kProtDBTag).GetData().GetStr();

    vector<string>  databases;
    databases.insert(databases.end(),
        obj.GetField(kNucMRUDBsTag).GetData().GetStrs().begin(),
        obj.GetField(kNucMRUDBsTag).GetData().GetStrs().end());
    m_NucMRUDatabases.SetItems(databases);

    databases.clear();
    databases.insert(databases.end(),
        obj.GetField(kProtMRUDBsTag).GetData().GetStrs().begin(),
        obj.GetField(kProtMRUDBsTag).GetData().GetStrs().end());
    m_ProtMRUDatabases.SetItems(databases);

    // Queries
    m_EntrezQuery = obj.GetField(kEntrezQueryTag).GetData().GetStr();

    // Parameters
    NON_CONST_ITERATE(TProgramParamsMap, it, m_ProgToParams)  {
        EProgram prg = it->first;
        val = GetProgramLabel(prg);
        SProgParams& params = it->second;
        string name = "Params for " + val;
        if(obj.HasField(name))  {
            const CUser_object& p_obj = obj.GetField(name).GetData().GetObject();
            x_FromUserObject(p_obj, prg, params);
        }
    }

    m_AdvancedParams = obj.GetField(kAdvParamsTag).GetData().GetStr();
    m_JobTitle = obj.GetField(kJobTitleTag).GetData().GetStr();

    CConstRef<CUser_field> field = obj.GetFieldRef(kLocalNucDB);
    if (field) m_LocalNucDB = field->GetData().GetStr();
    field = obj.GetFieldRef(kLocalProtDB);
    if (field) m_LocalProtDB = field->GetData().GetStr();
    field = obj.GetFieldRef(kSubjectType);
    if (field) {
        switch (field->GetData().GetInt()) {
        case 1:
            m_SubjectType = eNCBIDB;
            break;
        case 2:
            m_SubjectType = eLocalDB;
            break;
        default:
            m_SubjectType = eSequences;
            break;
        }
    }
    field = obj.GetFieldRef(kLocalDBLoader);
    if (field) m_LocalDBLoader = field->GetData().GetBool();
}


static const char* kThresholdTag = "Threshold";
static const char* kWordSizeTag = "WordSize";
static const char* keValueTag = "eValue";
static const char* kFilterLowComplexTag = "FilterLowComplex";
static const char* kGeneticCodeTag = "GeneticCode";
static const char* kDbGeneticCodeTag = "DbGeneticCode";
static const char* kMaskLowercaseTag = "MaskLowercase";
static const char* kMaskRepeatsTag = "MaskRepeats";

/// converts SProgParams into CUser_object
CUser_object* CBLASTParams::x_ToUserObject(EProgram prg,
                                                 const SProgParams& params)
{
    CRef<CUser_object> obj(new CUser_object());
    obj->SetType().SetStr(kBLASTParamsTag);

    if(NeedsThreshold(prg)) {
        obj->AddField(kThresholdTag, params.m_Threshold);
    }
    obj->AddField(kWordSizeTag, params.m_WordSize);
    obj->AddField(keValueTag, params.m_eValue);
    obj->AddField(kFilterLowComplexTag, params.m_FilterLowComplex);
    if(NeedsQueryGeneticCode(prg)) {
        obj->AddField(kGeneticCodeTag, params.m_QueryGeneticCode);
    }
    if(NeedsDbGeneticCode(prg)) {
        obj->AddField(kDbGeneticCodeTag, params.m_DbGeneticCode);
    }

    obj->AddField(kMaskLowercaseTag, params.m_MaskLowercase);
    obj->AddField(kMaskRepeatsTag, params.m_MaskRepeats);
    return obj.Release();
}


/// reads SProgParams fields from CUser_object
void CBLASTParams::x_FromUserObject(const CUser_object& user_obj,
                                    EProgram prg,
                                    CBLASTParams::SProgParams& params)
{
    if(NeedsThreshold(prg)) {
        params.m_Threshold = user_obj.GetField(kThresholdTag).GetData().GetInt();
    }
    params.m_WordSize = user_obj.GetField(kWordSizeTag).GetData().GetInt();
    params.m_eValue = user_obj.GetField(keValueTag).GetData().GetReal();
    params.m_FilterLowComplex = user_obj.GetField(kFilterLowComplexTag).GetData().GetBool();

    if(NeedsQueryGeneticCode(prg)) {
        int code = user_obj.GetField(kGeneticCodeTag).GetData().GetInt();
        /// avoid ridiculous values
        if (code > 0  &&  code < 50) {
            params.m_QueryGeneticCode = code;
        }
    }
    if(NeedsDbGeneticCode(prg) && user_obj.HasField(kDbGeneticCodeTag)) {
        
        int code = user_obj.GetField(kDbGeneticCodeTag).GetData().GetInt();
        /// avoid ridiculous values
        if (code > 0  &&  code < 50) {
            params.m_DbGeneticCode = code;
        }
    }

    params.m_MaskLowercase = user_obj.GetField(kMaskLowercaseTag).GetData().GetBool();
    params.m_MaskRepeats = user_obj.GetField(kMaskRepeatsTag).GetData().GetBool();
}


void CBLASTParams::GetProgramLabels(bool nuc_input, vector<string>& labels)
{
    ITERATE(TProgramAttrMap, it, sm_ProgramToAttr)  {
        const SProgInfo* info = it->second;
        if(info->m_NucInput == nuc_input)   {
            labels.push_back(info->m_Label);
        }
    }
}


string CBLASTParams::GetProgramLabel(blast::EProgram program)
{
    TProgramAttrMap::const_iterator it = sm_ProgramToAttr.find(program);
    if(it != sm_ProgramToAttr.end())    {
        return it->second->m_Label;
    } else {
        _ASSERT(false);
        return kEmptyStr;
    }
}


blast::EProgram CBLASTParams::GetProgramByLabel(const string& label)
{
    ITERATE(TProgramAttrMap, it, sm_ProgramToAttr)  {
        if(it->second->m_Label == label)   {
            return it->first;
        }
    }
    return blast::eBlastNotSet;
}

static void s_NextToken(string::const_iterator& begin,
                        string::const_iterator  end,
                        string::const_iterator& tok_begin,
                        string::const_iterator& tok_end)
{
    static const string s_WhiteSpace(" \n\t\r");

    tok_begin = begin;
    for ( ;  tok_begin != end;  ++tok_begin) {
        if (s_WhiteSpace.find_first_of(*tok_begin) == string::npos) {
            break;
        }
    }

    tok_end = tok_begin;
    if (tok_begin != tok_end  &&
        (*tok_begin == '"'  ||  *tok_begin == '\'')) {
        char quot_balance = *tok_begin;
        for (++tok_begin, ++tok_end;  tok_end != end;  ++tok_end) {
            if (*tok_end == quot_balance) {
                break;
            }
        }
        begin = tok_end;
        ++begin;
    } else {
        for ( ;  tok_end != end;  ++tok_end) {
            if (s_WhiteSpace.find_first_of(*tok_end) != string::npos) {
                break;
            }
        }
        begin = tok_end;
    }
}

CRef<blast::CBlastOptionsHandle> CBLASTParams::ToArgVecBlastOptions( 
    vector<string>& arg_vec, bool local
) const {
    blast::EProgram prg = GetCurrProgram();

    CRef<blast::CBlastOptionsHandle> opts(
        CBlastOptionsFactory::Create( 
            prg, local ? CBlastOptions::eLocal : CBlastOptions::eRemote 
        )
    );
    if( !opts ) return null;

    // program executable name
	switch (prg) {
	case blast::eBlastn:
	case blast::eMegablast:
	case blast::eDiscMegablast:
		arg_vec.push_back(EProgramToTaskName(blast::eBlastn));
		arg_vec.push_back("-" + kTask);
		break;
	default:
		break;
	}
    arg_vec.push_back( EProgramToTaskName( prg ) );

    // arguments setup
    ToArgVec( arg_vec );

    // blast options setup
    CRef<CBlastAppArgs> blast_args;
    switch( prg ){
    case blast::eBlastn:
    case blast::eMegablast:
    case blast::eDiscMegablast:
        blast_args.Reset( new CBlastnAppArgs() );
        break;
    case blast::eBlastp:
        blast_args.Reset( new CBlastpAppArgs() );
        break;
    case blast::eBlastx:
        blast_args.Reset( new CBlastxAppArgs() );
        break;
    case blast::eTblastn:
        blast_args.Reset( new CTblastnAppArgs() );
        break;
    case blast::eTblastx:
        blast_args.Reset( new CTblastxAppArgs() );
        break;
    default:
        NCBI_THROW(
            CException, eUnknown,
            "CBLASTParams::ToBlastOptions() - invalid program " + EProgramToTaskName( prg ) 
        );
    }

    auto_ptr<CArgDescriptions> arg_descs( blast_args->SetCommandLine() );
    auto_ptr<CArgs> args( arg_descs->CreateArgs( arg_vec.size(), arg_vec ) );

    blast_args->SetOptionsHandle( opts );
    opts = blast_args->SetOptions( *args );

    //options.SetGapExtnAlgorithm(eGreedyScoreOnly);
    //options.SetGapTracebackAlgorithm(eGreedyTbck);

    return opts;
}


//TODO may need to move outside (to the tool?)
CRef<blast::CBlastOptionsHandle> CBLASTParams::ToBlastOptions( bool local ) const
{
    blast::EProgram prg = GetCurrProgram();

    CRef<blast::CBlastOptionsHandle> opts(
        CBlastOptionsFactory::Create( prg, local ? CBlastOptions::eLocal : CBlastOptions::eRemote )
    );
    if( !opts ) return null;

    /*
	blast::CBlastOptions& options = opts->SetOptions();
    const SProgParams& params = GetCurrParams();

	// Set up all params in order:
	// int m_Threshold;
	if( NeedsThreshold( prg ) ){
		options.SetWordThreshold( params.m_Threshold );
	}
	// int m_WordSize;
	options.SetWordSize( params.m_WordSize );
	// double m_eValue;
	options.SetEvalueThreshold( params.m_eValue );
	// bool m_FilterLowComplex;
    if( params.m_FilterLowComplex ){
        if( IsNucInput() ){
		    options.SetDustFiltering(); // for nucleotides 
        } else {
		    options.SetSegFiltering();  // for proteins
	    }
    }
	// int m_GeneticCode;
	if( NeedsGeneticCode( prg ) ){
		options.SetDbGeneticCode( params.m_GeneticCode );
	}
	// bool m_MaskLowercase;    // mask Lowercase features
	// bool m_MaskRepeats;      // mask Repeat features
	if( params.m_MaskLowercase || params.m_MaskRepeats ){
		options.SetMaskAtHash();
	}
	// They are also used to convert locations to BlastVec

	// string m_RepeatLib;
	const string& repeat_lib = params.m_RepeatLib;
	if( !repeat_lib.empty() && repeat_lib != "none" ){
		string repeat_db = GetRepeatDb( repeat_lib );
		if( !repeat_db.empty() ){
			options.SetRepeatFilteringDB( repeat_db.c_str() );
		}
	}

    // WM filtering  (only for nuc input (BLASTn & MegaBlast)
    if (params.m_WM_TaxId != 0) {
        if (IsNucInput()) {
            if (!local || !this->m_WM_Dir.empty()) {
                options.SetWindowMaskerTaxId(params.m_WM_TaxId);
            } else {
                ERR_POST(Error << "BLAST window masker tax-id filtering ignored. WindowMasker DB directory not set.");
            }
        } else {
            ERR_POST(Warning << "BLAST window masker tax-id filtering ignored. Incompatible program or input.");
        }
    }


    */

    vector<string> argv;
    argv.push_back( EProgramToTaskName( prg ) );
    ToArgVec( argv );

    /*
	string arg_params = GetAdvancedParams();
	static const string s_whitespace(" \n\t\r");
	
	vector<string> argv;
	argv.push_back( EProgramToTaskName( prg ) );

	NStr::Split( arg_params, s_whitespace, argv );

    // Set up default values for BLAST params.
    // This way is ugly, if you have better idea please implement it.

    vector<string>::iterator argit;
    argit = find( argv.begin(), argv.end(), "-" + kArgNumThreads );
    if( argit == argv.end() ){
        argv.push_back( "-" + kArgNumThreads );
        argv.push_back( NStr::IntToString( GetCpuCount() ) );
    }
    */

	CRef<CBlastAppArgs> blast_args;
	switch( prg ){
	case blast::eBlastn:
	case blast::eMegablast:
	case blast::eDiscMegablast:
		blast_args.Reset( new CBlastnAppArgs() );
		break;
	case blast::eBlastp:
		blast_args.Reset( new CBlastpAppArgs() );
		break;
	case blast::eBlastx:
		blast_args.Reset( new CBlastxAppArgs() );
		break;
	case blast::eTblastn:
		blast_args.Reset( new CTblastnAppArgs() );
		break;
	case blast::eTblastx:
		blast_args.Reset( new CTblastxAppArgs() );
		break;
	default:
		NCBI_THROW(
			CException, eUnknown,
			"CBLASTParams::ToBlastOptions() - invalid program " + EProgramToTaskName( prg ) 
		);
	}

	auto_ptr<CArgDescriptions> arg_descs( blast_args->SetCommandLine() );
	auto_ptr<CArgs> args( arg_descs->CreateArgs( argv.size(), argv ) );

	blast_args->SetOptionsHandle( opts );
	opts = blast_args->SetOptions( *args );

	//options.SetGapExtnAlgorithm(eGreedyScoreOnly);
    //options.SetGapTracebackAlgorithm(eGreedyTbck);

    /// call Validate() - this may throw an exception, and the exception will
    /// be caught by the framework we have done nothing irreversible (yet)
    opts->SetOptions().Validate();

    return opts;
}

vector<string>& CBLASTParams::ToArgVec( vector<string>& arg_vec ) const
{
    blast::EProgram prg = GetCurrProgram();
    const SProgParams& params = GetCurrParams();

    // Set up all params in order:
    // int m_Threshold;
    if( NeedsThreshold( prg ) ){
        arg_vec.push_back( "-" + kArgWordScoreThreshold );
        arg_vec.push_back( NStr::IntToString( params.m_Threshold ) );
    }
    // int m_WordSize;
    arg_vec.push_back( "-" + kArgWordSize );
    arg_vec.push_back( NStr::IntToString( params.m_WordSize ) );
    // double m_eValue;
    arg_vec.push_back( "-" + kArgEvalue );
    arg_vec.push_back( NStr::DoubleToString( params.m_eValue ) );
    // bool m_FilterLowComplex;
    if( params.m_FilterLowComplex ){
        if (UseDustFilter(prg)) {
            arg_vec.push_back( "-" + kArgDustFiltering );
        } else {
            arg_vec.push_back( "-" + kArgSegFiltering );
        }
        arg_vec.push_back( kDfltArgApplyFiltering );
    }
    // int m_GeneticCode;
    if( NeedsQueryGeneticCode( prg ) ){
        arg_vec.push_back( "-" + kArgQueryGeneticCode ); 
        arg_vec.push_back( NStr::IntToString( params.m_QueryGeneticCode ) );
    }

    if( NeedsDbGeneticCode( prg ) ){
        arg_vec.push_back( "-" + kArgDbGeneticCode ); 
        arg_vec.push_back( NStr::IntToString( params.m_DbGeneticCode ) );
    }

    // bool m_MaskLowercase;    // mask Lowercase features
    if( params.m_MaskLowercase ){
        arg_vec.push_back( "-" + kArgUseLCaseMasking );
    }
    // bool m_MaskRepeats;      // mask Repeat features
    if( params.m_MaskRepeats ){
        //???
    }

    // string m_RepeatLib;
    const string& repeat_lib = params.m_RepeatLib;
    if( !repeat_lib.empty() && repeat_lib != "none" ){
        string repeat_db = GetRepeatDb( repeat_lib );
        if( !repeat_db.empty() ){
            arg_vec.push_back( "-" + kArgFilteringDb );
            arg_vec.push_back( repeat_db );
        }
    }

    // WM filtering  (only for nuc input (BLASTn & MegaBlast)
    if( params.m_WM_TaxId != 0 ){
        if( IsNucInput() ){
            const string& wm_dir = GetWmDir();
            if( !wm_dir.empty() ){
                arg_vec.push_back( "-" + kArgWindowMaskerTaxId );
                arg_vec.push_back( NStr::IntToString( params.m_WM_TaxId ) );
            } else {
                ERR_POST(Error << "BLAST window masker tax-id filtering ignored. WindowMasker DB directory not set.");
            }
        } else {
            ERR_POST(Warning << "BLAST window masker tax-id filtering ignored. Incompatible program or input.");
        }
    }    

    string arg_params = GetAdvancedParams();
    static const string s_whitespace(" \n\t\r");

    NStr::Split( arg_params, s_whitespace, arg_vec );

    // Set up default values for BLAST params.
    // This way is ugly, if you have better idea please implement it.
    
    vector<string>::iterator argit;
    argit = find( arg_vec.begin(), arg_vec.end(), "-" + kArgNumThreads );
    if (argit == arg_vec.end()) {
        arg_vec.push_back("-" + kArgNumThreads);
        arg_vec.push_back(NStr::IntToString(CSystemInfo::GetCpuCount()));
    }

    return arg_vec;
}

CArgs* CBLASTParams::ToArgs( vector<string>& arg_vec ) const
{
    ToArgVec( arg_vec );

    blast::EProgram prg = GetCurrProgram();
    
    vector<string> argv;
    argv.push_back( EProgramToTaskName( prg ) );
    argv.insert( argv.end(), arg_vec.begin(), arg_vec.end() );

    CRef<CBlastAppArgs> blast_args;
    switch( prg ){
    case blast::eBlastn:
    case blast::eMegablast:
    case blast::eDiscMegablast:
        blast_args.Reset( new CBlastnAppArgs() );
        break;
    case blast::eBlastp:
        blast_args.Reset( new CBlastpAppArgs() );
        break;
    case blast::eBlastx:
        blast_args.Reset( new CBlastxAppArgs() );
        break;
    case blast::eTblastn:
        blast_args.Reset( new CTblastnAppArgs() );
        break;
    case blast::eTblastx:
        blast_args.Reset( new CTblastxAppArgs() );
        break;
    default:
        NCBI_THROW(
            CException, eUnknown,
            "CBLASTParams::ToBlastOptions() - invalid program " + EProgramToTaskName( prg ) 
        );
    }

    auto_ptr<CArgDescriptions> arg_descs( blast_args->SetCommandLine() );
    auto_ptr<CArgs> args( arg_descs->CreateArgs( argv.size(), argv ) );

    return args.release();
}

void CBLASTParams::x_ArgsToBlastOptions(
	const string& cmd_line, blast::CBlastOptions& opts
) const {
    string::const_iterator begin = cmd_line.begin();
    string::const_iterator end   = cmd_line.end();
    string::const_iterator tok_begin = begin;
    string::const_iterator tok_end = end;

    for (;;) {
        s_NextToken(begin, end, tok_begin, tok_end);
        string arg(tok_begin, tok_end);
        if (arg.empty()) {
            break;
        }

        if( arg[0] != '-' || arg.size() != 2 ){
            string msg( "invalid command-line switch to BLAST: " );
            msg += arg;
            NCBI_THROW( CException, eUnknown, msg );
        }

        s_NextToken( begin, end, tok_begin, tok_end );
        string val(tok_begin, tok_end);
        string err;
        const char* err_postfix = "";

        try {
            switch (arg[1]) {
            /*case 'e':   /// e-value
                err_postfix = "': expected a floating point number";
                opts.SetEvalueThreshold(NStr::StringToDouble(val));
                break;
            */
			/* -- deprecated
            case 'F':   /// filtering options
                err_postfix = "'";
                opts.SetFilterString(val.c_str());
                break;
			*/

            case 'G':   /// gap open cost
                err_postfix = "': expected an integer";
                opts.SetGapOpeningCost(NStr::StringToInt(val));
                break;

            case 'E':   /// gap extension cost
                err_postfix = "': expected an integer";
                opts.SetGapExtensionCost(NStr::StringToInt(val));
                break;

            case 'X':   /// X dropoff value
                err_postfix = "': expected a floating point number";
                opts.SetXDropoff(NStr::StringToDouble(val));
                break;

            case 'q':   /// nucleotide mismatch penalty
                err_postfix = "': expected an integer";
                opts.SetMismatchPenalty(NStr::StringToInt(val));
                break;

            case 'r':   /// nucleotide match reward
                err_postfix = "': expected an integer";
                opts.SetMatchReward(NStr::StringToInt(val));
                break;

            case 'M':   /// matrix to use
                err_postfix = "'";
                opts.SetMatrixName(val.c_str());
                break;

            /*case 'W':  /// word size
                        opts.SetWordSize(NStr::StringToInt(val));
                        msg += "': expected an integer";
                break;*/

            case 'y':   /// X dropoff value for ungapped extension
                err_postfix = "': expected a floating point number";
                opts.SetGapXDropoff(NStr::StringToDouble(val));
                break;

            case 'Z':   /// X dropoff for final gapped alignment
                err_postfix = "': expected a floating point number";
                opts.SetGapXDropoffFinal(NStr::StringToDouble(val));
                break;

            case 'w':   /// frame shift penalty for BLASTx
                err_postfix = "': expected an integer";
                opts.SetFrameShiftPenalty(NStr::StringToInt(val));
                break;

            default:    {{
                err = "parameter not understood: " + arg;
                break;
            }}
            }
        } catch(CException&)  {
            if(err.empty()) {
                string err = "-" + arg + ": invalid parameter: '";
                err += val + string(err_postfix);
            }
            NCBI_THROW(CException, eUnknown, err);
        }
    }
}

void CBLASTParams::PrintParams(const string& log_title) const
{
    blast::EProgram curr_prog = GetCurrProgram();
    const CBLASTParams::SProgParams& prog_params = GetCurrParams();
    
    LOG_POST(Info << log_title <<
             "\n CurrProg=" << GetProgramLabel(curr_prog) <<
             "\n CurrDB=" << GetCurrDatabase() <<
             "\n EntrezQuery=" << GetEntrezQuery() <<
             "\n Threshold=" << prog_params.m_Threshold <<
             "\n WordSize=" << prog_params.m_WordSize <<
             "\n eValue=" << prog_params.m_eValue <<
             "\n FilterLowComplex=" << prog_params.m_FilterLowComplex <<
             "\n Query GeneticCode=" << prog_params.m_QueryGeneticCode <<
             "\n Db GeneticCode=" << prog_params.m_DbGeneticCode <<
             "\n MaskLowercase=" << prog_params.m_MaskLowercase <<
             "\n MaskRepeats=" << prog_params.m_MaskRepeats <<
             "\n RepeatLib=" << prog_params.m_RepeatLib <<
             "\n advanced_params=" << GetAdvancedParams()
             );
}

END_NCBI_SCOPE
