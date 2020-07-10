/*  $Id: blastdb_aliastool.cpp 593112 2019-09-12 12:56:14Z fongah2 $
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
 * Author: Christiam Camacho
 *
 */

/** @file blastdb_aliastool.cpp
 * Command line tool to create BLAST database aliases and associated files. 
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbistre.hpp>
#include <algo/blast/api/version.hpp>
#include <objtools/blast/seqdb_reader/seqdbcommon.hpp>
#include <objtools/blast/seqdb_reader/impl/seqdbgeneral.hpp>
#include <objtools/blast/seqdb_writer/writedb.hpp>
#include <objtools/blast/seqdb_writer/writedb_error.hpp>
#include <objtools/blast/seqdb_writer/seqidlist_writer.hpp>
#include <objtools/blast/seqdb_reader/seqidlist_reader.hpp>

#include <algo/blast/blastinput/blast_input.hpp>
#include "../blast/blast_app_util.hpp"

#ifndef SKIP_DOXYGEN_PROCESSING
USING_NCBI_SCOPE;
USING_SCOPE(blast);
#endif

/// The main application class
class CBlastDBAliasApp : public CNcbiApplication
{
public:
    /** @inheritDoc */
    CBlastDBAliasApp() {
        CRef<CVersion> version(new CVersion());
        version->SetVersionInfo(new CBlastVersion());
        SetFullVersion(version);
    }
private:
    /** @inheritDoc */
    virtual void Init();
    /** @inheritDoc */
    virtual int Run();

    /// Converts gi files from binary to text format
    /// @param input Input stream with text file [in]
    /// @param output Output stream where converted binary gi list will be
    /// written [out]
    /// @return 0 on success
    int ConvertGiFile(CNcbiIstream& input, CNcbiOstream& output,
                      const string* input_fname = NULL,
                      const string* output_fname = NULL) const;
    /// Invokes function to create an alias file with the arguments provided on
    /// the command line
    void CreateAliasFile() const;

    int x_ConvertSeqIDFile() const;
    void x_SeqIDFileInfo() const;

    /// Documentation for this program
    static const char * const DOCUMENTATION;

    /// Describes the modes of operation of this application
    enum EOperationMode {
        eCreateAlias,       ///< Create alias files
        eConvertGiFile,     ///< Convert gi files from text to binary format
        eConvertSeqIDFile,  ///< Convert text seqidlist files from proprietory binary format
        eSeqIDFileInfo      ///< Display info about seqidlist file
    };

    /// Determine what mode of operation is being used
    EOperationMode x_GetOperationMode() const {
        EOperationMode retval = eCreateAlias;
        if (GetArgs()["gi_file_in"].HasValue()) {
            retval = eConvertGiFile;
        }
        if (GetArgs()["seqid_file_in"].HasValue()) {
            retval = eConvertSeqIDFile;
        }
        if (GetArgs()["seqid_file_info"].HasValue()) {
            retval = eSeqIDFileInfo;
        }
        return retval;
    }
    vector<string> x_GetDbsToAggregate(const string dbs, const string file) const;
    void x_AddVDBsToAliasFile( string filename, bool append, string title = kEmptyStr) const;
};

const char * const CBlastDBAliasApp::DOCUMENTATION = "\n\n"
"This application has three modes of operation:\n\n"
"1) GI file conversion:\n"
"   Converts a text file containing GIs (one per line) to a more efficient\n"
"   binary format. This can be provided as an argument to the -gilist option\n"
"   of the BLAST search command line binaries or to the -gilist option of\n"
"   this program to create an alias file for a BLAST database (see below).\n\n"
"2) Alias file creation (restricting with GI List or Sequence ID List):\n"
"   Creates an alias for a BLAST database and a GI or ID list which\n"
"   restricts this database. This is useful if one often searches a subset\n"
"   of a database (e.g., based on organism or a curated list). The alias\n"
"   file makes the search appear as if one were searching a regular BLAST\n"
"   database rather than the subset of one.\n\n"
"3) Alias file creation (aggregating BLAST databases):\n"
"   Creates an alias for multiple BLAST databases. All databases must be of\n"
"   the same molecule type (no validation is done). The relevant options are\n" 
"   -dblist and -num_volumes.\n";

static const string kOutput("out");

