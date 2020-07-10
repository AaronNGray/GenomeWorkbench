/*  $Id: test_asn_io.cpp 39421 2017-09-21 20:32:11Z katargir $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbitime.hpp>
#include <corelib/ncbifile.hpp>
#include <serial/objostr.hpp>
#include <serial/objistr.hpp>
#include <serial/serial.hpp>

#include <objects/seq/Seq_annot.hpp>


#include <util/math/matrix.hpp>


USING_SCOPE(ncbi);
USING_SCOPE(objects);

/////////////////////////////////////////////////////////////////////////////
//  CTestmathApp::


class CTestAsnIOApp : public CNcbiApplication
{
public:
    CTestAsnIOApp();

private:
    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);
};


CTestAsnIOApp::CTestAsnIOApp()
{
}


/////////////////////////////////////////////////////////////////////////////
//  Init test for all different types of arguments


void CTestAsnIOApp::Init(void)
{
    // Create command - line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "ASN (Seq-annot) serializing/deserializing test app");

    arg_desc->AddKey("f", "file", "ASN file (seq-annot)",
                     CArgDescriptions::eString);

    arg_desc->AddOptionalKey("s", "serial_format", "must be one of asnb or asnt",
                     CArgDescriptions::eString);

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());

}


/////////////////////////////////////////////////////////////////////////////
/// test the performance of reading and writing an asn file without creating
/// ASN objects.

static void s_TestPlainDataReadWrite(const string& file)
{
    cout << "testing reading the file and dumping it into a new file" << endl;
    CStopWatch sw;
    sw.Start();

    CNcbiIfstream f_in(file.c_str());
    CTmpFile temp_file;
    NcbiStreamCopy(temp_file.AsOutputFile(CTmpFile::eIfExists_Throw), f_in);

    cout << "elapsed time: " << sw.Elapsed() << endl;
}


/////////////////////////////////////////////////////////////////////////////
/// test the performance of serialization/deserialization of seq-annots

static void s_TestSeqAnnotReadAndWrite(const string& file, ESerialDataFormat fmt)
{
    vector< CRef<CSeq_annot> > annots;
    CStopWatch sw;
    sw.Start();

    try {
        cout << "testing deserializing seq-annots from an ASN file" << endl;
        auto_ptr<CObjectIStream> obj_istr(CObjectIStream::Open(fmt, file));
        while (obj_istr->HaveMoreData()) {
            CRef<CSeq_annot> annot;
            annot.Reset(new CSeq_annot);
            *obj_istr >> *annot;
            annots.push_back(annot);
        }

        cout << "elapsed time: " << sw.Elapsed() << endl;
        sw.Restart();

        cout << "testing serializing seq-annots to an ASN file" << endl;

        CTmpFile temp_file;
        auto_ptr<CObjectOStream> obj_ostr(
            CObjectOStream::Open(fmt, temp_file.AsOutputFile(CTmpFile::eIfExists_Throw)));
        ITERATE (vector< CRef<CSeq_annot> >, iter, annots) {
            *obj_ostr << **iter;
        }

        cout << "elapsed time: " << sw.Elapsed() << endl;

    } catch (CException& e) {
        cout << "Testing failed" << endl;
        cout << "Error: " << e.GetMsg() << endl;
        cout << "The input file must be  not a seq-annot ASN file: " << file << endl;;
    }
}


/////////////////////////////////////////////////////////////////////////////
//  Run test

int CTestAsnIOApp::Run(void)
{
    // Get arguments
    const CArgs& args = GetArgs();

    const string& file = args["f"].AsString();
    ESerialDataFormat fmt = eSerial_AsnBinary;
    if (NStr::EqualNocase(args["s"].AsString(), "asnt")) {
        fmt = eSerial_AsnText;
    }
    s_TestPlainDataReadWrite(file);
    s_TestSeqAnnotReadAndWrite(file, fmt);

    return 0;
}



/////////////////////////////////////////////////////////////////////////////
//  Cleanup

void CTestAsnIOApp::Exit(void)
{
    SetDiagStream(0);
}



/////////////////////////////////////////////////////////////////////////////
//  MAIN


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return CTestAsnIOApp().AppMain(argc, argv);
}
