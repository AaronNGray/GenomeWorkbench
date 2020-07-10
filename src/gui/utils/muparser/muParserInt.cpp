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
#include <gui/utils/muparser/muParserInt.h>

#include <cmath>
#include <algorithm>
#include <numeric>

#define wxT(x) MUP_T(x)

using namespace std;

/** \file
    \brief Implementation of a parser using integer value.
*/

/** \brief Namespace for mathematical applications. */
namespace mu
{

value_type ParserInt::Abs(value_type v)  { return  Round(fabs(v)); }
value_type ParserInt::Sign(value_type v) { return (Round(v)<0) ? -1 : (Round(v)>0) ? 1 : 0; }
value_type ParserInt::Ite(value_type v1, 
                          value_type v2, 
                          value_type v3) { return (Round(v1)==1) ? Round(v2) : Round(v3); }
value_type ParserInt::Add(value_type v1, value_type v2) { return Round(v1)  + Round(v2); }
value_type ParserInt::Sub(value_type v1, value_type v2) { return Round(v1)  - Round(v2); }
value_type ParserInt::Mul(value_type v1, value_type v2) { return Round(v1)  * Round(v2); }
value_type ParserInt::Div(value_type v1, value_type v2) { return Round(v1)  / Round(v2); }
value_type ParserInt::Mod(value_type v1, value_type v2) { return Round(v1)  % Round(v2); }
value_type ParserInt::Shr(value_type v1, value_type v2) { return Round(v1) >> Round(v2); }
value_type ParserInt::Shl(value_type v1, value_type v2) { return Round(v1) << Round(v2); }
value_type ParserInt::LogAnd(value_type v1, value_type v2) { return Round(v1) & Round(v2); }
value_type ParserInt::LogOr(value_type v1, value_type v2)  { return Round(v1) | Round(v2); }
value_type ParserInt::LogXor(value_type v1, value_type v2) { return Round(v1) ^ Round(v2); }
value_type ParserInt::And(value_type v1, value_type v2) { return Round(v1) && Round(v2); }
value_type ParserInt::Or(value_type v1, value_type v2)  { return Round(v1) || Round(v2); }
value_type ParserInt::Less(value_type v1, value_type v2)      { return Round(v1)  < Round(v2); }
value_type ParserInt::Greater(value_type v1, value_type v2)   { return Round(v1)  > Round(v2); }
value_type ParserInt::LessEq(value_type v1, value_type v2)    { return Round(v1) <= Round(v2); }
value_type ParserInt::GreaterEq(value_type v1, value_type v2) { return Round(v1) >= Round(v2); }
value_type ParserInt::Equal(value_type v1, value_type v2)     { return Round(v1) == Round(v2); }
value_type ParserInt::NotEqual(value_type v1, value_type v2)  { return Round(v1) != Round(v2); }
value_type ParserInt::Not(value_type v) { return !Round(v); }

//---------------------------------------------------------------------------
// Unary operator Callbacks: Infix operators
value_type ParserInt::UnaryMinus(value_type v) 
{ 
  return -Round(v); 
}

//---------------------------------------------------------------------------
value_type ParserInt::Sum(const value_type* a_afArg, int a_iArgc)
{ 
  if (!a_iArgc)	
    throw ParserError(wxT("too few arguments for function sum."));

  value_type fRes=0;
  for (int i=0; i<a_iArgc; ++i) 
    fRes += a_afArg[i];

  return fRes;
}

//---------------------------------------------------------------------------
value_type ParserInt::Min(const value_type* a_afArg, int a_iArgc)
{ 
    if (!a_iArgc)	
        throw ParserError( wxT("too few arguments for function min.") );

    value_type fRes=a_afArg[0];
    for (int i=0; i<a_iArgc; ++i) 
      fRes = std::min(fRes, a_afArg[i]);

    return fRes;
}

//---------------------------------------------------------------------------
value_type ParserInt::Max(const value_type* a_afArg, int a_iArgc)
{ 
    if (!a_iArgc)	
        throw ParserError(wxT("too few arguments for function min."));

    value_type fRes=a_afArg[0];
    for (int i=0; i<a_iArgc; ++i) 
      fRes = std::max(fRes, a_afArg[i]);

    return fRes;
}

//---------------------------------------------------------------------------
// Default value recognition callback
int ParserInt::IsVal(const char_type *a_szExpr, int *a_iPos, value_type *a_fVal)
{
  string_type buf(a_szExpr);
  std::size_t pos = buf.find_first_not_of(wxT("0123456789"));
  if (pos==std::string::npos)
    return 0;

  stringstream_type stream( buf.substr(0, pos ) );
  int iVal(0);

  stream >> iVal;
  int iEnd = stream.tellg();   // Position after reading

  if (iEnd==-1)
    return 0;

  *a_iPos += iEnd;
  *a_fVal = (value_type)iVal;
  return 1;
}

//---------------------------------------------------------------------------
int ParserInt::IsHexVal(const char_type *a_szExpr, int *a_iPos, value_type *a_fVal)
{
  if (a_szExpr[0]!='$') 
    return 0;

  unsigned iVal(0);

// New code based on streams for UNICODE compliance:
  stringstream_type::pos_type nPos(0);
  stringstream_type ss(a_szExpr+1);
  ss >> std::hex >> iVal;
  nPos = ss.tellg();

  if (nPos==(stringstream_type::pos_type)0)
    return 1;

  *a_iPos += 1 + nPos;
  *a_fVal = iVal;
  return true;
}

//---------------------------------------------------------------------------
int ParserInt::IsBinVal(const char_type *a_szExpr, int *a_iPos, value_type *a_fVal)
{
  if (a_szExpr[0]!='#') 
    return 0;

  unsigned iVal(0), 
           iBits(sizeof(iVal)*8),
           i(0);

  for (i=0; (a_szExpr[i+1]=='0' || a_szExpr[i+1]=='1') && i<iBits; ++i)
    iVal |= (int)(a_szExpr[i+1]=='1') << ((iBits-1)-i);

  if (i==0) 
    return 0;

  if (i==iBits)
    throw exception_type(wxT("Binary to integer conversion error (overflow)."));

  *a_fVal = (unsigned)(iVal >> (iBits-i) );
  *a_iPos += i+1;

  return 1;
}

//---------------------------------------------------------------------------
/** \brief Constructor. 

  Call ParserBase class constructor and trigger Function, Operator and Constant initialization.
*/
ParserInt::ParserInt()
  :ParserBase()
{
  AddValIdent(IsVal);
  AddValIdent(IsHexVal);
  AddValIdent(IsBinVal);

  InitCharSets();
  InitFun();
  InitOprt();
}

//---------------------------------------------------------------------------
void ParserInt::InitConst()
{
}

//---------------------------------------------------------------------------
void ParserInt::InitCharSets()
{
  DefineNameChars( wxT("0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") );
  DefineOprtChars( wxT("+-*^/?<>=!%&|~'_") );
  DefineInfixOprtChars( wxT("/+-*^?<>=!%&|~'_") );
}

//---------------------------------------------------------------------------
/** \brief Initialize the default functions. */
void ParserInt::InitFun()
{
  DefineFun( wxT("sign"), Sign);
  DefineFun( wxT("abs"), Abs);
  DefineFun( wxT("if"), Ite);
  DefineFun( wxT("sum"), Sum);
  DefineFun( wxT("min"), Min);
  DefineFun( wxT("max"), Max);
}

//---------------------------------------------------------------------------
/** \brief Initialize operators. */
void ParserInt::InitOprt()
{
  // disable all built in operators, not all of them usefull for integer numbers
  // (they don't do rounding of values)
  EnableBuiltInOprt(false);

  // Disable all built in operators, they wont work with integer numbers
  // since they are designed for floating point numbers
  DefineInfixOprt( wxT("-"), UnaryMinus);
  DefineInfixOprt( wxT("!"), Not);

  DefineOprt( wxT("&"), LogAnd, prLOGIC);
  DefineOprt( wxT("|"), LogOr, prLOGIC);
  DefineOprt( wxT("^"), LogXor, prLOGIC);
  DefineOprt( wxT("&&"), And, prLOGIC);
  DefineOprt( wxT("||"), Or, prLOGIC);

  DefineOprt( wxT("<"), Less, prCMP);
  DefineOprt( wxT(">"), Greater, prCMP);
  DefineOprt( wxT("<="), LessEq, prCMP);
  DefineOprt( wxT(">="), GreaterEq, prCMP);
  DefineOprt( wxT("=="), Equal, prCMP);
  DefineOprt( wxT("!="), NotEqual, prCMP);

  DefineOprt( wxT("+"), Add, prADD_SUB);
  DefineOprt( wxT("-"), Sub, prADD_SUB);

  DefineOprt( wxT("*"), Mul, prMUL_DIV);
  DefineOprt( wxT("/"), Div, prMUL_DIV);
  DefineOprt( wxT("%"), Mod, prMUL_DIV);

  DefineOprt( wxT(">>"), Shr, prMUL_DIV+1);
  DefineOprt( wxT("<<"), Shl, prMUL_DIV+1);
}

} // namespace mu
