/*  $Id: macro_fn_lookup.cpp 41476 2018-08-02 20:44:57Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
  *
 */

#include <ncbi_pch.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/taxon3/taxon3.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objtools/edit/source_edit.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_lookup.hpp>


/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

 
///////////////////////////////////////////////////////////////////////////////////
/// constants used in the uncultured macro tool

static const char* sSpeciesSpecificPrimers[] = {
    "[BankIt_uncultured16S_wizard]; [species_specific primers]; [tgge]",
    "[BankIt_uncultured16S_wizard]; [species_specific primers]; [dgge]",
    "[BankIt_uncultured16S_wizard]; [species_specific primers]",
    "[uncultured (with species-specific primers)]",
    "[uncultured]; [amplified with species-specific primers]",
    "[uncultured (using species-specific primers) bacterial source]",
    "[amplified with species-specific primers]; [uncultured; wizard]",
    "[amplified with species-specific primers]",
    "amplified with species-specific primers",
    NULL
};

static const char* sUniversalPrimers[] = {
    "[BankIt_uncultured16S_wizard]; [universal primers]; [tgge]",
    "[BankIt_uncultured23S_wizard]; [universal primers]; [tgge]",
    "[BankIt_uncultured16S-23SIGS_wizard]; [universal primers]; [tgge]",
    "[BankIt_uncultured16S_wizard]; [universal primers]; [dgge]",
    "[BankIt_uncultured23S_wizard]; [universal primers]; [dgge]",
    "[BankIt_uncultured16S-23SIGS_wizard]; [universal primers]; [dgge]",
    "[BankIt_uncultured16S_wizard]; [universal primers]",
    "[BankIt_uncultured23S_wizard]; [universal primers]",
    "[BankIt_uncultured16S-23SIGS_wizard]; [universal primers]",
    "[universal primers]; [uncultured; wizard]",
    "[uncultured (using universal primers)]",
    "[uncultured (using universal primers) bacterial source]",
    "[uncultured]; [universal primers]",
    NULL
};

static bool s_HasSubSourceNote(const CBioSource &bsrc, const char* note_list[])
{
    if (!bsrc.IsSetSubtype()) {
        return false;
    }

    ITERATE(CBioSource::TSubtype, subtype, bsrc.GetSubtype()) {
        if ((*subtype)->IsSetSubtype()
            && (*subtype)->GetSubtype() == CSubSource::eSubtype_other
            && (*subtype)->IsSetName()) {
            int i = 0;
            while (note_list[i]) {
                if (NStr::FindNoCase((*subtype)->GetName(), note_list[i]) != NPOS) {
                    return true;
                }
                i++;
            }
        }
    }
    return false;
}

void CUnculturedTool::CorrectBioSource(CBioSource& bsrc, bool& converted_note, bool& removed_note_sp, bool& removed_note_univ)
{
    m_Bsrc.Reset(const_cast<const CBioSource*>(&bsrc));
    x_Reset();

    string suggested_taxname = GetSuggestedCorrection();
    // update only if suggested taxname is different from the original one
    if (!NStr::EqualCase(m_Bsrc->GetTaxname(), suggested_taxname)) {
        bsrc.SetOrg().SetTaxname(suggested_taxname);
        edit::CleanupForTaxnameChange(bsrc);
    }
    else if (s_HasTaxId(bsrc) && s_IsUnculturedName(bsrc.GetTaxname())) {
        if (s_HasSubSourceNote(m_Bsrc.GetObject(), sUniversalPrimers)) {
            m_RemoveNote_Univ = true;
        }
        if (s_HasSubSourceNote(m_Bsrc.GetObject(), sSpeciesSpecificPrimers)) {
            m_RemoveNote_Sp = true;
        }
    }


    if (m_ConvertNote) {
        x_ConvertNote(bsrc, sSpeciesSpecificPrimers, "amplified with species-specific primers");
        s_AddEnvironmentalSample(bsrc);
    } else if (m_RemoveNote_Sp) {
        x_ConvertNote(bsrc, sSpeciesSpecificPrimers, "");
    }
    if (m_RemoveNote_Univ) {
        x_ConvertNote(bsrc, sUniversalPrimers, "");
    }

    converted_note = m_ConvertNote;
    removed_note_sp = m_RemoveNote_Sp;
    removed_note_univ = m_RemoveNote_Univ;
}

