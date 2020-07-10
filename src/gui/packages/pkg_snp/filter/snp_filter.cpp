/*  $Id: snp_filter.cpp 36594 2016-10-12 20:17:36Z evgeniev $
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
 * Authors:  Melvin Quintos
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <sstream>
#include <vector>

#include <gui/widgets/snp/filter/snp_filter.hpp>
#include <stdio.h>

BEGIN_NCBI_SCOPE

/* The Structure of the serialized filter is as follows:
Three parts separated by a pipe '|'
<version>|<name>|<fields>

<version> is the version of the filter format. It is an integer starting at 1

<name> is the name of the filter.  It can contain any alpha numeric character.

<fields> are encoded properties grouped into 'words' separated by spaces.
A detailed explanation follows:

The first 'word' is 9 characters representing the boolean 'check' variables.
The order is:
1 checkLinks;
2 checkGeneFxn;
3 checkMapping;
4 checkWeight;
5 checkAlleleFreqList;
6 checkAlleleFreqChoice;
7 checkGaP_Hapmap;
8 checkVarClass;
9 checkQualityCheck;

The next 3 words are integer values of the choices
The order is:
1 choiceWeight;
2 choiceAlleleFreq;
3 choiceVarClass;

The next 6 words are lists of ids of the specific properties.  Values are separated
by the ':' character.

The special character '_' denotes an empty list
The order is:
1 listLinks;
2 listGeneFxns
3 listMappings;
4 listAlleleFreqs;
5 listGaP_Hapmaps;
6 listQualityChecks;

example:
1|My Custom Filter|100000011 -1 -1 0 12:14:43:48 _ _ _ _ 2:3
*/

static const string kIdDelimiter = ":";
static const string kPartDelimiter = "|";
static const string kEscapedPartDelimiter = "\\|";

SSnpFilter::SSnpFilter()
{
    Clear(); // zeroize filter
}

SSnpFilter::SSnpFilter(const string &str)
{
    SerializeFrom(str);
}

void SSnpFilter::Clear()
{
    checkLinks          = checkGeneFxn          = checkMapping          =
    checkWeight         = checkAlleleFreqList   = checkAlleleFreqChoice =
    checkGaP_Hapmap     = checkVarClass         = checkQualityCheck     = false;


    choiceWeight = choiceAlleleFreq = choiceVarClass = 0;

    listLinks.clear();
    listGeneFxns.clear();
    listMappings.clear();
    listAlleleFreqs.clear();
    listGaP_Hapmaps.clear();
    listQualityChecks.clear();
}