void CBlastDBAliasApp::Init()
{
    HideStdArgs(fHideConffile | fHideFullVersion | fHideXmlHelp | fHideDryRun);

    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(), 
                              "Application to create BLAST database aliases, version " 
                              + CBlastVersion().Print() + DOCUMENTATION);

    string dflt("Default = input file name provided to -gi_file_in argument");
    dflt += " with the .bgl extension";

    set<string> exclusions  = {
        kArgDb, kArgDbType, kArgDbTitle, kArgGiList, kArgSeqIdList, kArgOutput,
        "dblist", "num_volumes", "vdblist", "seqid_file_in", "seqid_file_out",
        "seqid_db", "seqid_dbtype", "seqid_file_info"
    };
    
    arg_desc->SetCurrentGroup("GI file conversion options");

    arg_desc->AddOptionalKey("gi_file_in", "input_file",
                             "Text file to convert, should contain one GI per line",
                             CArgDescriptions::eInputFile);
    for (string exclusion : exclusions) {
        arg_desc->SetDependency("gi_file_in", CArgDescriptions::eExcludes, exclusion);
    }

    arg_desc->AddOptionalKey("gi_file_out", "output_file",
                             "File name of converted GI file\n" + dflt,
                             CArgDescriptions::eOutputFile,
                             CArgDescriptions::fPreOpen | CArgDescriptions::fBinary);
    arg_desc->SetDependency("gi_file_out", CArgDescriptions::eRequires,
                            "gi_file_in");
    for (string exclusion : exclusions) {
        arg_desc->SetDependency("gi_file_out", CArgDescriptions::eExcludes, exclusion);
    }

    arg_desc->SetCurrentGroup("Alias file creation options");

    arg_desc->AddOptionalKey(kArgDb, "dbname", "BLAST database name", 
                             CArgDescriptions::eString);
    arg_desc->SetDependency(kArgDb, CArgDescriptions::eRequires, kOutput);

    arg_desc->AddDefaultKey(kArgDbType, "molecule_type",
                            "Molecule type stored in BLAST database",
                            CArgDescriptions::eString, "prot");
    arg_desc->SetConstraint(kArgDbType, &(*new CArgAllow_Strings,
                                          "nucl", "prot"));

    arg_desc->AddOptionalKey(kArgDbTitle, "database_title",
                             "Title for BLAST database\n"
                             "Default = name of BLAST database provided to -db"
                             " argument with the -gifile argument appended to it",
                             CArgDescriptions::eString);
    arg_desc->SetDependency(kArgDbTitle, CArgDescriptions::eRequires, kOutput);

    arg_desc->AddOptionalKey(kArgGiList, "input_file", 
                             "Text or binary gi file to restrict the BLAST "
                             "database provided in -db argument\n"
                             "If text format is provided, it will be converted "
                             "to binary",
                             CArgDescriptions::eInputFile);
    arg_desc->SetDependency(kArgGiList, CArgDescriptions::eRequires, kOutput);

    arg_desc->AddOptionalKey(kArgSeqIdList, "input_file", 
                             "Text sequence id or accession file to restrict "
                             "the BLAST database provided in -db argument",
                             CArgDescriptions::eInputFile);

    arg_desc->SetDependency(kArgSeqIdList, CArgDescriptions::eRequires, kOutput);
    arg_desc->SetDependency(kArgSeqIdList, CArgDescriptions::eExcludes,
                            kArgGiList);

    arg_desc->AddOptionalKey(kArgTaxIdListFile, "input_file",
                                 "Text taxonomy id file to restrict "
                                 "the BLAST database provided in -db argument",
                                 CArgDescriptions::eInputFile);

    arg_desc->SetDependency(kArgTaxIdListFile, CArgDescriptions::eRequires, kOutput);
    arg_desc->SetDependency(kArgTaxIdListFile, CArgDescriptions::eExcludes, kArgGiList);
    arg_desc->SetDependency(kArgTaxIdListFile, CArgDescriptions::eExcludes, kArgSeqIdList);

#ifdef NCBI_TI
    arg_desc->AddFlag("process_as_tis", 
                      "Process all numeric ID lists as TIs instead of GIs", true);
