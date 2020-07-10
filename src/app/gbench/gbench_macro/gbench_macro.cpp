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
* Authors:  Vladislav Evgeniev, Jonathan Kans, Clifford Clausen, Aaron Ucko
*
* File Description:
*    Command-line tool to run macros on annotated biological sequence
*    data represented according to NCBI's ASN.1 specifications.
*
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbienv.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbitime.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>

#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <sra/data_loaders/snp/snploader.hpp>
#include <sra/data_loaders/vdbgraph/vdbgraphloader.hpp>
#include <sra/data_loaders/wgs/wgsloader.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/objectio.hpp>

#include <gui/objutils/macro_engine.hpp>
#include <gui/objutils/macro_engine_parallel.hpp>
#include <gui/objutils/macro_lib.hpp>
#include <gui/objutils/usage_report_job.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/thread_pool_engine.hpp>

#include <objects/submit/Seq_submit.hpp>

#include <connect/ncbi_core_cxx.hpp>
#include <connect/ncbi_util.h>

#include <util/format_guess.hpp>

#include "gbench_macro.hpp"


USING_NCBI_SCOPE;
USING_SCOPE(objects);

#define PRINT_ASN1(asn) cerr << MSerial_AsnText << (asn) << endl;

CGBenchMacroApplication::CGBenchMacroApplication() :
    m_BinaryInput(false),
    m_BinaryOutput(false),
    m_NumFiles(0)
{
    SetVersion(CVersionInfo(0,1,0));
}

CGBenchMacroApplication::~CGBenchMacroApplication()
{
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    disp.ShutDown();
    CAppJobDispatcher::ReleaseInstance();
}

void CGBenchMacroApplication::Init(void)
{
    //SetDiagTrace(eDT_Enable);
    SetDiagPostFlag(eDPF_All);
    SetDiagPostLevel(eDiag_Info);

    // Prepare command line descriptions

    // Create
    auto arg_desc = make_unique<CArgDescriptions>();

    HideStdArgs(fHideFullVersion | fHideXmlHelp);

    arg_desc->AddOptionalKey
        ("p", "Directory", "Path to a folder, containing the input ASN.1 files",
        CArgDescriptions::eInputFile);

    arg_desc->AddOptionalKey
        ("i", "InFile", "Path to the input file",
        CArgDescriptions::eInputFile);

    arg_desc->SetDependency("i",CArgDescriptions::eExcludes,"p"); 

    arg_desc->AddOptionalKey(
        "d", "Directory", "Path to a folder, where the output ASN.1 files will be stored",
        CArgDescriptions::eOutputFile);

    arg_desc->AddOptionalKey(
        "o", "OutFile", "Path to the output ASN.1 file",
        CArgDescriptions::eOutputFile);

    arg_desc->SetDependency("o",CArgDescriptions::eExcludes,"d"); 

    arg_desc->SetDependency("p",CArgDescriptions::eExcludes,"o"); 

    arg_desc->AddFlag("r", "Recurse sub-directories");

    arg_desc->AddFlag("read_only", "Do not assign default output files");

    arg_desc->SetDependency("o", CArgDescriptions::eExcludes, "read_only");

    arg_desc->SetDependency("d", CArgDescriptions::eExcludes, "read_only");

    arg_desc->AddDefaultKey(
        "a", "a", "ASN.1 Type (a Automatic, e Seq-entry, b Bioseq, s Bioseq-set, m Seq-submit)",
        CArgDescriptions::eString, "a");

    arg_desc->AddFlag("b", "Input is in binary format");

    arg_desc->AddFlag("f", "Output file(s) in binary format");

    arg_desc->AddDefaultKey(
        "m", "InFile", "Macro (library) file",
        CArgDescriptions::eInputFile, "<std>/etc/macro_scripts/list_of_macros.mql");

    arg_desc->AddOptionalKey(
        "e","","List of comma-separated macros to execute or all",
        CArgDescriptions::eString);

    arg_desc->AddDefaultKey(
        "y", "InFile", "Synonyms file",
        CArgDescriptions::eInputFile, "<std>/etc/synonyms.txt");

    arg_desc->AddOptionalKey(
        "w", "SuspectProductFile", "Suspect product rule file name",
        CArgDescriptions::eInputFile);

    arg_desc->AddDefaultKey(
        "s", "String", "File suffix for output files", 
        CArgDescriptions::eString, ".processed");

    arg_desc->AddDefaultKey(
        "x", "String", "File extension for input files with -p", 
        CArgDescriptions::eString, ".asn");

    arg_desc->AddOptionalKey(
        "l", "", "List of comma-separated accessions or local IDs to operate on.",
        CArgDescriptions::eString);

    arg_desc->AddOptionalKey(
        "print", "OutFile", "Path to the output file where values from the input file will be listed",
        CArgDescriptions::eOutputFile);

    arg_desc->SetDependency("p", CArgDescriptions::eExcludes, "print");

    arg_desc->AddFlag("load_NA", "Load named annotations");

    arg_desc->AddFlag("parallel", "Set this flag to execute the macro using multiple threads");

    // Program description
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
        "Command-line tool to run macros on annotated biological sequence\ndata represented according to NCBI's ASN.1 specifications.", false);

    // Pass argument descriptions to the application
    SetupArgDescriptions(arg_desc.release());

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    // register the default Thread Pool engine for executing background UI tasks
    CThreadPoolEngine* engine = new CThreadPoolEngine(1, 10);
    disp.RegisterEngine("ThreadPool", *engine);
}


