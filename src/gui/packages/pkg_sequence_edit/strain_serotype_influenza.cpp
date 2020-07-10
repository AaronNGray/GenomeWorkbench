/*  $Id: strain_serotype_influenza.cpp 42189 2019-01-09 22:13:08Z asztalos $
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
 * Authors:  Andrea Asztalos
 */

#include <ncbi_pch.hpp>
#include <objmgr/feat_ci.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objtools/edit/source_edit.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/strain_serotype_influenza.hpp>

#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> CStrainSerotypeInfluenza::GetCommand(CSeq_entry_Handle tse, const string& title)
{
    if (!tse)
        return CRef<CCmdComposite>();

    m_Seh = tse;
    CRef<CCmdComposite> composite(new CCmdComposite(title));
    if (m_Type != eCommandInfluenza_parse) {
        x_ApplyToSeqAndFeat(composite);
    }
    else {
        x_ParseStrainSerotype(composite);
    }
    return composite;
}

CRef<CCmdComposite> TestCStrainSerotypeInfluenza_Parse(CStrainSerotypeInfluenza& parser, CSeq_entry_Handle tse, const string& title, bool overwrite)
{
    if (!tse)
        return CRef<CCmdComposite>();

    parser.m_Seh = tse;
    CRef<CCmdComposite> composite(new CCmdComposite(title));
    parser.x_ParseStrainSerotype(composite, overwrite);
    return composite;
}

bool CStrainSerotypeInfluenza::x_ApplyToBioSource (CBioSource& biosource) 
{
    bool modified=false;
    switch (m_Type) {
        case eCommandInfluenza_add:
            modified = x_AddToInfluenzaNames(biosource);
            break;
        case eCommandInfluenza_fixup:
            modified = x_FixupOrganismNames(biosource);
            break;
        case eCommandSplitQualsAtCommas:
            modified = x_SplittingQualifiersAtCommas(biosource);
            break;
        case eCommandSplitStructuredCollections:
            modified = x_SplittingStructCollections(biosource);
            break;
        case eCommandTrimOrganismNames:
            modified = x_TrimmingOrganismNames(biosource);
            break;
        default:
            break;
    }
    return modified;
}

bool CStrainSerotypeInfluenza::x_AddToInfluenzaNames(CBioSource& biosource)
{
    if (!biosource.IsSetTaxname() || !NStr::StartsWith(biosource.GetTaxname(), "Influenza A virus", NStr::eNocase))
        return false;

    bool modified = false;
    if (biosource.IsSetOrgMod()) {
        string strain_str, serotype_str;
        bool found_strain = false, found_serotype = false;

        FOR_EACH_ORGMOD_ON_BIOSOURCE(orgmod, biosource)
            if ((*orgmod)->IsSetSubtype() && (*orgmod)->IsSetSubname()) {
                if ((*orgmod)->GetSubtype() == COrgMod::eSubtype_strain) {
                    strain_str = (*orgmod)->GetSubname();
                    found_strain = true;
                }
                if ((*orgmod)->GetSubtype() == COrgMod::eSubtype_serotype) {
                    serotype_str = (*orgmod)->GetSubname();
                    found_serotype = true;
                }

            }
        if (!found_strain || !found_serotype)
            return false;
        // both modifiers should be present

        string infl_name = "Influenza A virus";
        string infl_name_wstrain = infl_name + " (" + strain_str + ")";
        string new_name = infl_name + " (" + strain_str + "(" + serotype_str + "))";

        string old_taxname = biosource.GetOrg().GetTaxname();
        NStr::TruncateSpacesInPlace(old_taxname);

        if (NStr::Equal(old_taxname, infl_name, NStr::eNocase)
            || NStr::Equal(old_taxname, infl_name_wstrain, NStr::eNocase)) {
            biosource.SetOrg().SetTaxname(new_name);
            modified = true;
            modified |= edit::CleanupForTaxnameChange(biosource);
        }
    }
    return modified;
}