#endif

    arg_desc->AddOptionalKey(kOutput, "database_name",
                             "Name of BLAST database alias to be created",
                             CArgDescriptions::eString);

    arg_desc->AddOptionalKey("dblist", "database_names", 
                             "A space separated list of BLAST database names to"
                             " aggregate",
                             CArgDescriptions::eString);

    arg_desc->AddOptionalKey("dblist_file", "file_name", 
                             "A file containing a list of BLAST database names"
                             " to aggregate, one per line",
                             CArgDescriptions::eInputFile);

    /* For VDBLIST */
    arg_desc->AddOptionalKey("vdblist", "vdb_names",
                             "A space separated list of VDB names to aggregate",
                             CArgDescriptions::eString);

    arg_desc->AddOptionalKey("vdblist_file", "file_name",
                             "A file containing a list of vdb names"
                             " to aggregate, one per line",
                             CArgDescriptions::eInputFile);
    const char* key[] = { "dblist", "dblist_file", "vdblist", "vdblist_file" };
    for (size_t i = 0; i < sizeof(key)/sizeof(*key); i++) {
        arg_desc->SetDependency(key[i], CArgDescriptions::eExcludes, kArgDb);
        arg_desc->SetDependency(key[i], CArgDescriptions::eExcludes, "num_volumes");
        arg_desc->SetDependency(key[i], CArgDescriptions::eRequires, kOutput);
        arg_desc->SetDependency(key[i], CArgDescriptions::eRequires, kArgDbType);
        arg_desc->SetDependency(key[i], CArgDescriptions::eRequires, kArgDbTitle);
    }
    arg_desc->SetDependency("dblist", CArgDescriptions::eExcludes, "dblist_file");
    arg_desc->SetDependency("vdblist", CArgDescriptions::eExcludes, "vdblist_file");

    CNcbiOstrstream msg;
    msg << "Number of volumes to aggregate, in which case the "
        << "basename for the database is extracted from the "
        << kOutput << " option";
    arg_desc->AddOptionalKey("num_volumes", "positive_integer", 
                             CNcbiOstrstreamToString(msg),
                             CArgDescriptions::eInteger);
    arg_desc->SetDependency("num_volumes", CArgDescriptions::eExcludes, kArgDb);
    arg_desc->SetDependency("num_volumes", CArgDescriptions::eExcludes, kArgGiList);
    arg_desc->SetDependency("num_volumes", CArgDescriptions::eExcludes, kArgSeqIdList);
    arg_desc->SetDependency("num_volumes", CArgDescriptions::eRequires, kOutput);
    arg_desc->SetDependency("num_volumes", CArgDescriptions::eRequires, kArgDbType);
    arg_desc->SetDependency("num_volumes", CArgDescriptions::eRequires, kArgDbTitle);
    arg_desc->SetConstraint("num_volumes", new CArgAllowValuesGreaterThanOrEqual(1));

    string dflt_seqid("Default = input file name provided to -seqid_file_in argument");
    set<string> seqid_exclusions  = {
        kArgDb, kArgDbType, kArgDbTitle, kArgGiList, kArgSeqIdList, kArgOutput,
        "dblist", "num_volumes", "vdblist"
    };
    // "gi_file_in" and "gi_file_out" already exclude "seqid_file_in" and
    // "seqid_file_out".

    arg_desc->SetCurrentGroup("Seqd ID file conversion options");

    arg_desc->AddOptionalKey("seqid_file_in", "input_file",
                             "Text file to convert, should contain one seq id per line",
                             CArgDescriptions::eInputFile);
    for (string exclusion : seqid_exclusions) {
        arg_desc->SetDependency("seqid_file_in", CArgDescriptions::eExcludes, exclusion);
    }
    
    arg_desc->AddOptionalKey("seqid_title", "seqid_title", "Title for seqid list.\n " +
                             dflt_seqid, CArgDescriptions::eString);
    arg_desc->SetDependency("seqid_title", CArgDescriptions::eRequires, "seqid_file_in");

    arg_desc->AddOptionalKey("seqid_file_out", "output_file",
                             "File name of converted seq id file\n" + dflt_seqid + " with the .bsl extension",
                             CArgDescriptions::eString);

    arg_desc->AddOptionalKey("seqid_db", "dbname", "BLAST database for seqidlist",
                             CArgDescriptions::eString);
    arg_desc->SetDependency("seqid_db", CArgDescriptions::eRequires, "seqid_file_in");

    arg_desc->AddOptionalKey("seqid_dbtype", "molecule_type", "Molecule type BLAST database",
                             CArgDescriptions::eString);
    arg_desc->SetDependency("seqid_dbtype", CArgDescriptions::eRequires, "seqid_file_in");
    arg_desc->SetDependency("seqid_dbtype", CArgDescriptions::eRequires, "seqid_db");
    arg_desc->SetConstraint("seqid_dbtype", &(*new CArgAllow_Strings, "nucl", "prot"));

    for (string exclusion : seqid_exclusions) {
        arg_desc->SetDependency("seqid_file_out", CArgDescriptions::eExcludes, exclusion);
    }

    set<string> seqid_info_exclusions = {
        kArgDb, kArgDbType, kArgDbTitle, kArgGiList, kArgSeqIdList, kArgOutput,
        "dblist", "num_volumes", "vdblist", "seqid_file_in", "seqid_file_out"
    };
    // "gi_file_in" and "gi_file_out" already exclude "seqid_file_info".
    
    arg_desc->AddOptionalKey("seqid_file_info", "seqid_file_info", "Display seqidlist file info", CArgDescriptions::eString);
    for (string exclusion : seqid_info_exclusions) {
        arg_desc->SetDependency("seqid_file_info", CArgDescriptions::eExcludes, exclusion);
    }


    SetupArgDescriptions(arg_desc.release());
}

