/*  $Id: testmath.cpp 34670 2016-02-01 21:45:16Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbitime.hpp>

#include <gui/utils/vect3.hpp>
#include <gui/utils/matrix3.hpp>
#include <gui/utils/matrix4.hpp>
#include <util/math/matrix.hpp>


USING_SCOPE(ncbi);

/////////////////////////////////////////////////////////////////////////////
//  CTestmathApp::


class CTestmathApp : public CNcbiApplication
{
public:
    CTestmathApp();

private:
    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);
};


CTestmathApp::CTestmathApp()
{
}


/////////////////////////////////////////////////////////////////////////////
//  Init test for all different types of arguments


void CTestmathApp::Init(void)
{
    // Create command - line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "Math library test app");

    arg_desc->AddDefaultKey("iters", "Iterations",
                            "Number of iterations for timing tests",
                            CArgDescriptions::eInteger,
#ifdef _DEBUG
                            "5"
#else
                            "500"
#endif
                           );

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());

}



///
/// test vector addition
void testVectorAdd()
{
    cout << "testing vector addition...";
    cout.flush();

    {{
         CVect3<float> vec0(1.0f, 1.0f, 1.0f);
         CVect3<float> vec1(2.0f, 2.0f, 2.0f);

         vec0 = vec0 + vec1;
         _ASSERT(vec0[0] == 3.0f);
         _ASSERT(vec0[1] == 3.0f);
         _ASSERT(vec0[2] == 3.0f);
         if (vec0[0] != 3.0f  ||
             vec0[1] != 3.0f  ||
             vec0[2] != 3.0f) {
             string msg("vector addition of CVect3<> incorrect");
             NCBI_THROW(CException, eUnknown, msg);
         }
     }}

    {{
         CVect4<float> vec0(1.0f, 1.0f, 1.0f, 1.0f);
         CVect4<float> vec1(2.0f, 2.0f, 2.0f, 2.0f);

         vec0 = vec0 + vec1;
         _ASSERT(vec0[0] == 3.0f);
         _ASSERT(vec0[1] == 3.0f);
         _ASSERT(vec0[2] == 3.0f);
         _ASSERT(vec0[3] == 3.0f);
         if (vec0[0] != 3.0f  ||
             vec0[1] != 3.0f  ||
             vec0[2] != 3.0f  ||
             vec0[3] != 3.0f) {
             string msg("vector addition of CVect4<> incorrect");
             NCBI_THROW(CException, eUnknown, msg);
         }
     }}
    cout << "passed." << endl;
}


///
/// test matrix subtraction
void testVectorSubtract()
{
    cout << "testing vector subtraction...";
    cout.flush();

    {{
         CVect3<float> vec0(1.0f, 1.0f, 1.0f);
         CVect3<float> vec1(2.0f, 2.0f, 2.0f);

         vec0 = vec0 - vec1;
         _ASSERT(vec0[0] == -1.0f);
         _ASSERT(vec0[1] == -1.0f);
         _ASSERT(vec0[2] == -1.0f);
         if (vec0[0] != -1.0f  ||
             vec0[1] != -1.0f  ||
             vec0[2] != -1.0f) {
             string msg("vector subtraction of CVect3<> incorrect");
             NCBI_THROW(CException, eUnknown, msg);
         }
     }}

    {{
         CVect4<float> vec0(1.0f, 1.0f, 1.0f, 1.0f);
         CVect4<float> vec1(2.0f, 2.0f, 2.0f, 2.0f);

         vec0 = vec0 - vec1;
         _ASSERT(vec0[0] == -1.0f);
         _ASSERT(vec0[1] == -1.0f);
         _ASSERT(vec0[2] == -1.0f);
         _ASSERT(vec0[3] == -1.0f);
         if (vec0[0] != -1.0f  ||
             vec0[1] != -1.0f  ||
             vec0[2] != -1.0f  ||
             vec0[3] != -1.0f) {
             string msg("vector subtraction of CVect4<> incorrect");
             NCBI_THROW(CException, eUnknown, msg);
         }
     }}
    cout << "passed." << endl;
}



///
/// test matrix addition
void testMatrixAdd()
{
    cout << "testing matrix addition...";
    cout.flush();
    for (size_t r = 3;  r < 10;  ++r) {
        for (size_t c = 3;  c < 10;  ++c) {
            CNcbiMatrix<float> mat0(r, c, 1);
            CNcbiMatrix<float> mat1(r, c, 2);

            mat0 = mat0 + mat1;
            ITERATE (CNcbiMatrix<float>::TData, iter, mat0.GetData()) {
                _ASSERT(*iter == 3);
                if (*iter != 3) {
                    string msg("matrix addition of ");
                    msg += NStr::IntToString(r) + "x";
                    msg += NStr::IntToString(c) + " matrix incorrect";
                    NCBI_THROW(CException, eUnknown, msg);
                }
            }
        }
    }
    cout << "passed." << endl;
}


///
/// test matrix subtraction
void testMatrixSubtract()
{
    cout << "testing matrix subtraction...";
    cout.flush();
    for (size_t r = 3;  r < 10;  ++r) {
        for (size_t c = 3;  c < 10;  ++c) {
            CNcbiMatrix<float> mat0(r, c, 1);
            CNcbiMatrix<float> mat1(r, c, 2);

            mat0 = mat0 - mat1;
            ITERATE (CNcbiMatrix<float>::TData, iter, mat0.GetData()) {
                _ASSERT(*iter == -1);
                if (*iter != -1) {
                    string msg("matrix subtraction of ");
                    msg += NStr::IntToString(r) + "x";
                    msg += NStr::IntToString(c) + " matrix incorrect";
                    NCBI_THROW(CException, eUnknown, msg);
                }
            }
        }
    }
    cout << "passed." << endl;
}


///
/// test vecrix multiplication
void testMatrixMult()
{
    cout << "testing matrix multiplication...";
    cout.flush();

    for (size_t r = 3;  r < 10;  ++r) {
        for (size_t c = 3;  c < 10;  ++c) {
            const float lhs_val = 1;
            const float rhs_val = 2;

            CNcbiMatrix<float> mat0(r, c, lhs_val);
            CNcbiMatrix<float> mat1(c, r, rhs_val);

            mat0 = mat0 * mat1;

            float val = 0;
            for (size_t cc = 0;  cc < c;  ++cc) {
                val += lhs_val * rhs_val;
            }

            ITERATE (CNcbiMatrix<float>::TData, iter, mat0.GetData()) {
                _ASSERT(*iter == val);
                if (*iter != val) {
                    string msg("matrix multiplication of ");
                    msg += NStr::IntToString(r) + "x";
                    msg += NStr::IntToString(c) + " matrix incorrect";
                    NCBI_THROW(CException, eUnknown, msg);
                }
            }
        }
    }

    cout << "passed." << endl;
}


///
/// test determinant calculations
void testDeterminant()
{
    CNcbiMatrix<float> mat4(4,4);
    /**
      mat4[ 0] =  1;
      mat4[ 1] =  2;
      mat4[ 2] =  0;
      mat4[ 3] = -2;

      mat4[ 4] =  0;
      mat4[ 5] =  0;
      mat4[ 6] =  2;
      mat4[ 7] = -1;

      mat4[ 8] =  0;
      mat4[ 9] = -1;
      mat4[10] =  1;
      mat4[11] =  0;

      mat4[12] =  1;
      mat4[13] =  3;
      mat4[14] =  4;
      mat4[15] =  1;
     **/

    /// FIXME: implement determinants...
    //float det = mat4.determinant();
    //cout << "matrix is " << endl;
    //cout << mat4;
    //cout << "determinant is " << det << endl;

}