string CUnculturedTool::GetSuggestedCorrection()
{
    if (!m_Bsrc) {
        NCBI_THROW(CException, eUnknown, "No biosource is specified");
    }

    x_Reset();

    if (!x_ShouldLookupTaxname()) {
        return (m_Bsrc->IsSetTaxname()) ? m_Bsrc->GetTaxname() : kEmptyStr;
    }

    string standard_taxname = x_StandardFixes();
    if (standard_taxname.empty())
        return kEmptyStr;

    set<string> submit;
    submit.insert(standard_taxname);
    submit.insert(s_MakeUnculturedName(standard_taxname, " bacterium"));
    submit.insert(s_MakeUnculturedName(standard_taxname, " archaeon"));
    submit.insert(s_MakeUnculturedName(standard_taxname, " sp."));
    submit.insert(s_MakeUnculturedName(standard_taxname));
    if (NStr::FindNoCase(standard_taxname, " ") != NPOS) {
        string tmp, name2;
        NStr::SplitInTwo(standard_taxname, " ", name2, tmp);
        standard_taxname = name2;
        submit.insert(standard_taxname);
        submit.insert(s_MakeUnculturedName(standard_taxname, " bacterium"));
        submit.insert(s_MakeUnculturedName(standard_taxname, " archaeon"));
        submit.insert(s_MakeUnculturedName(standard_taxname, " sp."));
        submit.insert(s_MakeUnculturedName(standard_taxname));
    }

    // send this list to taxonomy
    vector<CRef<COrg_ref> > rq_list;
    ITERATE(set<string>, name, submit) {
        CRef<COrg_ref> org(new COrg_ref());
        org->SetTaxname(*name);
        rq_list.push_back(org);
    }

    CTaxon3 taxon3;
    taxon3.Init();
    CRef<CTaxon3_reply> reply = taxon3.SendOrgRefList(rq_list);
    if (!reply->IsSetReply())
        return kEmptyStr;

    size_t index = 0;
    ITERATE(CTaxon3_reply::TReply, rp_it, reply->GetReply()) {
        m_ReplyCache[rq_list[index]->GetTaxname()] = *rp_it;
        index++;
    }

    return x_GetCorrection();
}

void CUnculturedTool::x_Reset()
{
    m_ReplyCache.clear();
    m_ConvertNote = false;
    m_RemoveNote_Sp = false;
    m_RemoveNote_Univ = false;
}

bool CUnculturedTool::x_ShouldLookupTaxname(void)
{
    if (m_Bsrc->IsSetTaxname() && !m_Bsrc->GetTaxname().empty() && s_OkToTaxFix(m_Bsrc->GetTaxname())) {
        if (!s_HasTaxId(*m_Bsrc) || !s_IsUnculturedName(m_Bsrc->GetTaxname())) {
            return true;
        }
    }
    return false;
}

static const char* sUnfixable[] = {
    "rickettsia",
    "candidatus",
    "endosymbiont",
    "phytoplasma",
    "wolbachia"
};

bool CUnculturedTool::s_OkToTaxFix(const string& taxname)
{
    for (auto&& elem : sUnfixable) {
        if (NStr::FindNoCase(taxname, elem) != NPOS) {
            return false;
        }
    }
    return true;
}

bool CUnculturedTool::s_IsUnculturedName(const string& taxname)
{
    return NStr::StartsWith(taxname, "uncultured", NStr::eNocase);
}

bool CUnculturedTool::s_HasTaxId(const CBioSource& bsrc)
{
    if (bsrc.IsSetOrg()) {
        int taxid = bsrc.GetOrg().GetTaxId();
        if (taxid != 0) { // found taxid
            return true;
        }
    }
    return false;
}