int CGBenchMacroApplication::Run(void)
{
    const CArgs& args = GetArgs();
    if (Setup(args))
        return 1;

    bool exceptionCaught(false);
    try {
        if ( args["p"] ) {
            ExecuteMacroOnDirectory (args["p"].AsString(), args["r"]);
            REPORT_USAGE("macro", .Add("dialog_name", "GbenchMacro")
                                  .Add("action_name", "execute"));
        } else {
            if (args["i"]) {
                ExecuteMacroOnFile (args["i"].AsString());
                REPORT_USAGE("macro", .Add("dialog_name", "GbenchMacro")
                                      .Add("action_name", "execute"));
            }
        }
    } catch (const CException&) {
        exceptionCaught = true;
    }

    if (!exceptionCaught) {
        LOG_POST(Info << "===============================================================================");
        LOG_POST(Info << "Processed " << m_NumFiles << " file(s)");
    }

    return (exceptionCaught ? 1 : 0);
}

int CGBenchMacroApplication::DryRun(void)
{
    const CArgs& args = GetArgs();
    return Setup(args);
}

int CGBenchMacroApplication::Setup(const CArgs& args)
{
    CORE_SetLOG(LOG_cxx2c());

    if (!CNcbiApplication::HasLoadedConfig()) {
        string config_path;
        CDirEntry::SplitPath(GetProgramExecutablePath(), &config_path);
        config_path = CDirEntry::NormalizePath(config_path + "../etc/gbenchmacro.ini");

        if (CFile(config_path).Exists()) {
            CNcbiApplication::LoadConfig(GetConfig(), &config_path);
        }
    }

#ifdef _DEBUG
    LOG_POST(Info << "===============================================================================");

    CTime startTime(CTime::eCurrent);
    
    if (IsDryRun()) 
        LOG_POST(Info << "Dry-run from " << startTime.AsString());
    else
        LOG_POST(Info << "Run from " << startTime.AsString());

    LOG_POST(Info << "===============================================================================");
    LOG_POST(Info << "Input Arguments:");
    string argsList;
    args.Print(argsList);
    LOG_POST(Info << argsList);
#endif

    m_BinaryInput = args["b"];
    m_BinaryOutput = args["f"];

    m_ObjType = args["a"].AsString();

    if (!args["i"] && !args["p"]) {
        string message;
        GetArgDescriptions()->PrintUsage(message, true);
        LOG_POST(Error << "No input specified. Please use either -i or -p.");
        LOG_POST(Error << message);
        return 1;
    }

    macro::CMacroLib& macroLib = macro::CMacroLib::GetInstance();

    // Load the macro library file
    string sMacroLib = args["m"].AsString();
    ResolvePath(sMacroLib);
    macroLib.ClearLibrary();
    if (!macroLib.AppendToLibrary(sMacroLib)) {
        LOG_POST(Error << "Failed to parse macro library: " << sMacroLib);
        LOG_POST(Error << macroLib.GetLastError());
        return 1;
    }

    // Load the synonyms file
    string sSynonymsList = args["y"].AsString();
    ResolveSynonymPath(sSynonymsList);
    if (!macroLib.StoreSynonymList(sSynonymsList)) {
        LOG_POST(Error << "Failed to load synonyms list: " << sSynonymsList);
        LOG_POST(Error << macroLib.GetLastError());
        return 1;
    }

    if (args["w"]) {
        macroLib.SetSuspectRules(args["w"].AsString());
    }

    if ((macroLib.GetMacros().size() > 1) && !args["e"]) { // If the library contains multiple macros, the -e parameter is required
        LOG_POST(Error << "The input macro library contains several macros (" << macroLib.GetMacros().size() << ").\nPlease use the -e parameter to choose which macros to execute.");
        return 1;
    }

    if (args["e"] && (0 != NStr::CompareNocase(args["e"].AsString(),"all"))) {
            vector<string> macros_names;
            NStr::Split(args["e"].AsString(), ",", macros_names, NStr::fSplit_Tokenize);
            for (const auto& name : macros_names) {
                // Check if the macro is defined
                macro::CMacroRep *macroRep = macroLib.GetMacroRep(name);
                if (!macroRep) {
                    LOG_POST(Error << "Unknown macro " << name);
                    return 1;
                }
                m_MacrosToExecute.push_back(CRef<macro::CMacroRep>(macroRep));
            }
    }
    else {
        for (auto&& it : macroLib.GetMacros()) {
            m_MacrosToExecute.push_back(it);
        }
    }
   
    // Parse the list of IDs (accessions or local IDs) filter
    if (args["l"]) {
        list<string> IDs;
        NStr::Split(args["l"].AsString(), ",", IDs, NStr::fSplit_Tokenize);
        for (auto& id_it : IDs) {
            NStr::TruncateSpacesInPlace(id_it);
            try {
                m_IdentifierList.push_back(CRef<CSeq_id>(new CSeq_id(id_it, CSeq_id::fParse_Default | CSeq_id::fParse_AnyLocal)));
            }
            catch (const CException& ex)
            {
                LOG_POST(Error << "\"" << id_it << "\" is not a valid ID:");
                LOG_POST(Error << ex);
                return 1;
            }
        }
    }

    if (args["parallel"]) {
        m_Parallel = true;
    }

    return 0;
}