///
/// test the speed of matrix addition in the old and new matrix schemes
void testMatrixAddTime(size_t iters)
{
    cout << "testing matrix addition time..." << endl;
    CStopWatch sw;

    {{
         cout << "CMatrix3<>: ";
         cout.flush();
         CMatrix3<float> mat (1.0f, 1.0f, 1.0f,
                              1.0f, 1.0f, 1.0f,
                              1.0f, 1.0f, 1.0f);

         sw.Start();
         for (size_t i = 0;  i < iters;  ++i) {
             for (size_t j = 0;  j < iters;  ++j) {
                 mat += mat;
             }
         }
         cout << "elapsed time: " << sw.Elapsed() << endl;
     }}

    {{
         for (size_t r = 3;  r <= 10;  ++r) {
             cout << "CNcbiMatrix<>(" << r << ", " << r << "): ";
             cout.flush();
             CNcbiMatrix<float> mat (r, r, 1.0f);

             sw.Start();
             for (size_t i = 0;  i < iters;  ++i) {
                 for (size_t j = 0;  j < iters;  ++j) {
                     mat += mat;
                 }
             }
             cout << "elapsed time: " << sw.Elapsed() << endl;
         }
     }}
}


///
/// test the speed of vector addition in the old and new vector schemes
void testVectorAddTime(size_t iters)
{
    cout << "testing vector addition time..." << endl;
    CVect3<float> old_vect3 (1.0f, 1.0f, 1.0f);
    CStopWatch sw;

    sw.Start();
    for (size_t i = 0;  i < iters;  ++i) {
        for (size_t j = 0;  j < iters;  ++j) {
            old_vect3 += old_vect3;
        }
    }

    cout << "CVect3<>: elapsed time: " << sw.Elapsed() << endl;

#if 0
    std::vector<float> new_vect3 (3, 1.0f);
    sw.Start();
    for (size_t i = 0;  i < iters;  ++i) {
        for (size_t j = 0;  j < iters;  ++j) {
            new_vect3 += new_vect3;
        }
    }

    cout << "new vector3:" << endl;
    cout << "elapsed time: " << sw.Elapsed() << endl;
#endif
}