string CUnculturedTool::x_StandardFixes()
{
    string taxname = m_Bsrc->GetTaxname();
    string old;
    while (old != taxname) {
        old = taxname;
        const string uncultured = "uncultured ";
        const string sp = " sp";
        const string spdot = " sp.";
        if (NStr::StartsWith(taxname, uncultured, NStr::eNocase)) {
            taxname = taxname.substr(uncultured.length());
        }
        if (NStr::EndsWith(taxname, sp, NStr::eNocase)) {
            taxname = taxname.substr(0, taxname.length() - sp.length());
        }
        if (NStr::EndsWith(taxname, spdot, NStr::eNocase)) {
            taxname = taxname.substr(0, taxname.length() - spdot.length());
        }
        NStr::ReplaceInPlace(taxname, ", ", " ");
        NStr::ReplaceInPlace(taxname, ",", " ");
    }

    return taxname;
}

string CUnculturedTool::s_MakeUnculturedName(const string& taxname, const string& suffix)
{
    return "uncultured " + taxname + suffix;
}


string CUnculturedTool::x_GetCorrection()
{
    string standard_taxname = x_StandardFixes();
    if (standard_taxname.empty())
        return kEmptyStr;

    CRef<CT3Reply> reply = x_GetReply(standard_taxname);
    _ASSERT(!reply.IsNull());

    string suggestion;
    if (s_IsAmbiguous(reply)) {
        suggestion = s_MakeUnculturedName(standard_taxname, " bacterium");
        if (x_CheckSuggestedFix(suggestion))
            return suggestion;
        suggestion = s_MakeUnculturedName(standard_taxname, " archaeon");
        if (x_CheckSuggestedFix(suggestion))
            return suggestion;
    }

    suggestion = x_TryRankFix(reply, standard_taxname);
    if (x_CheckSuggestedFix(suggestion))
        return suggestion;

    suggestion = s_MakeUnculturedName(standard_taxname);
    if (x_CheckSuggestedFix(suggestion))
        return suggestion;
    suggestion = s_MakeUnculturedName(standard_taxname, " sp.");
    if (x_CheckSuggestedFix(suggestion))
        return suggestion;

    suggestion = s_MakeUnculturedName(standard_taxname);
    return suggestion;
}


CRef<CT3Reply> CUnculturedTool::x_GetReply(const string &standard_taxname)
{
    if (m_ReplyCache.find(standard_taxname) != m_ReplyCache.end())
        return m_ReplyCache[standard_taxname];

    _ASSERT(m_Bsrc);
    vector<CRef<COrg_ref> > rq_list;
    CRef<COrg_ref> org(new COrg_ref());
    org->SetTaxname(standard_taxname);
    rq_list.push_back(org);

    CTaxon3 taxon3;
    taxon3.Init();
    CRef<CTaxon3_reply> reply = taxon3.SendOrgRefList(rq_list);
    CRef<CT3Reply> t3reply;
    if (reply->IsSetReply() && !reply->GetReply().empty()) {
        t3reply = reply->GetReply().front();
        m_ReplyCache[standard_taxname] = t3reply;
    }
    return t3reply;
}


bool CUnculturedTool::x_CheckSuggestedFix(string &suggestion)
{
    CRef<CT3Reply> reply = x_GetReply(suggestion);
    _ASSERT(!reply.IsNull());

    string rank = s_GetRank(reply);
    if (NStr::EqualNocase(rank, "species")) {
        suggestion = s_GetSuggestion(reply);
        // Note that this does not only perform a check - it also potentially modifies suggestion. 
        // This is how it's done in the original sequin code.
        return true;
    }

    return false;
}

void CUnculturedTool::s_AddEnvironmentalSample(CBioSource& bsrc)
{
    // add the environmental_sample modifier if there is not one already!
    FOR_EACH_SUBSOURCE_ON_BIOSOURCE(subsrc, bsrc) {
        if ((*subsrc)->IsSetSubtype() 
            && (*subsrc)->GetSubtype() == CSubSource::eSubtype_environmental_sample) {
            // found
            return;
        }
    }

    bsrc.SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_environmental_sample, kEmptyStr)));
}

bool CUnculturedTool::s_IsAmbiguous(CRef<CT3Reply> reply)
{
    if (reply && reply->IsError() && reply->GetError().IsSetMessage()
        && NStr::EqualNocase(reply->GetError().GetMessage(), "Taxname is ambiguous")) {
        return true;
    }
    return false;
}

