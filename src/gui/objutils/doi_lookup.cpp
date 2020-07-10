/*  $Id: doi_lookup.cpp 44018 2019-10-09 14:37:01Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/doi_lookup.hpp>
#include <gui/objutils/registry.hpp>
#include <connect/ncbi_http_session.hpp>
#include <sstream>
#include <misc/jsonwrapp/jsonwrapp.hpp>

#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Title.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Date_std.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub_set.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/biblio/DOI.hpp>
#include <objects/biblio/ArticleId.hpp>
#include <objects/biblio/ArticleIdSet.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <misc/xmlwrapp/document.hpp>
#include <gui/objutils/gui_eutils_client.hpp>
#include <objtools/validator/utilities.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>
#include <html/html.hpp> 

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


string CDoiLookup::GetSpecialCharacterReplacement(TUnicodeSymbol ch)
{
	string r = "#";

	if (ch == 198 || ch == 196)
	{
		r = "Ae";
	}
	else if (ch == 230 || ch == 228)
	{
		r = "ae";
	}
	else if (ch == 197)
	{
		r = "Aa";
	}
	else if (ch == 229)
	{
		r = "aa";
	}
	else if (ch == 220)
	{
		r = "Ue";
	}
	else if (ch == 252)
	{
		r = "ue";
	}
	else if (ch == 214)
	{
		r = "Oe";
	}
	else if (ch == 246)
	{
		r = "oe";
	}
	else if (ch == 223)
	{
		r = "ss";
	}
	else if (ch == 199)
	{
		r = "C";
	}
	else if (ch >= 200 && ch <= 203)
	{
		r = "E";
	}
	else if (ch >= 204 && ch <= 207)
	{
		r = "I";
	}
	else if (ch == 209)
	{
		r = "N";
	}
	else if ((ch >= 210 && ch <= 214) || ch == 216)
	{
		r = "O";
	}
	else if (ch >= 217 && ch <= 220)
	{
		r = "U";
	}
	else if (ch == 221)
	{
		r = "Y";
	}
	else if (ch >= 224 && ch <= 229)
	{
		r = "a";
	}
	else if (ch == 231)
	{
		r = "c";
	}
	else if (ch >= 232 && ch <= 235)
	{
		r = "e";
	}
	else if (ch >= 236 && ch <= 239)
	{
		r = "i";
	}
	else if (ch == 241)
	{
		r = "n";
	}
	else if ((ch >= 242 && ch <= 246) || ch == 248)
	{
		r = "o";
	}
	else if (ch >= 249 && ch <= 252)
	{
		r = "u";
	}
	else if (ch == 253 || ch == 255)
	{
		r = "y";
	}

// https://www.fileformat.info/info/unicode/block/latin_extended_a/list.htm
	switch (ch)
	{
	case 0x100: r = "A"; break; //LATIN CAPITAL LETTER A WITH MACRON 
	case 0x101: r = "a"; break; //LATIN SMALL LETTER A WITH MACRON 
	case 0x102: r = "A"; break; //LATIN CAPITAL LETTER A WITH BREVE 
	case 0x103: r = "a"; break; //LATIN SMALL LETTER A WITH BREVE 
	case 0x104: r = "A"; break; //LATIN CAPITAL LETTER A WITH OGONEK 
	case 0x105: r = "a"; break; //LATIN SMALL LETTER A WITH OGONEK 
	case 0x106: r = "C"; break; //LATIN CAPITAL LETTER C WITH ACUTE 
	case 0x107: r = "c"; break; //LATIN SMALL LETTER C WITH ACUTE 
	case 0x108: r = "C"; break; //LATIN CAPITAL LETTER C WITH CIRCUMFLEX 
	case 0x109: r = "c"; break; //LATIN SMALL LETTER C WITH CIRCUMFLEX 
	case 0x10A: r = "C"; break; //LATIN CAPITAL LETTER C WITH DOT ABOVE 
	case 0x10B: r = "c"; break; //LATIN SMALL LETTER C WITH DOT ABOVE 
	case 0x10C: r = "C"; break; //LATIN CAPITAL LETTER C WITH CARON 
	case 0x10D: r = "c"; break; //LATIN SMALL LETTER C WITH CARON 
	case 0x10E: r = "D"; break; //LATIN CAPITAL LETTER D WITH CARON 
	case 0x10F: r = "d"; break; //LATIN SMALL LETTER D WITH CARON 
	case 0x110: r = "D"; break; //LATIN CAPITAL LETTER D WITH STROKE 
	case 0x111: r = "d"; break; //LATIN SMALL LETTER D WITH STROKE 
	case 0x112: r = "E"; break; //LATIN CAPITAL LETTER E WITH MACRON 
	case 0x113: r = "e"; break; //LATIN SMALL LETTER E WITH MACRON 
	case 0x114: r = "E"; break; //LATIN CAPITAL LETTER E WITH BREVE 
	case 0x115: r = "e"; break; //LATIN SMALL LETTER E WITH BREVE 
	case 0x116: r = "E"; break; //LATIN CAPITAL LETTER E WITH DOT ABOVE 
	case 0x117: r = "e"; break; //LATIN SMALL LETTER E WITH DOT ABOVE 
	case 0x118: r = "E"; break; //LATIN CAPITAL LETTER E WITH OGONEK 
	case 0x119: r = "e"; break; //LATIN SMALL LETTER E WITH OGONEK 
	case 0x11A: r = "E"; break; //LATIN CAPITAL LETTER E WITH CARON 
	case 0x11B: r = "e"; break; //LATIN SMALL LETTER E WITH CARON 
	case 0x11C: r = "G"; break; //LATIN CAPITAL LETTER G WITH CIRCUMFLEX 
	case 0x11D: r = "g"; break; //LATIN SMALL LETTER G WITH CIRCUMFLEX 
	case 0x11E: r = "G"; break; //LATIN CAPITAL LETTER G WITH BREVE 
	case 0x11F: r = "g"; break; //LATIN SMALL LETTER G WITH BREVE 
	case 0x120: r = "G"; break; //LATIN CAPITAL LETTER G WITH DOT ABOVE 
	case 0x121: r = "g"; break; //LATIN SMALL LETTER G WITH DOT ABOVE 
	case 0x122: r = "G"; break; //LATIN CAPITAL LETTER G WITH CEDILLA 
	case 0x123: r = "g"; break; //LATIN SMALL LETTER G WITH CEDILLA 
	case 0x124: r = "H"; break; //LATIN CAPITAL LETTER H WITH CIRCUMFLEX 
	case 0x125: r = "h"; break; //LATIN SMALL LETTER H WITH CIRCUMFLEX 
	case 0x126: r = "H"; break; //LATIN CAPITAL LETTER H WITH STROKE 
	case 0x127: r = "h"; break; //LATIN SMALL LETTER H WITH STROKE 
	case 0x128: r = "I"; break; //LATIN CAPITAL LETTER I WITH TILDE 
	case 0x129: r = "i"; break; //LATIN SMALL LETTER I WITH TILDE 
	case 0x12A: r = "I"; break; //LATIN CAPITAL LETTER I WITH MACRON 
	case 0x12B: r = "i"; break; //LATIN SMALL LETTER I WITH MACRON 
	case 0x12C: r = "I"; break; //LATIN CAPITAL LETTER I WITH BREVE 
	case 0x12D: r = "i"; break; //LATIN SMALL LETTER I WITH BREVE 
	case 0x12E: r = "I"; break; //LATIN CAPITAL LETTER I WITH OGONEK 
	case 0x12F: r = "i"; break; //LATIN SMALL LETTER I WITH OGONEK 
	case 0x130: r = "I"; break; //LATIN CAPITAL LETTER I WITH DOT ABOVE 
	case 0x131: r = "i"; break; //LATIN SMALL LETTER DOTLESS I 
	case 0x132: r = "IJ"; break; //LATIN CAPITAL LIGATURE IJ 
	case 0x133: r = "ij"; break; //LATIN SMALL LIGATURE IJ 
	case 0x134: r = "J"; break; //LATIN CAPITAL LETTER J WITH CIRCUMFLEX 
	case 0x135: r = "j"; break; //LATIN SMALL LETTER J WITH CIRCUMFLEX 
	case 0x136: r = "K"; break; //LATIN CAPITAL LETTER K WITH CEDILLA 
	case 0x137: r = "k"; break; //LATIN SMALL LETTER K WITH CEDILLA 
	case 0x138: r = "k"; break; //LATIN SMALL LETTER KRA 
	case 0x139: r = "L"; break; //LATIN CAPITAL LETTER L WITH ACUTE 
	case 0x13A: r = "l"; break; //LATIN SMALL LETTER L WITH ACUTE 
	case 0x13B: r = "L"; break; //LATIN CAPITAL LETTER L WITH CEDILLA 
	case 0x13C: r = "l"; break; //LATIN SMALL LETTER L WITH CEDILLA 
	case 0x13D: r = "L"; break; //LATIN CAPITAL LETTER L WITH CARON 
	case 0x13E: r = "l"; break; //LATIN SMALL LETTER L WITH CARON 
	case 0x13F: r = "L"; break; //LATIN CAPITAL LETTER L WITH MIDDLE DOT 
	case 0x140: r = "l"; break; //LATIN SMALL LETTER L WITH MIDDLE DOT 
	case 0x141: r = "L"; break; //LATIN CAPITAL LETTER L WITH STROKE 
	case 0x142: r = "l"; break; //LATIN SMALL LETTER L WITH STROKE 
	case 0x143: r = "N"; break; //LATIN CAPITAL LETTER N WITH ACUTE 
	case 0x144: r = "n"; break; //LATIN SMALL LETTER N WITH ACUTE 
	case 0x145: r = "N"; break; //LATIN CAPITAL LETTER N WITH CEDILLA 
	case 0x146: r = "n"; break; //LATIN SMALL LETTER N WITH CEDILLA 
	case 0x147: r = "N"; break; //LATIN CAPITAL LETTER N WITH CARON 
	case 0x148: r = "n"; break; //LATIN SMALL LETTER N WITH CARON 
	case 0x149: r = "n"; break; //LATIN SMALL LETTER N PRECEDED BY APOSTROPHE 
	case 0x14A: r = "N"; break; //LATIN CAPITAL LETTER ENG 
	case 0x14B: r = "n"; break; //LATIN SMALL LETTER ENG 
	case 0x14C: r = "O"; break; //LATIN CAPITAL LETTER O WITH MACRON 
	case 0x14D: r = "o"; break; //LATIN SMALL LETTER O WITH MACRON 
	case 0x14E: r = "O"; break; //LATIN CAPITAL LETTER O WITH BREVE 
	case 0x14F: r = "o"; break; //LATIN SMALL LETTER O WITH BREVE 
	case 0x150: r = "O"; break; //LATIN CAPITAL LETTER O WITH DOUBLE ACUTE 
	case 0x151: r = "o"; break; //LATIN SMALL LETTER O WITH DOUBLE ACUTE 
	case 0x152: r = "OE"; break; //LATIN CAPITAL LIGATURE OE 
	case 0x153: r = "oe"; break; //LATIN SMALL LIGATURE OE 
	case 0x154: r = "R"; break; //LATIN CAPITAL LETTER R WITH ACUTE 
	case 0x155: r = "r"; break; //LATIN SMALL LETTER R WITH ACUTE 
	case 0x156: r = "R"; break; //LATIN CAPITAL LETTER R WITH CEDILLA 
	case 0x157: r = "r"; break; //LATIN SMALL LETTER R WITH CEDILLA 
	case 0x158: r = "R"; break; //LATIN CAPITAL LETTER R WITH CARON 
	case 0x159: r = "r"; break; //LATIN SMALL LETTER R WITH CARON 
	case 0x15A: r = "S"; break; //LATIN CAPITAL LETTER S WITH ACUTE 
	case 0x15B: r = "s"; break; //LATIN SMALL LETTER S WITH ACUTE 
	case 0x15C: r = "S"; break; //LATIN CAPITAL LETTER S WITH CIRCUMFLEX 
	case 0x15D: r = "s"; break; //LATIN SMALL LETTER S WITH CIRCUMFLEX 
	case 0x15E: r = "S"; break; //LATIN CAPITAL LETTER S WITH CEDILLA 
	case 0x15F: r = "s"; break; //LATIN SMALL LETTER S WITH CEDILLA 
	case 0x160: r = "S"; break; //LATIN CAPITAL LETTER S WITH CARON 
	case 0x161: r = "s"; break; //LATIN SMALL LETTER S WITH CARON 
	case 0x162: r = "T"; break; //LATIN CAPITAL LETTER T WITH CEDILLA 
	case 0x163: r = "t"; break; //LATIN SMALL LETTER T WITH CEDILLA 
	case 0x164: r = "T"; break; //LATIN CAPITAL LETTER T WITH CARON 
	case 0x165: r = "t"; break; //LATIN SMALL LETTER T WITH CARON 
	case 0x166: r = "T"; break; //LATIN CAPITAL LETTER T WITH STROKE 
	case 0x167: r = "t"; break; //LATIN SMALL LETTER T WITH STROKE 
	case 0x168: r = "U"; break; //LATIN CAPITAL LETTER U WITH TILDE 
	case 0x169: r = "u"; break; //LATIN SMALL LETTER U WITH TILDE 
	case 0x16A: r = "U"; break; //LATIN CAPITAL LETTER U WITH MACRON 
	case 0x16B: r = "u"; break; //LATIN SMALL LETTER U WITH MACRON 
	case 0x16C: r = "U"; break; //LATIN CAPITAL LETTER U WITH BREVE 
	case 0x16D: r = "u"; break; //LATIN SMALL LETTER U WITH BREVE 
	case 0x16E: r = "U"; break; //LATIN CAPITAL LETTER U WITH RING ABOVE 
	case 0x16F: r = "u"; break; //LATIN SMALL LETTER U WITH RING ABOVE 
	case 0x170: r = "U"; break; //LATIN CAPITAL LETTER U WITH DOUBLE ACUTE 
	case 0x171: r = "u"; break; //LATIN SMALL LETTER U WITH DOUBLE ACUTE 
	case 0x172: r = "U"; break; //LATIN CAPITAL LETTER U WITH OGONEK 
	case 0x173: r = "u"; break; //LATIN SMALL LETTER U WITH OGONEK 
	case 0x174: r = "W"; break; //LATIN CAPITAL LETTER W WITH CIRCUMFLEX 
	case 0x175: r = "w"; break; //LATIN SMALL LETTER W WITH CIRCUMFLEX 
	case 0x176: r = "Y"; break; //LATIN CAPITAL LETTER Y WITH CIRCUMFLEX 
	case 0x177: r = "y"; break; //LATIN SMALL LETTER Y WITH CIRCUMFLEX 
	case 0x178: r = "Y"; break; //LATIN CAPITAL LETTER Y WITH DIAERESIS 
	case 0x179: r = "Z"; break; //LATIN CAPITAL LETTER Z WITH ACUTE 
	case 0x17A: r = "z"; break; //LATIN SMALL LETTER Z WITH ACUTE 
	case 0x17B: r = "Z"; break; //LATIN CAPITAL LETTER Z WITH DOT ABOVE 
	case 0x17C: r = "z"; break; //LATIN SMALL LETTER Z WITH DOT ABOVE 
	case 0x17D: r = "Z"; break; //LATIN CAPITAL LETTER Z WITH CARON 
	case 0x17E: r = "z"; break; //LATIN SMALL LETTER Z WITH CARON 
	case 0x17F: r = "s"; break; //LATIN SMALL LETTER LONG S 

// https://www.fileformat.info/info/unicode/block/latin_extended_additional/list.htm
	case 0x1E00: r = "A"; break; //LATIN CAPITAL LETTER A WITH RING BELOW 
	case 0x1E01: r = "a"; break; //LATIN SMALL LETTER A WITH RING BELOW 
	case 0x1E02: r = "B"; break; //LATIN CAPITAL LETTER B WITH DOT ABOVE 
	case 0x1E03: r = "b"; break; //LATIN SMALL LETTER B WITH DOT ABOVE 
	case 0x1E04: r = "B"; break; //LATIN CAPITAL LETTER B WITH DOT BELOW 
	case 0x1E05: r = "b"; break; //LATIN SMALL LETTER B WITH DOT BELOW 
	case 0x1E06: r = "B"; break; //LATIN CAPITAL LETTER B WITH LINE BELOW 
	case 0x1E07: r = "b"; break; //LATIN SMALL LETTER B WITH LINE BELOW 
	case 0x1E08: r = "C"; break; //LATIN CAPITAL LETTER C WITH CEDILLA AND ACUTE 
	case 0x1E09: r = "c"; break; //LATIN SMALL LETTER C WITH CEDILLA AND ACUTE 
	case 0x1E0A: r = "D"; break; //LATIN CAPITAL LETTER D WITH DOT ABOVE 
	case 0x1E0B: r = "d"; break; //LATIN SMALL LETTER D WITH DOT ABOVE 
	case 0x1E0C: r = "D"; break; //LATIN CAPITAL LETTER D WITH DOT BELOW 
	case 0x1E0D: r = "d"; break; //LATIN SMALL LETTER D WITH DOT BELOW 
	case 0x1E0E: r = "D"; break; //LATIN CAPITAL LETTER D WITH LINE BELOW 
	case 0x1E0F: r = "d"; break; //LATIN SMALL LETTER D WITH LINE BELOW 
	case 0x1E10: r = "D"; break; //LATIN CAPITAL LETTER D WITH CEDILLA 
	case 0x1E11: r = "d"; break; //LATIN SMALL LETTER D WITH CEDILLA 
	case 0x1E12: r = "D"; break; //LATIN CAPITAL LETTER D WITH CIRCUMFLEX BELOW 
	case 0x1E13: r = "d"; break; //LATIN SMALL LETTER D WITH CIRCUMFLEX BELOW 
	case 0x1E14: r = "E"; break; //LATIN CAPITAL LETTER E WITH MACRON AND GRAVE 
	case 0x1E15: r = "e"; break; //LATIN SMALL LETTER E WITH MACRON AND GRAVE 
	case 0x1E16: r = "E"; break; //LATIN CAPITAL LETTER E WITH MACRON AND ACUTE 
	case 0x1E17: r = "e"; break; //LATIN SMALL LETTER E WITH MACRON AND ACUTE 
	case 0x1E18: r = "E"; break; //LATIN CAPITAL LETTER E WITH CIRCUMFLEX BELOW 
	case 0x1E19: r = "e"; break; //LATIN SMALL LETTER E WITH CIRCUMFLEX BELOW 
	case 0x1E1A: r = "E"; break; //LATIN CAPITAL LETTER E WITH TILDE BELOW 
	case 0x1E1B: r = "e"; break; //LATIN SMALL LETTER E WITH TILDE BELOW 
	case 0x1E1C: r = "E"; break; //LATIN CAPITAL LETTER E WITH CEDILLA AND BREVE 
	case 0x1E1D: r = "e"; break; //LATIN SMALL LETTER E WITH CEDILLA AND BREVE 
	case 0x1E1E: r = "F"; break; //LATIN CAPITAL LETTER F WITH DOT ABOVE 
	case 0x1E1F: r = "f"; break; //LATIN SMALL LETTER F WITH DOT ABOVE 
	case 0x1E20: r = "G"; break; //LATIN CAPITAL LETTER G WITH MACRON 
	case 0x1E21: r = "g"; break; //LATIN SMALL LETTER G WITH MACRON 
	case 0x1E22: r = "H"; break; //LATIN CAPITAL LETTER H WITH DOT ABOVE 
	case 0x1E23: r = "h"; break; //LATIN SMALL LETTER H WITH DOT ABOVE 
	case 0x1E24: r = "H"; break; //LATIN CAPITAL LETTER H WITH DOT BELOW 
	case 0x1E25: r = "h"; break; //LATIN SMALL LETTER H WITH DOT BELOW 
	case 0x1E26: r = "H"; break; //LATIN CAPITAL LETTER H WITH DIAERESIS 
	case 0x1E27: r = "h"; break; //LATIN SMALL LETTER H WITH DIAERESIS 
	case 0x1E28: r = "H"; break; //LATIN CAPITAL LETTER H WITH CEDILLA 
	case 0x1E29: r = "h"; break; //LATIN SMALL LETTER H WITH CEDILLA 
	case 0x1E2A: r = "H"; break; //LATIN CAPITAL LETTER H WITH BREVE BELOW 
	case 0x1E2B: r = "h"; break; //LATIN SMALL LETTER H WITH BREVE BELOW 
	case 0x1E2C: r = "I"; break; //LATIN CAPITAL LETTER I WITH TILDE BELOW 
	case 0x1E2D: r = "i"; break; //LATIN SMALL LETTER I WITH TILDE BELOW 
	case 0x1E2E: r = "I"; break; //LATIN CAPITAL LETTER I WITH DIAERESIS AND ACUTE 
	case 0x1E2F: r = "i"; break; //LATIN SMALL LETTER I WITH DIAERESIS AND ACUTE 
	case 0x1E30: r = "K"; break; //LATIN CAPITAL LETTER K WITH ACUTE 
	case 0x1E31: r = "k"; break; //LATIN SMALL LETTER K WITH ACUTE 
	case 0x1E32: r = "K"; break; //LATIN CAPITAL LETTER K WITH DOT BELOW 
	case 0x1E33: r = "k"; break; //LATIN SMALL LETTER K WITH DOT BELOW 
	case 0x1E34: r = "K"; break; //LATIN CAPITAL LETTER K WITH LINE BELOW 
	case 0x1E35: r = "k"; break; //LATIN SMALL LETTER K WITH LINE BELOW 
	case 0x1E36: r = "L"; break; //LATIN CAPITAL LETTER L WITH DOT BELOW 
	case 0x1E37: r = "l"; break; //LATIN SMALL LETTER L WITH DOT BELOW 
	case 0x1E38: r = "L"; break; //LATIN CAPITAL LETTER L WITH DOT BELOW AND MACRON 
	case 0x1E39: r = "l"; break; //LATIN SMALL LETTER L WITH DOT BELOW AND MACRON 
	case 0x1E3A: r = "L"; break; //LATIN CAPITAL LETTER L WITH LINE BELOW 
	case 0x1E3B: r = "l"; break; //LATIN SMALL LETTER L WITH LINE BELOW 
	case 0x1E3C: r = "L"; break; //LATIN CAPITAL LETTER L WITH CIRCUMFLEX BELOW 
	case 0x1E3D: r = "l"; break; //LATIN SMALL LETTER L WITH CIRCUMFLEX BELOW 
	case 0x1E3E: r = "M"; break; //LATIN CAPITAL LETTER M WITH ACUTE 
	case 0x1E3F: r = "m"; break; //LATIN SMALL LETTER M WITH ACUTE 
	case 0x1E40: r = "M"; break; //LATIN CAPITAL LETTER M WITH DOT ABOVE 
	case 0x1E41: r = "m"; break; //LATIN SMALL LETTER M WITH DOT ABOVE 
	case 0x1E42: r = "M"; break; //LATIN CAPITAL LETTER M WITH DOT BELOW 
	case 0x1E43: r = "m"; break; //LATIN SMALL LETTER M WITH DOT BELOW 
	case 0x1E44: r = "N"; break; //LATIN CAPITAL LETTER N WITH DOT ABOVE 
	case 0x1E45: r = "n"; break; //LATIN SMALL LETTER N WITH DOT ABOVE 
	case 0x1E46: r = "N"; break; //LATIN CAPITAL LETTER N WITH DOT BELOW 
	case 0x1E47: r = "n"; break; //LATIN SMALL LETTER N WITH DOT BELOW 
	case 0x1E48: r = "N"; break; //LATIN CAPITAL LETTER N WITH LINE BELOW 
	case 0x1E49: r = "n"; break; //LATIN SMALL LETTER N WITH LINE BELOW 
	case 0x1E4A: r = "N"; break; //LATIN CAPITAL LETTER N WITH CIRCUMFLEX BELOW 
	case 0x1E4B: r = "n"; break; //LATIN SMALL LETTER N WITH CIRCUMFLEX BELOW 
	case 0x1E4C: r = "O"; break; //LATIN CAPITAL LETTER O WITH TILDE AND ACUTE 
	case 0x1E4D: r = "o"; break; //LATIN SMALL LETTER O WITH TILDE AND ACUTE 
	case 0x1E4E: r = "O"; break; //LATIN CAPITAL LETTER O WITH TILDE AND DIAERESIS 
	case 0x1E4F: r = "o"; break; //LATIN SMALL LETTER O WITH TILDE AND DIAERESIS 
	case 0x1E50: r = "O"; break; //LATIN CAPITAL LETTER O WITH MACRON AND GRAVE 
	case 0x1E51: r = "o"; break; //LATIN SMALL LETTER O WITH MACRON AND GRAVE 
	case 0x1E52: r = "O"; break; //LATIN CAPITAL LETTER O WITH MACRON AND ACUTE 
	case 0x1E53: r = "o"; break; //LATIN SMALL LETTER O WITH MACRON AND ACUTE 
	case 0x1E54: r = "P"; break; //LATIN CAPITAL LETTER P WITH ACUTE 
	case 0x1E55: r = "p"; break; //LATIN SMALL LETTER P WITH ACUTE 
	case 0x1E56: r = "P"; break; //LATIN CAPITAL LETTER P WITH DOT ABOVE 
	case 0x1E57: r = "p"; break; //LATIN SMALL LETTER P WITH DOT ABOVE 
	case 0x1E58: r = "P"; break; //LATIN CAPITAL LETTER R WITH DOT ABOVE 
	case 0x1E59: r = "p"; break; //LATIN SMALL LETTER R WITH DOT ABOVE 
	case 0x1E5A: r = "R"; break; //LATIN CAPITAL LETTER R WITH DOT BELOW 
	case 0x1E5B: r = "r"; break; //LATIN SMALL LETTER R WITH DOT BELOW 
	case 0x1E5C: r = "R"; break; //LATIN CAPITAL LETTER R WITH DOT BELOW AND MACRON 
	case 0x1E5D: r = "r"; break; //LATIN SMALL LETTER R WITH DOT BELOW AND MACRON 
	case 0x1E5E: r = "R"; break; //LATIN CAPITAL LETTER R WITH LINE BELOW 
	case 0x1E5F: r = "r"; break; //LATIN SMALL LETTER R WITH LINE BELOW 
	case 0x1E60: r = "S"; break; //LATIN CAPITAL LETTER S WITH DOT ABOVE 
	case 0x1E61: r = "s"; break; //LATIN SMALL LETTER S WITH DOT ABOVE 
	case 0x1E62: r = "S"; break; //LATIN CAPITAL LETTER S WITH DOT BELOW 
	case 0x1E63: r = "s"; break; //LATIN SMALL LETTER S WITH DOT BELOW 
	case 0x1E64: r = "S"; break; //LATIN CAPITAL LETTER S WITH ACUTE AND DOT ABOVE 
	case 0x1E65: r = "s"; break; //LATIN SMALL LETTER S WITH ACUTE AND DOT ABOVE 
	case 0x1E66: r = "S"; break; //LATIN CAPITAL LETTER S WITH CARON AND DOT ABOVE 
	case 0x1E67: r = "s"; break; //LATIN SMALL LETTER S WITH CARON AND DOT ABOVE 
	case 0x1E68: r = "S"; break; //LATIN CAPITAL LETTER S WITH DOT BELOW AND DOT ABOVE 
	case 0x1E69: r = "s"; break; //LATIN SMALL LETTER S WITH DOT BELOW AND DOT ABOVE 
	case 0x1E6A: r = "T"; break; //LATIN CAPITAL LETTER T WITH DOT ABOVE 
	case 0x1E6B: r = "t"; break; //LATIN SMALL LETTER T WITH DOT ABOVE 
	case 0x1E6C: r = "T"; break; //LATIN CAPITAL LETTER T WITH DOT BELOW 
	case 0x1E6D: r = "t"; break; //LATIN SMALL LETTER T WITH DOT BELOW 
	case 0x1E6E: r = "T"; break; //LATIN CAPITAL LETTER T WITH LINE BELOW 
	case 0x1E6F: r = "t"; break; //LATIN SMALL LETTER T WITH LINE BELOW 
	case 0x1E70: r = "T"; break; //LATIN CAPITAL LETTER T WITH CIRCUMFLEX BELOW 
	case 0x1E71: r = "t"; break; //LATIN SMALL LETTER T WITH CIRCUMFLEX BELOW 
	case 0x1E72: r = "U"; break; //LATIN CAPITAL LETTER U WITH DIAERESIS BELOW 
	case 0x1E73: r = "u"; break; //LATIN SMALL LETTER U WITH DIAERESIS BELOW 
	case 0x1E74: r = "U"; break; //LATIN CAPITAL LETTER U WITH TILDE BELOW 
	case 0x1E75: r = "u"; break; //LATIN SMALL LETTER U WITH TILDE BELOW 
	case 0x1E76: r = "U"; break; //LATIN CAPITAL LETTER U WITH CIRCUMFLEX BELOW 
	case 0x1E77: r = "u"; break; //LATIN SMALL LETTER U WITH CIRCUMFLEX BELOW 
	case 0x1E78: r = "U"; break; //LATIN CAPITAL LETTER U WITH TILDE AND ACUTE 
	case 0x1E79: r = "u"; break; //LATIN SMALL LETTER U WITH TILDE AND ACUTE 
	case 0x1E7A: r = "U"; break; //LATIN CAPITAL LETTER U WITH MACRON AND DIAERESIS 
	case 0x1E7B: r = "u"; break; //LATIN SMALL LETTER U WITH MACRON AND DIAERESIS 
	case 0x1E7C: r = "V"; break; //LATIN CAPITAL LETTER V WITH TILDE 
	case 0x1E7D: r = "v"; break; //LATIN SMALL LETTER V WITH TILDE 
	case 0x1E7E: r = "V"; break; //LATIN CAPITAL LETTER V WITH DOT BELOW 
	case 0x1E7F: r = "v"; break; //LATIN SMALL LETTER V WITH DOT BELOW 
	case 0x1E80: r = "V"; break; //LATIN CAPITAL LETTER W WITH GRAVE 
	case 0x1E81: r = "w"; break; //LATIN SMALL LETTER W WITH GRAVE 
	case 0x1E82: r = "W"; break; //LATIN CAPITAL LETTER W WITH ACUTE 
	case 0x1E83: r = "w"; break; //LATIN SMALL LETTER W WITH ACUTE 
	case 0x1E84: r = "W"; break; //LATIN CAPITAL LETTER W WITH DIAERESIS 
	case 0x1E85: r = "w"; break; //LATIN SMALL LETTER W WITH DIAERESIS 
	case 0x1E86: r = "W"; break; //LATIN CAPITAL LETTER W WITH DOT ABOVE 
	case 0x1E87: r = "w"; break; //LATIN SMALL LETTER W WITH DOT ABOVE 
	case 0x1E88: r = "W"; break; //LATIN CAPITAL LETTER W WITH DOT BELOW 
	case 0x1E89: r = "w"; break; //LATIN SMALL LETTER W WITH DOT BELOW 
	case 0x1E8A: r = "W"; break; //LATIN CAPITAL LETTER X WITH DOT ABOVE 
	case 0x1E8B: r = "x"; break; //LATIN SMALL LETTER X WITH DOT ABOVE 
	case 0x1E8C: r = "X"; break; //LATIN CAPITAL LETTER X WITH DIAERESIS 
	case 0x1E8D: r = "x"; break; //LATIN SMALL LETTER X WITH DIAERESIS 
	case 0x1E8E: r = "Y"; break; //LATIN CAPITAL LETTER Y WITH DOT ABOVE 
	case 0x1E8F: r = "y"; break; //LATIN SMALL LETTER Y WITH DOT ABOVE 
	case 0x1E90: r = "Z"; break; //LATIN CAPITAL LETTER Z WITH CIRCUMFLEX 
	case 0x1E91: r = "z"; break; //LATIN SMALL LETTER Z WITH CIRCUMFLEX 
	case 0x1E92: r = "Z"; break; //LATIN CAPITAL LETTER Z WITH DOT BELOW 
	case 0x1E93: r = "z"; break; //LATIN SMALL LETTER Z WITH DOT BELOW 
	case 0x1E94: r = "Z"; break; //LATIN CAPITAL LETTER Z WITH LINE BELOW 
	case 0x1E95: r = "z"; break; //LATIN SMALL LETTER Z WITH LINE BELOW 
	case 0x1E96: r = "h"; break; //LATIN SMALL LETTER H WITH LINE BELOW 
	case 0x1E97: r = "t"; break; //LATIN SMALL LETTER T WITH DIAERESIS 
	case 0x1E98: r = "w"; break; //LATIN SMALL LETTER W WITH RING ABOVE 
	case 0x1E99: r = "y"; break; //LATIN SMALL LETTER Y WITH RING ABOVE 
	case 0x1E9A: r = "a"; break; //LATIN SMALL LETTER A WITH RIGHT HALF RING 
	case 0x1E9B: r = "s"; break; //LATIN SMALL LETTER LONG S WITH DOT ABOVE 
	case 0x1E9C: r = "s"; break; //LATIN SMALL LETTER LONG S WITH DIAGONAL STROKE 
	case 0x1E9D: r = "s"; break; //LATIN SMALL LETTER LONG S WITH HIGH STROKE 
	case 0x1E9E: r = "S"; break; //LATIN CAPITAL LETTER SHARP S 
	case 0x1E9F: r = "d"; break; //LATIN SMALL LETTER DELTA 
	case 0x1EA0: r = "A"; break; //LATIN CAPITAL LETTER A WITH DOT BELOW 
	case 0x1EA1: r = "a"; break; //LATIN SMALL LETTER A WITH DOT BELOW 
	case 0x1EA2: r = "A"; break; //LATIN CAPITAL LETTER A WITH HOOK ABOVE 
	case 0x1EA3: r = "a"; break; //LATIN SMALL LETTER A WITH HOOK ABOVE 
	case 0x1EA4: r = "A"; break; //LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND ACUTE 
	case 0x1EA5: r = "a"; break; //LATIN SMALL LETTER A WITH CIRCUMFLEX AND ACUTE 
	case 0x1EA6: r = "A"; break; //LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND GRAVE 
	case 0x1EA7: r = "a"; break; //LATIN SMALL LETTER A WITH CIRCUMFLEX AND GRAVE 
	case 0x1EA8: r = "A"; break; //LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE 
	case 0x1EA9: r = "a"; break; //LATIN SMALL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE 
	case 0x1EAA: r = "A"; break; //LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND TILDE 
	case 0x1EAB: r = "a"; break; //LATIN SMALL LETTER A WITH CIRCUMFLEX AND TILDE 
	case 0x1EAC: r = "A"; break; //LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND DOT BELOW 
	case 0x1EAD: r = "a"; break; //LATIN SMALL LETTER A WITH CIRCUMFLEX AND DOT BELOW 
	case 0x1EAE: r = "A"; break; //LATIN CAPITAL LETTER A WITH BREVE AND ACUTE 
	case 0x1EAF: r = "a"; break; //LATIN SMALL LETTER A WITH BREVE AND ACUTE 
	case 0x1EB0: r = "A"; break; //LATIN CAPITAL LETTER A WITH BREVE AND GRAVE 
	case 0x1EB1: r = "a"; break; //LATIN SMALL LETTER A WITH BREVE AND GRAVE 
	case 0x1EB2: r = "A"; break; //LATIN CAPITAL LETTER A WITH BREVE AND HOOK ABOVE 
	case 0x1EB3: r = "a"; break; //LATIN SMALL LETTER A WITH BREVE AND HOOK ABOVE 
	case 0x1EB4: r = "A"; break; //LATIN CAPITAL LETTER A WITH BREVE AND TILDE 
	case 0x1EB5: r = "a"; break; //LATIN SMALL LETTER A WITH BREVE AND TILDE 
	case 0x1EB6: r = "A"; break; //LATIN CAPITAL LETTER A WITH BREVE AND DOT BELOW 
	case 0x1EB7: r = "a"; break; //LATIN SMALL LETTER A WITH BREVE AND DOT BELOW 
	case 0x1EB8: r = "E"; break; //LATIN CAPITAL LETTER E WITH DOT BELOW 
	case 0x1EB9: r = "e"; break; //LATIN SMALL LETTER E WITH DOT BELOW 
	case 0x1EBA: r = "E"; break; //LATIN CAPITAL LETTER E WITH HOOK ABOVE 
	case 0x1EBB: r = "e"; break; //LATIN SMALL LETTER E WITH HOOK ABOVE 
	case 0x1EBC: r = "E"; break; //LATIN CAPITAL LETTER E WITH TILDE 
	case 0x1EBD: r = "e"; break; //LATIN SMALL LETTER E WITH TILDE 
	case 0x1EBE: r = "E"; break; //LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND ACUTE 
	case 0x1EBF: r = "e"; break; //LATIN SMALL LETTER E WITH CIRCUMFLEX AND ACUTE 
	case 0x1EC0: r = "E"; break; //LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND GRAVE 
	case 0x1EC1: r = "e"; break; //LATIN SMALL LETTER E WITH CIRCUMFLEX AND GRAVE 
	case 0x1EC2: r = "E"; break; //LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE 
	case 0x1EC3: r = "e"; break; //LATIN SMALL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE 
	case 0x1EC4: r = "E"; break; //LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND TILDE 
	case 0x1EC5: r = "e"; break; //LATIN SMALL LETTER E WITH CIRCUMFLEX AND TILDE 
	case 0x1EC6: r = "E"; break; //LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND DOT BELOW 
	case 0x1EC7: r = "e"; break; //LATIN SMALL LETTER E WITH CIRCUMFLEX AND DOT BELOW 
	case 0x1EC8: r = "I"; break; //LATIN CAPITAL LETTER I WITH HOOK ABOVE 
	case 0x1EC9: r = "i"; break; //LATIN SMALL LETTER I WITH HOOK ABOVE 
	case 0x1ECA: r = "I"; break; //LATIN CAPITAL LETTER I WITH DOT BELOW 
	case 0x1ECB: r = "i"; break; //LATIN SMALL LETTER I WITH DOT BELOW 
	case 0x1ECC: r = "O"; break; //LATIN CAPITAL LETTER O WITH DOT BELOW 
	case 0x1ECD: r = "o"; break; //LATIN SMALL LETTER O WITH DOT BELOW 
	case 0x1ECE: r = "O"; break; //LATIN CAPITAL LETTER O WITH HOOK ABOVE 
	case 0x1ECF: r = "o"; break; //LATIN SMALL LETTER O WITH HOOK ABOVE 
	case 0x1ED0: r = "O"; break; //LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND ACUTE 
	case 0x1ED1: r = "o"; break; //LATIN SMALL LETTER O WITH CIRCUMFLEX AND ACUTE 
	case 0x1ED2: r = "O"; break; //LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND GRAVE 
	case 0x1ED3: r = "o"; break; //LATIN SMALL LETTER O WITH CIRCUMFLEX AND GRAVE 
	case 0x1ED4: r = "O"; break; //LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE 
	case 0x1ED5: r = "o"; break; //LATIN SMALL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE 
	case 0x1ED6: r = "O"; break; //LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND TILDE 
	case 0x1ED7: r = "o"; break; //LATIN SMALL LETTER O WITH CIRCUMFLEX AND TILDE 
	case 0x1ED8: r = "O"; break; //LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND DOT BELOW 
	case 0x1ED9: r = "o"; break; //LATIN SMALL LETTER O WITH CIRCUMFLEX AND DOT BELOW 
	case 0x1EDA: r = "O"; break; //LATIN CAPITAL LETTER O WITH HORN AND ACUTE 
	case 0x1EDB: r = "o"; break; //LATIN SMALL LETTER O WITH HORN AND ACUTE 
	case 0x1EDC: r = "O"; break; //LATIN CAPITAL LETTER O WITH HORN AND GRAVE 
	case 0x1EDD: r = "o"; break; //LATIN SMALL LETTER O WITH HORN AND GRAVE 
	case 0x1EDE: r = "O"; break; //LATIN CAPITAL LETTER O WITH HORN AND HOOK ABOVE 
	case 0x1EDF: r = "o"; break; //LATIN SMALL LETTER O WITH HORN AND HOOK ABOVE 
	case 0x1EE0: r = "O"; break; //LATIN CAPITAL LETTER O WITH HORN AND TILDE 
	case 0x1EE1: r = "o"; break; //LATIN SMALL LETTER O WITH HORN AND TILDE 
	case 0x1EE2: r = "O"; break; //LATIN CAPITAL LETTER O WITH HORN AND DOT BELOW 
	case 0x1EE3: r = "o"; break; //LATIN SMALL LETTER O WITH HORN AND DOT BELOW 
	case 0x1EE4: r = "U"; break; //LATIN CAPITAL LETTER U WITH DOT BELOW 
	case 0x1EE5: r = "u"; break; //LATIN SMALL LETTER U WITH DOT BELOW 
	case 0x1EE6: r = "U"; break; //LATIN CAPITAL LETTER U WITH HOOK ABOVE 
	case 0x1EE7: r = "u"; break; //LATIN SMALL LETTER U WITH HOOK ABOVE 
	case 0x1EE8: r = "U"; break; //LATIN CAPITAL LETTER U WITH HORN AND ACUTE 
	case 0x1EE9: r = "u"; break; //LATIN SMALL LETTER U WITH HORN AND ACUTE 
	case 0x1EEA: r = "U"; break; //LATIN CAPITAL LETTER U WITH HORN AND GRAVE 
	case 0x1EEB: r = "u"; break; //LATIN SMALL LETTER U WITH HORN AND GRAVE 
	case 0x1EEC: r = "U"; break; //LATIN CAPITAL LETTER U WITH HORN AND HOOK ABOVE 
	case 0x1EED: r = "u"; break; //LATIN SMALL LETTER U WITH HORN AND HOOK ABOVE 
	case 0x1EEE: r = "U"; break; //LATIN CAPITAL LETTER U WITH HORN AND TILDE 
	case 0x1EEF: r = "u"; break; //LATIN SMALL LETTER U WITH HORN AND TILDE 
	case 0x1EF0: r = "U"; break; //LATIN CAPITAL LETTER U WITH HORN AND DOT BELOW 
	case 0x1EF1: r = "u"; break; //LATIN SMALL LETTER U WITH HORN AND DOT BELOW 
	case 0x1EF2: r = "Y"; break; //LATIN CAPITAL LETTER Y WITH GRAVE 
	case 0x1EF3: r = "y"; break; //LATIN SMALL LETTER Y WITH GRAVE 
	case 0x1EF4: r = "Y"; break; //LATIN CAPITAL LETTER Y WITH DOT BELOW 
	case 0x1EF5: r = "y"; break; //LATIN SMALL LETTER Y WITH DOT BELOW 
	case 0x1EF6: r = "Y"; break; //LATIN CAPITAL LETTER Y WITH HOOK ABOVE 
	case 0x1EF7: r = "y"; break; //LATIN SMALL LETTER Y WITH HOOK ABOVE 
	case 0x1EF8: r = "Y"; break; //LATIN CAPITAL LETTER Y WITH TILDE 
	case 0x1EF9: r = "y"; break; //LATIN SMALL LETTER Y WITH TILDE 
	case 0x1EFA: r = "LL"; break; //LATIN CAPITAL LETTER MIDDLE-WELSH LL 
	case 0x1EFB: r = "ll"; break; //LATIN SMALL LETTER MIDDLE-WELSH LL 
	case 0x1EFC: r = "V"; break; //LATIN CAPITAL LETTER MIDDLE-WELSH V 
	case 0x1EFD: r = "v"; break; //LATIN SMALL LETTER MIDDLE-WELSH V 
	case 0x1EFE: r = "Y"; break; //LATIN CAPITAL LETTER Y WITH LOOP 
	case 0x1EFF: r = "y"; break; //LATIN SMALL LETTER Y WITH LOOP 

	case 0x2019: r = "'"; break; 
	default : break;
	}
//        if (r == "#")
//            cout << "Unicode: " << ch << endl;
	return r;
}

string CDoiLookup::Transcode(const CStringUTF8 &input)
{
 	string new_str;
	for (string::const_iterator i = input.begin(); i != input.end(); ++i) 
	{
		TUnicodeSymbol sym = CUtf8::Decode(i);
		if (sym < 0x80)
		{
			new_str += static_cast<char>(sym);
		}
		else
		{
			string replacement = GetSpecialCharacterReplacement(sym);
			new_str += replacement;
		}		
	}
	return new_str;
}

void FixAuthorCap(string &name, bool bApostrophes) // FixCapitalizationInElement in sqnutil2.c
{
    if (!NStr::IsUpper(name))
        return;

    FixCapitalizationInElement(name);
    if (bApostrophes) 
        CapitalizeAfterApostrophe(name);
}


string s_GetValFromChildren (xml::node& node)
{
    string val = "";
    if (node.size() > 0) {
        xml::node::iterator it = node.begin();
        while (it != node.end()) {
            const char* c = node.get_content();
            if (c) {
                val = c;
                if (!NStr::IsBlank(val)) {
                    return val;
                }
            }
            ++it;
        }
    }
    return val;
}



string s_GetTitleSuggestion(const string& iso, const string& title, const string& issn)
{
    if (NStr::IsBlank(iso)) {
        return "";
    }
    string suggest = iso;
    if (!NStr::IsBlank(title) || !NStr::IsBlank(issn)) {
        suggest += "||(";
        if (!NStr::IsBlank(title)) {
            suggest += title;
            if (!NStr::IsBlank(issn)) {
                suggest += ":";
            }
        }
        if (!NStr::IsBlank(issn)) {
            suggest += issn;
        }
        suggest += ")";
    }
    return suggest;
}


void s_GetTitle(xml::node& node, string& iso, string& title, string &issn, vector<string>& titles) 
{
    string name = "";
    string val = "";
    const char * n = node.get_name();
    if (n) {
        name = n;
        if (NStr::EqualNocase(name, "Title")) {
            title = s_GetValFromChildren(node);
        } else if (NStr::EqualNocase(name, "issn")) {
            issn = s_GetValFromChildren(node);
        } else if (NStr::StartsWith(name, "iso", NStr::eNocase)) {
            iso = s_GetValFromChildren(node);
        } else if (NStr::EqualNocase(name, "DocumentSummary")) {
            if (!NStr::IsBlank(iso)) {
                titles.push_back(s_GetTitleSuggestion(iso, title, issn));
            }
            iso = "";
            title = "";
            issn = "";
        }
    }
    if (node.is_text()) {
        
        const char * c = node.get_content();
        if (c) {
            val = c;
        }

    }

    if (node.size() > 0) {
        xml::node::iterator it = node.begin();
        while (it != node.end()) {
            s_GetTitle(*it, iso, title, issn, titles);
            ++it;
        }
    }
        
}


void s_GetJournalIds(const string& old_title, const string& collection, vector<int>& uids)
{
    uids.clear();
    // get ISO JTA titles
    string db("nlmcatalog");

    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(200);

    try {
        ecli.Search(db, old_title + collection, uids);
    } catch (CException& e) {
        LOG_POST(Error << "Error: " << e.GetMsg());
    }     
}

void CDoiLookup::LookupIsojta(string old_title, vector<string> &titles)
{
    vector<int> search_ids;
    // shortcuts
    string shortcut_title = macro::CMacroFunction_ISOJTALookup::s_GetISOShortcut(old_title);
    if (!shortcut_title.empty()) {
        s_GetJournalIds(shortcut_title, "[iso]", search_ids);
    }

    validator::ConvertToEntrezTerm(old_title);
    if (NStr::IsBlank(old_title)) {
        return;
    }   

    if (search_ids.empty()) {
        // try to get IDs from multi
        s_GetJournalIds(old_title, "[issn]", search_ids);
    }

    if (search_ids.empty()) {
        s_GetJournalIds(old_title, "[ti] AND ncbijournals[sb]", search_ids);
    }
    if (search_ids.empty()) {
        s_GetJournalIds(old_title, "[jour]", search_ids);
    }
    if (search_ids.empty()) {
        string title_no_punct = NStr::Replace(old_title, ",", " ");
        NStr::ReplaceInPlace(title_no_punct, "&", " ");
        NStr::ReplaceInPlace(title_no_punct, ";", " ");
        NStr::ReplaceInPlace(title_no_punct, ":", " ");
        vector<string> tokens;
        NStr::Split(title_no_punct, " ", tokens, NStr::fSplit_Tokenize);
        string fields = NStr::Join(tokens, "[All Fields] AND ");
        s_GetJournalIds(fields, "[All Fields] AND ncbijournals[All Fields]", search_ids);
    }
    if (!search_ids.empty()) {
        try {
            string db = "nlmcatalog";
            xml::document docsums;

            CGuiEutilsClient ecli;
            ecli.SetMaxReturn(200);
            
            ecli.Summary(db, search_ids, docsums, "2.0");

            xml::node::iterator node_it = docsums.begin();
            string iso = "", title = "", issn = "";
            while (node_it != docsums.end()) {
                s_GetTitle(*node_it, iso, title, issn, titles);
                node_it++;
            }
            if (!NStr::IsBlank(iso)) {
                titles.push_back(s_GetTitleSuggestion(iso, title, issn));
            }
        } catch (CException& e) {
            LOG_POST(Error << "Error: " << e.GetMsg());
        }
    }    
}

static void ExtractMiddleInitial(string &name, string &initial)
{
    vector<string> parts;
    NStr::Split(name, " ", parts, NStr::fSplit_Tokenize);
    if (parts.size() != 2)
        return;
    if (parts[1].size() != 2)
        return;
    if (!isupper(parts[1][0]) || parts[1][1] != '.')
        return;
    name = parts[0];
    initial = parts[1][0];
}

// http://jsonviewer.stack.hu/ online json viewer
pair<CRef<CPubdesc>, string> CDoiLookup::GetPubFromCrossRef(const string &doi)
{
    if (doi.empty())
	return make_pair(CRef<CPubdesc>(NULL), "Empty doi request");

    CHttpSession session;
    int response_timeout = CGuiRegistry::GetInstance().GetInt("GBENCH.System.SeqConfigTimeout", 40);
    CHttpResponse response = session.Get(CUrl("https://api.crossref.org/works/" + NStr::URLEncode(doi)), CTimeout(response_timeout), 0);
    if (response.GetStatusCode() != 200)
        return make_pair(CRef<CPubdesc>(NULL), "Server api.crossref.org returned error code: " + NStr::IntToString(response.GetStatusCode()) + " \n" + response.GetStatusText());
    stringstream ss;
    NcbiStreamCopy(ss, response.ContentStream());
    string json = ss.str();
    if (json.empty())
	return make_pair(CRef<CPubdesc>(NULL), "Empty json object returned from api.crossref.org");
    
    CJson_Document doc;
    doc.ParseString(json);
    
    
    if (!doc.ReadSucceeded() || !doc.IsObject())
	return make_pair(CRef<CPubdesc>(NULL), "Unable to parse json returned from api.crossref.org");
    CJson_ConstObject obj = doc.GetObject();
    if ( !obj.has("status"))
	return make_pair(CRef<CPubdesc>(NULL), "Record status is invalid");
    if (obj["status"].GetValue().GetString() != "ok")
        return make_pair(CRef<CPubdesc>(NULL), "Record status is " + obj["status"].GetValue().GetString());
    if (!obj.has("message") || !obj["message"].IsObject())
	return make_pair(CRef<CPubdesc>(NULL), "Empty record returned from api.crossref.org");
    
    CJson_ConstObject msg = obj["message"].GetObject();

    CRef<CPub> pub(new CPub);

    vector<string> issn;
    if (msg.has("ISSN") && msg["ISSN"].IsArray())
    {
        for (size_t i = 0; i <  msg["ISSN"].GetArray().size(); i++)
	 {
	     if (msg["ISSN"].GetArray()[i].IsValue() && msg["ISSN"].GetArray()[i].GetValue().IsString())
             {
                 string value = msg["ISSN"].GetArray()[i].GetValue().GetString();
                 if (!value.empty())
                     issn.push_back(value);
             }
         }
    }

    string journal_title;
    //  "container-title" "short-container-title"
    if (msg.has("container-title")
	&& msg["container-title"].IsArray() 
	&& !msg["container-title"].GetArray().empty()
	&& msg["container-title"].GetArray().front().IsValue()
	&& msg["container-title"].GetArray().front().GetValue().IsString())
    {
        journal_title = CHTMLHelper::StripHTML(Transcode(msg["container-title"].GetArray().front().GetValue().GetString()));
    }
    else if (msg.has("institution") && msg["institution"].IsObject() && msg["institution"].GetObject().has("name")
             && msg["institution"].GetObject()["name"].IsValue() && msg["institution"].GetObject()["name"].GetValue().IsString())
    {
        journal_title = CHTMLHelper::StripHTML(Transcode(msg["institution"].GetObject()["name"].GetValue().GetString()));
    }
    
    if (!journal_title.empty())
        issn.push_back(journal_title);
    
    bool is_isojta = false;
    for (const auto& old_title : issn)
    {
        vector<string> titles;
        LookupIsojta(old_title, titles);
        if (titles.size() == 1) 
        {
            string new_title = titles[0];
            size_t pos = NStr::Find(new_title, "||");
            if (pos != string::npos) 
            {
                new_title = new_title.substr(0, pos);
            }
            if (!NStr::IsBlank(new_title)) 
            {
                journal_title = new_title;
                is_isojta = true;
                break;
            }
        }
    }
    CRef< CTitle::C_E > title(new CTitle::C_E);
    if (is_isojta)
        title->SetIso_jta(journal_title);
    else
        title->SetName(journal_title);
    pub->SetArticle().SetFrom().SetJournal().SetTitle().Set().push_back(title);
    
    
    if (msg.has("issue") && msg["issue"].IsValue() &&
	msg["issue"].GetValue().IsString())
    {
	pub->SetArticle().SetFrom().SetJournal().SetImp().SetIssue(CHTMLHelper::StripHTML(Transcode(msg["issue"].GetValue().GetString())));
    }

    if (msg.has("volume") && msg["volume"].IsValue() &&
	msg["volume"].GetValue().IsString())
    {
	pub->SetArticle().SetFrom().SetJournal().SetImp().SetVolume(CHTMLHelper::StripHTML(Transcode(msg["volume"].GetValue().GetString())));
    }
    
    if (msg.has("page") && msg["page"].IsValue() &&
	msg["page"].GetValue().IsString())
    {
	pub->SetArticle().SetFrom().SetJournal().SetImp().SetPages(CHTMLHelper::StripHTML(Transcode(msg["page"].GetValue().GetString())));
        pub->SetArticle().SetFrom().SetJournal().SetImp().SetPubstatus(4);
    }

     if (msg.has("title") && msg["title"].IsArray() 
	 && !msg["title"].GetArray().empty()
	 && msg["title"].GetArray().front().IsValue()
	 && msg["title"].GetArray().front().GetValue().IsString())
    {
	CRef< CTitle::C_E > title(new CTitle::C_E);
	title->SetName(CHTMLHelper::StripHTML(Transcode(msg["title"].GetArray().front().GetValue().GetString())));
	pub->SetArticle().SetTitle().Set().push_back(title);
    }
    
     if (msg.has("author") && msg["author"].IsArray())
     {
	 //bool first_affil = true;
	 for (size_t i = 0; i <  msg["author"].GetArray().size(); i++)
	 {
	     if (!msg["author"].GetArray()[i].IsObject())
		 continue;
	     CJson_ConstObject author = msg["author"].GetArray()[i].GetObject();
	     CRef< CAuthor > auth(new CAuthor);
	     if (author.has("given") && author["given"].IsValue() && author["given"].GetValue().IsString())
	     {
                 string name = CHTMLHelper::StripHTML(Transcode(author["given"].GetValue().GetString()));
                 FixAuthorCap(name, false);
                 string initial;
                 ExtractMiddleInitial(name, initial);
		 auth->SetName().SetName().SetFirst(name);
                 if (!initial.empty())
                     auth->SetName().SetName().SetInitials(initial);
	     }
	     if (author.has("family") && author["family"].IsValue() && author["family"].GetValue().IsString())
	     {
                 string name = CHTMLHelper::StripHTML(Transcode(author["family"].GetValue().GetString()));
                 FixAuthorCap(name, true);
		 auth->SetName().SetName().SetLast(name);
	     }

	     if (auth->IsSetName())
		 pub->SetArticle().SetAuthors().SetNames().SetStd().push_back(auth);
/* // Do not populate affiliation even when available
	     vector<string> affil;
	     if (author.has("affiliation") && author["affiliation"].IsArray())
	     {
		 for (size_t j = 0; j < author["affiliation"].GetArray().size(); j++)
		 {
		     if (!author["affiliation"].GetArray()[j].IsObject())
			 continue;
		     CJson_ConstObject name = author["affiliation"].GetArray()[j].GetObject();
		      if (name.has("name") && name["name"].IsValue() && name["name"].GetValue().IsString() && !name["name"].GetValue().GetString().empty())
			  affil.push_back(name["name"].GetValue().GetString());
		 }
	     }
	     if (!affil.empty() && first_affil)
	     {
		 pub->SetArticle().SetAuthors().SetAffil().SetStr(NStr::Join(affil, ", "));
		 first_affil = false;
	     }
*/
	 }
     }

     string date;
     if (msg.has("published-print") && msg["published-print"].IsObject())
     {
	 date = "published-print";
	 pub->SetArticle().SetFrom().SetJournal().SetImp().SetPubstatus(4);
     }
     else if (msg.has("published-online") && msg["published-online"].IsObject())
     {
	 date = "published-online";
         // pub->SetArticle().SetFrom().SetJournal().SetImp().SetPubstatus(3);
     }
     else if (msg.has("posted") && msg["posted"].IsObject())
     {
         date = "posted";
     }

     if ( !date.empty()
	  && msg[date].GetObject().has("date-parts")
	  && msg[date].GetObject()["date-parts"].IsArray()
	  && !msg[date].GetObject()["date-parts"].GetArray().empty()
	  && msg[date].GetObject()["date-parts"].GetArray().front().IsArray()
	  && !msg[date].GetObject()["date-parts"].GetArray().front().GetArray().empty())
     {
	 
	 if (msg[date].GetObject()["date-parts"].GetArray().front().GetArray()[0].IsValue()
	     && msg[date].GetObject()["date-parts"].GetArray().front().GetArray()[0].GetValue().IsInt4())
	 {
	     int year = msg[date].GetObject()["date-parts"].GetArray().front().GetArray()[0].GetValue().GetInt4();
	     pub->SetArticle().SetFrom().SetJournal().SetImp().SetDate().SetStd().SetYear(year);
	 }
	 if (msg[date].GetObject()["date-parts"].GetArray().front().GetArray().size() > 1
	     && msg[date].GetObject()["date-parts"].GetArray().front().GetArray()[1].IsValue()
	     && msg[date].GetObject()["date-parts"].GetArray().front().GetArray()[1].GetValue().IsInt4())
	 {
	     int month = msg[date].GetObject()["date-parts"].GetArray().front().GetArray()[1].GetValue().GetInt4();
	     pub->SetArticle().SetFrom().SetJournal().SetImp().SetDate().SetStd().SetMonth(month);
	 }
	 if (msg[date].GetObject()["date-parts"].GetArray().front().GetArray().size() > 2
	     && msg[date].GetObject()["date-parts"].GetArray().front().GetArray()[2].IsValue()
	     && msg[date].GetObject()["date-parts"].GetArray().front().GetArray()[2].GetValue().IsInt4())
	 {
	     int day = msg[date].GetObject()["date-parts"].GetArray().front().GetArray()[2].GetValue().GetInt4();
	     pub->SetArticle().SetFrom().SetJournal().SetImp().SetDate().SetStd().SetDay(day);
	 }
     }

     CRef< CArticleId > doi_id(new CArticleId);
     doi_id->SetDoi(CDOI(doi));
     pub->SetArticle().SetIds().Set().push_back(doi_id);
    

    CRef<CPubdesc> pubdesc(new CPubdesc);
    pubdesc->SetPub().Set().push_back(pub);
    return make_pair(pubdesc, "");
}

END_NCBI_SCOPE