bool CStrainSerotypeInfluenza::x_FixupOrganismNames(CBioSource& biosource)
{
    if (!biosource.IsSetTaxname() || !NStr::StartsWith(biosource.GetTaxname(), "Influenza A virus", NStr::eNocase))
        return false;

    bool modified = false;
    string new_taxname = biosource.GetOrg().GetTaxname();
    while (NStr::FindNoCase(new_taxname, "  ") != NPOS) {
        NStr::ReplaceInPlace(new_taxname, "  ", " ");
    }

    NStr::ReplaceInPlace(new_taxname, "( ", "(");
    NStr::ReplaceInPlace(new_taxname, " (", "("); // for the case: "strain (sero"
    NStr::ReplaceInPlace(new_taxname, " )", ")");

    // correct for too much space deletion
    SIZE_TYPE pos = NStr::FindNoCase(new_taxname, "virus(");
    if (pos != NPOS)
        new_taxname.insert(pos + 5, " ");

    NStr::TruncateSpacesInPlace(new_taxname);
    if (!NStr::Equal(new_taxname, biosource.GetOrg().GetTaxname(), NStr::eCase)) {
        biosource.SetOrg().SetTaxname(new_taxname);
        modified = true;
        modified |= edit::CleanupForTaxnameChange(biosource);
    }
    return modified;
}

bool CStrainSerotypeInfluenza::x_SplittingQualifiersAtCommas(CBioSource& biosource)
{
    bool modified = false;
    if (biosource.IsSetOrgMod()) {

        bool found_multiple_strain = false;
        list<string> array_str;
        EDIT_EACH_ORGMOD_ON_BIOSOURCE(orgmod, biosource)
            if ((*orgmod)->IsSetSubtype() && (*orgmod)->IsSetSubname()) {
                if ((*orgmod)->GetSubtype() == COrgMod::eSubtype_strain) {
                    string multiple_strain = (*orgmod)->GetSubname();
                    if (NStr::FindNoCase(multiple_strain, ",") != NPOS) {

                        found_multiple_strain = true;
                        NStr::Split(multiple_strain, ",", array_str, NStr::fSplit_Tokenize);
                        NStr::TruncateSpacesInPlace(array_str.front());
                        (*orgmod)->SetSubname() = array_str.front();
                        modified = true;
                    }
                }
            }

        if (found_multiple_strain && array_str.size()>1) {
            list<string>::iterator it = array_str.begin();
            it++;
            for (it; it != array_str.end(); ++it) {
                string strain = *it;
                NStr::TruncateSpacesInPlace(strain);
                if (!strain.empty()) {
                    CRef<COrgMod> new_orgmod(new COrgMod(COrgMod_Base::eSubtype_strain, strain));
                    biosource.SetOrg().SetOrgname().SetMod().push_back(new_orgmod);
                    modified = true;
                }
            }
        }
    }
    return modified;
}

bool CStrainSerotypeInfluenza::x_SplittingStructCollections(CBioSource& biosource)
{
    bool modified = false;
    EDIT_EACH_ORGMOD_ON_BIOSOURCE(orgmod, biosource) {
        if ((*orgmod)->IsSetSubtype() && (*orgmod)->IsSetSubname()) {
            if ((*orgmod)->GetSubtype() == COrgMod::eSubtype_bio_material ||
                (*orgmod)->GetSubtype() == COrgMod::eSubtype_culture_collection ||
                (*orgmod)->GetSubtype() == COrgMod::eSubtype_specimen_voucher) {

                if (NStr::FindNoCase((*orgmod)->GetSubname(), ";") != NPOS) {

                    string multiple_mod = (*orgmod)->GetSubname();
                    COrgMod_Base::TSubtype tsub = (*orgmod)->GetSubtype();
                    bool atleastone = false;  // there is at least one string that is not empty
                    list<string> tmp;
                    list<string>::iterator iter;
                    NStr::Split(multiple_mod, ";", tmp, NStr::fSplit_Tokenize);

                    for (iter = tmp.begin(); iter != tmp.end(); iter++) {
                        NStr::TruncateSpacesInPlace(*iter);
                        if (!(*iter).empty())
                            atleastone = true;
                    }

                    if (atleastone) {
                        iter = tmp.begin();
                        while (iter != tmp.end() && (*iter).empty())
                            iter++;
                        (*orgmod)->SetSubname() = *iter;
                        modified = true;
                        orgmod++;

                        iter++;
                        while (iter != tmp.end()) {
                            if (!(*iter).empty()) {
                                CRef<COrgMod> new_orgmod(new COrgMod(tsub, *iter));
                                biosource.SetOrg().SetOrgname().SetMod().insert(orgmod, new_orgmod);
                                modified = true;
                            }
                            iter++;
                        }
                    }
                    tmp.clear();
                }
            }
        }
    }
    return modified;
}