bool CUnculturedTool::s_OrganismNotFound(CRef<CT3Reply> reply)
{
    if (reply && reply->IsError() && reply->GetError().IsSetMessage()
        && NStr::EqualNocase(reply->GetError().GetMessage(), "Organism not found")) {
        return true;
    }
    return false;
}

string CUnculturedTool::s_GetRank(CRef<CT3Reply> reply)
{
    string rank;
    if (reply && reply->IsData() && reply->GetData().IsSetStatus())
        ITERATE(CT3Data::TStatus, status, reply->GetData().GetStatus()) {
        if ((*status)->IsSetProperty()
            && NStr::EqualNocase((*status)->GetProperty(), "rank")
            && (*status)->IsSetValue()
            && (*status)->GetValue().IsStr()) {
            rank = (*status)->GetValue().GetStr();
        }
    }

    return rank;
}

string CUnculturedTool::s_GetSuggestion(CRef<CT3Reply> reply)
{
    if (reply && reply->IsData()) {
        return reply->GetData().GetOrg().GetTaxname();
    }

    return kEmptyStr;
}

bool CUnculturedTool::s_CompareOrgnameLineage(CRef<CT3Reply> reply, const string &lineage)
{
    if (reply && reply->IsData() && reply->GetData().IsSetOrg()
        && reply->GetData().GetOrg().IsSetOrgname()
        && reply->GetData().GetOrg().GetOrgname().IsSetLineage()
        && NStr::FindNoCase(reply->GetData().GetOrg().GetOrgname().GetLineage(), lineage) != NPOS) {
        return true;
    }
    return false;
}


string CUnculturedTool::x_TryRankFix(CRef<CT3Reply> reply, string& name)
// In case of binomial truncation the input name will be modified. This is what's happening in the original sequin code
{
    string suggestion;
    bool is_species_level(false), force_consult(false), has_nucleomorphs(false);
    if (reply->IsData()) {
        reply->GetData().GetTaxFlags(is_species_level, force_consult, has_nucleomorphs);
    }
    bool has_species_specific_note = s_HasSubSourceNote(m_Bsrc.GetObject(), sSpeciesSpecificPrimers);
    bool has_universal_note = s_HasSubSourceNote(m_Bsrc.GetObject(), sUniversalPrimers);

    if (is_species_level) {
        if (has_species_specific_note) {
            suggestion = s_GetSuggestion(reply);
            m_ConvertNote = true;
        } else {
            if (has_universal_note) {
                m_RemoveNote_Univ = true;
            }
            if (NStr::FindNoCase(name, " ") != NPOS) {
                string tmp, name2;
                NStr::SplitInTwo(name, " ", name2, tmp);
                name = name2;
                CRef<CT3Reply> reply2 = x_GetReply(name);
                if (!reply2) {
                    return suggestion;
                }

                return x_TryRankFix(reply2, name);
            }
        }
    } else {
        if (!s_OrganismNotFound(reply)) {
            // remove notes only if organism name has been found
            if (has_species_specific_note) {
                m_RemoveNote_Sp = true;
            }
            if (has_universal_note) {
                m_RemoveNote_Univ = true;
            }
        }

        string rank = s_GetRank(reply);
        if (NStr::EqualNocase(rank, "genus")) {
            if (s_CompareOrgnameLineage(reply, "archaea") || s_CompareOrgnameLineage(reply, "bacteria"))
                suggestion = s_MakeUnculturedName(s_GetSuggestion(reply), " sp.");
            else if (s_CompareOrgnameLineage(reply, " Fungi;"))
                suggestion = s_MakeUnculturedName(s_GetSuggestion(reply));
        }
        else {
            if (s_CompareOrgnameLineage(reply, "archaea"))
                suggestion = s_MakeUnculturedName(s_GetSuggestion(reply), " archaeon");
            else if (s_CompareOrgnameLineage(reply, "bacteria"))
                suggestion = s_MakeUnculturedName(s_GetSuggestion(reply), " bacterium");
            else if (s_CompareOrgnameLineage(reply, " Fungi;"))
                suggestion = s_MakeUnculturedName(s_GetSuggestion(reply));
        }
    }

    return suggestion;
}

