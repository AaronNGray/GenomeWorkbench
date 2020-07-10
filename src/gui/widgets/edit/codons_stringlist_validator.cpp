/*  $Id: codons_stringlist_validator.cpp 37586 2017-01-25 15:53:12Z filippov $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <objects/seqfeat/Trna_ext.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <util/sequtil/sequtil_convert.hpp>

#include <serial/objectinfo.hpp>

#include <gui/widgets/edit/codons_stringlist_validator.hpp>
#include "string_list_ctrl.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CSerialStringListValidator
 */


CCodonsStringListValidator::CCodonsStringListValidator(CSerialObject& object)
    : m_Object(object)
{
}


CCodonsStringListValidator::CCodonsStringListValidator(const CCodonsStringListValidator& val)
    : wxValidator(), m_Object(val.m_Object)
{
}


bool CCodonsStringListValidator::TransferToWindow()
{
    CStringListCtrl *control = (CStringListCtrl*)m_validatorWindow;
    control->Clear();
    size_t maxStringsCount =  control->GetMaxStringsCount();

    const CTrna_ext& trna = dynamic_cast<const CTrna_ext&>(m_Object);
    if (trna.IsSetCodon()) {
        ITERATE( CTrna_ext::TCodon, iter, trna.GetCodon() ) {
            if (*iter == 255 || *iter > 63) continue;

            string codon = CGen_code_table::IndexToCodon(*iter);
            control->AddString(codon);
            if (--maxStringsCount == 0)
                break;
		    }
	  }

    if (maxStringsCount > 0)
        control->AddString("");

    control->FitInside();

    return true;
}


static bool s_IsATGC(char ch)
{
    if (ch == 'A' || ch == 'T' || ch == 'G' || ch == 'C' || ch == 'U') {
        return true;
    } else {
        return false;
    }
}


static const  string kAmbiguities = "MRSVWYHKDBN";
static const string kReplacements[] = {
  "AC", "AG", "CG", "ACG", "AT", "CT", "ACT", "GT", "AGT", "CGT", "ACGT" };

static const string s_GetExpansion (const string& ch)
{
    size_t pos = NStr::Find(kAmbiguities, ch);
    if (pos != string::npos) {
        return kReplacements[pos];
    } else {
        return ch;
    }
}


static vector<string> ParseDegenerateCodons (string codon)
{
    vector<string> replacements;

    if (codon.length() == 3 && s_IsATGC(codon.c_str()[0])) {
        string this_codon = codon.substr(0, 1);
        replacements.push_back(this_codon);

        for (int i = 1; i < 3; i++) {
            string ch = s_GetExpansion (codon.substr(i, 1));
            int num_now = replacements.size();
            // add copies for each expansion letter beyond the first
            for (unsigned int j = 1; j < ch.length(); j++) {
                for (int k = 0; k < num_now; k++) {
                    string cpy = replacements[k];
                    replacements.push_back(cpy);
                }
            }
            for (int k = 0; k < num_now; k++) {
                for (unsigned int j = 0; j < ch.length(); j++) {
                    replacements[j * num_now + k].append(ch.substr(j, 1));
                }
            }
        }   
    } else {
        replacements.push_back(codon);
    }
    return replacements;
}


bool CCodonsStringListValidator::TransferFromWindow()
{
    CTrna_ext& trna = dynamic_cast<CTrna_ext&>(m_Object);

    // reset existing codons
    trna.ResetCodon();

    //now add new ones
    CStringListCtrl  *control = (CStringListCtrl *)m_validatorWindow;
    CStringListCtrl::CConstIterator it(*control);

    if (it) {
        for (; it; ++it) {
            string codon = it.GetValue();
            NStr::TruncateSpacesInPlace(codon);
            NStr::ToUpper(codon);
            if (!codon.empty()) {
                vector<string> codons = ParseDegenerateCodons(codon);
                for (unsigned int j = 0; j < codons.size(); j++) {
                    int val = CGen_code_table::CodonToIndex(codons[j]);
                    if (val > -1) {
                        trna.SetCodon().push_back(val);
                    }
                }
            }
        }
    }
    return true;
}

END_NCBI_SCOPE
