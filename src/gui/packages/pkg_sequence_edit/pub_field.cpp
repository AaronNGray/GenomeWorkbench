/*  $Id: pub_field.cpp 42440 2019-02-26 20:48:50Z asztalos $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/biblio/Cit_book.hpp>
#include <objects/biblio/Cit_proc.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Cit_pat.hpp>
#include <objects/biblio/Cit_let.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Title.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/general/Person_id.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>

#include <objtools/edit/seqid_guesser.hpp>
#include <objtools/edit/publication_edit.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>

#include <gui/packages/pkg_sequence_edit/pub_field.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/packages/pkg_sequence_edit/pubdesc_editor.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);
USING_SCOPE(edit);


CConstRef<CAuth_list> GetAuthList(const CPub& pub)
{
    if (pub.IsSetAuthors()) {
        return CConstRef<CAuth_list>(&(pub.GetAuthors()));
    }
    return CConstRef<CAuth_list>(NULL);
}


CConstRef<CAuth_list> GetAuthList(const CPubdesc& pubdesc)
{
    if (!pubdesc.IsSetPub()) {
        return CConstRef<CAuth_list>(NULL);
    }
    ITERATE (CPub_equiv::Tdata, it, pubdesc.GetPub().Get()) {
        if ((*it)->IsSetAuthors()) {
            return CConstRef<CAuth_list>(&((*it)->GetAuthors()));
        }
    }
    return CConstRef<CAuth_list>(NULL);
}


CConstRef<CAuth_list> GetAuthList(CConstRef<CObject> sel_pub)
{
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(sel_pub.GetPointer());
    const CSeq_feat* seqfeat = dynamic_cast<const CSeq_feat*>(sel_pub.GetPointer());
    const CCit_sub* citsub = dynamic_cast<const CCit_sub*>(sel_pub.GetPointer());
    const CPubdesc* pubdesc = dynamic_cast<const CPubdesc*>(sel_pub.GetPointer());
    const CSubmit_block* block = dynamic_cast<const CSubmit_block*>(sel_pub.GetPointer());

    if (seqdesc && seqdesc->IsPub()) {
        return GetAuthList(seqdesc->GetPub());
    } else if (seqfeat && seqfeat->IsSetData() && seqfeat->GetData().IsPub()) {
        return GetAuthList(seqfeat->GetData().GetPub());
    } else if (citsub && citsub->IsSetAuthors()) {
        return CConstRef<CAuth_list>(&(citsub->GetAuthors()));
    } else if (pubdesc) {
        return GetAuthList(*pubdesc);
    } else if (block && block->IsSetCit() && block->GetCit().IsSetAuthors()) {
        return CConstRef<CAuth_list>(&(block->GetCit().GetAuthors()));
    } else {
        return CConstRef<CAuth_list>(NULL);
    }
}


bool SetAuthList (CPubdesc& pdesc, const CAuth_list& auth_list)
{
    if (!pdesc.IsSetPub()) {
        return false;
    }
    bool any_change = false;
    NON_CONST_ITERATE (CPub_equiv::Tdata, it, pdesc.SetPub().Set()) {
        switch ((*it)->Which()) {
            case CPub::e_Gen:
                (*it)->SetGen().SetAuthors().Assign(auth_list);
                any_change = true;
                break;
            case CPub::e_Sub:
                (*it)->SetSub().SetAuthors().Assign(auth_list);
                any_change = true;
                break;
            case CPub::e_Article:
                (*it)->SetArticle().SetAuthors().Assign(auth_list);
                any_change = true;
                break;
            case CPub::e_Book:
                (*it)->SetBook().SetAuthors().Assign(auth_list);
                any_change = true;
                break;
            case CPub::e_Proc:
                (*it)->SetProc().SetBook().SetAuthors().Assign(auth_list);
                any_change = true;
                break;
            case CPub::e_Patent:
                (*it)->SetPatent().SetAuthors().Assign(auth_list);
                any_change = true;
                break;
            case CPub::e_Man:
                (*it)->SetMan().SetCit().SetAuthors().Assign(auth_list);
                any_change = true;
                break;
            case CPub::e_Medline:
            case CPub::e_Journal:
            case CPub::e_Pat_id:
            case CPub::e_Equiv:
            case CPub::e_Pmid:
            default:
                // do nothing
                break;
        }
    }

    return any_change;
}

bool SetAuthorNames (CPubdesc& pdesc, const CAuth_list& auth_list)
{
    if (!pdesc.IsSetPub() || !auth_list.IsSetNames())
        return false;

    bool any_change(false);
    const CAuth_list::TNames& names = auth_list.GetNames();
    NON_CONST_ITERATE (CPub_equiv::Tdata, it, pdesc.SetPub().Set()) {
        switch ((*it)->Which()) {
            case CPub::e_Gen:
                (*it)->SetGen().SetAuthors().SetNames().Assign(names);
                any_change = true;
                break;
            case CPub::e_Sub:
                (*it)->SetSub().SetAuthors().SetNames().Assign(names);
                any_change = true;
                break;
            case CPub::e_Article:
                (*it)->SetArticle().SetAuthors().SetNames().Assign(names);
                any_change = true;
                break;
            case CPub::e_Book:
                (*it)->SetBook().SetAuthors().SetNames().Assign(names);
                any_change = true;
                break;
            case CPub::e_Proc:
                (*it)->SetProc().SetBook().SetAuthors().SetNames().Assign(names);
                any_change = true;
                break;
            case CPub::e_Patent:
                (*it)->SetPatent().SetAuthors().SetNames().Assign(names);
                any_change = true;
                break;
            case CPub::e_Man:
                (*it)->SetMan().SetCit().SetAuthors().SetNames().Assign(names);
                any_change = true;
                break;
            case CPub::e_Medline:
            case CPub::e_Journal:
            case CPub::e_Pat_id:
            case CPub::e_Equiv:
            case CPub::e_Pmid:
            default:
                // do nothing
                break;
        }
    }
    return any_change;
}

string GetTitleName(const CTitle& title)
{
    string rval = "";
    try {
        rval = title.GetTitle(CTitle::C_E::e_Name);
    } catch (const exception &) {
    }
    return rval;
}


string CPubField::GetPubTitle (const CPubdesc& pdesc)
{
    string rval = "";
    if (!pdesc.IsSetPub()) {
        return rval;
    }
    ITERATE (CPub_equiv::Tdata, it, pdesc.GetPub().Get()) {
        rval = GetPubTitle(**it);
        if (!NStr::IsBlank(rval)) {
            break;
        }
    }

    return rval;
}


string CPubField::GetPubTitle (const CPub& pub)
{
    string rval = "";
    switch (pub.Which()) {
        case CPub::e_Gen:
            if (pub.GetGen().IsSetTitle()) {
                rval = pub.GetGen().GetTitle();
            }
            break;
        case CPub::e_Sub:
            rval = GetPubTitle(pub.GetSub());
            break;
        case CPub::e_Article:
            if (pub.GetArticle().IsSetTitle()) {
                rval = GetTitleName(pub.GetArticle().GetTitle());
            }
            break;
        case CPub::e_Book:
            if (pub.GetBook().IsSetTitle()) {
                rval = GetTitleName(pub.GetBook().GetTitle());
            }
            break;
        case CPub::e_Proc:
            if (pub.GetProc().IsSetBook()
                && pub.GetProc().GetBook().IsSetTitle()) {
                rval = GetTitleName(pub.GetProc().GetBook().GetTitle());
            }
            break;
        case CPub::e_Patent:
            if (pub.GetPatent().IsSetTitle()) {
                rval = pub.GetPatent().GetTitle();
            }
            break;
        case CPub::e_Man:
            if (pub.GetMan().IsSetCit()
                && pub.GetMan().GetCit().IsSetTitle()) {
                rval = GetTitleName(pub.GetMan().GetCit().GetTitle());
            }
            break;
        case CPub::e_Medline:
        case CPub::e_Journal:
        case CPub::e_Pat_id:
        case CPub::e_Equiv:
        case CPub::e_Pmid:
        default:
            // do nothing
            break;
    }

    return rval;
}


string CPubField::GetPubTitle (const CCit_sub& sub)
{
    string rval = "";

    if (sub.IsSetDescr()) {
        rval = sub.GetDescr();
    }
    return rval;
}


string GetPubTitle(CConstRef<CObject> sel_pub)
{
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(sel_pub.GetPointer());
    const CSeq_feat* seqfeat = dynamic_cast<const CSeq_feat*>(sel_pub.GetPointer());
    const CCit_sub* citsub = dynamic_cast<const CCit_sub*>(sel_pub.GetPointer());
    const CPubdesc* pubdesc = dynamic_cast<const CPubdesc*>(sel_pub.GetPointer());
    if (seqdesc && seqdesc->IsPub()) {
        return CPubField::GetPubTitle(seqdesc->GetPub());
    } else if (seqfeat && seqfeat->IsSetData() && seqfeat->GetData().IsPub()) {
        return CPubField::GetPubTitle(seqfeat->GetData().GetPub());
    } else if (citsub && citsub->IsSetDescr()) {
        return citsub->GetDescr();
    } else if (pubdesc) {
        return CPubField::GetPubTitle(*pubdesc);
    } else {
        return "";
    }
}


void SetTitleName(CTitle& title, string val)
{
    if (!title.IsSet() || title.Get().size() == 0) {
        CRef<CTitle::C_E> this_title(new CTitle::C_E());
        this_title->SetName(val);
        title.Set().push_back(this_title);
    } else {
        // if we find a Name, change that
        bool found = false;
        NON_CONST_ITERATE(CTitle::Tdata, it, title.Set()) {
            if ((*it)->IsName()) {
                (*it)->SetName(val);
                found = true;
            }
        }
        if (!found) {
            // if we find a not_set, change it to a Name
            NON_CONST_ITERATE(CTitle::Tdata, it, title.Set()) {
                if ((*it)->Which() == CTitle::C_E::e_not_set) {
                    (*it)->SetName(val);
                    found = true;
                }
            }
        }
        // otherwise add a Name
        if (!found) {
            CRef<CTitle::C_E> this_title(new CTitle::C_E());
            this_title->SetName(val);
            title.Set().push_back(this_title); 
        }
    }
}


bool CPubField::SetPubTitle(CPubdesc& pdesc, const string& title, EExistingText existing_text, bool skip_sub)
{
    bool rval = false;
    if (!pdesc.IsSetPub()) {
        return rval;
    }
    string orig_title = GetPubTitle (pdesc);
    if (AddValueToString (orig_title, title, existing_text)) {
        NON_CONST_ITERATE (CPub_equiv::Tdata, it, pdesc.SetPub().Set()) {
            switch ((*it)->Which()) {
                case CPub::e_Gen:
                    (*it)->SetGen().SetTitle(title);
                    rval = true;
                    break;
                case CPub::e_Sub:
                    if (skip_sub && !(*it)->GetSub().IsSetDescr()) {
                        rval = false;
                    } else {
                        (*it)->SetSub().SetDescr(title);
                        rval = true;
                    }
                    break;
                case CPub::e_Article:
                    SetTitleName((*it)->SetArticle().SetTitle(), title);
                    rval = true;
                    break;
                case CPub::e_Book:
                    SetTitleName((*it)->SetBook().SetTitle(), title);
                    rval = true;
                    break;
                case CPub::e_Proc:
                    SetTitleName((*it)->SetProc().SetBook().SetTitle(), title);
                    rval = true;
                    break;
                case CPub::e_Patent:
                    (*it)->SetPatent().SetTitle(title);
                    rval = true;
                    break;
                case CPub::e_Man:
                    SetTitleName((*it)->SetMan().SetCit().SetTitle(), title);
                    rval = true;
                    break;
                case CPub::e_Medline:
                case CPub::e_Journal:
                case CPub::e_Pat_id:
                case CPub::e_Equiv:
                case CPub::e_Pmid:
                default:
                    // do nothing
                    break;
             }
        }
    }
    return rval;
}


string CPubField::GetJournal (const CPubdesc& pdesc)
{
    string rval = "";
    ITERATE(CPubdesc::TPub::Tdata, it, pdesc.GetPub().Get()) {
        rval = GetJournal(**it);
        if (!NStr::IsBlank(rval)) {
            break;
        }
    }
    return rval;
}


string CPubField::GetJournal (const CPub& pub)
{
    string rval = "";

    switch (pub.Which()) {
        case CPub::e_Gen:
            if (pub.GetGen().IsSetJournal()) {
                rval = GetTitleName(pub.GetGen().GetJournal());
            }
            break;
        case CPub::e_Article:
            if (pub.GetArticle().IsSetFrom()
                && pub.GetArticle().GetFrom().IsJournal()
                && pub.GetArticle().GetFrom().GetJournal().IsSetTitle()) {
                rval = GetTitleName(pub.GetArticle().GetFrom().GetJournal().GetTitle());
            }
            break;
        case CPub::e_Journal:
            if (pub.GetJournal().IsSetTitle()) {
                rval = GetTitleName(pub.GetJournal().GetTitle());
            }
            break;
        default:
            break;
    }
    return rval;
}


bool CPubField::SetJournal (CPubdesc& pdesc, const string& journal, EExistingText existing_text)
{
    bool rval = false;
    NON_CONST_ITERATE(CPubdesc::TPub::Tdata, it, pdesc.SetPub().Set()) {
        rval |= SetJournal(**it, journal, existing_text);
    }
    return rval;
    
}


bool CPubField::SetJournal (CPub& pub, const string& journal, EExistingText existing_text)
{
    bool rval = false;

    string val = GetJournal(pub);
    if (!AddValueToString (val, journal, existing_text)) {
        return false;
    }
    switch (pub.Which()) {
        case CPub::e_Gen:
            SetTitleName(pub.SetGen().SetJournal(), val);
            rval = true;
            break;
        case CPub::e_Article:
            if (!pub.GetArticle().IsSetFrom()
                || pub.GetArticle().GetFrom().IsJournal()) {
                SetTitleName(pub.SetArticle().SetFrom().SetJournal().SetTitle(), val);
                rval = true;
            }
            break;
        case CPub::e_Journal:
            SetTitleName(pub.SetJournal().SetTitle(), val);
            rval = true;
            break;
        default:
            break;
    }
    return rval;
}


string CPubField::x_GetImprintField (const CPubdesc& pdesc, CPubFieldType::EPubFieldType field_type)
{
    string rval = "";
    ITERATE(CPubdesc::TPub::Tdata, it, pdesc.GetPub().Get()) {
        rval = x_GetImprintField(**it, field_type);
        if (!NStr::IsBlank(rval)) {
            break;
        }
    }
    return rval;
}


string CPubField::x_GetImprintField (const CPub& pub, CPubFieldType::EPubFieldType field_type)
{
    string rval = "";
    if (pub.IsGen()) {
        rval = x_GetImprintField(pub.GetGen(), field_type);
    } else {
        CConstRef<CImprint> imp = GetImprint(pub);
        if (imp) {
            rval = x_GetImprintField(*imp, field_type);
        }
    }
    return rval;
}

string CPubField::x_GetImprintField (const CCit_gen& gen, CPubFieldType::EPubFieldType field_type)
{
    string rval = "";
    switch (field_type) {
        case CPubFieldType::ePubFieldType_Volume:
            if (gen.IsSetVolume()) {
                rval = gen.GetVolume();
            }
            break;
        case CPubFieldType::ePubFieldType_Issue:
            if (gen.IsSetIssue()) {
                rval = gen.GetIssue();
            }
            break;
        case CPubFieldType::ePubFieldType_Pages:
            if (gen.IsSetPages()) {
                rval = gen.GetPages();
            }
            break;
        case CPubFieldType::ePubFieldType_Date:
            if (gen.IsSetDate()) {
                gen.GetDate().GetDate(&rval);
            }
            break;
        default:
            break;
    }
    return rval;
}


string CPubField::x_GetImprintField (const CCit_sub& sub, CPubFieldType::EPubFieldType field_type)
{
    string rval = "";
    if (sub.IsSetImp()) {
        rval = x_GetImprintField(sub.GetImp(), field_type);
    }
    return rval;
}


string CPubField::x_GetImprintField (const CImprint& imp, CPubFieldType::EPubFieldType field_type)
{
    string rval = "";
    switch (field_type) {
        case CPubFieldType::ePubFieldType_Volume:
            if (imp.IsSetVolume()) {
                rval = imp.GetVolume();
            }
            break;
        case CPubFieldType::ePubFieldType_Issue:
            if (imp.IsSetIssue()) {
                rval = imp.GetIssue();
            }
            break;
        case CPubFieldType::ePubFieldType_Pages:
            if (imp.IsSetPages()) {
                rval = imp.GetPages();
            }
            break;
        case CPubFieldType::ePubFieldType_Date:
            if (imp.IsSetDate()) {
                imp.GetDate().GetDate(&rval);
            }
            break;
        default:
            break;
    }
    return rval;
}


CConstRef<CImprint> CPubField::GetImprint(const CPub& pub)
{
    CConstRef<CImprint> rval(NULL);

    switch (pub.Which()) {
        case CPub::e_Article:
            if (pub.GetArticle().IsSetFrom()) {
                switch (pub.GetArticle().GetFrom().Which()) {
                    case CCit_art::TFrom::e_Book:
                        if (pub.GetArticle().GetFrom().GetBook().IsSetImp()) {
                            rval = &(pub.GetArticle().GetFrom().GetBook().GetImp());
                        }
                        break;
                    case CCit_art::TFrom::e_Journal:
                        if (pub.GetArticle().GetFrom().GetJournal().IsSetImp()) {
                            rval = &(pub.GetArticle().GetFrom().GetJournal().GetImp());
                        }
                        break;
                    case CCit_art::TFrom::e_Proc:
                        if (pub.GetArticle().GetFrom().GetProc().IsSetBook()
                            && pub.GetArticle().GetFrom().GetProc().GetBook().IsSetImp()) {
                            rval = &(pub.GetArticle().GetFrom().GetProc().GetBook().GetImp());
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        case CPub::e_Book:
            if (pub.GetBook().IsSetImp()) {
                rval = &(pub.GetBook().GetImp());
            }
            break;
        case CPub::e_Gen:
            // Cit-gen has no imprint
            break;
        case CPub::e_Journal:
            if (pub.GetJournal().IsSetImp()) {
                rval = &(pub.GetJournal().GetImp());
            }
            break;
        case CPub::e_Man:
            if (pub.GetMan().IsSetCit() && pub.GetMan().GetCit().IsSetImp()) {
                rval = &(pub.GetMan().GetCit().GetImp());
            }
            break;
        case CPub::e_Patent:
            // Cit-pat has no imprint
            break;
        case CPub::e_Proc:
            if (pub.GetProc().IsSetBook() && pub.GetProc().GetBook().IsSetImp()) {
                rval = &(pub.GetProc().GetBook().GetImp());
            }
            break;
        case CPub::e_Sub:
            // Cit-sub has no imprint
            break;
        case CPub::e_Medline:
            // no imprint
            break;
        case CPub::e_Muid:
            // no imprint
            break;
        case CPub::e_Pat_id:
            // no imprint
            break;
        case CPub::e_Pmid:
            // no imprint
            break;
        case CPub::e_Equiv:
            ITERATE(CPub::TEquiv::Tdata, it, pub.GetEquiv().Get()) {
                rval = GetImprint(**it);
                if (rval) {
                    break;
                }
            }
            break;
        case CPub::e_not_set:
            // no imprint
            break;
    }
    return rval;
}


bool CPubField::SetImprint(CPub& pub, const CImprint& imp)
{
    bool rval = false;
    switch (pub.Which()) {
        case CPub::e_Article:
            if (pub.GetArticle().IsSetFrom()) {
                switch (pub.GetArticle().GetFrom().Which()) {
                    case CCit_art::TFrom::e_Book:
                        pub.SetArticle().SetFrom().SetBook().SetImp().Assign(imp);
                        rval = true;
                        break;
                    case CCit_art::TFrom::e_Journal:
                        pub.SetArticle().SetFrom().SetJournal().SetImp().Assign(imp);
                        rval = true;
                        break;
                    case CCit_art::TFrom::e_Proc:
                        pub.SetArticle().SetFrom().SetProc().SetBook().SetImp().Assign(imp);
                        rval = true;
                        break;
                    default:
                        break;
                }
            }
            break;
        case CPub::e_Book:
            pub.SetBook().SetImp().Assign(imp);
            rval = true;
            break;
        case CPub::e_Gen:
            // Cit-gen has no imprint
            break;
        case CPub::e_Journal:
            pub.SetJournal().SetImp().Assign(imp);
            rval = true;
            break;
        case CPub::e_Man:
            pub.SetMan().SetCit().SetImp().Assign(imp);
            rval = true;
            break;
        case CPub::e_Patent:
            // Cit-pat has no imprint
            break;
        case CPub::e_Proc:
            pub.SetProc().SetBook().SetImp().Assign(imp);
            break;
        case CPub::e_Sub:
            // Cit-sub has no imprint
            break;
        case CPub::e_Medline:
            // no imprint
            break;
        case CPub::e_Muid:
            // no imprint
            break;
        case CPub::e_Pat_id:
            // no imprint
            break;
        case CPub::e_Pmid:
            // no imprint
            break;
        case CPub::e_Equiv:
            NON_CONST_ITERATE(CPub::TEquiv::Tdata, it, pub.SetEquiv().Set()) {
                rval |= SetImprint(**it, imp);
            }
            break;
        default:
            break;
    }
    return rval;
}


bool CPubField::x_SetImprintField (CPubdesc& pdesc, CPubFieldType::EPubFieldType field_type, const string& val, EExistingText existing_text)
{
    bool rval = false;
    NON_CONST_ITERATE(CPubdesc::TPub::Tdata, it, pdesc.SetPub().Set()) {
        rval |= x_SetImprintField(**it, field_type, val, existing_text);
    }
    return rval;
    
}


bool CPubField::x_SetImprintField (CPub& pub, CPubFieldType::EPubFieldType field_type, const string& val, EExistingText existing_text)
{
    bool rval = false;

    string str = x_GetImprintField(pub, field_type);
    if (!AddValueToString (str, val, existing_text)) {
        return false;
    }
    if (pub.IsGen()) {
        rval = x_SetImprintField(pub.SetGen(), field_type, val);
    } else {
        CConstRef<CImprint> imp = GetImprint(pub);
        CRef<CImprint> new_imp(new CImprint());
        if (imp) {
            new_imp->Assign(*imp);
        }
        x_SetImprintField(*new_imp, field_type, val);
        rval = SetImprint(pub, *new_imp);
    }
    return rval;
}

CRef<CDate> CPubField::GuessDate(const string &val)
{
    CRef<CDate> date;
    if (val.length() == 4 && NStr::StringToInt(val, NStr::fConvErr_NoThrow ) != 0)
    {
        int year = NStr::StringToInt(val, NStr::fConvErr_NoThrow );
        date.Reset(new CDate);
        date->SetStd().SetYear(year);
    }
    else
    {
        date.Reset(new CDate(CTime(val)));
    }
    return date;
}

bool CPubField::x_SetImprintField(CCit_gen& gen, CPubFieldType::EPubFieldType field_type, const string& val)
{
    bool rval = false;

    switch (field_type) {
        case CPubFieldType::ePubFieldType_Volume:
            gen.SetVolume(val);
            rval = true;
            break;
        case CPubFieldType::ePubFieldType_Issue:
            gen.SetIssue(val);
            rval = true;
            break;
        case CPubFieldType::ePubFieldType_Pages:
            gen.SetPages(val);
            rval = true;
            break;
        case CPubFieldType::ePubFieldType_Date:
        {
            CRef<CDate> date = GuessDate(val);
            gen.SetDate(*date);
            rval = true;
        }
            break;
        default:
            break;
    }
    return rval;
}


bool CPubField::x_SetImprintField(CImprint& imp, CPubFieldType::EPubFieldType field_type, const string& val)
{
    bool rval = false;

    switch (field_type) {
        case CPubFieldType::ePubFieldType_Volume:
            imp.SetVolume(val);
            rval = true;
            break;
        case CPubFieldType::ePubFieldType_Issue:
            imp.SetIssue(val);
            rval = true;
            break;
        case CPubFieldType::ePubFieldType_Pages:
            imp.SetPages(val);
            rval = true;
            break;
        case CPubFieldType::ePubFieldType_Date:
        {
            CRef<CDate> date = GuessDate(val);
            imp.SetDate(*date);
            rval = true;
        }
            break;
        default:
            break;
    }
    return rval;
}


bool CPubField::x_SetAuthorField
(CPubdesc &pdesc,
 CPubFieldType::EPubFieldType field_type, 
 string value, 
 CPubFieldType::EPubFieldType constraint_field, 
 CRef<CStringConstraint> string_constraint,
 EExistingText existing_text)
{
    bool rval = false;
    CConstRef<CAuth_list> orig = GetAuthList(pdesc);
    CRef<CAuth_list> new_list(new CAuth_list());
    if (orig) {
        new_list->Assign(*orig);
    }
    rval = x_SetAuthorField(*new_list, field_type, value, constraint_field, string_constraint, existing_text);
    if (!rval && (!IsAuthorField(m_ConstraintFieldType) || !m_StringConstraint)) {
        CRef<CAuthor> new_auth( new CAuthor());
        if (field_type != CPubFieldType::ePubFieldType_AuthorConsortium) {
            if (field_type != CPubFieldType::ePubFieldType_AuthorLastName) {
                // need blank author last name to avoid flatfile formatting fail
                SetAuthorField(*new_auth, CPubFieldType::ePubFieldType_AuthorLastName, " ", eExistingText_replace_old);
            }
            if (field_type != CPubFieldType::ePubFieldType_AuthorFirstName) {
                // need blank author first name to avoid flatfile formatting fail
                SetAuthorField(*new_auth, CPubFieldType::ePubFieldType_AuthorFirstName, " ", eExistingText_replace_old);
            }
        }
        SetAuthorField(*new_auth, field_type, value, eExistingText_replace_old);
        new_list->SetNames().SetStd().push_back(new_auth);
        rval = true;
    }
    if (rval) {
        SetAuthList(pdesc, *new_list);
    }

    return rval;
}


bool CPubField::x_SetAuthorField
(CAuth_list &auth_list,
 CPubFieldType::EPubFieldType field_type,
 string value,
 CPubFieldType::EPubFieldType constraint_field,
 CRef<CStringConstraint> string_constraint,
 EExistingText existing_text)
{
    bool rval = false;

    if (IsAuthorField(m_ConstraintFieldType) && m_StringConstraint)
    {
        NON_CONST_ITERATE(CAuth_list::TNames::TStd, it, auth_list.SetNames().SetStd()) {
            if (DoesAuthorMatchConstraint(**it, constraint_field, string_constraint)) {
                rval |= SetAuthorField(**it, field_type, value, existing_text);
            }
        }
    }
    else if (field_type == CPubFieldType::ePubFieldType_Authors && existing_text ==  eExistingText_replace_old)
    {
        list<string>  arr;
        NStr::Split(value, ",", arr, NStr::fSplit_Tokenize);
        auth_list.ResetNames();
        for (list<string>::iterator name = arr.begin(); name != arr.end(); ++name)
        {
            vector<string> full_name;
            NStr::Split(*name, " ", full_name, NStr::fSplit_Tokenize);            
            string first_name, last_name, middle_init, suffix;
            if (full_name.size() == 4)
            {
                first_name = full_name[0];
                string middle = full_name[1];
                last_name = full_name[2];
                suffix = full_name[3];
                middle_init = edit::GetFirstInitial(first_name, true) + edit::GetFirstInitial(middle, false);
                rval = true;
            }
            else if (full_name.size() == 3)
            {
                first_name = full_name[0];
                string middle = full_name[1];
                last_name = full_name[2];
                middle_init = edit::GetFirstInitial(first_name, true) + edit::GetFirstInitial(middle, false);
                rval = true;       
            }
            else if (full_name.size() == 2)
            {
                first_name = full_name[0];
                last_name = full_name[1];
                middle_init = edit::GetFirstInitial(first_name, true);
                rval = true;
            }
            else if (full_name.size() == 1)
            {
                last_name = full_name[0];
                rval = true;
            }
            if (rval)
            {
                CRef<CAuthor> new_auth( new CAuthor());
                new_auth->SetName().SetName().SetLast(last_name);
                if (!first_name.empty())
                    new_auth->SetName().SetName().SetFirst(first_name);
                if (!middle_init.empty())
                    new_auth->SetName().SetName().SetInitials(middle_init);
                if (!suffix.empty())
                    new_auth->SetName().SetName().SetSuffix(suffix);
                auth_list.SetNames().SetStd().push_back(new_auth);
            }
        }
    }
    else
    {
        CRef<CAuthor> new_auth( new CAuthor());
        rval = SetAuthorField(*new_auth, field_type, value, eExistingText_replace_old);
        if (existing_text ==  eExistingText_replace_old)
            auth_list.SetNames().SetStd().clear();
        auth_list.SetNames().SetStd().push_back(new_auth);
    }

    return rval;
}


string CPubField::x_GetAuthorField(const CPubdesc &pdesc, CPubFieldType::EPubFieldType field_type, CPubFieldType::EPubFieldType constraint_field, CRef<CStringConstraint> string_constraint)
{
    string rval = "";
    CConstRef<CAuth_list> auth_list = GetAuthList(pdesc);
    if (auth_list) {
        rval = x_GetAuthorField(*auth_list, field_type, constraint_field, string_constraint);
    }
    return rval;
}


string CPubField::x_GetAuthorField(const CAuth_list &auth_list, CPubFieldType::EPubFieldType field_type, CPubFieldType::EPubFieldType constraint_field, CRef<CStringConstraint> string_constraint)
{
    string rval = "";
    if (auth_list.IsSetNames() && auth_list.GetNames().IsStd()) {
        ITERATE(CAuth_list::TNames::TStd, it, auth_list.GetNames().GetStd()) {
            if (DoesAuthorMatchConstraint(**it, constraint_field, string_constraint)) {
                rval = GetAuthorField(**it, field_type);
                if (!NStr::IsBlank(rval)) {
                    return rval;
                }
            }
        }
    }
    return rval;
}


bool CPubField::SetVal(CObject& object, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    CSeqdesc* desc = dynamic_cast<CSeqdesc * >(&object);
    CSeq_feat* feat = dynamic_cast<CSeq_feat * >(&object);
    CCit_sub* citsub = dynamic_cast<CCit_sub*>(&object);
    CPubdesc* pubdesc = dynamic_cast<CPubdesc*>(&object);
    CSubmit_block* block = dynamic_cast<CSubmit_block*>(&object);

    if (desc && desc->IsPub()) {
        SetVal(desc->SetPub(), newValue, existing_text);
        rval = true;
    } else if (feat && feat->IsSetData() && feat->GetData().IsPub()) {
        SetVal(feat->SetData().SetPub(), newValue, existing_text);
        rval = true;
    } else if (pubdesc) {
        SetVal(*pubdesc, newValue, existing_text);
    } else if (citsub) {
        SetVal(*citsub, newValue, existing_text);
    } else if (block) {
        SetVal(block->SetCit(), newValue, existing_text);
    }
    return rval;
}


void CPubField::SetVal(CSeqdesc & desc, const string & newValue, EExistingText existing_text )
{
    if (!desc.IsPub()) {
        return;
    }
    SetVal(desc.SetPub(), newValue, existing_text);
}


void CPubField::SetVal(CPubdesc& pdesc, const string & newValue, EExistingText existing_text)
{
    if (IsAffilField(m_FieldType)) {
        x_SetAffilField(pdesc, m_FieldType, newValue, existing_text);
        return;
    } else if (IsAuthorField(m_FieldType)) {
        x_SetAuthorField(pdesc, m_FieldType, newValue, m_ConstraintFieldType, m_StringConstraint, existing_text);
        return;
    } else if (IsImprintField(m_FieldType)) {
        x_SetImprintField(pdesc, m_FieldType, newValue, existing_text);
    }

    switch (m_FieldType) {
        case CPubFieldType::ePubFieldType_Title:
            SetPubTitle(pdesc, newValue, existing_text);
            break;
        case CPubFieldType::ePubFieldType_Journal:
            SetJournal(pdesc, newValue, existing_text);
            break;
        case CPubFieldType::ePubFieldType_Status:
            x_SetStatus(pdesc, newValue);
            break;
        case CPubFieldType::ePubFieldType_Pmid:
        {
            bool found(false);
            if (pdesc.IsSetPub()) 
            {
                NON_CONST_ITERATE (CPub_equiv::Tdata, it, pdesc.SetPub().Set()) 
                {
                    if ((*it)->IsPmid())
                    {
                        (*it)->SetPmid(CPubMedId(NStr::StringToInt(newValue, NStr::fConvErr_NoThrow)));
                        found = true;
                    }
                }
            }
            if (!found)
            {
                CRef<CPub> pub(new CPub);
                pub->SetPmid(CPubMedId(NStr::StringToInt(newValue, NStr::fConvErr_NoThrow)));
                pdesc.SetPub().Set().push_back(pub);
            }
        }
            break;
        case CPubFieldType::ePubFieldType_Unknown:
        default:
            break;
    }
}


void CPubField::SetVal(CCit_sub& sub, const string & newValue, EExistingText existing_text)
{
    if (IsAffilField(m_FieldType)) {
        x_SetAffilField(sub.SetAuthors(), m_FieldType, newValue, existing_text);
        return;
    } else if (IsAuthorField(m_FieldType)) {
        x_SetAuthorField(sub.SetAuthors(), m_FieldType, newValue, m_ConstraintFieldType, m_StringConstraint, existing_text);
        return;
    } else if (IsImprintField(m_FieldType)) {
        string orig_val = "";
        if (sub.IsSetImp()) {
            orig_val = x_GetImprintField(sub.GetImp(), m_FieldType);
        }
        if (AddValueToString(orig_val, newValue, existing_text)) {
            x_SetImprintField(sub.SetImp(), m_FieldType, orig_val);
        }
    }

    switch (m_FieldType) {
        case CPubFieldType::ePubFieldType_Title:
            {{
                string curr_val = "";
                if (sub.IsSetDescr()) {
                    curr_val = sub.GetDescr();
                }
                if (AddValueToString(curr_val, newValue, existing_text)) {
                    sub.SetDescr(curr_val);
                }
            }}
            break;
        case CPubFieldType::ePubFieldType_Journal:
            // cit-sub has no journal
            break;
        case CPubFieldType::ePubFieldType_Unknown:
        default:
            break;
    }
}


void CPubField::ClearVal( CPubdesc& pdesc)
{
    SetVal (pdesc, " ", eExistingText_replace_old);
}


void CPubField::ClearVal( CCit_sub& sub)
{
    SetVal (sub, " ", eExistingText_replace_old);
}


void CPubField::ClearVal(CSeqdesc & in_out_desc )
{
    SetVal(in_out_desc, " ", eExistingText_replace_old);
}


string CPubField::GetVal( const CObject& object)
{
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(&object);
    const CSeq_feat* seqfeat = dynamic_cast<const CSeq_feat*>(&object);
    const CCit_sub* citsub = dynamic_cast<const CCit_sub*>(&object);
    const CPubdesc* pubdesc = dynamic_cast<const CPubdesc*>(&object);
    const CSubmit_block* block = dynamic_cast<const CSubmit_block*>(&object);
    if (seqdesc && seqdesc->IsPub()) {
        return GetVal(seqdesc->GetPub());
    } else if (seqfeat && seqfeat->IsSetData() && seqfeat->GetData().IsPub()) {
        return GetVal(seqfeat->GetData().GetPub());
    } else if (pubdesc) {
        return GetVal(*pubdesc);
    } else if (citsub) {
        return GetVal(*citsub);
    }
    return "";
}


string CPubField::GetVal(const CPubdesc& pdesc)
{
    string val = "";
    vector<string> vals = GetVals(pdesc);
    if (vals.size() > 0) {
        val = vals[0];
    }
    return val;
}


string CPubField::GetVal(const CPub& pub)
{
    string val = "";
    vector<string> vals = GetVals(pub);
    if (vals.size() > 0) {
        val = vals[0];
    }
    return val;
}


string CPubField::GetVal(const CCit_sub& sub)
{
    string val = "";
    vector<string> vals = GetVals(sub);
    if (vals.size() > 0) {
        val = vals[0];
    }
    return val;
}


vector<string> CPubField::GetVals(const CObject& object)
{
    vector<string> vals;
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(&object);
    const CSeq_feat* seqfeat = dynamic_cast<const CSeq_feat*>(&object);
    const CCit_sub* citsub = dynamic_cast<const CCit_sub*>(&object);
    const CPubdesc* pubdesc = dynamic_cast<const CPubdesc*>(&object);
    const CSubmit_block* block = dynamic_cast<const CSubmit_block*>(&object);
    if (seqdesc && seqdesc->IsPub()) {
        vals = GetVals(seqdesc->GetPub());
    } else if (seqfeat && seqfeat->IsSetData() && seqfeat->GetData().IsPub()) {
        vals = GetVals(seqfeat->GetData().GetPub());
    } else if (pubdesc) {
        vals = GetVals(*pubdesc);
    } else if (citsub) {
        vals = GetVals(*citsub);
    } else if (block && block->IsSetCit()) {
        vals = GetVals(block->GetCit());
    }
    return vals;
}


vector<string> CPubField::GetVals( const CPubdesc& pdesc)
{
    vector<string> vals;

    if (IsAuthorField(m_FieldType)) {
        CConstRef<CAuth_list> auth_list = GetAuthList(pdesc);
        if (auth_list->IsSetNames() && auth_list->GetNames().IsStd()) {
            ITERATE(CAuth_list::TNames::TStd, it, auth_list->GetNames().GetStd()) {
                if (DoesAuthorMatchConstraint(**it, m_ConstraintFieldType, m_StringConstraint)) {
                    string str = GetAuthorField(**it, m_FieldType);
                    if (!NStr::IsBlank(str)) {
                        vals.push_back(str);
                    }
                }
            }
        }
    } else if (IsAffilField(m_FieldType)) {
        vals.push_back(x_GetAffilField(pdesc, m_FieldType));
    } else if (IsImprintField(m_FieldType)) {
        vals.push_back(x_GetImprintField(pdesc, m_FieldType));
    } else {
        switch (m_FieldType) {
            case CPubFieldType::ePubFieldType_Title:
                vals.push_back(GetPubTitle(pdesc));
                break;
            case CPubFieldType::ePubFieldType_Journal:
                vals.push_back(GetJournal(pdesc));
                break;
            case CPubFieldType::ePubFieldType_Status:
                 vals.push_back(GetStatus(pdesc));
                break;
            case CPubFieldType::ePubFieldType_Pmid:
            {
                if (pdesc.IsSetPub()) 
                {
                    ITERATE (CPub_equiv::Tdata, it, pdesc.GetPub().Get()) 
                    {
                        if ((*it)->IsPmid())
                        {
                            vals.push_back(NStr::IntToString((*it)->GetPmid()));
                        }
                    }
                }
            }
                break;
            case CPubFieldType::ePubFieldType_Unknown:
            default:
                break;
        }
    }

    return vals;
}


vector<string> CPubField::GetVals( const CPub& pub)
{
    vector<string> vals;

    if (IsAuthorField(m_FieldType)) {
        CConstRef<CAuth_list> auth_list = GetAuthList(pub);
        if (auth_list->IsSetNames() && auth_list->GetNames().IsStd()) {
            ITERATE(CAuth_list::TNames::TStd, it, auth_list->GetNames().GetStd()) {
                if (DoesAuthorMatchConstraint(**it, m_ConstraintFieldType, m_StringConstraint)) {
                    string str = GetAuthorField(**it, m_FieldType);
                    if (!NStr::IsBlank(str)) {
                        vals.push_back(str);
                    }
                }
            }
        }
    } else if (IsAffilField(m_FieldType)) {
        vals.push_back(x_GetAffilField(pub, m_FieldType));
    } else if (IsImprintField(m_FieldType)) {
        vals.push_back(x_GetImprintField(pub, m_FieldType));
    } else {
        switch (m_FieldType) {
            case CPubFieldType::ePubFieldType_Title:
                vals.push_back(GetPubTitle(pub));
                break;
            case CPubFieldType::ePubFieldType_Journal:
                vals.push_back(GetJournal(pub));
                break;
            case CPubFieldType::ePubFieldType_Status:
                 vals.push_back(GetStatus(pub));
                 break;
            case CPubFieldType::ePubFieldType_Pmid:
            {
                if (pub.IsPmid())
                {
                    vals.push_back(NStr::IntToString(pub.GetPmid()));
                }
            }
                break;
            case CPubFieldType::ePubFieldType_Unknown:
            default:
                break;
        }
    }

    return vals;
}


vector<string> CPubField::GetVals( const CCit_sub& sub)
{
    vector<string> vals;

    if (IsAuthorField(m_FieldType)) {
        if (sub.IsSetAuthors()) {
            const CAuth_list& auth_list = sub.GetAuthors();
            if (auth_list.IsSetNames() && auth_list.GetNames().IsStd()) {
                ITERATE(CAuth_list::TNames::TStd, it, auth_list.GetNames().GetStd()) {
                    if (DoesAuthorMatchConstraint(**it, m_ConstraintFieldType, m_StringConstraint)) {
                        string str = GetAuthorField(**it, m_FieldType);
                        if (!NStr::IsBlank(str)) {
                            vals.push_back(str);
                        }
                    }
                }
            }
        }
    } else if (IsAffilField(m_FieldType)) {
        vals.push_back(x_GetAffilField(sub, m_FieldType));
    } else if (IsImprintField(m_FieldType)) {
        vals.push_back(x_GetImprintField(sub, m_FieldType));
    } else {
        switch (m_FieldType) {
            case CPubFieldType::ePubFieldType_Title:
                vals.push_back(GetPubTitle(sub));
                break;
            case CPubFieldType::ePubFieldType_Journal:
                // Cit-sub has no journal
                break;
            case CPubFieldType::ePubFieldType_Unknown:
            default:
                break;
        }
    }

    return vals;
}


vector<string> CPubField::GetVals( const CSeqdesc & in_out_desc)
{
    vector<string> vals;

    if (in_out_desc.Which() == CSeqdesc::e_Pub) {
        vals = GetVals(in_out_desc.GetPub());
    }

    return vals;
}


string CPubField::GetLabel() const
{
    return GetLabelForType(m_FieldType);
}


void CPubField::SetConstraint(const string& field_name, CConstRef<CStringConstraint> string_constraint)
{
    // constraints only apply for authors, since there could be more than one
    m_ConstraintFieldType = GetTypeForLabel(field_name);
    if (m_ConstraintFieldType == CPubFieldType::ePubFieldType_Unknown || !string_constraint) {
        string_constraint.Reset(NULL);
    } else {
        m_StringConstraint = new CStringConstraint(" ");
        m_StringConstraint->Assign(*string_constraint);
    }
}


bool CPubField::AllowMultipleValues()
{
    if (IsAuthorField(m_FieldType)) {
        return true;
    } else {
        return false;
    }
}


string CPubField::NormalizePubFieldName(string orig_label)
{
    return CPubFieldType::NormalizePubFieldName(orig_label);
}


CPubFieldType::EPubFieldType CPubField::GetTypeForLabel(string label)
{
    return CPubFieldType::GetTypeForLabel(label);
}


string CPubField::GetLabelForType(CPubFieldType::EPubFieldType field_type)
{
    return CPubFieldType::GetLabelForType(field_type);
}


bool CPubField::IsAffilField(CPubFieldType::EPubFieldType field_type)
{
    return CPubFieldType::IsAffilField(field_type);
}


bool CPubField::IsAuthorField(CPubFieldType::EPubFieldType field_type)
{
    return CPubFieldType::IsAuthorField(field_type);
}


bool CPubField::IsImprintField(CPubFieldType::EPubFieldType field_type)
{
    if (field_type >= CPubFieldType::ePubFieldType_Volume && field_type <= CPubFieldType::ePubFieldType_Pages) {
        return true;
    } else {
        return false;
    }
}


string CPubField::GetAuthorField(const CAuthor& auth, CPubFieldType::EPubFieldType field_type)
{
    string rval = "";
    if (!auth.IsSetName()) {
        return rval;
    }
    if (field_type == CPubFieldType::ePubFieldType_AuthorConsortium) {
        if (auth.GetName().IsConsortium()) {
            return auth.GetName().GetConsortium();
        } else {
            return rval;
        }
    } else if (!auth.GetName().IsName()) {
        return rval;
    }
    const CName_std& std = auth.GetName().GetName();
    
    switch (field_type) {
        case CPubFieldType::ePubFieldType_AuthorFirstName:
            if (std.IsSetFirst()) {
                rval = std.GetFirst();
            }
            break;
        case CPubFieldType::ePubFieldType_AuthorMiddleInitial:
            if (std.IsSetInitials()) {
                rval = std.GetInitials();
                rval = macro::CMacroFunction_AuthorFix::s_InsertInitialPeriods(rval);
                if (std.IsSetFirst()) {
                    string first_init = macro::CMacroFunction_AuthorFix::s_GetFirstNameInitials(auth.GetName().GetName().GetFirst());
                    if (NStr::StartsWith(rval, first_init)) {
                        rval = rval.substr(first_init.length());
                    }
                }
            }
            break;
        case CPubFieldType::ePubFieldType_AuthorLastName:
            if (std.IsSetLast()) {
                rval = std.GetLast();
            }
            break;
        case CPubFieldType::ePubFieldType_AuthorSuffix:
            if (std.IsSetSuffix()) {
                rval = std.GetSuffix();
            }
            break;
        case CPubFieldType::ePubFieldType_AuthorConsortium:
            break;
        case CPubFieldType::ePubFieldType_Authors:
            break;
        default:
            break;
    }
    return rval;
}


bool CPubField::SetAuthorField(CAuthor& auth, CPubFieldType::EPubFieldType field_type, string val, EExistingText existing_text)
{
    bool rval = false;
    switch (field_type) {
        case CPubFieldType::ePubFieldType_AuthorFirstName:
            {{
                string orig_first_init = "";
                if (auth.IsSetName() && auth.GetName().IsName() 
                    && auth.GetName().GetName().IsSetFirst()) {
                    orig_first_init = macro::CMacroFunction_AuthorFix::s_GetFirstNameInitials(auth.GetName().GetName().GetFirst());
                }
                string orig_first = "";
                if (auth.IsSetName() && auth.GetName().IsName() && auth.GetName().GetName().IsSetFirst()) {
                    orig_first = auth.GetName().GetName().GetFirst();
                }
                if (AddValueToString(orig_first, val, existing_text)) {
                    auth.SetName().SetName().SetFirst(val);
                    string new_first_init = macro::CMacroFunction_AuthorFix::s_GetFirstNameInitials(val);
                    // Fix initials
                    string initials = "";
                    if (auth.GetName().GetName().IsSetInitials()) {
                        initials = auth.GetName().GetName().GetInitials();
                        initials = macro::CMacroFunction_AuthorFix::s_InsertInitialPeriods(initials);
                    }
                    if (NStr::StartsWith(initials, orig_first_init)) {
                        initials = initials.substr(orig_first_init.length());
                    } 
                    initials = new_first_init + initials;
                    if (!NStr::EndsWith(initials, ".")) {
                        initials = initials + ".";
                    }
                    auth.SetName().SetName().SetInitials(initials);
                    rval = true;
                }
            }}
            break;
        case CPubFieldType::ePubFieldType_AuthorMiddleInitial:
            {{
                string first_initials = "";
                string orig_initials = "";
                if (auth.IsSetName() && auth.GetName().IsName()) {
                    if (auth.GetName().GetName().IsSetFirst()) {
                        first_initials = macro::CMacroFunction_AuthorFix::s_GetFirstNameInitials(auth.GetName().GetName().GetFirst());
                    }
                    if (auth.GetName().GetName().IsSetInitials()) {
                        orig_initials = auth.GetName().GetName().GetInitials();
                        orig_initials = macro::CMacroFunction_AuthorFix::s_InsertInitialPeriods(orig_initials);
                        if (NStr::StartsWith(orig_initials, first_initials)) {
                            orig_initials = orig_initials.substr(first_initials.length());
                        }
                    }
                }
                if (AddValueToString(orig_initials, val, existing_text)) {
                    first_initials.append(orig_initials);
                    first_initials = macro::CMacroFunction_AuthorFix::s_InsertInitialPeriods(first_initials);
                    auth.SetName().SetName().SetInitials(orig_initials);
                    rval = true;
                }
            }}
            break;
        case CPubFieldType::ePubFieldType_AuthorLastName:
            {{
                string orig_val = "";
                if (auth.IsSetName() && auth.GetName().IsName()
                    && auth.GetName().GetName().IsSetLast()) {
                    orig_val = auth.GetName().GetName().GetLast();
                }
                if (AddValueToString(orig_val, val, existing_text)) {
                    auth.SetName().SetName().SetLast(orig_val);
                    rval = true;
                }
            }}
            break;
        case CPubFieldType::ePubFieldType_AuthorSuffix:
            {{
                string orig_val = "";
                if (auth.IsSetName() && auth.GetName().IsName()
                    && auth.GetName().GetName().IsSetSuffix()) {
                    orig_val = auth.GetName().GetName().GetSuffix();
                }
                if (AddValueToString(orig_val, val, existing_text)) {
                    auth.SetName().SetName().SetSuffix(orig_val);
                    rval = true;
                }
            }}
            break;
        case CPubFieldType::ePubFieldType_AuthorConsortium:
            {{
                string orig_val = "";
                if (auth.IsSetName() && auth.GetName().IsConsortium()) {
                    orig_val = auth.GetName().GetConsortium();
                }
                if (AddValueToString(orig_val, val, existing_text)) {
                    auth.SetName().SetConsortium(orig_val);
                    rval = true;
                }
            }}
            break;  
        default:
            break;
    }
    return rval;
}


bool CPubField::DoesAuthorMatchConstraint(const CAuthor& auth, CPubFieldType::EPubFieldType constraint_field, CRef<CStringConstraint> string_constraint)
{
    if (!IsAuthorField(constraint_field) || !string_constraint) {
        return true;
    }
    string val = GetAuthorField(auth, constraint_field);
    return string_constraint->DoesTextMatch(val);
}


bool CPubField::x_SetAffilField(CPubdesc &pdesc, CPubFieldType::EPubFieldType field_type, string value, EExistingText existing_text)
{
    bool rval = false;
    CConstRef<CAuth_list> orig_auth = GetAuthList(pdesc);
    CRef<CAuth_list> new_auth(new CAuth_list());
    if (orig_auth) {
        new_auth->Assign(*orig_auth);
    }
    if (x_SetAffilField(*new_auth, field_type, value, existing_text)) {
        rval = true;
        SetAuthList(pdesc, *new_auth);        
    }
    return rval;
}


bool CPubField::x_SetAffilField(CAuth_list &auth_list, CPubFieldType::EPubFieldType field_type, string value, EExistingText existing_text)
{
    bool rval = false;
    string orig_val = x_GetAffilField(auth_list, field_type);
    if (AddValueToString(orig_val, value, existing_text)) {
        rval = true;
        switch (field_type) {
            case CPubFieldType::ePubFieldType_AffilAffil:
                auth_list.SetAffil().SetStd().SetAffil(value);
                break;
            case CPubFieldType::ePubFieldType_AffilDept:
                auth_list.SetAffil().SetStd().SetDiv(value);
                break;
            case CPubFieldType::ePubFieldType_AffilStreet:
                auth_list.SetAffil().SetStd().SetStreet(value);
                break;
            case CPubFieldType::ePubFieldType_AffilCity:
                auth_list.SetAffil().SetStd().SetCity(value);
                break;
            case CPubFieldType::ePubFieldType_AffilState:
                auth_list.SetAffil().SetStd().SetSub(value);
                break;
            case CPubFieldType::ePubFieldType_AffilCountry:
                auth_list.SetAffil().SetStd().SetCountry(value);
                break;
            case CPubFieldType::ePubFieldType_AffilEmail:
                auth_list.SetAffil().SetStd().SetEmail(value);
                break;
            default:
                rval = false;
                break;
        }      
    }
    return rval;
}


string CPubField::x_GetAffilField(const CPubdesc &pdesc, CPubFieldType::EPubFieldType field_type)
{
    string rval = "";
    CConstRef<CAuth_list> orig_auth = GetAuthList(pdesc);

    if (orig_auth && orig_auth->IsSetAffil()) {
        rval = x_GetAffilField(*orig_auth, field_type);
    }
    return rval;
}


string CPubField::x_GetAffilField(const CPub &pub, CPubFieldType::EPubFieldType field_type)
{
    string rval = "";
    CConstRef<CAuth_list> orig_auth = GetAuthList(pub);

    if (orig_auth && orig_auth->IsSetAffil()) {
        rval = x_GetAffilField(*orig_auth, field_type);
    }
    return rval;
}


string CPubField::x_GetAffilField(const CCit_sub &sub, CPubFieldType::EPubFieldType field_type)
{
    string rval = "";
    if (sub.IsSetAuthors()) {
        rval = x_GetAffilField(sub.GetAuthors(), field_type);
    }

    return rval;
}


string CPubField::x_GetAffilField(const CAuth_list& auth_list, CPubFieldType::EPubFieldType field_type)
{
    string rval = "";
    if (auth_list.GetAffil().IsStr()) {
        if (field_type == CPubFieldType::ePubFieldType_AffilAffil) {
            return auth_list.GetAffil().GetStr();
        } else {
            return rval;
        }
    } else if (!auth_list.GetAffil().IsStd()) {
        return rval;
    }
    const CAffil::TStd& std = auth_list.GetAffil().GetStd();
    switch (field_type) {
        case CPubFieldType::ePubFieldType_AffilAffil:
           if (std.IsSetAffil()) {
               rval = std.GetAffil();
            }
            break;
        case CPubFieldType::ePubFieldType_AffilDept:
            if (std.IsSetDiv()) {
                rval = std.GetDiv();
            }
            break;
        case CPubFieldType::ePubFieldType_AffilStreet:
            if (std.IsSetStreet()) {
                rval = std.GetStreet();
            }
            break;
        case CPubFieldType::ePubFieldType_AffilCity:
            if (std.IsSetCity()) {
                rval = std.GetCity();
            }
            break;
        case CPubFieldType::ePubFieldType_AffilState:
            if (std.IsSetSub()) {
                rval = std.GetSub();
            }
            break;
        case CPubFieldType::ePubFieldType_AffilCountry:
            if (std.IsSetCountry()) {
                rval = std.GetCountry();
            }
            break;
        case CPubFieldType::ePubFieldType_AffilEmail:
            if (std.IsSetEmail()) {
                rval = std.GetEmail();
            }
            break;
        default:
            break;
    }
    return rval;
}


vector<CConstRef<CObject> > CPubField::GetObjects(CBioseq_Handle bsh)
{
    vector<CConstRef<CObject> > objects;

    // add pub descriptors
    CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_Pub);
    while (desc_ci) {
        CConstRef<CObject> object;
        object.Reset(&(*desc_ci));
        objects.push_back(object);
        ++desc_ci;
    }

    // also add pub features
    CFeat_CI feat_ci(bsh, CSeqFeatData::e_Pub);
    while (feat_ci) {
        CConstRef<CObject> object;
        object.Reset(&(feat_ci->GetOriginalFeature()));
        objects.push_back(object);
        ++feat_ci;
    }


    return objects;
}


vector<CRef<CApplyObject> > CPubField::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objects;

    // add pub descriptors
    CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_Pub);
    while (desc_ci) {
        CRef<CApplyObject> obj(new CApplyObject(bsh, *desc_ci));
        objects.push_back(obj);
        ++desc_ci;
    }

    // also add pub features
    CFeat_CI feat_ci(bsh, CSeqFeatData::e_Pub);
    while (feat_ci) {
        CRef<CApplyObject> obj(new CApplyObject(bsh, feat_ci->GetOriginalFeature()));
        objects.push_back(obj);
        ++feat_ci;
    }


    return objects;
}



vector<CConstRef<CObject> > CPubField::GetObjects(CSeq_entry_Handle seh, const string& constraint_field, CRef<CStringConstraint> string_constraint)
{
    vector<CConstRef<CObject> > objs;
    CRef<CScope> scope(&seh.GetScope());

    CBioseq_CI bi (seh, objects::CSeq_inst::eMol_na);
    while (bi) {
        if (NStr::EqualNocase(constraint_field, kFieldTypeSeqId)) {
            if (edit::CSeqIdGuesser::DoesSeqMatchConstraint(*bi, string_constraint)) {
                vector<CConstRef<CObject> > these_objs = GetObjects(*bi);
                objs.insert(objs.end(), these_objs.begin(), these_objs.end());
            }
        } else {
            vector<CConstRef<CObject> > these_objs = GetObjects(*bi);
            ITERATE (vector<CConstRef<CObject> >, it, these_objs) {
                if (DoesObjectMatchFieldConstraint (**it, constraint_field, string_constraint, scope)) {
                    objs.push_back (*it);
                }
            }
        }
        ++bi;
    }

    return objs;
}


bool CPubField::IsEmpty(const CObject& object) const
{
    return false;
}


void CPubField::ClearVal(CObject& object)
{
    CSeqdesc* seqdesc = dynamic_cast<CSeqdesc*>(&object);
    CSeq_feat* seqfeat = dynamic_cast<CSeq_feat*>(&object);
    CCit_sub* citsub = dynamic_cast<CCit_sub*>(&object);
    CPubdesc* pubdesc = dynamic_cast<CPubdesc*>(&object);
    CSubmit_block* block = dynamic_cast<CSubmit_block*>(&object);

    if (seqdesc && seqdesc->IsPub()) {
        ClearVal(seqdesc->SetPub());
    } else if (seqfeat && seqfeat->IsSetData() && seqfeat->GetData().IsPub()) {
        ClearVal(seqfeat->SetData().SetPub());
    } else if (pubdesc) {
        ClearVal(*pubdesc);
    } else if (citsub) {
        ClearVal(*citsub);
    } else if (block && block->IsSetCit()) {
        ClearVal(block->SetCit());
    }
}


vector<CConstRef<CObject> > CPubField::GetRelatedObjects(const CObject& object, CRef<CScope> scope)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&object);

   if (obj_feat) {
       if (obj_feat->IsSetData() && obj_feat->GetData().IsPub()) {
            CConstRef<CObject> obj(obj_feat);
            related.push_back(obj);
        } else {
            // find closest related Pubs
            CBioseq_Handle bsh = scope->GetBioseqHandle(obj_feat->GetLocation());
            related = GetObjects(bsh);
        }
    } else if (obj_desc) {
        if (obj_desc->IsPub()) {
            CConstRef<CObject> obj(obj_desc);
            related.push_back(obj);
        } else {
            CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(scope, *obj_desc);
            related = GetObjects(seh, GetLabelForType(m_ConstraintFieldType), m_StringConstraint);
        }
    }

    return related;
}


vector<CConstRef<CObject> > CPubField::GetRelatedObjects(const CApplyObject& object)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&(object.GetObject()));
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&(object.GetObject()));

   if (obj_feat) {
       if (obj_feat->IsSetData() && obj_feat->GetData().IsPub()) {
            CConstRef<CObject> obj(obj_feat);
            related.push_back(obj);
        } else {
            // find closest related Pubs
            related = GetObjects(object.GetSEH(), GetLabelForType(m_ConstraintFieldType), m_StringConstraint);
        }
    } else if (obj_desc) {
        if (obj_desc->IsPub()) {
            CConstRef<CObject> obj(obj_desc);
            related.push_back(obj);
        } else {
            related = GetObjects(object.GetSEH(), GetLabelForType(m_ConstraintFieldType), m_StringConstraint);
        }
    }

    return related;
}


vector<string> CPubField::GetFieldNames()
{
    return CPubFieldType::GetFieldNames();
}


string CPubField::GetLabelForStatus(CPubFieldType::EPubFieldStatus status)
{
    return CPubFieldType::GetLabelForStatus(status);
}


CPubFieldType::EPubFieldStatus CPubField::GetStatusFromString(const string& str)
{
    return CPubFieldType::GetStatusFromString(str);   
}


string CPubField::GetStatus(const CObject& obj)
{
    string status = "";
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&obj);
    const CSeqdesc* desc = dynamic_cast<const CSeqdesc*>(&obj);
    const CSubmit_block* submit = dynamic_cast<const CSubmit_block*>(&obj);
    if (feat && feat->IsSetData() && feat->GetData().IsPub()) {
        status = GetStatus(feat->GetData().GetPub());
    } else if (desc && desc->IsPub()) {
        status = GetStatus(desc->GetPub());
    } else if (submit && submit->IsSetCit()) {
	status = GetLabelForStatus(CPubFieldType::ePubFieldStatus_Submit);
    }
    return status;
}


string CPubField::GetStatus (const CPubdesc& pdesc)
{
    string rval = "";
    if (pdesc.IsSetPub()) {
        ITERATE(CPubdesc::TPub::Tdata, it, pdesc.GetPub().Get()) {
            rval = GetStatus(**it);
            if (!NStr::IsBlank(rval)) {
                break;
            }
        }
    }
    return rval;
}


string CPubField::GetStatus (const CPub& pub)
{
    CPubFieldType::EPubFieldStatus status = GetStatusEnum(pub);
    if (status != CPubFieldType::ePubFieldStatus_Any) {
        return GetLabelForStatus(status);
    } else {
        return "";
    }
}


CPubFieldType::EPubFieldStatus CPubField::GetStatusEnum(const CPub& pub)
{
    CPubFieldType::EPubFieldStatus status = CPubFieldType::ePubFieldStatus_Any;
    switch (pub.Which()) {
        case CPub::e_Gen:
            if (pub.GetGen().IsSetCit() && NStr::EqualNocase(pub.GetGen().GetCit(), "unpublished")) {
                status = CPubFieldType::ePubFieldStatus_Unpublished;
            } else {
                status = CPubFieldType::ePubFieldStatus_Published;
            }
            break;
        case CPub::e_Sub:
            status = CPubFieldType::ePubFieldStatus_Submit;
            break;
        case CPub::e_Patent:
            status = CPubFieldType::ePubFieldStatus_Published;
            break;
        default:
            {{
                    status = CPubFieldType::ePubFieldStatus_Unpublished;
                CConstRef<CImprint> imp = GetImprint(pub);
                if (imp) {
                    if (imp->IsSetPrepub()) {
                        if (imp->GetPrepub() == CImprint::ePrepub_in_press) {
                            status = CPubFieldType::ePubFieldStatus_InPress;
                        } else {
                            status = CPubFieldType::ePubFieldStatus_Unpublished;
                        }
                    } else {
                        status = CPubFieldType::ePubFieldStatus_Published;
                    }
                }
            }}
            break;
    
    }

    return status;
}


bool CPubField::x_SetStatus (CPubdesc& pdesc, const string& val)
{
    CPubFieldType::EPubFieldStatus status = GetStatusFromString(val);
    if (status == CPubFieldType::ePubFieldStatus_Any || status == CPubFieldType::ePubFieldStatus_Submit) {
        return false;
    } else {
        return x_SetStatus (pdesc, status);
    }
}


bool CPubField::x_SetStatus(CPubdesc& pdesc, CPubFieldType::EPubFieldStatus status)
{
    bool rval = false;
    if (status == CPubFieldType::ePubFieldStatus_Any || status == CPubFieldType::ePubFieldStatus_Submit) {
        return false;
    }
    NON_CONST_ITERATE(CPubdesc::TPub::Tdata, it, pdesc.SetPub().Set()) {
        rval |= x_SetStatus(**it, status);
    }
    return rval;
}


bool CPubField::x_SetStatus(CPub& pub, CPubFieldType::EPubFieldStatus status)
{
    if (status == CPubFieldType::ePubFieldStatus_Any || status == CPubFieldType::ePubFieldStatus_Submit || pub.IsSub()) {
        return false;
    }
    CPubFieldType::EPubFieldStatus curr_status = GetStatusEnum(pub);
    if (status == curr_status) {
        return false;
    }

    bool rval = false;
    CConstRef<CImprint> old_imp = GetImprint (pub);
    if (old_imp) {
        CRef<CImprint> new_imp (new CImprint());
        new_imp->Assign(*old_imp);
        switch (status) {
        case CPubFieldType::ePubFieldStatus_Unpublished:
                new_imp->SetPrepub(CImprint::ePrepub_other);
                rval = true;
                break;
            case CPubFieldType::ePubFieldStatus_Published:
                new_imp->ResetPrepub();
                rval = true;
                break;
            case CPubFieldType::ePubFieldStatus_InPress:
                new_imp->SetPrepub(CImprint::ePrepub_in_press);
                rval = true;
                break;
            default:
                break;
        }
        if (rval) {
            SetImprint(pub, *new_imp);
        }
    } else if (pub.IsGen()) {
        if (status == CPubFieldType::ePubFieldStatus_Unpublished) {
            pub.SetGen().SetCit("unpublished");
            rval = true;
        }
        else if (status == CPubFieldType::ePubFieldStatus_Published) {
            pub.SetGen().ResetCit();
            rval = true;
        }
    }
    return rval;
}


vector<string> CPubField::GetChoicesForField(CPubFieldType::EPubFieldType field_type, bool& allow_other)
{
    return CPubFieldType::GetChoicesForField(field_type, allow_other);
}


END_NCBI_SCOPE