auto_ptr<CObjectIStream> CGBenchMacroApplication::OpenFile(const string& fname)
{
    auto_ptr<CNcbiIstream> InputStream(new CNcbiIfstream (fname.c_str(), ios::binary));

    CFormatGuess::EFormat format = CFormatGuess::Format(*InputStream);
    switch (format)
    {
    case CFormatGuess::eBinaryASN:
    case CFormatGuess::eTextASN:
        break;
    default:
        format = m_BinaryInput ? CFormatGuess::eBinaryASN : CFormatGuess::eTextASN;
        break;
    }

    auto_ptr<CObjectIStream> objectStream;
    objectStream.reset(CObjectIStream::Open( CFormatGuess::eBinaryASN == format ? eSerial_AsnBinary : eSerial_AsnText, *InputStream, eTakeOwnership));
    InputStream.release();
    return objectStream;
}

CSeq_entry_Handle CGBenchMacroApplication::GetParentEntry(CScope &scope, const CSeq_id &seq_id)
{
    CSeq_entry_Handle result;

    CBioseq_Handle bsh = scope.GetBioseqHandle(seq_id);
    if (!bsh)
        return result;

    CBioseq_set_Handle bssh = bsh.GetParentBioseq_set();
    if (bssh &&
        bssh.IsSetClass() &&
        bssh.GetClass() == CBioseq_set::eClass_nuc_prot) {
        result = bssh.GetParentEntry();
    } else {
        result = bsh.GetParentEntry();
    }

    return result;
}

void CGBenchMacroApplication::ProcessSeqSubmit(void)
{
    CRef<CSeq_submit> ss(new CSeq_submit);

    // Get seq-submit
    m_In->Read(ObjectInfo(*ss), CObjectIStream::eNoFileHeader);

    if (!ss->GetData().IsEntrys()) 
        NCBI_THROW(CException, eUnknown, "CSeq_submit contains no sequences.");

    CRef<CScope> scope = BuildScope();

    NON_CONST_ITERATE(CSeq_submit::TData::TEntrys, se, ss->SetData().SetEntrys()) {
        CSeq_entry_Handle hSeqEntry = scope->AddTopLevelSeqEntry(**se);
        if (!m_IdentifierList.empty())
            continue;
        // Execute the macro(s) on the root Seq_entry
        ExecuteMacro(hSeqEntry, ss);
    }

    if (!m_IdentifierList.empty()) {
        // Execute the macro(s) on the Seq_entry(s) from the identifier list
        ITERATE(TSeqIdsList, it, m_IdentifierList) {
            ExecuteMacro(GetParentEntry(*scope, *(*it)));
        }
    }

    if (m_OutputStream) {
        m_OutputStream->Write(ss, ss->GetThisTypeInfo());
    }
}