int 
CBlastDBAliasApp::ConvertGiFile(CNcbiIstream& input,
                                CNcbiOstream& output,
                                const string* input_fname /* = NULL */,
                                const string* output_fname /* = NULL */) const
{
    const CArgs& args = GetArgs();
    CBinaryListBuilder::EIdType type = CBinaryListBuilder::eGi;
    string product("GI");
    if (args.Exist("process_as_tis") && args["process_as_tis"]) {
        type = CBinaryListBuilder::eTi;
        product.assign("TI");
    }
    CBinaryListBuilder builder(type);

    unsigned int line_ctr = 0;
    while (input) {
        string line;
        NcbiGetlineEOL(input, line);
        line_ctr++;
        if ( !line.empty() ) {
            if (NStr::StartsWith(line, "#")) continue;
            try { builder.AppendId(NStr::StringToInt8(line)); }
            catch (const CStringException& e) {
                ERR_POST(Warning << "error in line " << line_ctr 
                         << ": " << e.GetMsg());
            }
        }
    }

    builder.Write(output);
    if (input_fname && output_fname) {
        LOG_POST("Converted " << builder.Size() << " " << product << "s from " 
                 << *input_fname << " to binary format in " << *output_fname);
    } else {
        LOG_POST("Converted " << builder.Size() << " " << product << "s into "
                 << "binary " << product << " file");
    }
    return 0;
}