///
/// test the speed of vector addition in the old and new vector schemes
void testVectorDotTime(size_t iters)
{
    cout << "testing vector dot time..." << endl;
    float random = rand();
    CVect3<float> old_vect3 (random, random, random);
    float temp;
    CStopWatch sw;

    temp = 0;
    sw.Start();
    for (size_t i = 0;  i < iters;  ++i) {
        for (size_t j = 0;  j < iters;  ++j) {
            temp += old_vect3.Dot(old_vect3);
        }
    }

    cout << "CVect3<>: elapsed time: " << sw.Elapsed() << endl;

    temp = 0;
}


///
/// test the speed of matrix multiplication in the old and new matrix schemes
void testMatrixMultTime(size_t iters)
{
    cout << "testing matrix multiplication time..." << endl;
    CStopWatch sw;

    {{
         cout << "CMatrix4<>: ";
         cout.flush();
         CMatrix4<float> old_mat4 (1.0f, 1.0f, 1.0f, 1.0f,
                                   1.0f, 1.0f, 1.0f, 1.0f,
                                   1.0f, 1.0f, 1.0f, 1.0f,
                                   1.0f, 1.0f, 1.0f, 1.0f);
         sw.Start();
         for (size_t i = 0;  i < iters;  ++i) {
             for (size_t j = 0;  j < iters;  ++j) {
                 old_mat4 *= old_mat4;
             }
         }

         cout << "elapsed time: " << sw.Elapsed() << endl;
     }}

    {{
         for (size_t rc = 2;  rc <= 10;  ++rc) {
             cout << "CNcbiMatrix<>(" << rc << ", " << rc << "): ";
             cout.flush();
             CNcbiMatrix<float> mat(rc, rc, 1.0f);
             sw.Start();
             for (size_t i = 0;  i < iters;  ++i) {
                 for (size_t j = 0;  j < iters;  ++j) {
                     mat *= mat;
                 }
             }

             cout << "elapsed time: " << sw.Elapsed() << endl;
         }
     }}
}