void SSnpFilter::SerializeFrom(string input)
{
    Clear();

    // there is a possibility that the pipe "|" may come escaped depending on the source of the filter,
    // so it must be replaced with the canonic form
    NStr::ReplaceInPlace(input, kEscapedPartDelimiter, kPartDelimiter);

    typedef vector<string> TListString;
    TListString tokens;
    string strFields;

    // parse data
    NStr::Split(input, kPartDelimiter, tokens);

    // determine version
    if (tokens.size() < 2) {
        return;
    }
    else if (tokens.size() == 2) {
        // No version number
        name        = tokens.at(0);
        strFields   = tokens.at(1);
    }
    else {
        // We have a version.
        // For now, there is only 1 possible version number
        // later version formats will need to handle the data differently
        // version = tokens.at(0);  // will be '1', so skip for now
        name        = tokens.at(1);
        strFields   = tokens.at(2);
    }

    // Parse the specific properties
    stringstream s(strFields);

    try {{

    // first word
    string checks;
    s >> checks;

    checkLinks              = (checks.at(0)=='0') ? false : true;
    checkGeneFxn            = (checks.at(1)=='0') ? false : true;
    checkMapping            = (checks.at(2)=='0') ? false : true;
    checkWeight             = (checks.at(3)=='0') ? false : true;
    checkAlleleFreqList     = (checks.at(4)=='0') ? false : true;
    checkAlleleFreqChoice   = (checks.at(5)=='0') ? false : true;
    checkGaP_Hapmap         = (checks.at(6)=='0') ? false : true;
    checkVarClass           = (checks.at(7)=='0') ? false : true;
    checkQualityCheck       = (checks.at(8)=='0') ? false : true;

    // next 3 words for choices
    s >> choiceWeight >> choiceAlleleFreq >> choiceVarClass;

    // next 6 words for lists
    string word;

    // List Links
    s >> word;
    if (word.at(0) != '_') {
        tokens.clear();
        NStr::Split(word, kIdDelimiter, tokens);

        ITERATE(TListString, iter, tokens) {
            int val = NStr::StringToInt(*iter);
            listLinks.push_back(val);
        }
    }

    // List Gene Functions
    s >> word;
    if (word.at(0) != '_') {
        tokens.clear();
        NStr::Split(word, kIdDelimiter, tokens);

        ITERATE(TListString, iter, tokens) {
            int val = NStr::StringToInt(*iter);
            listGeneFxns.push_back(val);
        }
    }

    // List Mappings
    s >> word;
    if (word.at(0) != '_') {
        tokens.clear();
        NStr::Split(word, kIdDelimiter, tokens);

        ITERATE(TListString, iter, tokens) {
            int val = NStr::StringToInt(*iter);
            listMappings.push_back(val);
        }
    }

    // List AlleleFreqs
    s >> word;
    if (word.at(0) != '_') {
        tokens.clear();
        NStr::Split(word, kIdDelimiter, tokens);

        ITERATE(TListString, iter, tokens) {
            int val = NStr::StringToInt(*iter);
            listAlleleFreqs.push_back(val);
        }
    }

    // List Gap_Hapmaps
    s >> word;
    if (word.at(0) != '_') {
        tokens.clear();
        NStr::Split(word, kIdDelimiter, tokens);

        ITERATE(TListString, iter, tokens) {
            int val = NStr::StringToInt(*iter);
            listGaP_Hapmaps.push_back(val);
        }
    }

    // List Quality Checks
    s >> word;
    if (word.at(0) != '_') {
        tokens.clear();
        NStr::Split(word, kIdDelimiter, tokens);

        ITERATE(TListString, iter, tokens) {
            int val = NStr::StringToInt(*iter);
            listQualityChecks.push_back(val);
        }
    }

    }}
    catch (std::out_of_range &) { /*ignore out of range */ }
    catch (CException &) { /*ignore exceptions from NStr operations */ }

}

void SSnpFilter::SerializeTo(string &output) const
{
    output.clear();

    // version
    {{
        output = "1|";
    }}

    // name
    {{
        output += name + "|";
    }}

    // first word
    {{
    char checks[11]; // 11 chars for word, space, and end of string characters
    sprintf(checks, "%1d%1d%1d%1d%1d%1d%1d%1d%1d ",
                    checkLinks,
                    checkGeneFxn,
                    checkMapping,
                    checkWeight,
                    checkAlleleFreqList,
                    checkAlleleFreqChoice,
                    checkGaP_Hapmap,
                    checkVarClass,
                    checkQualityCheck);
    output += checks;
    }}

    // Next 3 words
    {{
        output += NStr::IntToString(choiceWeight)       + " ";
        output += NStr::IntToString(choiceAlleleFreq)   + " ";
        output += NStr::IntToString(choiceVarClass);
    }}

    // Next 6 words
    {{
        output += " ";

        // links
        if (listLinks.empty()) {
            output += "_ ";
        }
        else {
            ITERATE(SSnpFilter::TList, iter, listLinks) {
                string val = NStr::IntToString(*iter);
                output += val + kIdDelimiter;
            }
            output.replace(output.length()-1, 1, " "); // replace the last delimitter with space
        }

        // gene fxn
        if (listGeneFxns.empty()) {
            output += "_ ";
        }
        else {
            ITERATE(SSnpFilter::TList, iter, listGeneFxns) {
                string val = NStr::IntToString(*iter);
                output += val + kIdDelimiter;
            }
            output.replace(output.length()-1, 1, " "); // replace the last delimitter with space
        }

        // mapping
        if (listMappings.empty()) {
            output += "_ ";
        }
        else {
            ITERATE(SSnpFilter::TList, iter, listMappings) {
                string val = NStr::IntToString(*iter);
                output += val + kIdDelimiter;
            }
            output.replace(output.length()-1, 1, " "); // replace the last delimitter with space
        }

        // allelefreq
        if (listAlleleFreqs.empty()) {
            output += "_ ";
        }
        else {
            ITERATE(SSnpFilter::TList, iter, listAlleleFreqs) {
                string val = NStr::IntToString(*iter);
                output += val + kIdDelimiter;
            }
            output.replace(output.length()-1, 1, " "); // replace the last delimitter with space
        }

        // GaP Hapmap
        if (listGaP_Hapmaps.empty()) {
            output += "_ ";
        }
        else {
            ITERATE(SSnpFilter::TList, iter, listGaP_Hapmaps) {
                string val = NStr::IntToString(*iter);
                output += val + kIdDelimiter;
            }
            output.replace(output.length()-1, 1, " "); // replace the last delimitter with space
        }

        // Quality checks
        if (listQualityChecks.empty()) {
            output += "_ ";
        }
        else {
            ITERATE(SSnpFilter::TList, iter, listQualityChecks) {
                string val = NStr::IntToString(*iter);
                output += val + kIdDelimiter;
            }
            output.erase(output.length()-1, 1); // erase the last delimitter
        }
    }}
}