void
CBlastDBAliasApp::CreateAliasFile() const
{
    const CArgs& args = GetArgs();
    string title;
    bool isTiList = false;
    if (args.Exist("process_as_tis") && args["process_as_tis"]) {
        isTiList = true;
    }

    if (args[kArgDb].HasValue() && !args[kArgGiList].HasValue() &&
        !args[kArgSeqIdList].HasValue()&& ! args[kArgTaxIdListFile].HasValue()) {

        NCBI_THROW(CInputException, eInvalidInput, "Either gilist or "
                   "seqid_list must be specified if database name is used");
    }

    if (args[kArgDbTitle].HasValue()) {
        title = args[kArgDbTitle].AsString();
    } else if (args[kArgDb].HasValue()) {
        _ASSERT(args[kArgGiList].HasValue() || args[kArgSeqIdList].HasValue() ||
        		args[kArgTaxIdListFile].HasValue());
        title = args[kArgDb].AsString() + " limited by ";
        if (args[kArgGiList]) {
            title += args[kArgGiList].AsString();
        }
        else if (args[kArgSeqIdList]){
            title += args[kArgSeqIdList].AsString();
        } else {
            title += args[kArgTaxIdListFile].AsString();
        }
    }
    const CWriteDB::ESeqType seq_type = 
        args[kArgDbType].AsString() == "prot"
        ? CWriteDB::eProtein
        : CWriteDB::eNucleotide;

    string gilist = args[kArgGiList] ? args[kArgGiList].AsString() : kEmptyStr;
    if ( !gilist.empty() ) {
        if ( !CFile(gilist).Exists() ) {
            NCBI_THROW(CSeqDBException, eFileErr, gilist + " not found");
        }
        if ( (!isTiList && !SeqDB_IsBinaryGiList(gilist)) ||
             (isTiList && !SeqDB_IsBinaryTiList(gilist)) ) {
            const char mol_type = args[kArgDbType].AsString()[0];
            _ASSERT(mol_type == 'p' || mol_type == 'n');
            CNcbiOstrstream oss;
            oss << args[kOutput].AsString() << "." << mol_type << 
                (isTiList ? ".btl" : ".gil");
            gilist.assign(CNcbiOstrstreamToString(oss));
            const string& ifname = args[kArgGiList].AsString();
            ifstream input(ifname.c_str());
            ofstream output(gilist.c_str(), std::ios::binary);
            ConvertGiFile(input, output, &ifname, &gilist);
        }
    }


    const EAliasFileFilterType alias_type = (isTiList ? eTiList : eGiList);
    if (args["dblist"].HasValue() || args["dblist_file"].HasValue()) {
        vector<string> dbs2aggregate = x_GetDbsToAggregate("dblist", "dblist_file");
        CWriteDB_CreateAliasFile(args[kOutput].AsString(), dbs2aggregate,
                                 seq_type, gilist, title, alias_type);
    } else if (args["num_volumes"].HasValue()) {
        const unsigned int num_vols = 
            static_cast<unsigned int>(args["num_volumes"].AsInteger());
        CWriteDB_CreateAliasFile(args[kOutput].AsString(), num_vols, seq_type,
                                 title);
    } else if (args[kArgDb].HasValue() && args[kArgGiList]){
        CWriteDB_CreateAliasFile(args[kOutput].AsString(),
                                 args[kArgDb].AsString(),
                                 seq_type, gilist,
                                 title, alias_type);
    } else if (args[kArgDb].HasValue() && args[kArgSeqIdList]){
    	string seqid_list = args[kArgSeqIdList].AsString();
    	if ( !seqid_list.empty() ) {
    	    if ( !CFile(seqid_list).Exists() ) {
    	        NCBI_THROW(CSeqDBException, eFileErr, seqid_list + " not found");
    	    }
    	}
        CWriteDB_CreateAliasFile(args[kOutput].AsString(),
                                 args[kArgDb].AsString(),
                                 seq_type, seqid_list,
                                 title, eSeqIdList);
    } else if (args[kArgDb].HasValue() && args[kArgTaxIdListFile]) {
    	string taxid_list = args[kArgTaxIdListFile].AsString();
    	if ( !taxid_list.empty() ) {
    	    if ( !CFile(taxid_list).Exists() ) {
    	        NCBI_THROW(CSeqDBException, eFileErr, taxid_list + " not found");
    	    }
    	}
        CWriteDB_CreateAliasFile(args[kOutput].AsString(),
                                 args[kArgDb].AsString(),
                                 seq_type, taxid_list,
                                 title, eTaxIdList);
    }

    if (args["vdblist"].HasValue() || args["vdblist_file"].HasValue()) {
    	CNcbiOstrstream fname;
    	if (args["dblist"].HasValue() || args["dblist_file"].HasValue()) {
    		fname << args[kOutput].AsString() << (seq_type == CWriteDB::eProtein ? ".pal" : ".nal");
    		x_AddVDBsToAliasFile( CNcbiOstrstreamToString(fname), true );
    	}
    	else {
    		fname << args[kOutput].AsString() << (seq_type == CWriteDB::eProtein ? ".pvl" : ".nvl");
    		string title;
    		if(args["title"].HasValue()) {
    			title = args["title"].AsString();
    		}
    		x_AddVDBsToAliasFile( CNcbiOstrstreamToString(fname), false, title);
    	}
    }
}

void CBlastDBAliasApp::x_AddVDBsToAliasFile( string filename, bool append, string title) const
{
    vector<string> vdbs = x_GetDbsToAggregate("vdblist", "vdblist_file");
    if(vdbs.empty()) {
    	 LOG_POST(Warning <<"Empty vdb list");
    	 return;
    }

	IOS_BASE::openmode op_mode = IOS_BASE::out;
	if(append) {
		op_mode |= IOS_BASE::app;
	}
	CNcbiOfstream alias_file(filename.c_str(), op_mode);

	if(!append) {
		alias_file <<  "#\n# Alias file created " << CTime(CTime::eCurrent).AsString() << "\n#\n";
	}

	if(kEmptyStr != title) {
		alias_file << "TITLE " << title << "\n";
	}

	 alias_file << "VDBLIST ";
	 ITERATE(vector< string >, iter, vdbs) {
		 alias_file << "\"" << *iter << "\" ";
	 }
	 alias_file << "\n";
}