bool CStrainSerotypeInfluenza::x_TrimmingOrganismNames(CBioSource& biosource)
{
    if (!biosource.IsSetTaxname())
        return false;

    bool modified = false;
    string old_taxname = biosource.GetOrg().GetTaxname();
    NStr::TruncateSpacesInPlace(old_taxname);

    SIZE_TYPE pos = NStr::FindNoCase(old_taxname, " ");
    vector<string> array_str;
    if (pos != NPOS)
        NStr::Split(old_taxname, " ", array_str, NStr::fSplit_Tokenize);
    SIZE_TYPE len = 1;
    if (array_str.size() > len) {
        string new_taxname = kEmptyStr;
        bool uncult = false; // flag to indicate whether it's an uncultured organism 
        if (NStr::Equal(array_str[0], "Uncultured", NStr::eNocase)) {
            uncult = true;
            new_taxname = array_str[0] + " ";
        }

        short index = uncult ? 1 : 0;
        bool is_spec_word = false;

        while (index < (short)array_str.size()
            && ((uncult && index < 4) || (!uncult && index < 3))
            && !is_spec_word) {

            if (NStr::Equal(array_str[index], "sp.", NStr::eNocase) ||
                NStr::Equal(array_str[index], "aff.", NStr::eNocase) ||
                NStr::Equal(array_str[index], "cf.", NStr::eNocase)) {
                is_spec_word = true;
                new_taxname += array_str[index] + " ";
                index++;
                if (index < (short)array_str.size()) {
                    new_taxname += array_str[index];
                }
            }
            else {
                if ((uncult && index < 3) || (!uncult && index < 2)) {
                    new_taxname += array_str[index] + " ";
                }
            }

            index++;
        }

        NStr::TruncateSpacesInPlace(new_taxname, NStr::eTrunc_End);
        if (new_taxname.back() == ':') {
            new_taxname.pop_back();
        }
        if (!NStr::Equal(new_taxname, biosource.GetOrg().GetTaxname(), NStr::eCase)) {
            biosource.SetOrg().SetTaxname() = new_taxname;
            modified = true;
            modified |= edit::CleanupForTaxnameChange(biosource);
        }
    }
    return modified;
}

void CStrainSerotypeInfluenza::x_ParseStrainSerotype(CCmdComposite* composite)
{
    // if this function is updated, please update 
    // void x_ParseStrainSerotype(CSeq_entry_Handle tse, CCmdComposite* composite, bool overwrite) function as well
    // as this does the testing
    int nr_conflicts = 0;
    m_Parsable = false; // if there is at least one seq which can be parsed, this will be TRUE

    x_TestEntryForParsing(*(m_Seh.GetCompleteSeq_entry()), nr_conflicts);
    for (CFeat_CI feat_it(m_Seh, SAnnotSelector(CSeqFeatData::e_Biosrc)); feat_it; ++feat_it) {
        x_TestBiosourceForParsing(feat_it->GetOriginalFeature().GetData().GetBiosrc(), nr_conflicts);
    }

    if (m_Parsable) { // if the Seq_entry is parsable
        bool update;
        if (nr_conflicts > 0) {
            string error = NStr::NumericToString(nr_conflicts) 
                + " affected fields already contain a value. Do you wish to overwrite existing text?";
            int answer = wxMessageBox(ToWxString(error), ToWxString("Warning"),
                wxYES_NO | wxCANCEL | wxICON_QUESTION);

            switch (answer) {
            case wxYES:
                update = true;
                x_ParseEntry(composite, update);
                break;
            case wxNO:
                update = false;
                x_ParseEntry(composite, update);
                break;
            case wxCANCEL:
                break;
            }
        }
        else if (nr_conflicts == 0) {
            update = false;
            x_ParseEntry(composite, update);
        }
    }
}

