/*  $Id: genus_species_fixup.cpp 42425 2019-02-21 15:31:49Z asztalos $
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
 */


#include <ncbi_pch.hpp>
#include <objects/taxon3/taxon3.hpp>
#include <objtools/edit/source_edit.hpp>
#include <objects/taxon3/Taxon3_reply.hpp>
#include <objects/misc/sequence_macros.hpp>

#include <gui/packages/pkg_sequence_edit/genus_species_fixup.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> CGenusSpeciesFixup::GetCommand(CSeq_entry_Handle tse)
{
    return x_GetCommand(tse, "Genus-species fixup");
}

bool CGenusSpeciesFixup::x_ApplyToBioSource(CBioSource& biosource)
{
    bool modified = false;
    if (biosource.IsSetTaxname() && !biosource.GetTaxname().empty() && !s_HasTaxId(biosource))
    {
        string name = biosource.GetTaxname();
        biosource.SetOrg().ResetCommon();
        EDIT_EACH_ORGMOD_ON_ORGNAME(orgmod, biosource.SetOrg().SetOrgname())
        {
             if ((*orgmod)->IsSetSubtype() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_old_name)
                 ERASE_ORGMOD_ON_ORGNAME(orgmod, biosource.SetOrg().SetOrgname());
        }

        CRef<COrg_ref> new_org(new COrg_ref);
        new_org->Assign(biosource.SetOrg());
        s_GetNameFromTaxonServer(new_org);

        biosource.SetOrg(*new_org);
        string new_name = biosource.SetOrg().GetTaxname();
        if (!new_name.empty() && new_name != name)
        {
            CRef< CSubSource > subsource(new CSubSource);
            subsource->SetSubtype(CSubSource::eSubtype_other);
            subsource->SetName(name);
            biosource.SetSubtype().push_back(subsource);
        }
        modified = true;
    }

    return modified;
}

bool CGenusSpeciesFixup::s_HasTaxId(const CBioSource& biosource)
{
    if (biosource.IsSetOrg()) {
        int taxid = biosource.GetOrg().GetTaxId();
        if (taxid != 0) { // found taxid
            return true;
        }
    }
    return false;
}

void CGenusSpeciesFixup::s_GetNameFromTaxonServer(CRef<COrg_ref> org)
{
    string name = org->GetTaxname();
    vector<string> submit; 
    s_PermuteTaxname(name,submit);
    if (submit.empty())
        return;

    vector<CRef<COrg_ref>> request_list;
    for (auto&& it : submit)
    {
        CRef<COrg_ref> new_org(new COrg_ref());
        new_org->Assign(*org);
        new_org->SetTaxname(it);
        request_list.push_back(new_org);
    }

    CTaxon3 taxon3;
    taxon3.Init();
    CRef<CTaxon3_reply> reply;
    reply = taxon3.SendOrgRefList(request_list);
    if (reply->IsSetReply() )
    {
        const auto& ans = reply->GetReply();
        for (auto&& it : ans)
        {
            if (it->IsData() && 
                it->GetData().IsSetOrg() && 
                it->GetData().GetOrg().GetTaxId() >= 0)
            {
                org->Assign(it->GetData().GetOrg());
                break;
            }
        }
    }
}

void CGenusSpeciesFixup::s_PermuteTaxname(const string &name, vector<string> &submit)
{
    vector<string> words;
    NStr::Split(name, " ", words, NStr::fSplit_Tokenize);
    if (words.size() == 1)
    {
        vector<string> subwords;
        NStr::Split(name, "_", subwords, NStr::fSplit_Tokenize);

        for (int i = 0; i < subwords.size() - 1; i++)
        {
            string new_name = subwords[0];
            for (int j = 1; j <= i; j++)
                new_name += "_" + subwords[j];
            new_name += " " + subwords[i+1];
            for (int j = i+2; j < subwords.size(); j++)
                new_name += "_" + subwords[j];
            submit.push_back(new_name);
        }
        return;
    }
    if (words.size() == 2)
        return;
    if (words.size() >= 4 && NStr::Equal(words[2],"subsp.",NStr::eNocase))
    {
        string new_name = words[0] + " " + words[1] + " " + words[2] + " " + words[3];
        submit.push_back(new_name);
    }
    if (words.size() >= 3 && !NStr::Equal(words[2],"subsp.",NStr::eNocase))
    {
        string new_name = words[0] + " " + words[1] + " " + words[2];
        submit.push_back(new_name);
    }
    if (words.size() >= 2)
    {
        string new_name = words[0] + " " + words[1];
        submit.push_back(new_name);
    }
}

END_NCBI_SCOPE