bool SSnpFilter::Passes(const CSnpBitfield &b) const
{
    bool pass = true;

    if (pass && checkWeight) {
        pass = (choiceWeight == b.GetWeight());
    }
    if (pass && checkVarClass) {
        pass = (choiceVarClass == b.GetVariationClass());
    }
    if (pass && checkMapping) {
        ITERATE(SSnpFilter::TList, iter, listMappings) {
            CSnpBitfield::EProperty prop;
            prop = (CSnpBitfield::EProperty)(*iter);
            pass = b.IsTrue(prop);
            if (pass == false)
                break;
        }
    }
    if (pass && checkAlleleFreqChoice) {
        CSnpBitfield::EProperty prop;
        prop = (CSnpBitfield::EProperty)choiceAlleleFreq;
        pass = b.IsTrue(prop);
    }
    if (pass && checkAlleleFreqList) {
        ITERATE(SSnpFilter::TList, iter, listAlleleFreqs) {
            CSnpBitfield::EProperty prop;
            prop = (CSnpBitfield::EProperty)(*iter);
            pass = b.IsTrue(prop);
            if (pass == false)
                break;
        }
    }
    if (pass && checkGaP_Hapmap) {
        ITERATE(SSnpFilter::TList, iter, listGaP_Hapmaps) {
            CSnpBitfield::EProperty prop;
            prop = (CSnpBitfield::EProperty)(*iter);
            pass = b.IsTrue(prop);
            if (pass == false)
                break;
        }
    }
    if (pass && checkGeneFxn) {
        // for this property, we do a logical 'OR'.
        // As long as one property is true, the test passes
        bool has_prop = false;
        ITERATE(SSnpFilter::TList, iter, listGeneFxns) {
            CSnpBitfield::EFunctionClass prop;
            prop = (CSnpBitfield::EFunctionClass)(*iter);
            has_prop = b.IsTrue(prop);
            if (has_prop)
                break;
        }
        pass = has_prop;
    }
    if (pass && checkLinks) {
        ITERATE(SSnpFilter::TList, iter, listLinks) {
            CSnpBitfield::EProperty prop;
            prop = (CSnpBitfield::EProperty)(*iter);
            pass = b.IsTrue(prop);
            if (pass == false)
                break;
        }
    }
    if (pass && checkQualityCheck) {
        ITERATE(SSnpFilter::TList, iter, listQualityChecks) {
            CSnpBitfield::EProperty prop;
            prop = (CSnpBitfield::EProperty)(*iter);
            pass = b.IsTrue(prop);
            if (pass == false)
                break;
        }
    }

    return pass;
}

END_NCBI_SCOPE