void CGBenchMacroApplication::ProcessSeqEntry(CSeq_entry& se)
{
    CRef<CScope> scope = BuildScope();
    CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(se);
    if (m_IdentifierList.empty()) {
        // Execute the macro(s) on the root Seq_entry
        ExecuteMacro(seh);
    }
    else { 
        // Execute the macro(s) on the Seq_entry(s) from the identifier list
        ITERATE(TSeqIdsList, it, m_IdentifierList) {
            ExecuteMacro(GetParentEntry(*scope, *(*it)));
        }
    }
}

void CGBenchMacroApplication::ProcessSeqEntry(void) 
{
    CRef<CSeq_entry> se(new CSeq_entry);
    m_In->Read(ObjectInfo(*se), CObjectIStream::eNoFileHeader);
    ProcessSeqEntry(*se);
    if (m_OutputStream) {
        m_OutputStream->Write(se, se->GetThisTypeInfo());
    }
}

void CGBenchMacroApplication::ProcessBioseq(void)
{
    // Get seq-entry
    CRef<CSeq_entry> se(new CSeq_entry);
    CBioseq& bioseq = se->SetSeq();
    m_In->Read(ObjectInfo(bioseq), CObjectIStream::eNoFileHeader);
    ProcessSeqEntry(*se);
    if (m_OutputStream) {
        m_OutputStream->Write(&bioseq, bioseq.GetThisTypeInfo());
    }
}

void CGBenchMacroApplication::ProcessBioseqset(void)
{
    // Get seq-entry
    CRef<CSeq_entry> se(new CSeq_entry);
    CBioseq_set& bioseqset = se->SetSet();
    m_In->Read(ObjectInfo(bioseqset), CObjectIStream::eNoFileHeader);
    ProcessSeqEntry(*se);
    if (m_OutputStream) {
        m_OutputStream->Write(&bioseqset, bioseqset.GetThisTypeInfo());
    }
}

void CGBenchMacroApplication::ExecuteMacro (const CSeq_entry_Handle &entry, CSeq_submit* submit)
{
    if (!m_Parallel) {
        macro::CMacroEngine macroEngine;
        CRef<CMacroCmdComposite> macro_cmd(new CMacroCmdComposite("Executing various macros"));

        string report_all;
        for (auto& macro : m_MacrosToExecute) {
            macro::CMacroBioData bioData(entry, ConstRef(submit));
            macroEngine.Exec(*macro, bioData, macro_cmd, true, m_ListStream);

            const macro::CMacroStat::CMacroLog& report = macroEngine.GetStatistics().GetMacroReport();
            if (!report.GetLog().empty()) {
                report_all += report.GetName() + ":\n" + report.GetLog() + "\n";
            }
        }
        if (report_all.empty()) {
            report_all = "Macro made no changes";
        }

        LOG_POST(Info << report_all);
    }
    else {
        macro::CMacroEngineParallel macroEnginePar;
        CRef<CMacroCmdComposite> macro_cmd(new CMacroCmdComposite("Executing various macros"));

        string report_all;
        for (auto& macro : m_MacrosToExecute) {
            macro::CMacroBioData bioData(entry, ConstRef(submit));
            macroEnginePar.Exec(*macro, bioData, macro_cmd, true, m_ListStream);
            
            const macro::CMacroStat::CMacroLog& report = macroEnginePar.GetStatistics().GetMacroReport();
            if (!report.GetLog().empty()) {
                report_all += report.GetName() + ":\n" + report.GetLog() + "\n";
            }
        }
        if (report_all.empty()) {
            report_all = "Macro made no changes";
        }

        LOG_POST(Info << report_all);

    }
}

