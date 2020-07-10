/*  $Id: test_table_reader.cpp 34674 2016-02-01 21:51:48Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description
 *
 */

#include <ncbi_pch.hpp>
#include <stdio.h>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbireg.hpp>
#include <corelib/ncbifile.hpp>
#include <algorithm>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>

#include <gui/widgets/loaders/table_import_data_source.hpp>
#include <gui/widgets/loaders/table_annot_data_source.hpp>
#include <gui/widgets/loaders/table_column_type_guesser.hpp>
#include <gui/widgets/loaders/map_assembly_params.hpp>

#include <common/test_assert.h>


USING_NCBI_SCOPE;
USING_SCOPE(objects);

/////////////////////////////////
// Test application
//

class CTestApplication : public CNcbiApplication
{
public:
    void Init(void);
    int Run(void);

    void LoadTable(const string& fname, 
                   const string& parms_file,
                   const string& table_xform,
                   const string& accession);
};


void CTestApplication::Init(void)
{
    // Set err.-posting and tracing to maximum
    SetDiagTrace(eDT_Enable);
    SetDiagPostFlag(eDPF_All);
    SetDiagPostLevel(eDiag_Info);
       
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions());

    arg_desc->AddOptionalKey("xform", "TransformationType", 
        "Desired transformation for table - none, seqlocs or features.  Default: none", 
        CArgDescriptions::eString);

    arg_desc->AddOptionalKey("acc", "Accession", 
        "Optional accession, e.g. GCF_000001405.17, used to look up ids for chromosome fields", 
        CArgDescriptions::eString);

    arg_desc->AddOptionalKey("table-format", "TableFormatDescription", 
        "ASN table format description for table(s) being imported", 
        CArgDescriptions::eString);

    arg_desc->SetArgsType(CArgDescriptions::eRegularArgs);
    arg_desc->AddExtra(1, kMax_UInt, "Files or Directories", CArgDescriptions::eString);

    SetupArgDescriptions(arg_desc.release());
}

void CTestApplication::LoadTable(const string& fname, 
                                 const string& parms_file,
                                 const string& table_xform,
                                 const string& accession)
{
    LOG_POST(Info << "Loading Table: " << fname);

    //CTableImportDataSource ds;
    CRef<CTableImportDataSource> ds(new CTableImportDataSource());
    string transformation_type = table_xform;
    bool success;

    try {
        if (parms_file == "") {
            success = ds->LoadTable(fname);

            if (!success) {                
                LOG_POST(Error << "Error loading file: " << fname);
                return;
            }

            if (ds->GetTableType() == CTableImportDataSource::eDelimitedTable)
            {
                LOG_POST(Info << "Delimiters: ");
                vector<char> delims = ds->GetDelimiterRules().GetDelimiters();
                for (size_t i=0; i<delims.size(); ++i)
                    LOG_POST(Info << "\'" << delims[i] << "\', ");
                LOG_POST(Info << "Merge Delimiters: " << ds->GetDelimiterRules().GetMergeDelimiters());

                // guess column types
                CTableColumnTypeGuesser  guesser(ds);
                guesser.GuessColumns();

                // If an accession is provided, look for and add it to any fields
                // that need id mapping (chromosome #'s)                
                if (accession != "") {
                    for (size_t i=0; i<ds->GetColumns().size(); ++i) {
                        if (ds->GetColumns()[i].GetDataType() == 
                            CTableImportColumn::eChromosome) {
                                //cout << "Adding accession to col: " << i << " " << accession << endl;
                                CMapAssemblyParams ainfo;
                                ainfo.SetAssemblyAcc(accession);                              
                                ds->GetColumns()[i].SetAssembly(ainfo);
                        }
                    }

                }
            }
            else {
                LOG_POST(Error << "Could not find a delimiter for table: " << fname);
            }
        }
        else {
            CNcbiIfstream in_file(parms_file.c_str());
            if (!in_file) {
                LOG_POST(Error << "Could not open parameters file: " << parms_file);
                return;
            }

            CUser_object user_object;
            LOG_POST(Info << "Loading Parameters file: " << parms_file);
            in_file >> MSerial_AsnText >> user_object;            

            // If the command line paramaters specify an explicit transformation
            // type use that instead of the type stored in the parameters file
            if (table_xform == "" &&
                user_object.HasField("conversion-type") && 
                user_object.GetField("conversion-type").GetData().IsStr()) {
                    transformation_type = user_object.
                        GetField("conversion-type").GetData().GetStr();
            }

            success = ds->LoadTable(fname, user_object);

            if (!success) {
                LOG_POST(Error << "Error loading file: " << fname);
                return;
            }
            else {
                LOG_POST(Info << "Loaded table from: " << fname);
            }
        }
    }
    catch( CException& c) {
        LOG_POST(Error <<"Error loading table: " << c.ReportAll());
        return;
    }

    string output_type = "";

    try {
        // Convert table to asn and then (if table_xform is not 'none') transform
        // it to the desired format.  After (optional) transform write table out
        // to a file with same name but of type ".asn".
        CRef<CTableAnnotDataSource> annot_table_ds(new CTableAnnotDataSource());
        ds->ConvertToSeqAnnot(annot_table_ds->GetContainer());

        string msg;        
        if (transformation_type ==  "seqlocs") {
            vector<CTableAnnotDataSource::STableLocation> locations =
                annot_table_ds->FindLocations(msg, false);                    

            if (locations.size() == 0) {
                LOG_POST(Error <<  msg << " - Unable to create locations");
            }
            else {
                for (size_t i=0; i<locations.size(); ++i) {
                    if (!locations[i].m_IsRsid)
                        success = annot_table_ds->AddSeqLoc(locations[i], i+1, NULL);
                    else
                        success = annot_table_ds->AddSnpSeqLoc(locations[i], i+1, NULL);
                }
                output_type = ".locs";
            }
        }
        else if (transformation_type == "features") {           
            vector<CTableAnnotDataSource::STableLocation> locations =
                annot_table_ds->FindLocations(msg, false);

            // Any location can be a feature, but only 1.  If we find more than 1
            // we choose to to create any features. 
            if (locations.size() > 1) {
                LOG_POST(Error << "Unable to create features - table has more than 1 location per row");
            }
            // unable to identify columns needed to create location info needed by features.
            else if (locations.size() == 0) {    
                LOG_POST(Error <<  msg << " - Unable to create features");
            }
            else {
                if (!locations[0].m_IsRsid)
                    success = annot_table_ds->CreateFeature(locations[0], NULL);
                else 
                    success = annot_table_ds->CreateSnpFeature(locations[0], NULL);
                output_type = ".feats";
            }
        }

        string out_fname = fname + output_type + ".asn";    

        annot_table_ds->WriteAsn(out_fname);
    }
    catch( CException& c) {
        LOG_POST(Error <<"Error transforming and saving table: " << c.ReportAll());
    }
    
}


