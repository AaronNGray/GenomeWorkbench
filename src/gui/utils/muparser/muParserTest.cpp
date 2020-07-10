/*
                 __________                                      
    _____   __ __\______   \_____  _______  ______  ____ _______ 
   /     \ |  |  \|     ___/\__  \ \_  __ \/  ___/_/ __ \\_  __ \
  |  Y Y  \|  |  /|    |     / __ \_|  | \/\___ \ \  ___/ |  | \/
  |__|_|  /|____/ |____|    (____  /|__|  /____  > \___  >|__|   
        \/                       \/            \/      \/        
  Copyright (C) 2004-2008 Ingo Berg

  Permission is hereby granted, free of charge, to any person obtaining a copy of this 
  software and associated documentation files (the "Software"), to deal in the Software
  without restriction, including without limitation the rights to use, copy, modify, 
  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
  permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or 
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/
#include <ncbi_pch.hpp>
#include <gui/utils/muparser/muParserTest.h>

#include <cstdio>
#include <cmath>
#include <iostream>

#define PARSER_CONST_PI  3.141592653589793238462643
#define PARSER_CONST_E   2.718281828459045235360287

#define wxT(x) MUP_T(x)

using namespace std;

/** \file
    \brief This file contains the implementation of parser test cases.
*/


namespace mu
{
  namespace Test
  {
    int ParserTester::c_iCount = 0;