void CUnculturedTool::x_ConvertNote(CBioSource& bsrc, const char* note_list[], const string& new_note)
{
    if (!bsrc.IsSetSubtype())
        return;

    CBioSource::TSubtype::iterator subtype = bsrc.SetSubtype().begin();
    while (subtype != bsrc.SetSubtype().end()) {
        bool erased = false;
        if ((*subtype)->IsSetSubtype()
            && (*subtype)->GetSubtype() == CSubSource::eSubtype_other
            && (*subtype)->IsSetName()) {
            const string& name = (*subtype)->GetName();
            int i = 0;
            while (note_list[i]) {
                if (NStr::FindNoCase(name, note_list[i]) != NPOS) {
                    if (new_note.empty()) {
                        subtype = bsrc.SetSubtype().erase(subtype);
                        erased = true;
                    } else {
                        (*subtype)->SetName(new_note);
                    }
                    break;
                }
                i++;
            }
        }

        if (!erased) {
            ++subtype;
        }
    }

    if (bsrc.GetSubtype().empty()) {
        bsrc.ResetSubtype();
    }
}


////////////////////////////////////////////////////////////////////////
/// class CCulturedTool

static const char* sCulturedNodes[] = {
    "[BankIt_cultured16S_wizard]",
    "[cultured bacterial source]",
    "[BankIt_cultured16S_wizard]; [universal primers]",
    "[cultured; wizard]",
    "[BankIt_cultured16S_wizard]; [species_specific primers]; [tgge]" ,
    "[BankIt_cultured16S_wizard]; [species_specific primers]; [dgge]" ,
    "[BankIt_cultured16S_wizard]; [species_specific primers]",
    "[BankIt_cultured23S_wizard]",
    "[BankIt_cultured16S-23SIGS_wizard]",
    NULL
};

void CCulturedTool::CorrectBioSource(CBioSource& bsrc, bool& remove_note)
{
    m_Bsrc.Reset(const_cast<const CBioSource*>(&bsrc));

    // Combined here steps 1 - 10 fom cultured_macro script (C version).
    // Possible extension in the future to also combine steps 14 - 16.
    string taxname = m_Bsrc->GetTaxname();
    if (taxname.empty()) return;

    NStr::ReplaceInPlace(taxname, "_", " ");  // step 3

    CMacroFunction_EditStringQual::s_EditText(taxname, "Novel ", "",
        CMacroFunction_EditStringQual::eBeginning, false);   // step 4

    CMacroFunction_EditStringQual::s_EditText(taxname, "unclassified", "",
        CMacroFunction_EditStringQual::eAnywhere, false);   // step 5

    CMacroFunction_EditStringQual::s_EditText(taxname, " SP.", " sp.",
        CMacroFunction_EditStringQual::eEnd, false);   // step 6

    CMacroFunction_EditStringQual::s_EditText(taxname, " SP", " sp.",
        CMacroFunction_EditStringQual::eEnd, false); // step 7

    CMacroFunction_EditStringQual::s_EditText(taxname, " species", " sp.",
        CMacroFunction_EditStringQual::eEnd, false); // step 8

    CMacroFunction_EditStringQual::s_EditText(taxname, " spp", " sp.",
        CMacroFunction_EditStringQual::eEnd, false);  // step 9

    CMacroFunction_EditStringQual::s_EditText(taxname, " spp.", " sp.",
        CMacroFunction_EditStringQual::eEnd, false);  // step 10



    if (!NStr::EqualCase(m_Bsrc->GetTaxname(), taxname)) {
        bsrc.SetOrg().SetTaxname(taxname);
        edit::CleanupForTaxnameChange(bsrc);
    }

    remove_note = false;
    remove_note |= x_RemoveIfFoundSubSourceNote(bsrc, sCulturedNodes);
    remove_note |= x_RemoveIfFoundOrgModNote(bsrc, sCulturedNodes);
}