vector<string> CBlastDBAliasApp::x_GetDbsToAggregate(const string dbs, const string file) const
{
    vector<string> retval;
    const CArgs& args = GetArgs();
    if (args[dbs].HasValue()) {
        const string dblist = args[dbs].AsString();
        NStr::Split(dblist, " ", retval);
    } else if (args[file].HasValue()) {
        CNcbiIstream& in(args[file].AsInputFile());
        string line;
        while (getline(in, line)) {
            line = NStr::TruncateSpaces(line);
            if (line.empty()) {
                continue;
            }
            retval.push_back(line);
        }
    } else {
        abort();
    }
    return retval;
}


int
CBlastDBAliasApp::x_ConvertSeqIDFile() const
{
	const CArgs& args = GetArgs();
	CNcbiIstream& input = args["seqid_file_in"].AsInputFile();
	string out_filename = kEmptyStr;
	string title = kEmptyStr;
   	if(args["seqid_file_out"].HasValue()) {
   		out_filename = args["seqid_file_out"].AsString();
   	}
   	else {
		out_filename = args["seqid_file_in"].AsString() + ".bsl";
   	}

   	if(args["seqid_title"].HasValue()) {
   		title = args["seqid_title"].AsString();
   	}
   	else {
   		CSeqDB_Path(args["seqid_file_in"].AsString()).FindFileName().GetString(title);
   	}

	CNcbiOfstream output(out_filename.c_str(), IOS_BASE::binary | IOS_BASE::out);
    unsigned int line_ctr = 0;
    vector<string> seqid_list;
    while (input) {
        string line;
        NcbiGetlineEOL(input, line);
        line_ctr++;
        if ( !line.empty() ) {
            if (NStr::StartsWith(line, "#")) continue;
            seqid_list.push_back(line);
        }
    }

    if (args["seqid_db"].HasValue()) {
    	CSeqDB::ESeqType type = CSeqDB::eUnknown;
    	if (args["seqid_dbtype"].HasValue()) {
    		type = (args["seqid_dbtype"].AsString()[0] == 'p') ? CSeqDB::eProtein : CSeqDB::eNucleotide;
    	}
    	CSeqDB seqdb(args["seqid_db"].AsString(), type);
    	return WriteBlastSeqidlistFile(seqid_list, output, title, &seqdb);
    }
    else {
    	return WriteBlastSeqidlistFile(seqid_list, output, title);
    }

}

void
CBlastDBAliasApp::x_SeqIDFileInfo() const
{
	const CArgs& args = GetArgs();
	CBlastSeqidlistFile::PrintSeqidlistInfo(args["seqid_file_info"].AsString(), std::cout);
}


int CBlastDBAliasApp::Run(void)
{
    const CArgs& args = GetArgs();
    int status = 0;

    try {

        if (x_GetOperationMode() == eConvertGiFile) {
            CNcbiIstream& input = args["gi_file_in"].AsInputFile();
            string gi_file_out;
            if (args["gi_file_out"].HasValue()) {
                gi_file_out = args["gi_file_out"].AsString();
            } else {
                gi_file_out = args["gi_file_in"].AsString();
                gi_file_out += ".bgl";
            }
            {
                // output will close at end of scope.
                CNcbiOfstream output(gi_file_out.c_str(),std::ios::binary);
                status = ConvertGiFile(input, output);
            }
            if (!CFile(gi_file_out).Exists()) {
                NCBI_THROW(CSeqDBException, eFileErr, gi_file_out + " not written");
            }
        } else if(x_GetOperationMode() == eConvertSeqIDFile) {
        	status = x_ConvertSeqIDFile();
        } else if(x_GetOperationMode() == eSeqIDFileInfo) {
        	x_SeqIDFileInfo();
        }
        else {
            CreateAliasFile();
        }

    } CATCH_ALL(status)
    return status;
}


#ifndef SKIP_DOXYGEN_PROCESSING
int main(int argc, const char* argv[] /*, const char* envp[]*/)
{
    return CBlastDBAliasApp().AppMain(argc, argv);
}
#endif /* SKIP_DOXYGEN_PROCESSING */