void CGBenchMacroApplication::ProcessFile (void)
{
    // Process file based on its content
    // ASN.1 Type (a Automatic, e Seq-entry, b Bioseq, s Bioseq-set, m Seq-submit",
    bool fGuessFormat(false);
    string header = m_In->ReadFileHeader();
    if (header.empty() && !m_ObjType.empty())
    {
        switch (m_ObjType[0])
        {
        case 'e':
            header = "Seq-entry";
            break;
        case 'm':
            header = "Seq-submit";
            break;
        case 's':
            header = "Bioseq-set";
            break;
        case 'b':
            header = "Bioseq";
            break;
        case 'a':
            if (eSerial_AsnBinary == m_In->GetDataFormat())
                fGuessFormat = true;
            else
                NCBI_THROW(CException, eUnknown, "The format of the input file cannot be detected.");
            break;
        default:
            NCBI_THROW(CException, eUnknown, "Unknown parameter -a value " + m_ObjType);
            break;
        }
    }

    if (!fGuessFormat) {
        if ( "Seq-submit" == header )			// Seq-submit
            ProcessSeqSubmit();
        else if ( "Seq-entry" == header )		// Seq-entry
            ProcessSeqEntry();
        else if ( "Bioseq-set" == header )		// Bioseq-set
            ProcessBioseqset();
        else if ( "Bioseq" == header )			// Bioseq
            ProcessBioseq();
        else 
            NCBI_THROW(CException, eUnknown, "Unhandled ASN.1 type " + header);
    }
    else {
        do {
            LOG_POST(Info << "Detecting top-level element:");
            try {
                LOG_POST(Info << "Trying to parse the file as Seq-entry...");
                ProcessSeqEntry();
                break;
            }
            catch (const CException &e)
            {
                LOG_POST(Info << "FAILED:");
                LOG_POST(Info << e);
            }
            m_In->SetStreamPos(0);
            try {
                LOG_POST(Info << "Trying to parse the file as Seq-submit...");
                ProcessSeqSubmit();
                break;
            }
            catch (const CException &e)
            {
                LOG_POST(Info << "FAILED:");
                LOG_POST(Info << e);
            }
            m_In->SetStreamPos(0);
            try {
                LOG_POST(Info << "Trying to parse the file as Bioseq-set...");
                ProcessBioseqset();
                break;
            }
            catch (const CException &e)
            {
                LOG_POST(Info << "FAILED:");
                LOG_POST(Info << e);
            }
            m_In->SetStreamPos(0);
            try {
                LOG_POST(Info << "Trying to parse the file as Bioseq...");
                ProcessBioseq();
                break;
            }
            catch (const CException &e)
            {
                LOG_POST(Info << "FAILED:");
                LOG_POST(Info << e);
            }
            NCBI_THROW(CException, eUnknown, "Unhandled ASN.1 type");
        }
        while (false);
    }
}

void CGBenchMacroApplication::ExecuteMacroOnDirectory(const string &dir_name, bool recurse)
{
    const CArgs& args = GetArgs();

    CDir dir(dir_name);

    string suffix = ".ent";
    if (args["x"]) {
        suffix = args["x"].AsString();
    }
    string mask = "*" + suffix;

    CDir::TEntries files (dir.GetEntries(mask, CDir::eFile));
    ITERATE(CDir::TEntries, ii, files) {
        string fname = (*ii)->GetName();
        if ((*ii)->IsFile()) {
            string fname = CDirEntry::MakePath(dir_name, (*ii)->GetName());
            ExecuteMacroOnFile (fname);
        }
    }
    if (recurse) {
        CDir::TEntries subdirs (dir.GetEntries("", CDir::eDir));
        ITERATE(CDir::TEntries, ii, subdirs) {
            string subdir = (*ii)->GetName();
            if ((*ii)->IsDir() && !NStr::Equal(subdir, ".") && !NStr::Equal(subdir, "..")) {
                string subname = CDirEntry::MakePath(dir_name, (*ii)->GetName());
                ExecuteMacroOnDirectory (subname, recurse);
            }
        }
    }
}