    //---------------------------------------------------------------------------
    ParserTester::ParserTester()
      :m_vTestFun()
    {
      AddTest(&ParserTester::TestNames);
      AddTest(&ParserTester::TestSyntax);
      AddTest(&ParserTester::TestPostFix);
      AddTest(&ParserTester::TestInfixOprt);
      AddTest(&ParserTester::TestVarConst);
      AddTest(&ParserTester::TestVolatile);
      AddTest(&ParserTester::TestMultiArg);
      AddTest(&ParserTester::TestExpression);
      AddTest(&ParserTester::TestInterface);
      AddTest(&ParserTester::TestBinOprt);
      AddTest(&ParserTester::TestException);
      AddTest(&ParserTester::TestStrArg);

      ParserTester::c_iCount = 0;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestInterface()
    {
      int iStat = 0;
      mu::console() << wxT("testing member functions...");
   
      // Test RemoveVar
      value_type afVal[3] = {1,2,3};
      Parser p;
  
      try
      {
        p.DefineVar( wxT("a"), &afVal[0]);
        p.DefineVar( wxT("b"), &afVal[1]);
        p.DefineVar( wxT("c"), &afVal[2]);
        p.SetExpr( wxT("a+b+c") );
        p.Eval();
      }
      catch(...)
      {
        iStat += 1;  // this is not supposed to happen 
      }

      try
      {
        p.RemoveVar( wxT("c") );
        p.Eval();
        iStat += 1;  // not supposed to reach this, nonexisting variable "c" deleted...
      }
      catch(...)
      {
        // failure is expected...
      }

      if (iStat==0) 
        mu::console() << wxT("passed") << endl;
      else 
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestStrArg()
    {
      int iStat = 0;
      mu::console() << wxT("testing string arguments...");
 
      iStat += EqnTest(wxT("valueof(\"aaa\")+valueof(\"bbb\")  "), 246, true);
      iStat += EqnTest(wxT("2*(valueof(\"aaa\")-23)+valueof(\"bbb\")"), 323, true);
      // use in expressions with variables
      iStat += EqnTest(wxT("a*(atof(\"10\")-b)"), 8, true);
      iStat += EqnTest(wxT("a-(atof(\"10\")*b)"), -19, true);
      // string + numeric arguments
      iStat += EqnTest(wxT("strfun1(\"100\")"), 100, true);
      iStat += EqnTest(wxT("strfun2(\"100\",1)"), 101, true);
      iStat += EqnTest(wxT("strfun3(\"99\",1,2)"), 102, true);

      if (iStat==0)
        mu::console() << wxT("passed") << endl;
      else 
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestBinOprt()
    {
      int iStat = 0;
      mu::console() << wxT("testing binary operators...");
   
      // built in operators
      // xor operator
      iStat += EqnTest(wxT("1 xor 2"), 3, true); 
      iStat += EqnTest(wxT("a xor b"), 3, true);            // with a=1 and b=2
      iStat += EqnTest(wxT("1 xor 2 xor 3"), 0, true); 
      iStat += EqnTest(wxT("a xor b xor 3"), 0, true);      // with a=1 and b=2
      iStat += EqnTest(wxT("a xor b xor c"), 0, true);      // with a=1 and b=2
      iStat += EqnTest(wxT("(1 xor 2) xor 3"), 0, true); 
      iStat += EqnTest(wxT("(a xor b) xor c"), 0, true);    // with a=1 and b=2
      iStat += EqnTest(wxT("(a) xor (b) xor c"), 0, true);  // with a=1 and b=2
      iStat += EqnTest(wxT("1 or 2"), 3, true); 
      iStat += EqnTest(wxT("a or b"), 3, true);             // with a=1 and b=2

      // Assignement operator
      iStat += EqnTest(wxT("a = b"), 2, true); 
      iStat += EqnTest(wxT("a = sin(b)"), 0.909297, true); 
      iStat += EqnTest(wxT("a = 1+sin(b)"), 1.909297, true);

      // Test user defined binary operators
      iStat += EqnTestInt(wxT("1 | 2"), 3, true);          
      iStat += EqnTestInt(wxT("1 || 2"), 1, true);          
      iStat += EqnTestInt(wxT("123 & 456"), 72, true);          
      iStat += EqnTestInt(wxT("(123 & 456) % 10"), 2, true);
      iStat += EqnTestInt(wxT("1 && 0"), 0, true);          
      iStat += EqnTestInt(wxT("123 && 456"), 1, true);          
      iStat += EqnTestInt(wxT("1 << 3"), 8, true);          
      iStat += EqnTestInt(wxT("8 >> 3"), 1, true);          
      iStat += EqnTestInt(wxT("10 ^ 10"), 0, true);          
      iStat += EqnTestInt(wxT("10 * 10 ^ 99"), 7, true);          
      iStat += EqnTestInt(wxT("9 / 4"), 2, true);  
      iStat += EqnTestInt(wxT("9 % 4"), 1, true);  
      iStat += EqnTestInt(wxT("if(5%2,1,0)"), 1, true);
      iStat += EqnTestInt(wxT("if(4%2,1,0)"), 0, true);
      iStat += EqnTestInt(wxT("-10+1"), -9, true);
      iStat += EqnTestInt(wxT("1+2*3"), 7, true);
      iStat += EqnTestInt(wxT("const1 != const2"), 1, true);
      iStat += EqnTestInt(wxT("const1 != const2"), 0, false);
      iStat += EqnTestInt(wxT("const1 == const2"), 0, true);
      iStat += EqnTestInt(wxT("const1 == 1"), 1, true);
      iStat += EqnTestInt(wxT("10*(const1 == 1)"), 10, true);
      iStat += EqnTestInt(wxT("2*(const1 | const2)"), 6, true);
      iStat += EqnTestInt(wxT("2*(const1 | const2)"), 7, false);
      iStat += EqnTestInt(wxT("const1 < const2"), 1, true);
      iStat += EqnTestInt(wxT("const2 > const1"), 1, true);
      iStat += EqnTestInt(wxT("const1 <= 1"), 1, true);
      iStat += EqnTestInt(wxT("const2 >= 2"), 1, true);
      iStat += EqnTestInt(wxT("2*(const1 + const2)"), 6, true);
      iStat += EqnTestInt(wxT("2*(const1 - const2)"), -2, true);

      iStat += EqnTestInt(wxT("a != b"), 1, true);
      iStat += EqnTestInt(wxT("a != b"), 0, false);
      iStat += EqnTestInt(wxT("a == b"), 0, true);
      iStat += EqnTestInt(wxT("a == 1"), 1, true);
      iStat += EqnTestInt(wxT("10*(a == 1)"), 10, true);
      iStat += EqnTestInt(wxT("2*(a | b)"), 6, true);
      iStat += EqnTestInt(wxT("2*(a | b)"), 7, false);
      iStat += EqnTestInt(wxT("a < b"), 1, true);
      iStat += EqnTestInt(wxT("b > a"), 1, true);
      iStat += EqnTestInt(wxT("a <= 1"), 1, true);
      iStat += EqnTestInt(wxT("b >= 2"), 1, true);
      iStat += EqnTestInt(wxT("2*(a + b)"), 6, true);
      iStat += EqnTestInt(wxT("2*(a - b)"), -2, true);
      iStat += EqnTestInt(wxT("a + (a << b)"), 5, true);
      iStat += EqnTestInt(wxT("-2^2"), -4, true);
// incorrect: '^' is yor here, not power
//    iStat += EqnTestInt("-(1+2)^2", -9, true);
//    iStat += EqnTestInt("-1^3", -1, true);          

      // Test precedence
      // a=1, b=2, c=3
      iStat += EqnTestInt(wxT("a + b * c"), 7, true);
      iStat += EqnTestInt(wxT("a * b + c"), 5, true);
      iStat += EqnTestInt(wxT("a<b && b>10"), 0, true);
      iStat += EqnTestInt(wxT("a<b && b<10"), 1, true);

      iStat += EqnTestInt(wxT("a + b << c"), 17, true);
      iStat += EqnTestInt(wxT("a << b + c"), 7, true);
      iStat += EqnTestInt(wxT("c * b < a"), 0, true);
      iStat += EqnTestInt(wxT("c * b == 6 * a"), 1, true);

      if (iStat==0)
        mu::console() << wxT("passed") << endl;
      else 
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    /** \brief Check muParser name restriction enforcement. */
    int ParserTester::TestNames()
    {
      int  iStat= 0,
           iErr = 0;

     mu::console() << "testing name restriction enforcement...";
    
      Parser p;

  #define PARSER_THROWCHECK(DOMAIN, FAIL, EXPR, ARG) \
      iErr = 0;                                      \
      ParserTester::c_iCount++;                      \
      try                                            \
      {                                              \
        p.Define##DOMAIN(EXPR, ARG);                 \
      }                                              \
      catch(Parser::exception_type&)                 \
      {                                              \
        iErr = (FAIL==false) ? 0 : 1;                \
      }                                              \
      iStat += iErr;      
      
      // constant names
      PARSER_THROWCHECK(Const, false, wxT("0a"), 1)
      PARSER_THROWCHECK(Const, false, wxT("9a"), 1)
      PARSER_THROWCHECK(Const, false, wxT("+a"), 1)
      PARSER_THROWCHECK(Const, false, wxT("-a"), 1)
      PARSER_THROWCHECK(Const, false, wxT("a-"), 1)
      PARSER_THROWCHECK(Const, false, wxT("a*"), 1)
      PARSER_THROWCHECK(Const, false, wxT("a?"), 1)
      PARSER_THROWCHECK(Const, true, wxT("a"), 1)
      PARSER_THROWCHECK(Const, true, wxT("a_min"), 1)
      PARSER_THROWCHECK(Const, true, wxT("a_min0"), 1)
      PARSER_THROWCHECK(Const, true, wxT("a_min9"), 1)
      // variable names
      value_type a;
      p.ClearConst();
      PARSER_THROWCHECK(Var, false, wxT("123abc"), &a)
      PARSER_THROWCHECK(Var, false, wxT("9a"), &a)
      PARSER_THROWCHECK(Var, false, wxT("0a"), &a)
      PARSER_THROWCHECK(Var, false, wxT("+a"), &a)
      PARSER_THROWCHECK(Var, false, wxT("-a"), &a)
      PARSER_THROWCHECK(Var, false, wxT("?a"), &a)
      PARSER_THROWCHECK(Var, false, wxT("!a"), &a)
      PARSER_THROWCHECK(Var, false, wxT("a+"), &a)
      PARSER_THROWCHECK(Var, false, wxT("a-"), &a)
      PARSER_THROWCHECK(Var, false, wxT("a*"), &a)
      PARSER_THROWCHECK(Var, false, wxT("a?"), &a)
      PARSER_THROWCHECK(Var, true, wxT("a"), &a)
      PARSER_THROWCHECK(Var, true, wxT("a_min"), &a)
      PARSER_THROWCHECK(Var, true, wxT("a_min0"), &a)
      PARSER_THROWCHECK(Var, true, wxT("a_min9"), &a)
      PARSER_THROWCHECK(Var, false, wxT("a_min9"), 0)
      // Postfix operators
      // fail
      PARSER_THROWCHECK(PostfixOprt, false, wxT("(k"), f1of1)
      PARSER_THROWCHECK(PostfixOprt, false, wxT("9+"), f1of1)
      PARSER_THROWCHECK(PostfixOprt, false, wxT("+"), 0)
      // pass
      PARSER_THROWCHECK(PostfixOprt, true, wxT("-a"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("?a"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("_"),   f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("#"),   f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("&&"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("||"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("&"),   f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("|"),   f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("++"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("--"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("?>"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("?<"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("**"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("xor"), f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("and"), f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("or"),  f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("not"), f1of1)
      PARSER_THROWCHECK(PostfixOprt, true, wxT("!"),   f1of1)
      // Binary operator
      // The following must fail with builtin operators activated
      // p.EnableBuiltInOp(true); -> this is the default
      PARSER_THROWCHECK(Oprt, false, wxT("+"),  f1of2)
      PARSER_THROWCHECK(Oprt, false, wxT("-"),  f1of2)
      PARSER_THROWCHECK(Oprt, false, wxT("*"),  f1of2)
      PARSER_THROWCHECK(Oprt, false, wxT("/"),  f1of2)
      // without activated built in operators it should work
      p.EnableBuiltInOprt(false);
      PARSER_THROWCHECK(Oprt, true, wxT("+"),  f1of2)
      PARSER_THROWCHECK(Oprt, true, wxT("-"),  f1of2)
      PARSER_THROWCHECK(Oprt, true, wxT("*"),  f1of2)
      PARSER_THROWCHECK(Oprt, true, wxT("/"),  f1of2)
  #undef PARSER_THROWCHECK

      if (iStat==0) 
        mu::console() << wxT("passed") << endl;
      else 
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestSyntax()
    {
      int iStat = 0;
      mu::console() << wxT("testing syntax engine...");

      iStat += EqnTest(wxT("(1+ 2*a)"), 3, true);   // Spaces within formula
      iStat += EqnTest(wxT("sqrt((4))"), 2, true);  // Multiple brackets
      iStat += EqnTest(wxT("sqrt((2)+2)"), 2, true);// Multiple brackets
      iStat += EqnTest(wxT("sqrt(2+(2))"), 2, true);// Multiple brackets
      iStat += EqnTest(wxT("sqrt(a+(3))"), 2, true);// Multiple brackets
      iStat += EqnTest(wxT("sqrt((3)+a)"), 2, true);// Multiple brackets
      iStat += EqnTest(wxT("order(1,2)"), 1, true); // May not cause name collision with operator "or"
      iStat += EqnTest(wxT("(2+"), 0, false);       // missing closing bracket 
      iStat += EqnTest(wxT("2++4"), 0, false);      // unexpected operator
      iStat += EqnTest(wxT("2+-4"), 0, false);      // unexpected operator
      iStat += EqnTest(wxT("(2+)"), 0, false);      // unexpected closing bracket
      iStat += EqnTest(wxT("--2"), 0, false);       // double sign
      iStat += EqnTest(wxT("ksdfj"), 0, false);     // unknown token
      iStat += EqnTest(wxT("()"), 0, false);        // empty bracket without a function
      iStat += EqnTest(wxT("5+()"), 0, false);      // empty bracket without a function
      iStat += EqnTest(wxT("sin(cos)"), 0, false);  // unexpected function
      iStat += EqnTest(wxT("5t6"), 0, false);       // unknown token
      iStat += EqnTest(wxT("5 t 6"), 0, false);     // unknown token
      iStat += EqnTest(wxT("8*"), 0, false);        // unexpected end of formula
      iStat += EqnTest(wxT(",3"), 0, false);        // unexpected comma
      iStat += EqnTest(wxT("3,5"), 0, false);       // unexpected comma
      iStat += EqnTest(wxT("sin(8,8)"), 0, false);  // too many function args
      iStat += EqnTest(wxT("(7,8)"), 0, false);     // too many function args
      iStat += EqnTest(wxT("sin)"), 0, false);      // unexpected closing bracket
      iStat += EqnTest(wxT("a)"), 0, false);        // unexpected closing bracket
      iStat += EqnTest(wxT("pi)"), 0, false);       // unexpected closing bracket
      iStat += EqnTest(wxT("sin(())"), 0, false);   // unexpected closing bracket
      iStat += EqnTest(wxT("sin()"), 0, false);     // unexpected closing bracket

      if (iStat==0)
        mu::console() << wxT("passed") << endl;
      else 
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestVarConst()
    {
      int iStat = 0;
      mu::console() << wxT("testing variable/constant name recognition...");

      // distinguish constants with same basename
      iStat += EqnTest( wxT("const"), 1, true);
      iStat += EqnTest( wxT("const1"), 2, true);
      iStat += EqnTest( wxT("const2"), 3, true);
      iStat += EqnTest( wxT("2*const"), 2, true);
      iStat += EqnTest( wxT("2*const1"), 4, true);
      iStat += EqnTest( wxT("2*const2"), 6, true);
      iStat += EqnTest( wxT("2*const+1"), 3, true);
      iStat += EqnTest( wxT("2*const1+1"), 5, true);
      iStat += EqnTest( wxT("2*const2+1"), 7, true);
      iStat += EqnTest( wxT("const"), 0, false);
      iStat += EqnTest( wxT("const1"), 0, false);
      iStat += EqnTest( wxT("const2"), 0, false);

      // distinguish variables with same basename
      iStat += EqnTest( wxT("a"), 1, true);
      iStat += EqnTest( wxT("aa"), 2, true);
      iStat += EqnTest( wxT("2*a"), 2, true);
      iStat += EqnTest( wxT("2*aa"), 4, true);
      iStat += EqnTest( wxT("2*a-1"), 1, true);
      iStat += EqnTest( wxT("2*aa-1"), 3, true);

      // Finally test querying of used variables
      try
      {
        int idx;
        mu::Parser p;
        mu::value_type vVarVal[] = { 1, 2, 3, 4, 5};
        p.DefineVar( wxT("a"), &vVarVal[0]);
        p.DefineVar( wxT("b"), &vVarVal[1]);
        p.DefineVar( wxT("c"), &vVarVal[2]);
        p.DefineVar( wxT("d"), &vVarVal[3]);
        p.DefineVar( wxT("e"), &vVarVal[4]);

        // Test lookup of defined variables
        // 4 used variables
        p.SetExpr( wxT("a+b+c+d") );
        mu::varmap_type UsedVar = p.GetUsedVar();
        int iCount = (int)UsedVar.size();
        if (iCount!=4) throw false;

        mu::varmap_type::const_iterator item = UsedVar.begin();
        for (idx=0; item!=UsedVar.end(); ++item)
        {
          if (&vVarVal[idx++]!=item->second) 
            throw false;
        }

        // Test lookup of undefined variables
        p.SetExpr( wxT("undef1+undef2+undef3") );
        UsedVar = p.GetUsedVar();
        iCount = (int)UsedVar.size();
        if (iCount!=3) throw false;

        for (item = UsedVar.begin(); item!=UsedVar.end(); ++item)
        {
          if (item->second!=0) 
            throw false; // all pointers to undefined variables must be null
        }

        // 1 used variables
        p.SetExpr( wxT("a+b") );
        UsedVar = p.GetUsedVar();
        iCount = (int)UsedVar.size();
        if (iCount!=2) throw false;
        item = UsedVar.begin();
        for (idx=0; item!=UsedVar.end(); ++item)
          if (&vVarVal[idx++]!=item->second) throw false;

      }
      catch(...)
      {
        iStat += 1;
      }

      if (iStat==0)  
        mu::console() << wxT("passed") << endl;
      else
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestMultiArg()
    {
      int iStat = 0;
      mu::console() << wxT("testing multiarg functions...");
    
      // picking the right argument
      iStat += EqnTest( wxT("f1of1(1)"), 1, true);
      iStat += EqnTest( wxT("f1of2(1, 2)"), 1, true);
      iStat += EqnTest( wxT("f2of2(1, 2)"), 2, true);
      iStat += EqnTest( wxT("f1of3(1, 2, 3)"), 1, true);
      iStat += EqnTest( wxT("f2of3(1, 2, 3)"), 2, true);
      iStat += EqnTest( wxT("f3of3(1, 2, 3)"), 3, true);
      iStat += EqnTest( wxT("f1of4(1, 2, 3, 4)"), 1, true);
      iStat += EqnTest( wxT("f2of4(1, 2, 3, 4)"), 2, true);
      iStat += EqnTest( wxT("f3of4(1, 2, 3, 4)"), 3, true);
      iStat += EqnTest( wxT("f4of4(1, 2, 3, 4)"), 4, true);
      iStat += EqnTest( wxT("f1of5(1, 2, 3, 4, 5)"), 1, true);
      iStat += EqnTest( wxT("f2of5(1, 2, 3, 4, 5)"), 2, true);
      iStat += EqnTest( wxT("f3of5(1, 2, 3, 4, 5)"), 3, true);
      iStat += EqnTest( wxT("f4of5(1, 2, 3, 4, 5)"), 4, true);
      iStat += EqnTest( wxT("f5of5(1, 2, 3, 4, 5)"), 5, true);
      // Too few arguments / Too many arguments
      iStat += EqnTest( wxT("1+ping()"), 11, true);
      iStat += EqnTest( wxT("ping()+1"), 11, true);
      iStat += EqnTest( wxT("2*ping()"), 20, true);
      iStat += EqnTest( wxT("ping()*2"), 20, true);
      iStat += EqnTest( wxT("ping(1,2)"), 0, false);
      iStat += EqnTest( wxT("1+ping(1,2)"), 0, false);
      iStat += EqnTest( wxT("f1of1(1,2)"), 0, false);
      iStat += EqnTest( wxT("f1of1()"), 0, false);
      iStat += EqnTest( wxT("f1of2(1, 2, 3)"), 0, false);
      iStat += EqnTest( wxT("f1of2(1)"), 0, false);
      iStat += EqnTest( wxT("f1of3(1, 2, 3, 4)"), 0, false);
      iStat += EqnTest( wxT("f1of3(1)"), 0, false);
      iStat += EqnTest( wxT("f1of4(1, 2, 3, 4, 5)"), 0, false);
      iStat += EqnTest( wxT("f1of4(1)"), 0, false);
      iStat += EqnTest( wxT("(1,2,3)"), 0, false);
      iStat += EqnTest( wxT("1,2,3"), 0, false);
      iStat += EqnTest( wxT("(1*a,2,3)"), 0, false);
      iStat += EqnTest( wxT("1,2*a,3"), 0, false);
     
      // correct calculation of arguments
      iStat += EqnTest( wxT("min(a, 1)"),  1, true);
      iStat += EqnTest( wxT("min(3*2, 1)"),  1, true);
      iStat += EqnTest( wxT("min(3*2, 1)"),  6, false);
      iStat += EqnTest( wxT("firstArg(2,3,4)"), 2, true);
      iStat += EqnTest( wxT("lastArg(2,3,4)"), 4, true);
      iStat += EqnTest( wxT("min(3*a+1, 1)"),  1, true);
      iStat += EqnTest( wxT("max(3*a+1, 1)"),  4, true);
      iStat += EqnTest( wxT("max(3*a+1, 1)*2"),  8, true);
      iStat += EqnTest( wxT("2*max(3*a+1, 1)+2"),  10, true);

      // functions with Variable argument count
      iStat += EqnTest( wxT("sum(a)"), 1, true);
      iStat += EqnTest( wxT("sum(1,2,3)"),  6, true);
      iStat += EqnTest( wxT("sum(a,b,c)"),  6, true);
      iStat += EqnTest( wxT("sum(1,-max(1,2),3)*2"),  4, true);
      iStat += EqnTest( wxT("2*sum(1,2,3)"),  12, true);
      iStat += EqnTest( wxT("2*sum(1,2,3)+2"),  14, true);
      iStat += EqnTest( wxT("2*sum(-1,2,3)+2"),  10, true);
      iStat += EqnTest( wxT("2*sum(-1,2,-(-a))+2"),  6, true);
      iStat += EqnTest( wxT("2*sum(-1,10,-a)+2"),  18, true);
      iStat += EqnTest( wxT("2*sum(1,2,3)*2"),  24, true);
      iStat += EqnTest( wxT("sum(1,-max(1,2),3)*2"),  4, true);
      iStat += EqnTest( wxT("sum(1*3, 4, a+2)"),  10, true);
      iStat += EqnTest( wxT("sum(1*3, 2*sum(1,2,2), a+2)"),  16, true);
      iStat += EqnTest( wxT("sum(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2)"), 24, true);

      // some failures
      iStat += EqnTest( wxT("sum()"),  0, false);
      iStat += EqnTest( wxT("sum(,)"),  0, false);
      iStat += EqnTest( wxT("sum(1,2,)"),  0, false);
      iStat += EqnTest( wxT("sum(,1,2)"),  0, false);

      if (iStat==0) 
        mu::console() << wxT("passed") << endl;
      else
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;
  
      return iStat;
    }


    //---------------------------------------------------------------------------
    int ParserTester::TestInfixOprt()
    {
      int iStat(0);
      mu::console() << "testing infix operators...";

      iStat += EqnTest( wxT("-1"),    -1, true);
      iStat += EqnTest( wxT("-(-1)"),  1, true);
      iStat += EqnTest( wxT("-(-1)*2"),  2, true);
      iStat += EqnTest( wxT("-(-2)*sqrt(4)"),  4, true);
      iStat += EqnTest( wxT("-a"),  -1, true);
      iStat += EqnTest( wxT("-(a)"),  -1, true);
      iStat += EqnTest( wxT("-(-a)"),  1, true);
      iStat += EqnTest( wxT("-(-a)*2"),  2, true);
      iStat += EqnTest( wxT("-(8)"), -8, true);
      iStat += EqnTest( wxT("-8"), -8, true);
      iStat += EqnTest( wxT("-(2+1)"), -3, true);
      iStat += EqnTest( wxT("-(f1of1(1+2*3)+1*2)"), -9, true);
      iStat += EqnTest( wxT("-(-f1of1(1+2*3)+1*2)"), 5, true);
      iStat += EqnTest( wxT("-sin(8)"), -0.989358, true);
      iStat += EqnTest( wxT("3-(-a)"), 4, true);
      iStat += EqnTest( wxT("3--a"), 4, true);
  
      // Postfix / infix priorities
      iStat += EqnTest( wxT("~2#"), 8, true);
      iStat += EqnTest( wxT("~f1of1(2)#"), 8, true);
      iStat += EqnTest( wxT("~(b)#"), 8, true);
      iStat += EqnTest( wxT("(~b)#"), 12, true);
      iStat += EqnTest( wxT("~(2#)"), 8, true);
      iStat += EqnTest( wxT("~(f1of1(2)#)"), 8, true);
      //
      iStat += EqnTest( wxT("-2^2"),-4, true);
      iStat += EqnTest( wxT("-(a+b)^2"),-9, true);
      iStat += EqnTest( wxT("(-3)^2"),9, true);
      iStat += EqnTest( wxT("-(-2^2)"),4, true);
      iStat += EqnTest( wxT("3+-3^2"),-6, true);
      // The following assumes use of sqr as postfix operator ("?") together
      // withn a sign operator of low priority:
      iStat += EqnTest( wxT("-2?"), -4, true);
      iStat += EqnTest( wxT("-(1+1)?"),-4, true);
      iStat += EqnTest( wxT("2+-(1+1)?"),-2, true);
      iStat += EqnTest( wxT("2+-2?"), -2, true);
      // This is the classic behaviour of the infix sign operator (here: "$") which is
      // now deprecated:
      iStat += EqnTest( wxT("$2^2"),4, true);
      iStat += EqnTest( wxT("$(a+b)^2"),9, true);
      iStat += EqnTest( wxT("($3)^2"),9, true);
      iStat += EqnTest( wxT("$($2^2)"),-4, true);
      iStat += EqnTest( wxT("3+$3^2"),12, true);

      if (iStat==0)
        mu::console() << wxT("passed") << endl;
      else
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;

      return iStat;
    }


    //---------------------------------------------------------------------------
    int ParserTester::TestPostFix()
    {
      int iStat = 0;
      mu::console() << wxT("testing postfix operators...");

      // application
      iStat += EqnTest( wxT("3m+5"), 5.003, true);
      iStat += EqnTest( wxT("1000m"), 1, true);
      iStat += EqnTest( wxT("1000 m"), 1, true);
      iStat += EqnTest( wxT("(a)m"), 1e-3, true);
      iStat += EqnTest( wxT("-(a)m"), -1e-3, true);
      iStat += EqnTest( wxT("-2m"), -2e-3, true);
      iStat += EqnTest( wxT("f1of1(1000)m"), 1, true);
      iStat += EqnTest( wxT("-f1of1(1000)m"), -1, true);
      iStat += EqnTest( wxT("-f1of1(-1000)m"), 1, true);
      iStat += EqnTest( wxT("f4of4(0,0,0,1000)m"), 1, true);
      iStat += EqnTest( wxT("2+(a*1000)m"), 3, true);
      // some incorrect results
      iStat += EqnTest( wxT("1000m"), 0.1, false);
      iStat += EqnTest( wxT("(a)m"), 2, false);
      // failure due to syntax checking
      iStat += EqnTest( wxT("a m"), 0, false);
      iStat += EqnTest( wxT("4 + m"), 0, false);
      iStat += EqnTest( wxT("m4"), 0, false);
      iStat += EqnTest( wxT("sin(m)"), 0, false);
      iStat += EqnTest( wxT("m m"), 0, false);
      iStat += EqnTest( wxT("m(8)"), 0, false);
      iStat += EqnTest( wxT("4,m"), 0, false);
      iStat += EqnTest( wxT("-m"), 0, false);
      iStat += EqnTest( wxT("2(-m)"), 0, false);
      iStat += EqnTest( wxT("2(m)"), 0, false);

      if (iStat==0)
        mu::console() << wxT("passed") << endl;
      else
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    /** \brief Test volatile (nonoptimizeable functions). */
    int ParserTester::TestVolatile()
    {
      int iStat = 0;
      mu::console() << "testing volatile/nonvolatile functions...";

      // First test with volatile flag turned on
      try
      {
        mu::Parser p;
        p.DefineFun( wxT("rnd"), Rnd, false);
        p.DefineFun( wxT("valueof"), RndWithString, false);

        // 1st test, compare results from sucessive calculations
        p.SetExpr( wxT("3+rnd(8)") );
        if (p.Eval()==p.Eval()) iStat += 1;

        // 2nd test, force bytecode creation, compare two results both 
        // calculated from bytecode
        p.SetExpr( wxT("3+rnd(8)") ); 
        p.Eval(); //<- Force bytecode creation
        if (p.Eval()==p.Eval()) iStat += 1;

        p.SetExpr( wxT("3*rnd(8)+3") );
        p.Eval(); //<- Force bytecode creation
        if (p.Eval()==p.Eval()) iStat += 1;

        p.SetExpr( wxT("10+3*sin(rnd(8))-1") );
        p.Eval(); //<- Force bytecode creation
        if (p.Eval()==p.Eval()) iStat += 1;

        p.SetExpr( wxT("3+rnd(rnd(8))*2") );
        p.Eval(); //<- Force bytecode creation
        if (p.Eval()==p.Eval()) iStat += 1;

        p.SetExpr( wxT("valueof(\"Das ist ein Test\")") );
        p.Eval(); //<- Force bytecode creation
        if (p.Eval()==p.Eval()) iStat += 1;
      }
      catch(Parser::exception_type &e)
      {
        mu::console() << wxT("\n  ") << e.GetExpr() << wxT(" : ") << e.GetMsg();
        iStat += 1;
      }

      // Second test with volatile flag turned off
      try
      {
        mu::Parser p;
        p.DefineFun( wxT("rnd"), Rnd);
        p.DefineFun( wxT("valueof"), RndWithString);

        // compare string parsing with bytecode
        p.SetExpr( wxT("3+rnd(8)") ); 
        if (p.Eval()!=p.Eval()) iStat += 1;

        p.SetExpr( wxT("3+rnd(8)") ); 
        p.Eval(); //<- Force bytecode creation
        if (p.Eval()!=p.Eval()) iStat += 1;

        p.SetExpr( wxT("3*rnd(8)+3") );
        p.Eval(); //<- Force bytecode creation
        if (p.Eval()!=p.Eval()) iStat += 1;

        p.SetExpr( wxT("10+3*sin(rnd(8))-1") );
        p.Eval(); //<- Force bytecode creation
        if (p.Eval()!=p.Eval()) iStat += 1;

        p.SetExpr( wxT("3+rnd(rnd(8))*2") );
        p.Eval(); //<- Force bytecode creation
        if (p.Eval()!=p.Eval()) iStat += 1;
      }
      catch(Parser::exception_type &e)
      {
        mu::console() << wxT("\n  ") << e.GetExpr() << wxT(" : ") << e.GetMsg();
        iStat += 1;
      }
      
      if (iStat==0)
        mu::console() << wxT("passed") << endl;
      else
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;

      return iStat;
    }

    //---------------------------------------------------------------------------
    int ParserTester::TestExpression()
    {
      int iStat = 0;
      mu::console() << wxT("testing sample formulas...");

      // operator precedencs
      iStat += EqnTest( wxT("1+2-3*4/5^6"), 2.99923, true);
      iStat += EqnTest( wxT("1^2/3*4-5+6"), 2.3333, true);
      iStat += EqnTest( wxT("1+2*3"), 7, true);
      iStat += EqnTest( wxT("1+2*3"), 7, true);
      iStat += EqnTest( wxT("(1+2)*3"), 9, true);
      iStat += EqnTest( wxT("(1+2)*(-3)"), -9, true);
      iStat += EqnTest( wxT("2/4"), 0.5, true);

      iStat += EqnTest( wxT("exp(ln(7))"), 7, true);
      iStat += EqnTest( wxT("e^ln(7)"), 7, true);
      iStat += EqnTest( wxT("e^(ln(7))"), 7, true);
      iStat += EqnTest( wxT("(e^(ln(7)))"), 7, true);
      iStat += EqnTest( wxT("1-(e^(ln(7)))"), -6, true);
      iStat += EqnTest( wxT("2*(e^(ln(7)))"), 14, true);
      iStat += EqnTest( wxT("10^log(5)"), 5, true);
      iStat += EqnTest( wxT("10^log10(5)"), 5, true);
      iStat += EqnTest( wxT("2^log2(4)"), 4, true);
      iStat += EqnTest( wxT("-(sin(0)+1)"), -1, true);
      iStat += EqnTest( wxT("-(2^1.1)"), -2.14354692, true);

      iStat += EqnTest( wxT("(cos(2.41)/b)"), -0.372056, true);

#if !defined(MUP_UNICODE)
      // I can't translate the following two tests to unicode without loosing 
      // readability.

      // long formula (Reference: Matlab)
      iStat += EqnTest(
        "(((-9))-e/(((((((pi-(((-7)+(-3)/4/e))))/(((-5))-2)-((pi+(-0))*(sqrt((e+e))*(-8))*(((-pi)+(-pi)-(-9)*(6*5))"
        "/(-e)-e))/2)/((((sqrt(2/(-e)+6)-(4-2))+((5/(-2))/(1*(-pi)+3))/8)*pi*((pi/((-2)/(-6)*1*(-1))*(-6)+(-e)))))/"
        "((e+(-2)+(-e)*((((-3)*9+(-e)))+(-9)))))))-((((e-7+(((5/pi-(3/1+pi)))))/e)/(-5))/(sqrt((((((1+(-7))))+((((-"
        "e)*(-e)))-8))*(-5)/((-e)))*(-6)-((((((-2)-(-9)-(-e)-1)/3))))/(sqrt((8+(e-((-6))+(9*(-9))))*(((3+2-8))*(7+6"
        "+(-5))+((0/(-e)*(-pi))+7)))+(((((-e)/e/e)+((-6)*5)*e+(3+(-5)/pi))))+pi))/sqrt((((9))+((((pi))-8+2))+pi))/e"
        "*4)*((-5)/(((-pi))*(sqrt(e)))))-(((((((-e)*(e)-pi))/4+(pi)*(-9)))))))+(-pi)", -12.23016549, true);

      // long formula (Reference: Matlab)
      iStat += EqnTest(
          "(atan(sin((((((((((((((((pi/cos((a/((((0.53-b)-pi)*e)/b))))+2.51)+a)-0.54)/0.98)+b)*b)+e)/a)+b)+a)+b)+pi)/e"
          ")+a)))*2.77)", -2.16995656, true);
#endif

      // long formula (Reference: Matlab)
      iStat += EqnTest( wxT("1+2-3*4/5^6*(2*(1-5+(3*7^9)*(4+6*7-3)))+12"), -7995810.09926, true);
	  
      if (iStat==0) 
        mu::console() << wxT("passed") << endl;  
      else 
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;

      return iStat;
    }


    //---------------------------------------------------------------------------
    int ParserTester::TestException()
    {
      int  iStat = 0;
      mu::console() << wxT("testing error codes...");

      iStat += ThrowTest(wxT("3+"),           ecUNEXPECTED_EOF);
      iStat += ThrowTest(wxT("3+)"),          ecUNEXPECTED_PARENS);
      iStat += ThrowTest(wxT("()"),           ecUNEXPECTED_PARENS);
      iStat += ThrowTest(wxT("3+()"),         ecUNEXPECTED_PARENS);
      iStat += ThrowTest(wxT("sin(3,4)"),     ecTOO_MANY_PARAMS);
      iStat += ThrowTest(wxT("3,4"),          ecUNEXPECTED_ARG_SEP);
      iStat += ThrowTest(wxT("if(3)"),        ecTOO_FEW_PARAMS);
      iStat += ThrowTest(wxT("(1+2"),         ecMISSING_PARENS);
      iStat += ThrowTest(wxT("sin(3)3"),      ecUNEXPECTED_VAL);
      iStat += ThrowTest(wxT("sin(3)xyz"),    ecUNASSIGNABLE_TOKEN);
      iStat += ThrowTest(wxT("sin(3)cos(3)"), ecUNEXPECTED_FUN);
      iStat += ThrowTest(wxT("a+b+c=10"),     ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest(wxT("a=b=3"),        ecUNEXPECTED_OPERATOR);

      // functions without parameter
      iStat += ThrowTest( wxT("3+ping(2)"),    ecTOO_MANY_PARAMS);
      iStat += ThrowTest( wxT("3+ping(a+2)"),  ecTOO_MANY_PARAMS);
      iStat += ThrowTest( wxT("3+ping(sin(a)+2)"),  ecTOO_MANY_PARAMS);
      iStat += ThrowTest( wxT("3+ping(1+sin(a))"),  ecTOO_MANY_PARAMS);

      // String function related
      iStat += ThrowTest( wxT("valueof(\"xxx\")"),  999, false);
      iStat += ThrowTest( wxT("valueof()"),          ecUNEXPECTED_PARENS);
      iStat += ThrowTest( wxT("1+valueof(\"abc\""),  ecMISSING_PARENS);
      iStat += ThrowTest( wxT("valueof(\"abc\""),    ecMISSING_PARENS);
      iStat += ThrowTest( wxT("valueof(\"abc"),      ecUNTERMINATED_STRING);
      iStat += ThrowTest( wxT("valueof(\"abc\",3)"), ecTOO_MANY_PARAMS);
      iStat += ThrowTest( wxT("valueof(3)"),         ecSTRING_EXPECTED);
      iStat += ThrowTest( wxT("sin(\"abc\")"),       ecVAL_EXPECTED);
      iStat += ThrowTest( wxT("valueof(\"\\\"abc\\\"\")"),  999, false);
      iStat += ThrowTest( wxT("\"hello world\""),    ecSTR_RESULT);
      iStat += ThrowTest( wxT("(\"hello world\")"),  ecSTR_RESULT);
      iStat += ThrowTest( wxT("\"abcd\"+100"),       ecOPRT_TYPE_CONFLICT);
      iStat += ThrowTest( wxT("\"a\"+\"b\""),        ecOPRT_TYPE_CONFLICT);
      iStat += ThrowTest( wxT("strfun1(\"100\",3)"),     ecTOO_MANY_PARAMS);
      iStat += ThrowTest( wxT("strfun2(\"100\",3,5)"),   ecTOO_MANY_PARAMS);
      iStat += ThrowTest( wxT("strfun3(\"100\",3,5,6)"), ecTOO_MANY_PARAMS);
      iStat += ThrowTest( wxT("strfun2(\"100\")"),       ecTOO_FEW_PARAMS);
      iStat += ThrowTest( wxT("strfun3(\"100\",6)"),   ecTOO_FEW_PARAMS);
      iStat += ThrowTest( wxT("strfun2(1,1)"),         ecSTRING_EXPECTED);
      iStat += ThrowTest( wxT("strfun2(a,1)"),         ecSTRING_EXPECTED);
      iStat += ThrowTest( wxT("strfun2(1,1,1)"),       ecTOO_MANY_PARAMS);
      iStat += ThrowTest( wxT("strfun2(a,1,1)"),       ecTOO_MANY_PARAMS);
      iStat += ThrowTest( wxT("strfun3(1,2,3)"),         ecSTRING_EXPECTED);
      iStat += ThrowTest( wxT("strfun3(1, \"100\",3)"),  ecSTRING_EXPECTED);
      iStat += ThrowTest( wxT("strfun3(\"1\", \"100\",3)"),  ecVAL_EXPECTED);
      iStat += ThrowTest( wxT("strfun3(\"1\", 3, \"100\")"),  ecVAL_EXPECTED);
      iStat += ThrowTest( wxT("strfun3(\"1\", \"100\", \"100\", \"100\")"),  ecTOO_MANY_PARAMS);

      // assignement operator
      iStat += ThrowTest( wxT("3=4"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( wxT("sin(8)=4"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( wxT("\"test\"=a"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( wxT("sin=9"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( wxT("(8)=5"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( wxT("(a)=5"), ecUNEXPECTED_OPERATOR);
      iStat += ThrowTest( wxT("a=\"tttt\""), ecOPRT_TYPE_CONFLICT);

      if (iStat==0) 
        mu::console() << wxT("passed") << endl;
      else 
        mu::console() << wxT("\n  failed with ") << iStat << wxT(" errors") << endl;

      return iStat;
    }


    //---------------------------------------------------------------------------
    void ParserTester::AddTest(testfun_type a_pFun)
    {
      m_vTestFun.push_back(a_pFun);
    }

    //---------------------------------------------------------------------------
    void ParserTester::Run()
    {
      int iStat = 0;
      try
      {
        for (int i=0; i<(int)m_vTestFun.size(); ++i)
          iStat += (this->*m_vTestFun[i])();
      }
      catch(Parser::exception_type &e)
      {
        mu::console() << "\n" << e.GetMsg() << endl;
        mu::console() << e.GetToken() << endl;
        Abort();
      }
      catch(std::exception &e)
      {
        mu::console() << e.what() << endl;
        Abort();
      }
      catch(...)
      {
        mu::console() << "Internal error";
        Abort();
      }

      if (iStat==0) 
      {
        mu::console() << "Test passed (" <<  ParserTester::c_iCount << " expressions)" << endl;
      }
      else 
      {
        mu::console() << "Test failed with " << iStat 
                  << " errors (" <<  ParserTester::c_iCount 
                  << " expressions)" << endl;
      }
      ParserTester::c_iCount = 0;
    }


    //---------------------------------------------------------------------------
    int ParserTester::ThrowTest(const string_type &a_str, int a_iErrc, bool a_bFail)
    {
      ParserTester::c_iCount++;

      try
      {
        value_type fVal[] = {1,1,1};
        Parser p;

        p.DefineVar( wxT("a"), &fVal[0]);
        p.DefineVar( wxT("b"), &fVal[1]);
        p.DefineVar( wxT("c"), &fVal[2]);

        p.DefineFun( wxT("ping"), Ping);
        p.DefineFun( wxT("valueof"), ValueOf);
        p.DefineFun( wxT("strfun1"), StrFun1);
        p.DefineFun( wxT("strfun2"), StrFun2);
        p.DefineFun( wxT("strfun3"), StrFun3);
        p.SetExpr(a_str);
        p.Eval();
      }
      catch(Parser::exception_type &e)
      {
        // output the formula in case of an failed test
        if (a_bFail==false || (a_bFail==true && a_iErrc!=e.GetCode()) )
        {
          mu::console() << wxT("\n  ") 
                        << wxT("Expression: ") << a_str 
                        << wxT("  Code:") << e.GetCode() 
                        << wxT("  Expected:") << a_iErrc;
        }

        return (a_iErrc==e.GetCode()) ? 0 : 1;
      }

      // if a_bFail==false no exception is expected
      bool bRet((a_bFail==false) ? 0 : 1);
      if (bRet==1)
      {
        mu::console() << wxT("\n  ") 
                      << wxT("Expression: ") << a_str 
                      << wxT("  did evaluate; Expected error:") << a_iErrc;
      }

      return bRet; 
    }

    //---------------------------------------------------------------------------
    /** \brief Evaluate a tet expression. 

        \return 1 in case of a failure, 0 otherwise.
    */
    int ParserTester::EqnTest(const string_type &a_str, double a_fRes, bool a_fPass)
    {
      ParserTester::c_iCount++;
      int iRet(0);

      try
      {
        Parser *p1, p2, p3;   // three parser objects
                              // they will be used for testing copy and assihnment operators
        // p1 is a pointer since i'm going to delete it in order to test if
        // parsers after copy construction still refer to members of it.
        // !! If this is the case this function will crash !!
      
        p1 = new mu::Parser(); 
        // Add constants
        p1->DefineConst( wxT("pi"), (value_type)PARSER_CONST_PI);
        p1->DefineConst( wxT("e"), (value_type)PARSER_CONST_E);
        p1->DefineConst( wxT("const"), 1);
        p1->DefineConst( wxT("const1"), 2);
        p1->DefineConst( wxT("const2"), 3);
        // variables
        value_type vVarVal[] = { 1, 2, 3, -2};
        p1->DefineVar( wxT("a"), &vVarVal[0]);
        p1->DefineVar( wxT("aa"), &vVarVal[1]);
        p1->DefineVar( wxT("b"), &vVarVal[1]);
        p1->DefineVar( wxT("c"), &vVarVal[2]);
        p1->DefineVar( wxT("d"), &vVarVal[3]);
        // functions
        p1->DefineFun( wxT("ping"), Ping);
        p1->DefineFun( wxT("f1of1"), f1of1);  // one parameter
        p1->DefineFun( wxT("f1of2"), f1of2);  // two parameter
        p1->DefineFun( wxT("f2of2"), f2of2);
        p1->DefineFun( wxT("f1of3"), f1of3);  // three parameter
        p1->DefineFun( wxT("f2of3"), f2of3);
        p1->DefineFun( wxT("f3of3"), f3of3);
        p1->DefineFun( wxT("f1of4"), f1of4);  // four parameter
        p1->DefineFun( wxT("f2of4"), f2of4);
        p1->DefineFun( wxT("f3of4"), f3of4);
        p1->DefineFun( wxT("f4of4"), f4of4);
        p1->DefineFun( wxT("f1of5"), f1of5);  // five parameter
        p1->DefineFun( wxT("f2of5"), f2of5);
        p1->DefineFun( wxT("f3of5"), f3of5);
        p1->DefineFun( wxT("f4of5"), f4of5);
        p1->DefineFun( wxT("f5of5"), f5of5);
        // sample functions
        p1->DefineFun( wxT("min"), Min);
        p1->DefineFun( wxT("max"), Max);
        p1->DefineFun( wxT("sum"), Sum);
        p1->DefineFun( wxT("valueof"), ValueOf);
        p1->DefineFun( wxT("atof"), StrToFloat);
        p1->DefineFun( wxT("strfun1"), StrFun1);
        p1->DefineFun( wxT("strfun2"), StrFun2);
        p1->DefineFun( wxT("strfun3"), StrFun3);
        p1->DefineFun( wxT("lastArg"), LastArg);
        p1->DefineFun( wxT("firstArg"), FirstArg);
        p1->DefineFun( wxT("order"), FirstArg);

        // infix / postfix operator
        // (identifiers used here do not have any meaning or make any sense at all)
        p1->DefineInfixOprt( wxT("$"), sign, prPOW+1);  // sign with high priority
        p1->DefineInfixOprt( wxT("~"), plus2);          // high priority
        p1->DefinePostfixOprt( wxT("m"), Milli);
        p1->DefinePostfixOprt( wxT("#"), times3);
        p1->DefinePostfixOprt( wxT("?"), sqr);  //
        p1->SetExpr(a_str);

        // Test bytecode integrity
        // String parsing and bytecode parsing must yield the same result
        value_type fVal[4] = {-999, -998, -997, -996}; // initially should be different
        fVal[0] = p1->Eval(); // result from stringparsing
        fVal[1] = p1->Eval(); // result from bytecode
        if (fVal[0]!=fVal[1])
          throw Parser::exception_type( wxT("Bytecode / string parsing mismatch.") );

        // Test copy and assignement operators
        try
        {
          // Test copy constructor
          std::vector<mu::Parser> vParser;
          vParser.push_back(*p1);
          mu::Parser p2 = vParser[0];   // take parser from vector
        
          // destroy the originals from p2
          vParser.clear();              // delete the vector
          delete p1;                    // delete the original
          p1 = 0;

          fVal[2] = p2.Eval();

          // Test assignement operator
          // additionally  disable Optimizer this time
          mu::Parser p3;
          p3 = p2;
          p3.EnableOptimizer(false);
          fVal[3] = p3.Eval();
        }
        catch(std::exception &e)
        {
          mu::console() << wxT("\n  ") << e.what() << wxT("\n");
        }

        // limited floating point accuracy requires the following test
        bool bCloseEnough(true);
        for (int i=0; i<4; ++i)
        {
          bCloseEnough &= (fabs(a_fRes-fVal[i]) <= fabs(fVal[i]*0.0001));
        }

        iRet = ((bCloseEnough && a_fPass) || (!bCloseEnough && !a_fPass)) ? 0 : 1;
        if (iRet==1)
        {
          mu::console() << wxT("\n  fail: ") << a_str.c_str() 
                        << wxT(" (incorrect result; expected: ") << a_fRes
                        << wxT(" ;calculated: ") << fVal[0]<< wxT(").");
        }
      }
      catch(Parser::exception_type &e)
      {
        if (a_fPass)
        {
          mu::console() << wxT("\n  fail: ") << a_str.c_str() << wxT(" (") << e.GetMsg() << wxT(")");
          return 1;
        }
      }
      catch(std::exception &e)
      {
        mu::console() << wxT("\n  fail: ") << a_str.c_str() << wxT(" (") << e.what() << wxT(")");
        return 1;  // always return a failure since this exception is not expected
      }
      catch(...)
      {
        mu::console() << wxT("\n  fail: ") << a_str.c_str() <<  wxT(" (unexpected exception)");
        return 1;  // exceptions other than ParserException are not allowed
      }

      return iRet;
    }

    //---------------------------------------------------------------------------
    int ParserTester::EqnTestInt(const string_type &a_str, double a_fRes, bool a_fPass)
    {
      ParserTester::c_iCount++;

      value_type vVarVal[] = {1, 2, 3};    // variable values
      value_type fVal[2] = {-99, -999}; // results: initially should be different
      int iRet(0);

      try
      {
        ParserInt p;
        p.DefineConst( wxT("const1"), 1);
        p.DefineConst( wxT("const2"), 2);
        p.DefineVar( wxT("a"), &vVarVal[0]);
        p.DefineVar( wxT("b"), &vVarVal[1]);
        p.DefineVar( wxT("c"), &vVarVal[2]);

        p.SetExpr(a_str);
        fVal[0] = p.Eval(); // result from stringparsing
        fVal[1] = p.Eval(); // result from bytecode

        if (fVal[0]!=fVal[1])
          throw Parser::exception_type( wxT("Bytecode corrupt.") );

        iRet =  ( (a_fRes==fVal[0] &&  a_fPass) || 
                  (a_fRes!=fVal[0] && !a_fPass) ) ? 0 : 1;
        if (iRet==1)
        {
          mu::console() << wxT("\n  fail: ") << a_str.c_str() 
                        << wxT(" (incorrect result; expected: ") << a_fRes 
                        << wxT(" ;calculated: ") << fVal[0]<< wxT(").");
        }
      }
      catch(Parser::exception_type &e)
      {
        if (a_fPass)
        {
          mu::console() << wxT("\n  fail: ") << e.GetExpr() << wxT(" : ") << e.GetMsg();
          iRet = 1;
        }
      }
      catch(...)
      {
        mu::console() << wxT("\n  fail: ") << a_str.c_str() <<  wxT(" (unexpected exception)");
        iRet = 1;  // exceptions other than ParserException are not allowed
      }

      return iRet;
    }

    //---------------------------------------------------------------------------
    /** \brief Internal error in test class Test is going to be aborted. */
    void ParserTester::Abort() const
    {
      mu::console() << wxT("Test failed (internal error in test class)") << endl;
      while (!getchar());
      exit(-1);
    }
  } // namespace test
} // namespace mu
