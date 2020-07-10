/*  $Id: make_score_method.cpp 36583 2016-10-12 15:11:14Z evgeniev $
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
 * Authors:  Robert Smith
 *
 * File Description:
 *   Create Alignment score method files from:
 *   1. Builtin scoring tables (-sm)
 *   2. aaindex tables and matrixes from
 *      http://www.genome.ad.jp/dbget/aaindex.html
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbiapp.hpp>
#include <corelib/ncbireg.hpp>
#include <corelib/ncbi_limits.hpp>

#include <util/tables/raw_scoremat.h>

#include <ctype.h>
#include <iostream>
#include <iomanip>
#include <algorithm>


BEGIN_NCBI_SCOPE

class CMakeScoreMethodApp : public CNcbiApplication
{
private:
    void Init(void);
    int  Run(void);
};


void CMakeScoreMethodApp::Init(void)
{
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "Make alignment scoring method files");

    arg_desc->AddDefaultKey("out", "output", "File name for scoring method",
                CArgDescriptions::eOutputFile, "-");

    arg_desc->AddOptionalKey("sm", "matrix", "name of score matrix to use",
                            CArgDescriptions::eString);
    arg_desc->SetConstraint
        ("sm", &(*new CArgAllow_Strings,
                 "blosum45", "blosum62", "blosum80", "pam30", "pam70", "pam250"));

    arg_desc->AddOptionalKey("aa", "accession",
        "Amino Acid Index Database accession number",
        CArgDescriptions::eString);
    arg_desc->AddDefaultKey("in", "aaindex_file",
        "Amino Acid Index Database input file.",
        CArgDescriptions::eInputFile, "-");

    SetupArgDescriptions(arg_desc.release());
}


inline static string s_FormatAA(int aa) {
    return isprint((unsigned char) aa) ? string(1, (char) aa) : NStr::IntToString(aa);
}


static void s_BuiltInSM(const string& sm, map<string, string>& out_map)
{
    const SNCBIPackedScoreMatrix*  psm = NULL;
    string desc;

    if        (sm == "blosum45") {
        psm = &NCBISM_Blosum45;
        desc =
        "    Matrix made by matblas from blosum45.iij\\\n"
        "    BLOSUM Clustered Scoring Matrix in 1/3 Bit Units\\\n"
        "    Blocks Database = /data/blocks_5.0/blocks.dat\\\n"
        "    Cluster Percentage: >= 45\\\n"
        "    Entropy =   0.3795, Expected =  -0.2789";
    } else if (sm == "blosum62") {
        psm = &NCBISM_Blosum62;
        desc =
        "    Matrix made by matblas from blosum62.iij\\\n"
        "    BLOSUM Clustered Scoring Matrix in 1/2 Bit Units\\\n"
        "    Blocks Database = /data/blocks_5.0/blocks.dat\\\n"
        "    Cluster Percentage: >= 62\\\n"
        "    Entropy =   0.6979, Expected =  -0.5209";
    } else if (sm == "blosum80") {
        psm = &NCBISM_Blosum80;
        desc =
        "    Matrix made by matblas from blosum80.iij\\\n"
        "    BLOSUM Clustered Scoring Matrix in 1/2 Bit Units\\\n"
        "    Blocks Database = /data/blocks_5.0/blocks.dat\\\n"
        "    Cluster Percentage: >= 80\\\n"
        "    Entropy =   0.9868, Expected =  -0.7442";
    } else if (sm == "pam30") {
        psm = &NCBISM_Pam30;
        desc =
        "    This matrix was produced by \\\"pam\\\" Version 1.0.6 [28-Jul-93]\\\n"
        "    PAM 30 substitution matrix, scale = ln(2)/2 = 0.346574\\\n"
        "    Expected score = -5.06, Entropy = 2.57 bits\\\n"
        "    Lowest score = -17, Highest score = 13";
    } else if (sm == "pam70") {
        psm = &NCBISM_Pam70;
        desc =
        "    This matrix was produced by \\\"pam\\\" Version 1.0.6 [28-Jul-93]\\\n"
        "    PAM 70 substitution matrix, scale = ln(2)/2 = 0.346574\\\n"
        "    Expected score = -2.77, Entropy = 1.60 bits\\\n"
        "    Lowest score = -11, Highest score = 13";
    } else if (sm == "pam250") {
        psm = &NCBISM_Pam250;
        desc =
        "    This matrix was produced by \\\"pam\\\" Version 1.0.7 [01-Feb-98]\\\n"
        "    using Dayhoff et al. (1978) mutability data.\\\n"
        "    PAM 250 substitution matrix, scale = ln(2)/3 = 0.231049\\\n"
        "    Expected score = -0.844, Entropy = 0.354 bits\\\n"
        "    Lowest score = -8, Highest score = 17";
    } else {
        _TROUBLE;
    }

    out_map["Name"] = sm;
    out_map["Builtin"] = sm;
    out_map["Description"] = desc.empty() ? sm : desc;


    int maxscore = numeric_limits<int>::min();
    int minscore = numeric_limits<int>::max();

    int l = strlen(psm->symbols);

    // find max and min.
    for (int i = 0;  i < l;  ++i) {
        if (isupper((unsigned char) psm->symbols[i])) {
            for (int j = 0;  j < l;  ++j) {
                int value = psm->scores[i * l + j];
                maxscore = max(value, maxscore);
                minscore = min(value, minscore);
            }
        }
    }
    out_map["MinimumValue"] = NStr::IntToString(minscore);
    out_map["MaximumValue"] = NStr::IntToString(maxscore);
}


static void sWriteLine(CNcbiOstream& out, map<string, string>& out_map, const string& name)
{
    if ( ! out_map[name].empty()) {
        out << name << " = " << out_map[name] << endl;
    }
}


static bool sReadAA_M(
    CNcbiIstream& in,
    const string& m_line,
    map<string, string>& out_map )
{
    out_map["Method"] = "MatrixScore";

    // parse the rest of the M line.
    // look for '='.  The token after the first is the rows.
    // the token after the second '=' is the columns.
    list<string> toks;
    NStr::Split(m_line, " ,", toks, NStr::fSplit_Tokenize);
    list<string>::iterator tok_it;
    tok_it = find(toks.begin(), toks.end(), "=");
    if (tok_it == toks.end())
        return false;
    ++tok_it;
    string row_bases(*tok_it);

    tok_it = find(++tok_it, toks.end(), "=");
    if (tok_it == toks.end())
        return false;
    ++tok_it;
    string col_bases(*tok_it);

    // make the Columns line.
    int cols = col_bases.size();
    string symbols(1, col_bases[0]);
    for (int i = 1;  i < cols;  ++i) {
        char c = col_bases[i];
        if (isupper((unsigned char) c)  ||  c == '-') {
            symbols += "       ";
            symbols += c;
        }
    }
    out_map["Columns"] = symbols;

    // Make all the TableRows lines.
    string line;
    string out_rows;
    int rows = row_bases.size();
    for (int r = 0; r < rows; ++r) {
        if (! getline(in, line))
            break;
        out_rows += row_bases[r];
        out_rows += " =";
        out_rows += line;
        out_rows += '\n';
    }
    out_map["TableRows"] = out_rows;
    return true;
}

// file aaindex1, with one score per amino acid,
// always has the same amino acid order.  So here it is:
static const string kAAIndexOrder("ARNDCQEGHILKMFPSTWYV");

static bool sReadAA_I(CNcbiIstream& in, map<string, string>& out_map)
{
    out_map["Method"] = "ColumnScore";

    string line1, line2;
    string out_rows;
    getline(in, line1);
    getline(in, line2);
    list<string> scores;
    NStr::Split(line1 + line2, " ", scores, NStr::fSplit_Tokenize);

    int r = 0;
    ITERATE(list<string>, score_it, scores) {
        out_rows += kAAIndexOrder[r];
        out_rows += " = ";
        out_rows += *score_it;
        out_rows += '\n';
        ++r;
    }
    out_map["TableRows"] = out_rows;
    return true;
}


bool s_ReadAAIndex(const string& accession, CNcbiIstream& in, map<string, string>& out_map)
{
    string line;
    // skip till we find the accession.
    while (getline(in, line)) {
        if (line == "H " + accession)
            break;
    }
    if ( ! in.good()) {
        cerr << "Accession \"" << accession << "\" not found." << endl;
        return false;
    }

    out_map["Name"] = NStr::TruncateSpaces(line.substr(2));

    string current_key;
    while (getline(in, line)) {
        if (line.empty())
            continue;
        char command = line[0];
        line = NStr::TruncateSpaces(line.substr(2));
        switch(command) {
        case 'D':   // Data description
            // assume that 'D' will come before the following.
            current_key = "Description";
            out_map[current_key] = line;
            break;
        case 'R':   // LITDB entry number
        case 'A':   // Author(s)
        case 'T':   // Title of the article
        case 'J':   // Journal reference
            // tack all of these on to the description.
            current_key = "Description";
            out_map[current_key] += "\\\n    " + line;
            break;
        case ' ':   // continuation lines.
            if ( ! current_key.empty()) {
                out_map[current_key] += "\\\n    " +line;
            }
            break;
        case '*':   // Comment or missing
        case 'C':   // Accession numbers of similar entries
            // ignore these.
            current_key.erase();
            break;
        case 'M':   // Matrix data
            if (! sReadAA_M(in, line, out_map)) {
                cerr << "Bad format in M section at accession \""
                    << accession << "\"" << endl;
                return false;
            }
            return true;
        case 'I':   // Amino acid index data
            if (! sReadAA_I(in, out_map)) {
                cerr << "Bad format in I section at accession \""
                    << accession << "\"" << endl;
                return false;
            }
            return true;
        case '/':
            return false;
        }
    }
    cerr << "No I or M section at accession \""
        << accession << "\"" << endl;
    return false;
}


int CMakeScoreMethodApp::Run(void)
{
    CArgs                          args = GetArgs();
    CNcbiOstream& out = args["out"].AsOutputFile();

    if (args["sm"]  &&  args["aa"] ) {
        string msg = "Options -sm and -aa are mutually exclusive.\n";
        cerr << GetArgDescriptions()->PrintUsage(msg);
        return 1;
    }

    // We could use a CNcbiRegistry to store and write out the lines
    // instead of this map.
    // But then we couldn't specify the order the lines in the file
    // and basically the resulting files would work just as well
    // but would look a lot worse.

    map<string, string> out_line;

    out_line["Method"] = "MatrixScore";
    out_line["Type"] = "Protein";
    out_line["MinimumColor"] = "yellow3";
    out_line["MaximumColor"] = "royal blue";

    if (args["sm"]) {
        s_BuiltInSM(args["sm"].AsString(), out_line);
    } else if (args["aa"]) {
        if (! s_ReadAAIndex(args["aa"].AsString(), args["in"].AsInputFile(), out_line)) {
            return 1;
        }
    }

    // specify order of the output lines here.
    out << "[Info]" << endl;
    sWriteLine(out, out_line, "Name");
    sWriteLine(out, out_line, "Description");
    sWriteLine(out, out_line, "Method");
    sWriteLine(out, out_line, "Type");

    out << "[Table]" << endl;
    sWriteLine(out, out_line, "MinimumValue");
    sWriteLine(out, out_line, "MinimumColor");
    sWriteLine(out, out_line, "MaximumValue");
    sWriteLine(out, out_line, "MaximumColor");
    if (args["sm"]) {
        sWriteLine(out, out_line, "Builtin");
    } else {
        sWriteLine(out, out_line, "Columns");

        out << "[TableRows]" << endl;
        out << out_line["TableRows"];
    }

    return 0;
}


END_NCBI_SCOPE

USING_NCBI_SCOPE;

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return CMakeScoreMethodApp().AppMain(argc, argv);
}