void CGBenchMacroApplication::ExecuteMacroOnFile(const string &fname)
{
    const CArgs& args = GetArgs();

    LOG_POST(Info << "===============================================================================");
    LOG_POST(Info << "Processing file: " << fname);

    do {
        string outputFile;

        if (args["print"]) {
            try {
                m_ListStream = &(args["print"].AsOutputFile());
                m_ListStream->exceptions(ios::failbit | ios::badbit);
            }
            catch (const CException& e) {
                ERR_POST("Failed to open output file for listing values");
                ERR_POST(e);
                throw;
            }
        }
        
        if (!args["read_only"]) {
            if (args["o"]) {
                outputFile = args["o"].AsString();
            }
            else if (args["d"]) {
                string base;
                string ext;
                CDirEntry::SplitPath(fname, 0, &base, &ext);
                base += ext;
                outputFile = CDirEntry::MakePath(args["d"].AsString(), base, args["s"].AsString());
            }
            else {
                outputFile = fname;
                outputFile += args["s"].AsString();
            }

            try {
                m_OutputStream.reset(CObjectOStream::Open(outputFile, m_BinaryOutput ? eSerial_AsnBinary : eSerial_AsnText));
            }
            catch (const CException &e)
            {
                ERR_POST("Failed to open output file (" << outputFile << ")");
                ERR_POST(e);
                throw;
            }
        }

        try {
            m_In = OpenFile(fname);
            ProcessFile();
        } 
        catch (const CException &e) {
            string all_messages = MergeExceptionMessages(e);
            ERR_POST("FAILED to process file (" << fname <<"):\n" << all_messages.c_str());          
            ERR_POST(e.ReportAll());
            throw;
        }
        LOG_POST(Info << "SUCCESS, processing file: " << fname);
        LOG_POST(Info << "Output file: " << outputFile);
        m_NumFiles++;
    }
    while (false);

    m_In.reset();
}

CRef<CScope> CGBenchMacroApplication::BuildScope (void)
{
    auto objmgr = CObjectManager::GetInstance();
    CGBDataLoader::RegisterInObjectManager(*objmgr);
    
    const CArgs& args = GetArgs();
    if (args["load_NA"]) {
        // registering adds ~70ms to the execution time - May 2020
        CVDBGraphDataLoader::RegisterInObjectManager(*objmgr, CObjectManager::eDefault, 88);
        CWGSDataLoader::RegisterInObjectManager(*objmgr, CObjectManager::eDefault, 88);
        CSNPDataLoader::RegisterInObjectManager(*objmgr, CObjectManager::eDefault, 88);
    }

    CRef<CScope> scope(new CScope (*objmgr));
    scope->AddDefaults();
    return scope;
}

void CGBenchMacroApplication::ResolvePath(string& sPath)
{
    if (string::npos == sPath.find("<std>"))
        return;

    sPath = sPath.substr(5);

    string sDir;
    CDirEntry::SplitPath(GetProgramExecutablePath(),&sDir);
    sPath = CDirEntry::NormalizePath(sDir+".."+sPath);
}

static const char* kSynonyms = "synonyms";
static const char* kExtension = "txt";
void CGBenchMacroApplication::ResolveSynonymPath(string& sPath)
{
    if (string::npos == sPath.find("<std>")) {
        // this option was specified at the command line, so we use that
        return;
    }

    // first, look in the user's home directory in .gbenchmacro/ hidden directory
    const string usergbDir = CDirEntry::ConcatPathEx(CDir::GetHome(), ".gbenchmacro");
    const string userPath = CDirEntry::MakePath(usergbDir, kSynonyms, kExtension);
    if (CFile(userPath).Exists()) {
        sPath = userPath;
        return;
    }

    // second, look near the application
    string appDir;
    CDirEntry::SplitPath(GetProgramExecutablePath(), &appDir);
    const string appPath = CDirEntry::MakePath(appDir, kSynonyms, kExtension);
    if (CFile(appPath).Exists()) {
        sPath = appPath;
        return;
    }

    // fall back on system wide settings
    sPath = sPath.substr(5);
    sPath = CDirEntry::NormalizePath(appDir + ".." + sPath);
}

string CGBenchMacroApplication::MergeExceptionMessages(const CException &error) const
{
    string result;
    const CException *current = &error;
    while (nullptr != current) {
        result += current->GetMsg();
        current = current->GetPredecessor();
        if (current)
            result += '\n';
    }
    return result;
}

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function; change argument list to
    // (argc, argv, 0, eDS_Default, 0) if there's no point in trying
    // to look for an application-specific configuration file.
    return CGBenchMacroApplication().AppMain(argc, argv);
}