///
/// test the speed of matrix multiplication in the old and new matrix schemes
void testMatrixMultVectorTime(size_t iters)
{
    cout << "testing matrix * vector multiplication time..." << endl;

    CStopWatch sw;

    {{
         CMatrix3<float> old_mat3 (1.0f, 1.0f, 1.0f,
                                   1.0f, 1.0f, 1.0f,
                                   1.0f, 1.0f, 1.0f);
         CVect3<float> old_vec3 (1.0f, 1.0f, 1.0f);

         cout << "CMatrx3<> * CVect3<>: ";
         cout.flush();
         sw.Start();
         for (size_t i = 0;  i < iters;  ++i) {
             for (size_t j = 0;  j < iters;  ++j) {
                 old_vec3 = old_vec3 * old_mat3;
             };
         }
         cout << "elapsed time: " << sw.Elapsed() << endl;
     }}

    {{
         for (size_t rc = 2;  rc <= 10;  ++rc) {
             std::vector<float> vec (rc, 1.0f);
             CNcbiMatrix<float> mat (rc, rc, 1.0f);
             cout << "CNcbiMatrix<>(" << rc << ", " << rc
                 << ") * vector<>(" << rc << "): ";
             cout.flush();
             sw.Start();
             for (size_t i = 0;  i < iters;  ++i) {
                 for (size_t j = 0;  j < iters;  ++j) {
                     vec = vec * mat;
                 };
             }
             cout << "elapsed time: " << sw.Elapsed() << endl;
         }
     }}
}



///
/// test promotion rules

void
testPromotion()
{
    cout << "sizeof(Promote(char, int)): "
        << sizeof(SPromoteTraits<char, int>::TPromote) << endl;
    cout << "sizeof(Promote(char, float)): "
        << sizeof(SPromoteTraits<char, float>::TPromote) << endl;
    cout << "sizeof(Promote(float, CVect3<float>)): "
        << sizeof(SPromoteTraits<float, CVect3<float> >::TPromote) << endl;

    CVect3<float> float_vec(1.1f, 1.1f, 1.1f);
    CVect3<int> int_vec(2, 3, 4);

    CVect3<float> res;

    res = int_vec + float_vec;

    cout << "float vec: "
        << float_vec[0] << ", " << float_vec[1] << ", " << float_vec[2] << endl;

    cout << "int vec: "
        << int_vec[0] << ", " << int_vec[1] << ", " << int_vec[2] << endl;

    res = float_vec + int_vec;
    cout << "f + i = "
        << res[0] << ", " << res[1] << ", " << res[2] << endl;

    res = int_vec + float_vec;
    cout << "i + f = "
        << res[0] << ", " << res[1] << ", " << res[2] << endl;
}

/////////////////////////////////////////////////////////////////////////////
//  Run test(printout arguments obtained from command - line)


int CTestmathApp::Run(void)
{
    // Get arguments
    CArgs args = GetArgs();

    size_t iters = args["iters"].AsInteger();

    testVectorAdd();
    testVectorSubtract();
    testMatrixAdd();
    testMatrixSubtract();
    testMatrixMult();

    testVectorAddTime(iters);
    testVectorDotTime(iters);
    testMatrixAddTime(iters);
    testMatrixMultVectorTime(iters);
    testMatrixMultTime(iters);

    testDeterminant();

    //testGaussJordan();
    //testGaussJordanTime();

    testPromotion();

    return 0;
}



/////////////////////////////////////////////////////////////////////////////
//  Cleanup


void CTestmathApp::Exit(void)
{
    SetDiagStream(0);
}



/////////////////////////////////////////////////////////////////////////////
//  MAIN


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return CTestmathApp().AppMain(argc, argv);
}