void CStrainSerotypeInfluenza::x_ParseStrainSerotype(CCmdComposite* composite, bool overwrite)
{
    int nr_conflicts = 0;
    m_Parsable = false;

    x_TestEntryForParsing(*(m_Seh.GetCompleteSeq_entry()), nr_conflicts);
    for (CFeat_CI feat_it(m_Seh, SAnnotSelector(CSeqFeatData::e_Biosrc)); feat_it; ++feat_it) {
        x_TestBiosourceForParsing(feat_it->GetOriginalFeature().GetData().GetBiosrc(), nr_conflicts);
    }

    if (m_Parsable) {
        if (nr_conflicts > 0) {
            x_ParseEntry(composite, overwrite);
        }
        else if (nr_conflicts == 0) {
            x_ParseEntry(composite, false);
        }
    }
}

void CStrainSerotypeInfluenza::x_TestEntryForParsing(const CSeq_entry& se, int& count)
{
    FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
        if ((*it)->IsSource()) {
            x_TestBiosourceForParsing((*it)->GetSource(), count);
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet()) {
            x_TestEntryForParsing(**it, count);
        }
    }
}


void CStrainSerotypeInfluenza::x_TestBiosourceForParsing(const CBioSource& biosource, int& count)
{
    string strain, serotype;
    if (x_IsParsableInfluenzaVirusName (biosource, strain, serotype)){
        m_Parsable = true;
        count += x_GetStrainSerotypeConflicts(biosource, strain, serotype);
    }
}

bool CStrainSerotypeInfluenza::x_IsParsableInfluenzaVirusName(const CBioSource& biosource, string& strain, string& serotype)
{
    /* the name is not parseable if it's not "Influenza A virus" and the strain is missing
    the presence of serotype is optional
    */

    if (!biosource.IsSetTaxname() || !NStr::StartsWith(biosource.GetTaxname(), "Influenza A virus", NStr::eNocase)) {
        return false;
    }

    string taxname = biosource.GetOrg().GetTaxname();
    NStr::TruncateSpacesInPlace(taxname);

    SIZE_TYPE pos = NStr::FindNoCase(taxname, "(");
    list<string> array_str;
    if (pos != NPOS) {
        NStr::Split(taxname, "(", array_str, NStr::fSplit_Tokenize);

        // if there are more strings than strain and serotype
        if (array_str.size() > 3) {
            wxMessageBox(wxT("Warning"),
                wxT("Cannot distinguish the strain from the serotype as there are additional '(' characters."),
                wxOK);
            return false;
        }
        else if (array_str.size() == 3) { // strain and serotype are both specified
            strain = *(++array_str.begin());
            serotype = array_str.back();

            // trimming the last ')' characters from the end if there are any
            NStr::TruncateSpacesInPlace(strain);
            pos = NStr::FindNoCase(strain, ")");
            while (pos != NPOS) {
                strain.erase(pos);
                pos = NStr::FindNoCase(strain, ")");
            }

            pos = NStr::FindNoCase(serotype, ")");
            while (pos != NPOS) {
                serotype.erase(pos);
                pos = NStr::FindNoCase(serotype, ")");
            }
            return true;
        }
        else if (array_str.size() == 2) { // only the strain is specified
            strain = array_str.back();
            pos = NStr::FindNoCase(strain, ")");
            while (pos != NPOS) {
                strain.erase(pos);
                pos = NStr::FindNoCase(strain, ")");
            }
            return true;
        }
    }

    return false;
}

