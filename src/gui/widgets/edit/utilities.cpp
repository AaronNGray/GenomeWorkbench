/*  $Id: utilities.cpp 42855 2019-04-21 11:24:32Z bollin $
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
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub_set.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/general/Person_id.hpp>

#include <gui/objutils/macro_fn_pubfields.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <misc/xmlwrapp/document.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void ReplaceInPlaceWholeWordNoCase (string& rna_name, const string& search, const string& replace)
{
    size_t pos = NStr::FindNoCase(rna_name, search);
    
    while (pos != string::npos) {
        // only if whole word
        size_t right_end = pos + search.length();
        if ((pos == 0 || !isalpha(rna_name.c_str()[pos - 1]))
            && (right_end == rna_name.length() || !isalpha(rna_name.c_str()[right_end]))) {
            string this_replace = replace;
            if (rna_name.c_str()[right_end] != 0 && !isspace(rna_name.c_str()[right_end])) {
                this_replace += " ";
            }
            rna_name = rna_name.substr(0, pos) + this_replace + rna_name.substr(right_end);
            right_end = pos + this_replace.length();
        } 
        pos = NStr::FindNoCase (rna_name, search, right_end);
    }
}


string GetAdjustedRnaName(const string& orig_rna_name)
{
    string rval = orig_rna_name;
    ReplaceInPlaceWholeWordNoCase(rval, "rrna", "ribosomal RNA");
    ReplaceInPlaceWholeWordNoCase(rval, "ITS", "internal transcribed spacer");
    return rval;
}

CRef<CPubdesc> GetPubdescFromEntrezById(int id)
{
    CRef<CPub> pub = macro::CMacroFunction_LookupPub::s_GetArticleFromEntrezById(id);
    if (pub) {
        CRef<CPubdesc> pubdesc(new CPubdesc());
        pubdesc->SetPub().Set().push_back(pub);
        CRef<CPub> pmid_pub(new CPub());
        pmid_pub->SetPmid().Set(id);
        pubdesc->SetPub().Set().push_back(pmid_pub);
        return pubdesc;
    } 

    return CRef<CPubdesc>(nullptr);
}

string GetAuthorsString (const CAuth_list& auth_list)
{
    string auth_str = "";

    if (!auth_list.IsSetNames()) {
        return auth_str;
    }

    vector<string> name_list;

    if (auth_list.GetNames().IsStd()) {
        ITERATE (CAuth_list::TNames::TStd, auth_it, auth_list.GetNames().GetStd()) {
            if ((*auth_it)->IsSetName()) {
                string label;
                (*auth_it)->GetName().GetLabel(&label);
                name_list.push_back(label);
            }
        }
    } else if (auth_list.GetNames().IsMl()) {
        ITERATE (CAuth_list::TNames::TMl, auth_it, auth_list.GetNames().GetMl()) {
            name_list.push_back((*auth_it));
        }
    } else if (auth_list.GetNames().IsStr()) {
        ITERATE (CAuth_list::TNames::TStr, auth_it, auth_list.GetNames().GetStr()) {
            name_list.push_back((*auth_it));
        }
    }

    if (name_list.size() == 0) {
        return auth_str;
    }

    auth_str = name_list.back();
    name_list.pop_back();
    if (name_list.size() > 0) {
        auth_str = "and " + auth_str;
        if (name_list.size() == 1) {
            auth_str = name_list.front() + auth_str;
        } else {        
            while (name_list.size() > 0) {
                string this_name = name_list.back();
                name_list.pop_back();
                auth_str = this_name + ", " + auth_str;
            }
        }
    }

    return auth_str;
}


static string s_GetAuthorsString (const CPubdesc& pd) 
{
    string authors_string = "";

    FOR_EACH_PUB_ON_PUBDESC (pub, pd) {
        if ((*pub)->IsSetAuthors()) {
            authors_string = GetAuthorsString ((*pub)->GetAuthors());
            break;
        }
    }
    return authors_string;
}


string GetPubdescLabel(const CPubdesc& pd)
{
    vector<int> pmids;
    vector<int> muids;
    vector<int> serials;
    vector<string> published_labels;
    vector<string> unpublished_labels;

    GetPubdescLabels(pd, pmids, muids, serials, published_labels, unpublished_labels);
    string label = "";
    if (published_labels.size() > 0) {
        return published_labels[0];
    } else if (unpublished_labels.size() > 0) {
        label = unpublished_labels[0];
    }
    return label;
}


void GetPubdescLabels 
(const CPubdesc& pd, 
 vector<int>& pmids, vector<int>& muids, vector<int>& serials,
 vector<string>& published_labels, vector<string>& unpublished_labels)
{
    string label = "";
    bool   is_published = false;
    bool   need_label = false;
    FOR_EACH_PUB_ON_PUBDESC (it, pd) {
        if ((*it)->IsPmid()) {
            pmids.push_back ((*it)->GetPmid());
            is_published = true;
        } else if ((*it)->IsMuid()) {
            muids.push_back ((*it)->GetMuid());
            is_published = true;
        } else if ((*it)->IsGen()) {
            if ((*it)->GetGen().IsSetCit() 
                && NStr::StartsWith ((*it)->GetGen().GetCit(), "BackBone id_pub", NStr::eNocase)) {
                need_label = true;
            }
            if ((*it)->GetGen().IsSetSerial_number()) {
                serials.push_back ((*it)->GetGen().GetSerial_number());
                if ((*it)->GetGen().IsSetCit() 
                    || (*it)->GetGen().IsSetJournal()
                    || (*it)->GetGen().IsSetDate()) {
                    need_label = true;
                }
            } else {
                need_label = true;
            }
        } else {
            need_label = true;
        }
        if (need_label && NStr::IsBlank(label)) {
            // create unique label
            (*it)->GetLabel(&label, CPub::eContent, true);
            label += "; " + s_GetAuthorsString (pd);
        }
    }
    if (!NStr::IsBlank(label)) {
        if (is_published) {
            published_labels.push_back(label);
        } else {
            unpublished_labels.push_back(label);
        }
    }
}


string GetDocSumLabel(const CPubdesc& pd)
{
    string author = s_GetAuthorsString(pd);
    string title = "";
    string source = "";
    string volume = "";
    string pages = "";
    string year_str = "";
    string pmid = "";

    if (pd.IsSetPub()) {
        ITERATE(CPubdesc::TPub::Tdata, it, pd.GetPub().Get()) {
            if ((*it)->IsPmid()) {
                pmid = NStr::NumericToString((*it)->GetPmid().Get());
            } else if ((*it)->IsArticle()) {
                const CCit_art& art = (*it)->GetArticle();
                if (art.IsSetTitle()) {
                    title = art.GetTitle().GetTitle();
                }
                if (art.IsSetFrom() && art.GetFrom().IsJournal()) {
                    const CCit_jour& journal = art.GetFrom().GetJournal();
                    if (journal.IsSetTitle()) {
                        source = journal.GetTitle().GetTitle();
                    }
                    if (journal.IsSetImp()) {
                        const CImprint& imp = journal.GetImp();
                        if (imp.IsSetVolume()) {
                            volume = imp.GetVolume();
                        }
                        if (imp.IsSetPages()) {
                            pages = imp.GetPages();
                        }
                        if (imp.IsSetDate() && imp.GetDate().IsStd() && imp.GetDate().GetStd().IsSetYear()) {
                            year_str = NStr::NumericToString(imp.GetDate().GetStd().GetYear());
                        }
                    }
                }
            }
        }
    }


    if (NStr::IsBlank(author)) {
        author = "NO AUTHOR AVAILABLE";
    }
    if (NStr::IsBlank(title)) {
        title = "NO TITLE AVAILABLE";
    }

    string rval =  author + "\n" + title + "\n" + source + ". " + year_str + "; " + volume + ":" + pages + "\nPMID: " + pmid + "\n";
    return rval;
}


string GetStringFromTitle(const objects::CTitle::C_E& title)
{
    string plain_string = kEmptyStr;

    switch (title.Which()) {
    case objects::CTitle::C_E::e_Abr:
        plain_string = title.GetAbr();
        break;
    case objects::CTitle::C_E::e_Coden:
        plain_string = title.GetCoden();
        break;
    case objects::CTitle::C_E::e_Isbn:
        plain_string = title.GetIsbn();
        break;
    case objects::CTitle::C_E::e_Iso_jta:
        plain_string = title.GetIso_jta();
        break;
    case objects::CTitle::C_E::e_Issn:
        plain_string = title.GetIssn();
        break;
    case objects::CTitle::C_E::e_Jta:
        plain_string = title.GetJta();
        break;
    case objects::CTitle::C_E::e_Ml_jta:
        plain_string = title.GetMl_jta();
        break;
    case objects::CTitle::C_E::e_Name:
        plain_string = title.GetName();
        break;
    case objects::CTitle::C_E::e_Trans:
        plain_string = title.GetTrans();
        break;
    case objects::CTitle::C_E::e_Tsub:
        plain_string = title.GetTsub();
        break;
    default:
        plain_string = kEmptyStr;
        break;
    }
    return plain_string;
}


void UpdateTitle(objects::CTitle::C_E& title, const string& val)
{
    switch (title.Which()) {
    case objects::CTitle::C_E::e_Abr:
        title.SetAbr(val);
        break;
    case objects::CTitle::C_E::e_Coden:
        title.SetCoden(val);
        break;
    case objects::CTitle::C_E::e_Isbn:
        title.SetIsbn(val);
        break;
    case objects::CTitle::C_E::e_Iso_jta:
        title.SetIso_jta(val);
        break;
    case objects::CTitle::C_E::e_Issn:
        title.SetIssn(val);
        break;
    case objects::CTitle::C_E::e_Jta:
        title.SetJta(val);
        break;
    case objects::CTitle::C_E::e_Ml_jta:
        title.SetMl_jta(val);
        break;
    case objects::CTitle::C_E::e_Name:
        title.SetName(val);
        break;
    case objects::CTitle::C_E::e_Trans:
        title.SetTrans(val);
        break;
    case objects::CTitle::C_E::e_Tsub:
        title.SetTsub(val);
        break;
    default:
        if (!NStr::IsBlank(val)) {
            title.SetName(val);
        }
        break;
    }
}

END_NCBI_SCOPE