bool CCulturedTool::x_RemoveIfFoundSubSourceNote(CBioSource &bsrc, const char* note_list[])
{
    if (!bsrc.IsSetSubtype()) {
        return false;
    }

    bool modified = false;
    auto&& subtype = bsrc.SetSubtype().begin();
    while (subtype != bsrc.SetSubtype().end()) {
        bool erased = false;
        if ((*subtype)->IsSetSubtype()
            && (*subtype)->GetSubtype() == CSubSource::eSubtype_other
            && (*subtype)->IsSetName()) {
            const string& name = (*subtype)->GetName();
            auto i = 0;
            while (note_list[i]) {
                if (NStr::FindNoCase(name, note_list[i]) != NPOS) {
                    subtype = bsrc.SetSubtype().erase(subtype);
                    erased = true;
                    modified = true;
                    break;
                }
                ++i;
            }
        }
        if (!erased) {
            ++subtype;
        }
    }

    if (bsrc.GetSubtype().empty()) {
        bsrc.ResetSubtype();
    }
    return modified;
}

bool CCulturedTool::x_RemoveIfFoundOrgModNote(CBioSource &bsrc, const char* note_list[])
{
    if (!bsrc.IsSetOrgMod()) {
        return false;
    }

    bool modified = false;
    COrgName::TMod& orgMods = bsrc.SetOrg().SetOrgname().SetMod();
    auto&& orgmod = orgMods.begin();
    while (orgmod != orgMods.end()) {
        bool erased = false;
        if ((*orgmod)->IsSetSubtype()
            && (*orgmod)->GetSubtype() == CSubSource::eSubtype_other
            && (*orgmod)->IsSetSubname()) {
            const string& name = (*orgmod)->GetSubname();
            auto i = 0;
            while (note_list[i]) {
                if (NStr::FindNoCase(name, note_list[i]) != NPOS) {
                    orgmod = orgMods.erase(orgmod);
                    erased = true;
                    modified = true;
                    break;
                }
                ++i;
            }
        }
        if (!erased) {
            ++orgmod;
        }
    }

    if (orgMods.empty()) {
        bsrc.SetOrg().SetOrgname().ResetMod();
    }
    return modified;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_UnculturedTaxLookup
/// DoUnculturedTaxLookup();
///
const char* CMacroFunction_UnculturedTaxLookup::sm_FunctionName = "DoUnculturedTaxLookup";
void CMacroFunction_UnculturedTaxLookup::TheFunction()
{
    // only for source descriptors
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
    if (!bsrc || m_DataIter->IsFeature()) {
        return;
    }

    const string orig_taxname = bsrc->GetTaxname();

    CUnculturedTool uncultured_taxtool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_taxtool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);
    bool changed = (!NStr::EqualCase(orig_taxname, bsrc->GetTaxname())) || converted_note || removed_note_sp || removed_note_univ;

    if (changed) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        if (!NStr::EqualCase(orig_taxname, bsrc->GetTaxname())) {
            log << "Corrected " << orig_taxname << " to " << bsrc->GetTaxname();
        }
        string msg;
        if (converted_note) {
            msg.assign("Converted species-specific primer note to \"amplified with species-specific primers\" for ");
        }
        else if (removed_note_sp) {
            msg.assign("Removed species-specific primer note for ");;
        }
        else if (removed_note_univ) {
            msg.assign("Removed universal primer note for ");
        }
        if (!msg.empty()) {
            if (!IsOssEmpty(log)) {
                log << endl;
            }
            log << msg << bsrc->GetTaxname();
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_UnculturedTaxLookup::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_CulturedTaxLookup
/// DoCulturedTaxLookup();
///
const char* CMacroFunction_CulturedTaxLookup::sm_FunctionName = "DoCulturedTaxLookup";
void CMacroFunction_CulturedTaxLookup::TheFunction()
{
    // only for source descriptors
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
    if (!bsrc || m_DataIter->IsFeature()) {
        return;
    }

    const string orig_taxname = bsrc->GetTaxname();

    CCulturedTool cultured_taxtool;
    bool removed_note(false);
    cultured_taxtool.CorrectBioSource(*bsrc, removed_note);
    bool changed = (!NStr::EqualCase(orig_taxname, bsrc->GetTaxname())) || removed_note;

    if (changed) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        if (!NStr::EqualCase(orig_taxname, bsrc->GetTaxname())) {
            log << "Corrected " << orig_taxname << " to " << bsrc->GetTaxname() << endl;
        }
        string msg;
        if (removed_note) {
            log << "Removed note for " << bsrc->GetTaxname() << endl;
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_CulturedTaxLookup::x_ValidArguments() const
{
    return (m_Args.empty());
}



END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