int CTestApplication::Run(void)
{   
	try{

        const CNcbiArguments&  args = GetArguments();

        if (args.Size() == 0)
            return 1;

        string parms_file;
        string table_xform = "";
        string accession = "";

        LOG_POST(Info << "starting run");

        const CArgs& parsed_args = GetArgs();

        if (parsed_args["table-format"].HasValue()) {
            parms_file = parsed_args["table-format"].AsString();
            LOG_POST(Info << "Loading tables using format from asn file: " << 
                parms_file);
        }

        if (parsed_args["xform"].HasValue()) {
            table_xform = parsed_args["xform"].AsString();
            table_xform = NStr::ToLower(table_xform);
        }

        if (parsed_args["acc"].HasValue()) {
            accession = parsed_args["acc"].AsString();           
        }

        for (size_t idx = 1; idx<=parsed_args.GetNExtra(); ++idx) {
            string arg = parsed_args[idx].AsString();

            // If it is a file name, read it, convert it, and write it out as asn.
            // Otherwise print a message and skip it.

            CFile file(arg);

            // Ignore asn files since they are never tables and this program creates
            // them as ouput (allow user to mix them into directory).
            string ext = file.GetExt();           
            if (NStr::ToLower(ext) == ".asn") {
                cout << "Skipping asn file: " << arg << endl;
                LOG_POST(Info << "Skipping asn file: " << arg);
                continue;
            }

            if (file.IsFile()) {
                LoadTable(arg, parms_file, table_xform, accession);
            }
            else {
                cout << "file: " << arg << " is not a file name" << endl;
                LOG_POST(Error << "file: " << arg << 
                    " is not a file name.\n");
            }
        }
    }
    catch(CException& e) {
        LOG_POST(Error << "Error: " << e.GetMsg());     
    }
    catch(...) {
        LOG_POST(Error << "Undefined Error");
    }


    return 0;
}



int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // ----------------------------------------------------------
    CTestApplication theTestApplication;

    return theTestApplication.AppMain(argc, argv, 0 /*envp*/, eDS_ToMemory);

    return 0;
}