int CStrainSerotypeInfluenza::x_GetStrainSerotypeConflicts(const CBioSource& biosource, const string& strain, const string& serotype)
{
    /* return the number of cases when either the strain or the serotype is different from
    the ones on the record (OrgMod qualifiers)
    */
    int count = 0;
    if (biosource.GetOrg().IsSetOrgname() && biosource.GetOrg().GetOrgname().IsSetMod()) {
        FOR_EACH_ORGMOD_ON_BIOSOURCE(orgmod, biosource) {
            if ((*orgmod)->IsSetSubtype() && (*orgmod)->IsSetSubname()) {
                if ((*orgmod)->GetSubtype() == COrgMod::eSubtype_strain)
                    if (!NStr::Equal((*orgmod)->GetSubname(), strain, NStr::eCase)) {
                        count++;
                    }

                if (!serotype.empty() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_serotype)
                    if (!NStr::Equal((*orgmod)->GetSubname(), serotype, NStr::eCase)) {
                        count++;
                    }
            }
        }
    }

    return count;
}


void CStrainSerotypeInfluenza::x_ParseEntry(CCmdComposite* composite, bool update)
{
    x_ParseDescriptors(*(m_Seh.GetCompleteSeq_entry()), composite, update);

    for (CFeat_CI feat_it(m_Seh, SAnnotSelector(CSeqFeatData::e_Biosrc)); feat_it; ++feat_it) {
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat_it->GetOriginalFeature());
        if (x_ParseBiosource(new_feat->SetData().SetBiosrc(), update)) {
            CRef<CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(*feat_it, *new_feat));
            composite->AddCommand(*cmd);
        }
    }
}

void CStrainSerotypeInfluenza::x_ParseDescriptors(const CSeq_entry& se, CCmdComposite* composite, bool update)
{
    FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
        if ((*it)->IsSource()) {
            const CSeqdesc& orig_desc = **it;
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(orig_desc);
            if (x_ParseBiosource(new_desc->SetSource(), update)) {
                CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(m_Seh.GetScope().GetSeq_entryHandle(se), orig_desc, *new_desc));
                composite->AddCommand(*cmd);
            }
        }
    }
    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet()) {
            x_ParseDescriptors(**it, composite, update);
        }
    }
}

bool CStrainSerotypeInfluenza::x_ParseBiosource(CBioSource& biosource, bool update)
{
    bool modified=false;
    string strain, serotype;
    if (x_IsParsableInfluenzaVirusName(biosource, strain, serotype)){
        modified = x_UpdateStrainSerotype(biosource, strain, serotype, update);
    }
    return modified;
}


bool CStrainSerotypeInfluenza::x_UpdateStrainSerotype(
    CBioSource& biosource, const string& strain, const string& serotype, bool update)
{
    /* if update = true
         update every OrgMod qualifier with the new strain & serotype values
         where necessary, add new OrgMod qualifiers
     
       if update = false
         do not overwrite the existing values with the new strain & serotype
         where necessary, add new OrgMod qualifiers
     */
    
    // the rest of the cases are when new OrgMod qualifiers need to be added to the biosource
    COrgName& orgname = biosource.SetOrg().SetOrgname();
    bool modified = false;
    
    bool found_strain = false, found_serotype = false;
    EDIT_EACH_ORGMOD_ON_BIOSOURCE (orgmod, biosource){
        if((*orgmod)->IsSetSubtype() && (*orgmod)->IsSetSubname()){
            if ((*orgmod)->GetSubtype() == COrgMod::eSubtype_strain) {
                found_strain=true;
                if (update && !NStr::EqualCase((*orgmod)->GetSubname(), strain)){
                    (*orgmod)->SetSubname() = strain;
                    modified = true;
                }
            }
                
            if ((*orgmod)->GetSubtype() == COrgMod::eSubtype_serotype){
                found_serotype=true;
                if (update && !serotype.empty() && !NStr::EqualCase((*orgmod)->GetSubname(), serotype)){
                    (*orgmod)->SetSubname() = serotype;
                    modified=true;
                }
            }
        }	
    }
        
    if (!found_strain){
        CRef<COrgMod> new_strain(new COrgMod(COrgMod_Base::eSubtype_strain, strain));
        orgname.SetMod().push_back(new_strain);
        modified = true;
    }
    if (!serotype.empty() && !found_serotype){
        CRef<COrgMod> new_serotype(new COrgMod(COrgMod_Base::eSubtype_serotype, serotype));
        orgname.SetMod().push_back(new_serotype);
        modified = true;
    }
    return modified;
}

END_NCBI_SCOPE
