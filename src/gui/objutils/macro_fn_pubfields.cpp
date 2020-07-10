/*  $Id: macro_fn_pubfields.cpp 44987 2020-05-04 15:47:41Z asztalos $
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
 */

#include <ncbi_pch.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Cit_book.hpp>
#include <objects/biblio/Cit_let.hpp>
#include <objects/biblio/Cit_proc.hpp>
#include <objects/biblio/Cit_pat.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/biblio/biblio_macros.hpp>
#include <objects/mla/Title_msg.hpp>
#include <objects/mla/Title_msg_list.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/pub_macros.hpp>
#include <objects/pubmed/Pubmed_entry.hpp>
#include <objects/medline/Medline_entry.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objtools/writers/write_util.hpp>
#include <objtools/edit/publication_edit.hpp>

#include <gui/objutils/gui_eutils_client.hpp>
#include <gui/objutils/doi_lookup.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_field_resolve.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);


/// class CMacroFunction_PubFields
/// PUB_TITLE(), PUB_ISSUE(), PUB_AFFIL(subfield) - returns a list of CObjectInfo (or string) that contain publication title 
/// subfield is optional, used for affiliation, author or date
///


const char* CMacroFunction_PubFields::sm_PubTitle = "PUB_TITLE";
const char* CMacroFunction_PubFields::sm_PubAffil = "PUB_AFFIL";
const char* CMacroFunction_PubFields::sm_PubAuthors = "PUB_AUTHORS";
const char* CMacroFunction_PubFields::sm_PubCit = "PUB_CIT";
const char* CMacroFunction_PubFields::sm_PubDate = "PUB_DATE";
const char* CMacroFunction_PubFields::sm_PubIssue = "PUB_ISSUE";
const char* CMacroFunction_PubFields::sm_PubJournal = "PUB_JOURNAL";
const char* CMacroFunction_PubFields::sm_PubPages = "PUB_PAGES";
const char* CMacroFunction_PubFields::sm_PubPMID = "PUB_PMID";
const char* CMacroFunction_PubFields::sm_PubClass = "PUB_CLASS";
const char* CMacroFunction_PubFields::sm_PubSerialNumber = "PUB_SERIAL_NUMBER";
const char* CMacroFunction_PubFields::sm_PubVolume = "PUB_VOLUME";
   
CMacroFunction_PubFields::CMacroFunction_PubFields(EScopeEnum func_scope, EPublication_field field)
    : IEditMacroFunction(func_scope), m_FieldType(field), m_ResField(kEmptyStr)
{
}

void CMacroFunction_PubFields::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());


    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    if (const_pubdesc) {
        if (!GetFieldsByName(&res_oi, oi, "pub..") || res_oi.size() != 1) {
            return;
        }
    }
    else if (const_block) {
        CSubmit_block* submit_block = CTypeConverter<CSubmit_block>::SafeCast(oi.GetObjectPtr());
        CObjectInfo parent;
        CObjectInfo block_oi(submit_block, submit_block->GetThisTypeInfo());
        res_oi.push_back(CMQueryNodeValue::SResolvedField(parent, block_oi));
    }
    
    if (res_oi.empty()) {
        return;
    }

    m_ResField = (m_Args.size() == 1) ? m_Args[0]->GetString() : kEmptyStr;

    CMQueryNodeValue::TObs objs;
    if (const_pubdesc) {
        // res_oi should be a container of pointers, each of them pointing to a Pub field
        CObjectInfoEI elem = res_oi.front().field.BeginElements();
        while (elem.Valid()) {
            CObjectInfo pub(elem.GetElement().GetPointedObject());  // the Pub field
            if (m_FieldType == ePublication_field_pub_class) {
                // on-the-fly variable, derived from a combination of the status and the type of pub
                string pubclass = x_GetPubClassFromPub(pub);
                CRef<CMQueryNodeValue> new_node(new CMQueryNodeValue);
                new_node->SetString(pubclass);
                m_Result->SetRef(new_node);
                return;
            }
            else {
                x_GetPubFieldObjectFromPub(pub, objs);
            }
            ++elem;
        }
    }
    else if (const_block) {
        if (m_FieldType == ePublication_field_pub_class) {
            string pubclass = ENUM_METHOD_NAME(EPub_type)()->FindName(ePub_type_submitter_block, true);
            CRef<CMQueryNodeValue> new_node(new CMQueryNodeValue);
            new_node->SetString(pubclass);
            m_Result->SetRef(new_node);
            return;
        }
        else {
            x_GetPubFieldObjectFromSubmitBlock(res_oi.front().field, objs);
        }
    }

    if (objs.empty())
        return;
    if (m_Nested == eNotNested) { // return a standard type value
        m_Result->AssignFromObjectInfo(objs.front().field); // for now, return only one value
    }
    else {
        m_Result->SetObjects(objs);
    }
}

void CMacroFunction_PubFields::s_CopyResolvedObjs(const CMQueryNodeValue::TObs& objs_from, CMQueryNodeValue::TObs& objs_to)
{
    ITERATE(CMQueryNodeValue::TObs, it, objs_from) {
        objs_to.push_back(*it);
    }
}

void CMacroFunction_PubFields::x_GetObjectsForPubField_Cit(const CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs)
{
    if (m_FieldType != ePublication_field_cit)
        return;

    if (pub.Which() == CPub::e_Gen) {
        CObjectInfoMI mem = pub_var.FindClassMember("cit");
        if (mem.IsSet()) {
            objs.push_back(CMQueryNodeValue::SResolvedField(pub_var, mem.GetMember()));
        }
    } 
}

void CMacroFunction_PubFields::x_GetObjectsForPubField_Title(const CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs)
{
    if (m_FieldType != ePublication_field_title)
        return;

    switch (pub.Which()) {
    case (CPub::e_Gen): 
    case (CPub::e_Patent):{
        CObjectInfoMI mem = pub_var.FindClassMember("title");
        if (mem.IsSet()) {
            objs.push_back(CMQueryNodeValue::SResolvedField(pub_var, mem.GetMember()));
        }
        break;
    }
    case (CPub::e_Sub): {
        CObjectInfoMI mem = pub_var.FindClassMember("descr");
        if (mem.IsSet()) {
            objs.push_back(CMQueryNodeValue::SResolvedField(pub_var, mem.GetMember()));
        }
        break;
    }
    case (CPub::e_Article): 
    case (CPub::e_Book): 
    case (CPub::e_Man): {
        CObjectInfo book_oi = pub_var;
        if (pub.IsMan()) {
            CObjectInfoMI mem = pub_var.FindClassMember("cit");
            if (mem.IsSet()) {
                book_oi = mem.GetMember();
                if (book_oi.GetTypeFamily() == eTypeFamilyPointer) {
                    book_oi = book_oi.GetPointedObject();
                }
            }
        }
        
        CObjectInfoMI mem = book_oi.FindClassMember("title");
        if (mem.IsSet() && mem.GetMember().GetTypeFamily() == eTypeFamilyPointer) {
            s_GetObjectsFromTitle(mem.GetMember().GetPointedObject(), objs);
        } else {
            // not set
            //NcbiCout << "Member not set or member is not pointer type" << NcbiEndl;
        }
        break;
    }
    default:
        break;
    } 
}

void CMacroFunction_PubFields::x_GetObjectsForPubField_SerialNumber(const CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs)
{
    if (m_FieldType != ePublication_field_serial_number)
        return;

    switch (pub.Which()) {
    case (CPub::e_Gen): {
        CObjectInfoMI mem = pub_var.FindClassMember("serial-number");
        if (mem.IsSet()) {
            objs.push_back(CMQueryNodeValue::SResolvedField(pub_var, mem.GetMember()));
        }
        break;
    }
    default:
        break;
    }
}

void CMacroFunction_PubFields::x_GetObjectsForPubField_Journal(const CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs)
{
    if (m_FieldType != ePublication_field_journal)
        return;

    switch (pub.Which()) {
    case (CPub::e_Gen): {
        CObjectInfoMI mem = pub_var.FindClassMember("journal"); 
        if (mem.IsSet() && mem.GetMember().GetTypeFamily() == eTypeFamilyPointer) {
            s_GetObjectsFromTitle(mem.GetMember().GetPointedObject(), objs);
        } else {
            //NcbiCout << "Member not set or member is not pointer type" << NcbiEndl;
        }
        break;
    }
    case (CPub::e_Article):
    case (CPub::e_Journal): {
        CObjectInfo jour_oi = pub_var;
        if (pub.IsArticle() && pub.GetArticle().IsSetFrom() && pub.GetArticle().GetFrom().IsJournal()) {
            const CCit_jour& const_journal = pub.GetArticle().GetFrom().GetJournal();
            CCit_jour& journal = const_cast<CCit_jour&> (const_journal);
            CObjectInfo oi(&journal, journal.GetTypeInfo());
            jour_oi = oi;
        }

        CObjectInfoMI mem = jour_oi.FindClassMember("title");
        if (mem.IsSet() && mem.GetMember().GetTypeFamily() == eTypeFamilyPointer) {
            s_GetObjectsFromTitle(mem.GetMember().GetPointedObject(), objs);
        } else {
            // not set
            //NcbiCout << "Member not set or member is not pointer type" << NcbiEndl;
        }
        break;
    }
    default:
        break;
    }
}

static CObjectInfo s_GetRelevantObjectInfoForPub(const CPub& pub, const CObjectInfo& pub_var)
{
    CObjectInfo oi = pub_var;
    if (pub.IsArticle() && pub.GetArticle().IsSetFrom()) {
        const CCit_art& article = pub.GetArticle();

        if (article.GetFrom().IsJournal()) {
            const CCit_jour& const_journal = article.GetFrom().GetJournal();
            CCit_jour& journal = const_cast<CCit_jour&> (const_journal);
            CObjectInfo jour_oi(&journal, journal.GetTypeInfo());
            oi = jour_oi;
        }
        else if (article.GetFrom().IsBook()) {
            const CCit_book& const_book = article.GetFrom().GetBook();
            CCit_book& book = const_cast<CCit_book&> (const_book);
            CObjectInfo book_oi(&book, book.GetTypeInfo());
            oi = book_oi;
        }
        else if (article.GetFrom().IsProc()) {
            const CCit_book& const_book = article.GetFrom().GetProc().GetBook();
            CCit_book& book = const_cast<CCit_book&> (const_book);
            CObjectInfo book_oi(&book, book.GetTypeInfo());
            oi = book_oi;
        }
    }
    else if (pub.IsMan()) {
        CObjectInfoMI mem = pub_var.FindClassMember("cit");
        if (mem.IsSet()) {
            oi = mem.GetMember();
        }
    }
    return oi;
}

void CMacroFunction_PubFields::x_GetObjectsForPubField_Date(const CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs)
{
    if (m_FieldType != ePublication_field_date)
        return;

    switch (pub.Which()) {
    case (CPub::e_Gen): 
    case (CPub::e_Sub): {
        CObjectInfoMI mem = pub_var.FindClassMember("date");
        if (mem.Valid() && mem.GetMember().GetTypeFamily() == eTypeFamilyPointer) {
            s_GetObjectsFromDate(mem.GetMember().GetPointedObject(), m_ResField, objs);
        }
        break;
    }
    case (CPub::e_Article):
    case (CPub::e_Book):
    case (CPub::e_Man):
    case (CPub::e_Journal): {
        CObjectInfo oi = s_GetRelevantObjectInfoForPub(pub, pub_var);

        CMQueryNodeValue::TObs res;
        if (GetFieldsByName(&res, oi, "imp.date") && res.size() == 1) {
            CObjectInfo date = res.front().field;
            if (date.GetTypeFamily() == eTypeFamilyPointer) {
                s_GetObjectsFromDate(date.GetPointedObject(), m_ResField, objs);
            }
        }
        break;
    }
    default:
        break;
    }
}

void CMacroFunction_PubFields::x_GetObjectsForPubField_VolIssuePage(const CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs)
{
    string field_name = kEmptyStr;
    switch (m_FieldType) {
    case ePublication_field_volume: 
        field_name.assign("volume");
        break;
    case ePublication_field_issue:
        field_name.assign("issue");
        break;
    case ePublication_field_pages:
        field_name.assign("pages");
        break;
    default:
        return;
    }

    if (NStr::IsBlank(field_name))
        return;

    switch (pub.Which()) {
    case (CPub::e_Gen): {
        CObjectInfoMI mem = pub_var.FindClassMember(field_name);
        if (mem.IsSet()) {
            objs.push_back(CMQueryNodeValue::SResolvedField(pub_var, mem.GetMember()));
        }
        break;
    }
    case (CPub::e_Article):
    case (CPub::e_Book):
    case (CPub::e_Man):
    case (CPub::e_Journal): {
        CObjectInfo oi = s_GetRelevantObjectInfoForPub(pub, pub_var);
        if (oi.GetTypeFamily() == eTypeFamilyPointer) {
            oi = oi.GetPointedObject();
        }

        CObjectInfoMI mem = oi.FindClassMember("imp");
        s_GetObjectsFromImprint(mem.GetMember(), field_name, objs);
        break;
    }
    default:
        break;
    }
}

void CMacroFunction_PubFields::x_GetObjectsForPubField_AffilField(const CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs)
{
    if (m_FieldType != ePublication_field_affiliation)
        return;

    switch (pub.Which()) {
    case (CPub::e_Gen): 
    case (CPub::e_Sub):
    case (CPub::e_Article): 
    case (CPub::e_Book): 
    case (CPub::e_Patent):
    case (CPub::e_Man): {
        CObjectInfo oi = pub_var;
        if (pub.IsMan()) {
            CObjectInfoMI mem = pub_var.FindClassMember("cit");
            if (mem.IsSet()) {
                oi = mem.GetMember();
            }
        }

        CMQueryNodeValue::TObs res;
        if (GetFieldsByName(&res, oi, "authors.affil") && res.size() == 1) {
            CObjectInfo affil = res.front().field;
            if (affil.GetTypeFamily() == eTypeFamilyPointer) {
                CObjectInfo affil_oi = affil.GetPointedObject();
                s_GetObjectsFromAffil(affil_oi, m_ResField, objs); 
            }
        }	
        break;
    }
    default:
        break;
    }

}

void CMacroFunction_PubFields::x_GetObjectsForPubField_Authors(const CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs)
{
    if (m_FieldType != ePublication_field_authors)
        return;

    switch (pub.Which()) {
    case (CPub::e_Gen): 
    case (CPub::e_Sub): 
    case (CPub::e_Article): 
    case (CPub::e_Book): 
    case (CPub::e_Patent):
    case (CPub::e_Man): {
        CObjectInfo oi = pub_var;
        if (pub.IsMan()) {
            CObjectInfoMI mem = pub_var.FindClassMember("cit");
            if (mem.IsSet()) {
                oi = mem.GetMember();
            }
        }

        CMQueryNodeValue::TObs res;
        if (GetFieldsByName(&res, oi, "authors.names") && res.size() == 1) {
            CObjectInfo affil = res.front().field;
            if (affil.GetTypeFamily() == eTypeFamilyPointer) {
                s_GetObjectsFromAuthListNames(affil.GetPointedObject(), m_ResField, objs); 
            }
        }	
        break;
    }
    default:
        break;
    }

}

void CMacroFunction_PubFields::x_GetObjectsForPubField_PMID(CPub& pub, const CObjectInfo& pub_var, CMQueryNodeValue::TObs& objs)
{
    if (m_FieldType != ePublication_field_pmid) 
        return;
    if (pub.IsPmid() && pub_var.GetTypeFamily() == eTypeFamilyPointer) {
        CObjectInfo parent_oi(&pub, pub.GetTypeInfo());
        objs.push_back(CMQueryNodeValue::SResolvedField(parent_oi, pub_var.GetPointedObject()));
    }
}

void CMacroFunction_PubFields::s_Init_PubClassMap(CMacroFunction_PubFields::MapPubClass& map)
{
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Gen, ePub_type_unpublished), 0), "unpublished") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Sub, ePub_type_in_press), 0), "in-press submission") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Sub, ePub_type_published), 0), "submission") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Article, ePub_type_in_press), 1), "in-press journal") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Article, ePub_type_published), 1), "journal") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Article, ePub_type_in_press), 2), "in-press book chapter") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Article, ePub_type_published), 2), "book chapter") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Article, ePub_type_in_press), 3), "in-press proceedings chapter") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Article, ePub_type_published), 3), "proceedings chapter") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Book, ePub_type_in_press), 0), "in-press book") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Book, ePub_type_published), 0), "book") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Man, ePub_type_in_press), 0), "in-press thesis") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Man, ePub_type_published), 0), "thesis") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Proc, ePub_type_in_press), 0), "in-press proceedings") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Proc, ePub_type_published), 0), "proceedings") );
    map.insert( MapPubClass::value_type( TPubChoiceType_Int( TPubChoice_Type( CPub::e_Patent, ePub_type_any), 0), "patent") );
}

void CMacroFunction_PubFields::s_SetStatus(const CImprint& imp, const CPub& pub, EPub_type& status)
{
    if (!imp.IsSetPrepub()) {
        status = ePub_type_published;
    } else {
        if (imp.GetPrepub() == CImprint::ePrepub_in_press) {
            status = ePub_type_in_press;
        } else if (imp.GetPrepub() == CImprint::ePrepub_submitted && pub.IsSub()) {
            status = ePub_type_submitter_block;
        } else {
            status = ePub_type_unpublished;
        }
    }
}
 
EPub_type CMacroFunction_PubFields::s_GetPubMLStatus (const CPub& pub)
{
    EPub_type status = ePub_type_any;

    switch (pub.Which()) {
    case (CPub::e_Gen):
        if (pub.GetGen().IsSetCit() && NStr::EqualNocase(pub.GetGen().GetCit(), "unpublished")) {
            status = ePub_type_unpublished;
        } else {
            status = ePub_type_published;
        }
        break;
    case (CPub::e_Sub):
        status = ePub_type_submitter_block;
        break;
    case (CPub::e_Article):
        if (pub.GetArticle().IsSetFrom()) {
            const CCit_art::C_From& from = pub.GetArticle().GetFrom();
            if (from.IsJournal()) {
                s_SetStatus(from.GetJournal().GetImp(), pub, status);
            } else if (from.IsBook()) {
                s_SetStatus(from.GetBook().GetImp(), pub, status);
            } else if (from.IsProc()) {
                s_SetStatus(from.GetProc().GetBook().GetImp(), pub, status);
            }
        }
        break;
    case (CPub::e_Journal):
        s_SetStatus(pub.GetJournal().GetImp(), pub, status);
        break;
    case (CPub::e_Book):
        s_SetStatus(pub.GetBook().GetImp(), pub, status);
        break;
    case (CPub::e_Man):
        if (pub.GetMan().IsSetCit()) {
            s_SetStatus(pub.GetMan().GetCit().GetImp(), pub, status);
        }
        break;
    case (CPub::e_Patent):
      status = ePub_type_published;
      break;
    default :
      break;
    }

    return status;
}

string CMacroFunction_PubFields::x_GetPubClassFromPub(const CObjectInfo& pub_var)
{
    const CPub* pub = CTypeConverter<CPub>::SafeCast(pub_var.GetObjectPtr());
    if (!pub) {
        return kEmptyStr;
    }
    // obtain first the status and then the class
    EPub_type status = s_GetPubMLStatus(*pub);
    unsigned int article_from = CCit_art::C_From::e_not_set;

    if (pub->IsArticle() && pub->GetArticle().IsSetFrom()) {
        article_from = pub->GetArticle().GetFrom().Which();
    }

    MapPubClass pubclass_quals;
    s_Init_PubClassMap(pubclass_quals);
    for ( MapPubClass::iterator it = pubclass_quals.begin(); it != pubclass_quals.end(); ++it ) {
        if (pub->Which() == it->first.first.first &&
            (status == it->first.first.second || status == ePub_type_any || it->first.first.second == ePub_type_any) &&
            (article_from == CCit_art::C_From::e_not_set || article_from == it->first.second || it->first.second == CCit_art::C_From::e_not_set)) {
            return string(it->second);
        }
    }
    return kEmptyStr;
}

void CMacroFunction_PubFields::s_GetObjectsFromDate(const CObjectInfo& date, const string& field_name, CMQueryNodeValue::TObs& objs)
{
    // fields from the class CDate
    if (!date || NStr::IsBlank(field_name))
        return;

    if (date.GetCurrentChoiceVariantIndex() == 1) { // str
        CObjectInfo date_oi = date.GetCurrentChoiceVariant().GetVariant();
        objs.push_back(CMQueryNodeValue::SResolvedField(date, date_oi));
    } else if (date.GetCurrentChoiceVariantIndex() == 2) {  // std
        CObjectInfo date_std = date.GetCurrentChoiceVariant().GetVariant();
        CMQueryNodeValue::TObs res;
        if (GetFieldsByName(&res, date_std, field_name)) {
            s_CopyResolvedObjs(res, objs);
        }
    }
}

void CMacroFunction_PubFields::s_GetObjectsFromAffil(const CObjectInfo& affil, const string& field_name, CMQueryNodeValue::TObs& objs)
{
    // resolving field from class CAffil
    if (!affil)
        return;

    if (NStr::IsBlank(field_name)) {
        objs.push_back(CMQueryNodeValue::SResolvedField(CObjectInfo(), affil));
        return;
    }
    
    if (affil.GetCurrentChoiceVariantIndex() == 1) { // str
        CObjectInfo affil_str = affil.GetCurrentChoiceVariant().GetVariant();
        objs.push_back(CMQueryNodeValue::SResolvedField(affil, affil_str));
    } else if (affil.GetCurrentChoiceVariantIndex() == 2) {  // std
        CObjectInfo affil_std = affil.GetCurrentChoiceVariant().GetVariant();
        CMQueryNodeValue::TObs res;
        if (GetFieldsByName(&res, affil_std, field_name)) {
            s_CopyResolvedObjs(res, objs);
        }
    }
}

void CMacroFunction_PubFields::s_GetObjectsFromPersonID(const CObjectInfo& names, const string& field_name, CMQueryNodeValue::TObs& objs)
{
    CObjectInfo var = names.GetCurrentChoiceVariant().GetVariant();
    string var_name = names.GetCurrentChoiceVariant().GetVariantInfo()->GetId().GetName();
    switch (names.GetCurrentChoiceVariantIndex()) {
    case (1): // dbtag
        objs.push_back(CMQueryNodeValue::SResolvedField(names, var)); // returns a Dbtag object
        break;
    case (2): { // name
        if (field_name.empty()) {
            objs.push_back(CMQueryNodeValue::SResolvedField(names, var));
        }
        else {
            CMQueryNodeValue::TObs res;
            if (GetFieldsByName(&res, var, field_name)) {
                s_CopyResolvedObjs(res, objs);
            }
        }
        break;
    }
    case (5): // consortium
        if (NStr::EndsWith(field_name, var_name)) {
            objs.push_back(CMQueryNodeValue::SResolvedField(names, var));
        }
        break;
    default:
        break;
    }
}

void CMacroFunction_PubFields::s_GetObjectsFromAuthListNames(const CObjectInfo& names, const string& field_name, CMQueryNodeValue::TObs& objs)
{
    // resolving field from Auth_list.names
    if (names.GetCurrentChoiceVariantIndex() == 1) { // std - container of pointers to Authors
        CObjectInfo names_std = names.GetCurrentChoiceVariant().GetVariant();
        if (names_std.GetTypeFamily() == eTypeFamilyContainer) {
            CObjectInfoEI elem = names_std.BeginElements();
            while (elem.Valid()) {
                CObjectInfo elem_oi(elem.GetElement().GetPointedObject()); // an Author object
                CObjectInfoMI mem = elem_oi.FindClassMember("name");
                if (mem.IsSet()) {
                    s_GetObjectsFromPersonID(mem.GetMember().GetPointedObject(), field_name, objs);
                }
                ++elem;
            }
        }
    } else if (names.GetCurrentChoiceVariantIndex() == 3) { // str - container of strings
        CObjectInfo names_str = names.GetCurrentChoiceVariant().GetVariant();
        if (names_str.GetTypeFamily() == eTypeFamilyContainer) {
            CObjectInfoEI elem = names_str.BeginElements();
            while (elem.Valid()) {
                objs.push_back(CMQueryNodeValue::SResolvedField(names_str, elem.GetElement()));
                ++elem;
            }
        }
    }
}

void CMacroFunction_PubFields::s_GetObjectsFromTitle(const CObjectInfo& title, CMQueryNodeValue::TObs& objs)
{
    if (!title)
        return;

    // Title is a class with its first member being a container of pointers of choice
    CObjectInfoMI mem = title.BeginMembers();
    while (mem.Valid() && mem.IsSet()) {
        CObjectInfo mem_oi = mem.GetMember(); // should be a container
        if (mem_oi.GetTypeFamily() == eTypeFamilyContainer) {
            CObjectInfoEI elem = mem_oi.BeginElements();
            while (elem.Valid()) {
                CObjectInfo elem_oi(elem.GetElement().GetPointedObject());
                CObjectInfoCV var = elem_oi.GetCurrentChoiceVariant();
                CObjectInfo pub_var = var.GetVariant();  // should be a primitive
                objs.push_back(CMQueryNodeValue::SResolvedField(elem_oi, pub_var));
        
                ++elem;
            }
        }
        ++mem;
    }
}


void CMacroFunction_PubFields::s_GetObjectsFromImprint(const CObjectInfo& imp, const string& field_name, CMQueryNodeValue::TObs& objs)
{
    if (NStr::IsBlank(field_name))
        return;

    CMQueryNodeValue::TObs res;
    if (GetFieldsByName(&res, imp, field_name)) {
        s_CopyResolvedObjs(res, objs);
    }
}

void CMacroFunction_PubFields::x_GetPubFieldObjectFromPub(const CObjectInfo& pub_oi, CMQueryNodeValue::TObs& objs)
{
    CPub* pub = CTypeConverter<CPub>::SafeCast(pub_oi.GetObjectPtr());
    if (!pub)
        return;

    CObjectInfo pub_var = pub_oi.GetCurrentChoiceVariant().GetVariant();

    switch (m_FieldType) {
    case (ePublication_field_cit): 
        x_GetObjectsForPubField_Cit(*pub, pub_var, objs);
        break;
    case ePublication_field_title: 
        x_GetObjectsForPubField_Title(*pub, pub_var, objs);
        break;
    case ePublication_field_serial_number:
        x_GetObjectsForPubField_SerialNumber(*pub, pub_var, objs);
        break;
    case ePublication_field_journal:
        x_GetObjectsForPubField_Journal(*pub, pub_var, objs);
        break;
    case ePublication_field_volume:
    case ePublication_field_issue:
    case ePublication_field_pages:
        x_GetObjectsForPubField_VolIssuePage(*pub, pub_var, objs);
    case ePublication_field_date:
        x_GetObjectsForPubField_Date(*pub, pub_var, objs);
        break;
    case ePublication_field_affiliation: 
        x_GetObjectsForPubField_AffilField(*pub, pub_var, objs);
        break;
    case ePublication_field_pmid:
        x_GetObjectsForPubField_PMID(*pub, pub_var, objs);
        break;
    case ePublication_field_authors:
        x_GetObjectsForPubField_Authors(*pub, pub_var, objs);
        break;
    default:
        break;
    }
}

void CMacroFunction_PubFields::x_GetPubFieldObjectFromSubmitBlock(const CObjectInfo& block_oi, CMQueryNodeValue::TObs& objs)
{
    CObjectInfo sub_oi = block_oi.FindClassMember("cit").GetMember();
    if (sub_oi.GetTypeFamily() == eTypeFamilyPointer) {
        sub_oi = sub_oi.GetPointedObject();
    }

    switch (m_FieldType) {
    case ePublication_field_title:
    {
        CObjectInfoMI mem = sub_oi.FindClassMember("descr");
        if (mem.IsSet()) {
            objs.push_back(CMQueryNodeValue::SResolvedField(sub_oi, mem.GetMember()));
        }
        break;
    }
    case (ePublication_field_cit):
    case ePublication_field_serial_number:
    case ePublication_field_journal:
    case ePublication_field_pmid:
        // not relevant
        break;
    case ePublication_field_volume:
        m_ResField = "volume";
    case ePublication_field_issue:
        m_ResField = "issue";
    case ePublication_field_pages:
    {
        m_ResField = "pages";

        CObjectInfoMI mem = sub_oi.FindClassMember("imp");
        if (mem.IsSet()) {
            s_GetObjectsFromImprint(mem.GetMember(), m_ResField, objs);
        }
        break;
    }
    case ePublication_field_date:
    {
        CObjectInfoMI mem = sub_oi.FindClassMember("date");
        if (mem.Valid() && mem.GetMember().GetTypeFamily() == eTypeFamilyPointer) {
            s_GetObjectsFromDate(mem.GetMember().GetPointedObject(), m_ResField, objs);
        }
        break;
    }
    case ePublication_field_affiliation:
    {
        CMQueryNodeValue::TObs res;
        if (GetFieldsByName(&res, sub_oi, "authors.affil") && res.size() == 1) {
            CObjectInfo affil = res.front().field;
            if (affil.GetTypeFamily() == eTypeFamilyPointer) {
                CObjectInfo affil_oi = affil.GetPointedObject();
                s_GetObjectsFromAffil(affil_oi, m_ResField, objs);
            }
        }
        break;
    }
    case ePublication_field_authors:
    {
        CMQueryNodeValue::TObs res;
        if (GetFieldsByName(&res, sub_oi, "authors.names") && res.size() == 1) {
            CObjectInfo affil = res.front().field;
            if (affil.GetTypeFamily() == eTypeFamilyPointer) {
                s_GetObjectsFromAuthListNames(affil.GetPointedObject(), m_ResField, objs);
            }
        }
    }
    default:
        break;
    }
}

bool CMacroFunction_PubFields::x_ValidArguments() const
{
    return (m_Args.empty() || (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eString));
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ApplyPublication
/// SetPub_Sub(author_field_name, author_field_value)
/// Apply new publication to the sequence 
///
const char* CMacroFunction_ApplyPublication::sm_FunctionName = "SetPub_Sub";
void CMacroFunction_ApplyPublication::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    if (!bseq || !scope)
        return;

    const string& author_field = m_Args[0]->GetString();
    CMQueryNodeValue& new_value = m_Args[1].GetNCObject();

    CRef<CPub> new_pub(new CPub());
    new_pub->Select(m_PubType);

    CObjectInfo oi;
    switch (m_PubType) {
    case CPub::e_Sub: {
        CCit_sub& cit_sub = new_pub->SetSub();
        oi = ObjectInfo(cit_sub);
        break;
    }
    default:
        // for now it only handles cit-sub types
        break;
    }

    CMQueryNodeValue::TObs res_oi;
    if (!SetFieldsByName(&res_oi, oi, author_field)) {
        return;
    }

    NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, res_oi) {
        if (SetSimpleTypeValue(it->field, new_value)) {
            m_QualsChangedCount++;
        }
    }

    CRef<CSeqdesc> new_desc(new CSeqdesc());
    new_desc->SetPub().SetPub().Set().push_back(new_pub);

    CBioseq_Handle bsh = scope->GetBioseqHandle(*bseq);
    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    CBioseq_set_Handle bssh = bsh.GetParentBioseq_set();
    if (bssh && bssh.IsSetClass() && bssh.GetClass() == CBioseq_set::eClass_nuc_prot) {
        seh = bssh.GetParentEntry();
    }
    CRef<CCmdCreateDesc> create_cmd(new CCmdCreateDesc(seh, *new_desc));

    CRef<CCmdComposite> cmd(new CCmdComposite("Create new publication"));
    cmd->AddCommand(*create_cmd);
    m_DataIter->RunCommand(cmd, m_CmdComposite);

    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr() << ": added new publication";
    x_LogFunction(log);
}

bool CMacroFunction_ApplyPublication::x_ValidArguments() const
{
    if (m_Args.size() != 2)
        return false;

    return (m_Args[0]->GetDataType() == CMQueryNodeValue::eString)
        && (m_Args[1]->GetDataType() == CMQueryNodeValue::eString);
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_AuthorFix
/// MoveMiddleToFirstName(); - moves middle name to first name and fixes the initials
/// RemoveAuthorSuffix(); - removes author suffix
/// TruncateMiddleInitials(); - truncates middle name initials
/// ReverseAuthorNames() - swaps the first and the last name

DEFINE_MACRO_FUNCNAME(CMacroFunction_MoveMiddleName, "MoveMiddleToFirstName");
DEFINE_MACRO_FUNCNAME(CMacroFunction_StripSuffix, "RemoveAuthorSuffix");
DEFINE_MACRO_FUNCNAME(CMacroFunction_TruncateMI, "TruncateMiddleInitials");
DEFINE_MACRO_FUNCNAME(CMacroFunction_ReverseAuthNames, "ReverseAuthorNames");

typedef SStaticPair<const char*, CMacroFunction_AuthorFix::EActionType> TAuthorFixPair;
static const TAuthorFixPair s_AuthorFixString[] = {
        { "",                          CMacroFunction_AuthorFix::eAuthorFixNotSet },
        { "Move middle name to first", CMacroFunction_AuthorFix::eMoveMiddleName },
        { "Remove author suffix",      CMacroFunction_AuthorFix::eStripSuffix },
        { "Reverse author names",      CMacroFunction_AuthorFix::eReverseNames},
        { "Truncate middle initials",  CMacroFunction_AuthorFix::eTruncateMI },
};

typedef CStaticArrayMap<string, CMacroFunction_AuthorFix::EActionType> TAuthorFixTypeMap;
DEFINE_STATIC_ARRAY_MAP(TAuthorFixTypeMap, sm_AuthorFixMap, s_AuthorFixString);

const string& CMacroFunction_AuthorFix::GetDescription(EActionType fix_type)
{
    TAuthorFixTypeMap::const_iterator iter = sm_AuthorFixMap.begin();
    for (; iter != sm_AuthorFixMap.end(); ++iter){
        if (iter->second == fix_type){
            return iter->first;
        }
    }
    return kEmptyStr;
}

CMacroFunction_AuthorFix::EActionType CMacroFunction_AuthorFix::GetActionType(const string& descr)
{
    TAuthorFixTypeMap::const_iterator iter = sm_AuthorFixMap.find(descr);
    if (iter != sm_AuthorFixMap.end()){
        return iter->second;
    }
    return CMacroFunction_AuthorFix::eAuthorFixNotSet;
}

void CMacroFunction_AuthorFix::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());

    int count = 0;
    if (const_pubdesc) {
        CObjectInfo oi = m_DataIter->GetEditedObject();
        CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());

        EDIT_EACH_PUB_ON_PUBEQUIV(it, pubdesc->SetPub()) {
            CPub& pub = **it;
            CAuth_list* authors = 0;
            switch (pub.Which()) {
            case CPub::e_Gen:
                if (pub.GetGen().IsSetAuthors()) {
                    authors = &(pub.SetGen().SetAuthors());
                }
                break;
            case CPub::e_Sub:
                authors = &(pub.SetSub().SetAuthors());
                break;
            case CPub::e_Article:
                if (pub.GetArticle().IsSetAuthors()) {
                    authors = &(pub.SetArticle().SetAuthors());
                }
                break;
            case CPub::e_Book:
                authors = &(pub.SetBook().SetAuthors());
                break;
            case CPub::e_Proc:
                authors = &(pub.SetProc().SetBook().SetAuthors());
                break;
            case CPub::e_Man:
                authors = &(pub.SetMan().SetCit().SetAuthors());
                break;
            case CPub::e_Patent:
                authors = &(pub.SetPatent().SetAuthors());
                break;
            default:
                break;
            }
            if (!authors)
                continue;

            count += x_MakeAuthorChanges(*authors);
        }
    }
    else if (const_block) {
        CObjectInfo oi = m_DataIter->GetEditedObject();
        CSubmit_block* submit_block = CTypeConverter<CSubmit_block>::SafeCast(oi.GetObjectPtr());

        if (submit_block->IsSetCit() && submit_block->GetCit().IsSetAuthors()) {
            CAuth_list& authors = submit_block->SetCit().SetAuthors();
            count = x_MakeAuthorChanges(authors);
        }

        // don't apply to Contact-info
    }

    if (count > 0) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": " << GetDescription(m_FixType) << " for " << count << " names";
        x_LogFunction(log);
    }
}

int CMacroFunction_AuthorFix::x_MakeAuthorChanges(CAuth_list& auth_list)
{
    if (!auth_list.IsSetNames())
        return 0;
    CAuth_list::TNames& names = auth_list.SetNames();
    if (!names.IsStd())
        return 0;

    int count = 0;
    NON_CONST_ITERATE(list<CRef<CAuthor> >, it, names.SetStd()) {
        CAuthor& auth = **it;
        CPerson_id& person = auth.SetName();
        if (!person.IsName())
            continue;
        CName_std& std_name = person.SetName();
        
        switch (m_FixType) {
        case (eMoveMiddleName) :
            if (s_MoveMiddleToFirst(std_name)) {
                count++;
            }
            break;
        case (eStripSuffix) :
            if (std_name.IsSetSuffix()) {
                std_name.ResetSuffix();
                count++;
            }
            break;
        case (eReverseNames):
            if (s_ReverseAuthorNames(std_name)) {
                count++;
            }
            break;
        case (eTruncateMI) :
            if (s_TruncateMiddleInitials(std_name)) {
                count++;
            }
            break;
        default:
            break;
        }
    }
    return count;
}

bool CMacroFunction_AuthorFix::s_MoveMiddleToFirst(CName_std& name)
{
    if (!name.IsSetInitials())
        return false;

    // return if the initials field contains exactly the first name initials
    if (name.IsSetFirst() && NStr::EqualCase(name.GetInitials(), s_GetFirstNameInitials(name.GetFirst()))) {
        return false;
    }

    string first_name = (name.IsSetFirst()) ? name.GetFirst() : kEmptyStr;
    string initials = name.GetInitials();
    string first_name_initial = s_GetFirstNameInitialsWithoutStops(first_name);

    vector<string> names;
    NStr::Split(initials, ".", names);
    vector<string>::iterator it = names.begin();
    while (it != names.end()) {
        if ((!first_name_initial.empty() && NStr::EqualCase(*it, first_name_initial)) ||
            it->length() <= 2) {
            it = names.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto& it : names) {
        if (!first_name.empty()) {
            first_name += " ";
        }
        first_name += it;
    }

    if (first_name.empty() || 
        (name.IsSetFirst() && NStr::EqualCase(first_name, name.GetFirst()))) {
            return false;
    }
    
    name.SetFirst(first_name);
    name.SetInitials(s_GetFirstNameInitials(name.GetFirst()));
    return true;
}

bool CMacroFunction_AuthorFix::s_FixInitials(CName_std& name)
{
    if (!name.IsSetInitials())
        return false;

    string first_init;
    if (name.IsSetFirst()) {
        first_init = s_GetFirstNameInitials(name.GetFirst());
    }
    
    string new_middle_init;
    string original_init = name.GetInitials();
    // when the initials already contain the first name initials
    if (!first_init.empty() && NStr::StartsWith(original_init, first_init, NStr::eCase)) {
        string middle_init = original_init.substr(first_init.length());
        if (!middle_init.empty()) {
            new_middle_init = edit::GetFirstInitial(middle_init, false);
        }
    }
    else {
        // assume the initials field contains middle name/initials only
        new_middle_init = s_GetFirstNameInitials(original_init);
    }

    if (!new_middle_init.empty()) {
        name.SetInitials(first_init + new_middle_init);
        return true;
    }

    return false;
}

bool CMacroFunction_AuthorFix::s_TruncateMiddleInitials(CName_std& name)
{
    if (!name.IsSetInitials())
        return false;

    string first_init;
    if (name.IsSetFirst()) {
        first_init = s_GetFirstNameInitials(name.GetFirst());
    }

    string original_init = name.GetInitials();
    string middle_init;
    if (!first_init.empty() && NStr::StartsWith(original_init, first_init, NStr::eCase)) {
        middle_init = original_init.substr(first_init.length());
        if (!middle_init.empty()) {
            middle_init = edit::GetFirstInitial(middle_init, false);
        }
    }
    else {
        // assume the initials field contains middle name/initials only
        middle_init = s_GetFirstNameInitials(original_init);
    }

    string new_initials = first_init + middle_init;
    if (!NStr::EqualCase(new_initials, name.GetInitials())) {
        name.SetInitials(new_initials);
        return true;
    }

    return false;
}

string CMacroFunction_AuthorFix::s_GetFirstNameInitials(const string& first_name)
{
    string inits = s_GetFirstNameInitialsWithoutStops(first_name);
    inits = s_InsertInitialPeriods(inits);
    return inits;
}

string CMacroFunction_AuthorFix::s_InsertInitialPeriods(const string& orig)
{
    char  buf[2];

    string inits = "";
    buf[1] = 0;

    string::const_iterator p = orig.begin();
    while (p != orig.end()) {
        string::const_iterator q = p;
        q++;
        buf[0] = *p;
        inits.append(buf);
        if (isalpha(*p) && (q == orig.end() || (*q != '.' && !islower(*q)))) {
            inits.append(".");
        }
        p++;
    }

    return inits;
}

string CMacroFunction_AuthorFix::s_GetFirstNameInitialsWithoutStops(const string& first_name)
{
    char  buf[2];

    string inits = "";
    buf[1] = 0;

    string::const_iterator p = first_name.begin();
    while (p != first_name.end()) {
        // skip leading punct
        while (p != first_name.end() && (*p <= ' ' || *p == '-')) {
            p++;
        }
        if (p != first_name.end() && isalpha(*p)) {
            buf[0] = *p;
            inits.append(buf);
            p++;
        }
        // skip rest of name
        while (p != first_name.end() && *p > ' ' && *p != '-') {
            p++;
        }
        if (p != first_name.end() && *p == '-') {
            buf[0] = *p;
            inits.append(buf);
            p++;
        }
    }
    return inits;
}

bool CMacroFunction_AuthorFix::s_ReverseAuthorNames(CName_std& name)
{
    bool modified = false;
    if (name.IsSetLast() && name.IsSetFirst() && !name.GetLast().empty() && !name.GetFirst().empty()) {
        string last = name.GetLast();
        string first = name.GetFirst();
        name.SetLast(first);
        name.SetFirst(last);
        if (name.IsSetInitials()) {
            string initials = name.GetInitials();
            string first_init = s_GetFirstNameInitials(first);
            string middle_init = initials;
            if (!first_init.empty() && NStr::StartsWith(initials, first_init, NStr::eNocase))
                middle_init = middle_init.substr(first_init.length());

            string new_init = s_GetFirstNameInitials(last) + middle_init;
            name.SetInitials(new_init);
        }
        s_FixInitials(name);
        modified = true;
    }
    return modified;
}

void CMacroFunction_AuthorFix::s_BuildName(const string& firstname, const string& mid_initials,
                            const string& lastname, const string& suffix, CName_std& name)
{
    if (NStr::IsBlank(firstname)) {
        name.ResetFirst();
    }
    else {
        name.SetFirst(firstname);
    }

    if (NStr::IsBlank(lastname)) {
        name.ResetLast();
    }
    else {
        name.SetLast(lastname);
    }

    // need to do parsing for middle initial
    if (NStr::IsBlank(firstname)) {
        if (!NStr::IsBlank(mid_initials)) {
            name.SetFirst(mid_initials);
        }
        name.ResetInitials();
    }
    else {
        string inits = macro::CMacroFunction_AuthorFix::s_GetFirstNameInitials(name.GetFirst());
        if (!NStr::IsBlank(mid_initials)) {
            inits.append(mid_initials);
        }
        // don't add another period if one is already there
        if (!NStr::EndsWith(inits, ".")) {
            inits.append(".");
        }
        name.SetInitials(inits);
    }

    CMacroFunction_AuthorFix::s_FixInitials(name);

    // suffix
    if (NStr::IsBlank(suffix)) {
        name.ResetSuffix();
    }
    else {
        name.SetSuffix(suffix.c_str());
    }
}

bool CMacroFunction_AuthorFix::x_ValidArguments() const
{
    return m_Args.empty();
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ISOJTALookup
/// ISOJTALookup();
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ISOJTALookup, "ISOJTALookup");
void CMacroFunction_ISOJTALookup::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(obj.GetPointer());
    if (!const_pubdesc) {
        return;
    }

    // look for an article
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
    if (!pubdesc) {
        return;
    }

    m_Hits.clear();
    m_TitlePairs.clear();
    if (pubdesc->IsSetPub() && pubdesc->GetPub().IsSet()) {
        for (auto&& it : pubdesc->SetPub().Set()) {
            if (it->IsArticle() 
                && it->GetArticle().IsSetFrom() 
                && it->GetArticle().GetFrom().IsJournal()) {
                x_LookupTitleInJournal(it->SetArticle().SetFrom().SetJournal());
            }
        }
    }

    CNcbiOstrstream log;
    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        log << m_QualsChangedCount << ";ISOJTA lookup, updated: ";
        for (const auto& it : m_TitlePairs) {
            log << "'" << it.first << "' to '" << it.second << "'\n";
        }
    }
    for (const auto& it : m_Hits) {
        if (it.second.empty()) {
            log << "No ISOJTA title was found for '" << it.first << "'\n";
        }
        else {
            log << "Multiple titles were found for '" << it.first << "':\n";
            for (const auto& title_it : it.second) {
                log << title_it << endl;
            }
        }
    }
    x_LogFunction(log);
}

void CMacroFunction_ISOJTALookup::x_LookupTitleInJournal(CCit_jour& journal)
{
    CTitle& title = journal.SetTitle();
    string old_title, new_title;

    for (auto&& title_it : title.Set()) {
        if (title_it->IsJta() || title_it->IsName()) {
            old_title = title.GetTitle();
            if (x_TitleToISO_Jta(*title_it)) {
                new_title = title_it->GetIso_jta();
                m_QualsChangedCount++;
                m_TitlePairs.emplace_back(old_title, new_title);
            }
        }
    }
}

bool CMacroFunction_ISOJTALookup::x_TitleToISO_Jta(CTitle::C_E& title_type)
{
    if (!title_type.IsJta() && !title_type.IsName())
        return false;
    
    string old_title = (title_type.IsJta()) ? title_type.GetJta() : title_type.GetName();
    //shortcuts
    string updated_title = s_GetISOShortcut(old_title);
    if (!updated_title.empty()) {
        title_type.SetIso_jta(updated_title);
        return true;
    }

    CRef<CTitle::C_E> type(new CTitle::C_E);
    type->SetIso_jta(old_title);

    CRef<CTitle> title(new CTitle);
    title->Set().push_back(type);

    // prepare a message for 'title lookup' request
    CRef<CTitle_msg> msg(new CTitle_msg);
    msg->SetType(eTitle_type_iso_jta);
    msg->SetTitle(*title);

    // post 'title lookup' request
    CRef<CTitle_msg_list> msg_list = m_MLAClient.AskGettitle(*msg);
    int num = msg_list->GetNum();
    if (num == 1) {
        const auto& item = msg_list->GetTitles().front();
        const CTitle& new_title = item->GetTitle();
        const auto& new_type = new_title.Get().front();

        if (new_type->Which() == CTitle::C_E::e_Iso_jta) {
            title_type.SetIso_jta(new_type->GetIso_jta());
            return true;
        }
    }
    else if (num > 1) {
        vector<string> hits;
        for (auto& it : msg_list->GetTitles()) {
            const CTitle& new_title = it->GetTitle();
            const auto& new_type = new_title.Get().front();
            if (new_type->Which() == CTitle::C_E::e_Iso_jta) {
                hits.push_back(new_type->GetIso_jta());
            }
        }
        m_Hits.emplace(old_title, hits);
    }
    else if (num == 0) {
        m_Hits.emplace(old_title, vector<string>{});
    }

    return false;
}

typedef SStaticPair<const char*, const char*>  TISOShortcutPairElem;
static const TISOShortcutPairElem k_iso_shortcut_pair_map[] = {
    { "Antimicrobial Agents and Chemotherapy", "Antimicrob. Agents Chemother." },
    { "Cell", "Cell" },
    { "Genes", "Genes Basel" },
    { "IJSEM", "Int. J. Syst. Evol. Microbiol" },
    { "journal of microbiology",  "J Microbiol"},
    { "Journal of Virology", "J. Virol." },
    { "mitochondrial DNA A", "Mitochondrial DNA A DNA Mapp Seq Anal" },
    { "mitochondrial DNA B", "Mitochondrial DNA B Resour" },
    { "Nature",  "Nature" },
    { "Nucleic Acids Research", "Nucleic Acids Res." },
    { "Science", "Science" },
    { "Virology", "Virology" },
};

typedef CStaticArrayMap<const char*, const char*, PNocase_CStr> TISOShortcutMap;
DEFINE_STATIC_ARRAY_MAP(TISOShortcutMap, sc_ISOShortcutMap, k_iso_shortcut_pair_map);

string CMacroFunction_ISOJTALookup::s_GetISOShortcut(const string& old_title)
{
    auto it = sc_ISOShortcutMap.find(old_title.c_str());
    if (it != sc_ISOShortcutMap.end()) {
        return it->second;
    }
    return kEmptyStr;
}

bool CMacroFunction_ISOJTALookup::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_LookupPub
/// PMIDLookup();
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_LookupPub, "PMIDLookup");
void CMacroFunction_LookupPub::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(obj.GetPointer());
    if (!const_pubdesc) {
        return;
    }

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
    if (!pubdesc) {
        return;
    }

    CRef<CMacroBioData_PubdescIter> pubdesc_iter =
        Ref(dynamic_cast<CMacroBioData_PubdescIter*>(m_DataIter.GetPointer()));


    // look for a PMID pub
    if (pubdesc->IsSetPub() && pubdesc->GetPub().IsSet()) {
        for (auto&& it : pubdesc->SetPub().Set()) {
            if (it->IsPmid()) {
                int pmid = it->GetPmid();
                CRef<CPub> looked_up_pub(nullptr);
                CMacroBioData_PubdescIter::TPmidLookupMap& map = pubdesc_iter->SetPmidLookupMap();
                auto it = map.find(pmid);
                if (it != map.end()) {
                    looked_up_pub = it->second;
                }
                else {
                    looked_up_pub = s_GetArticleFromEntrezById(pmid);
                    if (looked_up_pub) {
                        map.emplace(pmid, looked_up_pub);
                    }
                }

                if (looked_up_pub) {
                    pubdesc->Reset();
                    pubdesc->SetPub().Set().push_back(looked_up_pub);
                    CRef<CPub> pmid_pub(new CPub());
                    pmid_pub->SetPmid().Set(pmid);
                    pubdesc->SetPub().Set().push_back(pmid_pub);
                    m_QualsChangedCount++;
                    break;
                }
            }
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_QualsChangedCount << ";pmid lookup";
        x_LogFunction(log);
    }
}

bool CMacroFunction_LookupPub::x_ValidArguments() const
{
    return (m_Args.empty());
}

namespace {

    void FixMedLineList(CAuth_list& auth_list)
    {
        list<CRef<CAuthor>> standard_names;

        ITERATE(CAuth_list::TNames::TMl, it, auth_list.GetNames().GetMl()) {
            if (!NStr::IsBlank(*it)) {
                CRef<CAuthor> new_auth = CAuthor::ConvertMlToStandard(*it, true);
                standard_names.push_back(new_auth);
            }
        }
        auth_list.SetNames().Reset();
        auth_list.SetNames().SetStd().insert(auth_list.SetNames().SetStd().begin(), standard_names.begin(), standard_names.end());
    }

    void ConvertToStandardAuthors(CAuth_list& auth_list)
    {
        if (!auth_list.IsSetNames()) {
            return;
        }

        if (auth_list.GetNames().IsMl()) {
            FixMedLineList(auth_list);
            return;
        }
        else if (auth_list.GetNames().IsStd()) {
            NON_CONST_ITERATE(CAuth_list::TNames::TStd, it, auth_list.SetNames().SetStd()) {
                if ((*it)->GetName().IsMl()) {
                    CRef<CAuthor> new_auth = CAuthor::ConvertMlToStandard(**it, true);
                    (*it)->Assign(*new_auth);
                }
            }
            if (!auth_list.GetNames().GetStd().empty() && auth_list.GetNames().GetStd().front()->IsSetAffil()) {
                CRef<CAffil> affil(new CAffil);
                affil->Assign(auth_list.GetNames().GetStd().front()->GetAffil());
                auth_list.SetAffil(*affil);
            }
        }
    }
}

CRef<CPub> CMacroFunction_LookupPub::s_GetArticleFromEntrezById(int id)
{
    CRef<CPub> pub(nullptr);

    // prepare eFetch request
    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(1);

    vector<int> uids;
    uids.push_back(id);

    CRef<CPubmed_entry> pubmed_entry(new CPubmed_entry());
    CNcbiStrstream asnPubMedEntry;
    try {
        ecli.Fetch("pubmed", uids, asnPubMedEntry, "asn.1");
        asnPubMedEntry >> MSerial_AsnText >> *pubmed_entry;
    }
    catch (const CException& e) {
        LOG_POST(Error << "CMacroFunction_LookupPub::s_GetArticleFromEntrezById(): error fetching ID " << id << ": " << e.GetMsg());
        return pub;
    }
    if (pubmed_entry->IsSetMedent() && pubmed_entry->GetMedent().IsSetCit()) {
        pub.Reset(new CPub());
        pub->SetArticle().Assign(pubmed_entry->GetMedent().GetCit());
        if (pub->GetArticle().IsSetAuthors()) {
            ConvertToStandardAuthors(pub->SetArticle().SetAuthors());
        }
    }

    return pub;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_DOILookup
/// DOILookup(doi_number);
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_DOILookup, "DOILookup")
void CMacroFunction_DOILookup::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(obj.GetPointer());
    if (!const_pubdesc) {
        return;
    }

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
    if (!pubdesc) {
        return;
    }

    const string& new_doi = m_Args[0]->GetString();
    
    // update only unpublished cit-gen references
    bool modify = false;
    for (auto&& it : pubdesc->GetPub().Get()) {
        if (it->IsGen()) {
            const CCit_gen& genref = it->GetGen();
            if (genref.IsSetCit() && NStr::EqualNocase(genref.GetCit(), "unpublished")) {
                modify = true;
                break;
            }
        }
    }
    if (!modify)
        return;

    CRef<CMacroBioData_PubdescIter> pubdesc_iter =
        Ref(dynamic_cast<CMacroBioData_PubdescIter*>(m_DataIter.GetPointer()));

    CRef<CPubdesc> looked_up_pubdesc(nullptr);
    CMacroBioData_PubdescIter::TDOILookupMap& map = pubdesc_iter->SetDOILookupMap();
    auto it = map.find(new_doi);
    if (it != map.end()) {
        looked_up_pubdesc = it->second;
    }
    else {
        pair<CRef<CPubdesc>, string> new_pubdesc_str = CDoiLookup::GetPubFromCrossRef(new_doi);
        looked_up_pubdesc = new_pubdesc_str.first;
        if (looked_up_pubdesc) {
            map.emplace(new_doi, looked_up_pubdesc);
        }
        else {
            CNcbiOstrstream log;
            log << "Problem resolving DOI " << new_doi << " " << new_pubdesc_str.second;
            x_LogFunction(log);
        }
    }
    
    if (looked_up_pubdesc) {
        pubdesc->Reset();
        pubdesc->Assign(*looked_up_pubdesc);
        _ASSERT(pubdesc->Equals(*looked_up_pubdesc));
        m_QualsChangedCount++;
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_QualsChangedCount << ";doi lookup";
        x_LogFunction(log);
    }
}

bool CMacroFunction_DOILookup::x_ValidArguments() const
{
    return (m_Args.size() == 1) && (m_Args[0]->IsString());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SetPubTitle
/// SetPubTitle(title_object, newValue, existingtext_option, delimiter, remove_blank)
/// The last two parameters are optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetPubTitle, "SetPubTitle")
void CMacroFunction_SetPubTitle::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());
    if (!const_pubdesc && !const_block)
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    x_GetObjectsFromRef(res_oi, 0);

    size_t index = 1;
    const string& newValue = m_Args[index]->GetString();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    if (!newValue.empty()) {
        if (res_oi.empty()) {
            //create a new publication title
            if (const_pubdesc) {
                CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
                x_SetTitleInPubdesc(*pubdesc, newValue);
            }
            else if (const_block) {
                CSubmit_block* submit_block = CTypeConverter<CSubmit_block>::SafeCast(oi.GetObjectPtr());
                x_SetTitleInSubmitBlock(*submit_block, newValue);
            }
        }
        else {
            NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, res_oi) {
                CObjectInfo obj = it->field;
                if (obj.GetTypeFamily() == eTypeFamilyPrimitive) {
                    if (obj.GetPrimitiveValueType() == ePrimitiveValueString) {
                        string orig_value = obj.GetPrimitiveValueString();
                        if (edit::AddValueToString(orig_value, newValue, existing_text)) {
                            SetQualStringValue(obj, orig_value);
                        }
                    }
                }
            }
        }

        if (m_QualsChangedCount) {
            m_DataIter->SetModified();
            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr() << ": set " << m_QualsChangedCount << " publication title";
            x_LogFunction(log);
        }
    }
    else if (remove_field) {
        m_QualsChangedCount = CMacroFunction_RemoveQual::s_RemoveFields(m_DataIter, res_oi);
        if (m_QualsChangedCount) {
            m_DataIter->SetModified();
            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr() << ": removed " << m_QualsChangedCount << " publication title";
            x_LogFunction(log);
        }
    }
}

bool CMacroFunction_SetPubTitle::x_ValidArguments() const
{
    auto arg_nr = m_Args.size();
    if (arg_nr > 5 || arg_nr < 3) {
        return false;
    }
    size_t index = 0;
    if (!m_Args[index]->IsRef()) {
        return false;
    }
    NMacroUtil::GetPrimitiveFromRef(m_Args[++index].GetNCObject());
    bool second_ok = m_Args[index]->IsString() || m_Args[index]->IsInt() || m_Args[index]->IsDouble();
    if (!second_ok) return false;

    if (!m_Args[++index]->IsString()) return false;
    if (arg_nr > 3 && (!m_Args[++index]->IsString() && !m_Args[index]->IsBool())) return false;
    if (arg_nr > 4 && !m_Args[++index]->IsBool()) return false;
    return true;
}

void CMacroFunction_SetPubTitle::x_SetTitleInPubdesc(CPubdesc& pubdesc, const string& value)
{
    CRef<CTitle::C_E> title(new CTitle::C_E());
    title->SetName(value);
    for (auto& it : pubdesc.SetPub().Set()) {
        switch (it->Which())
        {
        case CPub::e_Gen:
            it->SetGen().SetTitle(value);
            m_QualsChangedCount++;
            break;
        case CPub::e_Sub:
            it->SetSub().SetDescr(value);
            m_QualsChangedCount++;
            break;
        case CPub::e_Article:
            it->SetArticle().SetTitle().Set().push_back(title);
            m_QualsChangedCount++;
            break;
        case CPub::e_Book:
            it->SetBook().SetTitle().Set().push_back(title);
            m_QualsChangedCount++;
            break;
        case CPub::e_Proc:
            it->SetProc().SetBook().SetTitle().Set().push_back(title);
            m_QualsChangedCount++;
            break;
        case CPub::e_Patent:
            it->SetPatent().SetTitle(value);
            m_QualsChangedCount++;
            break;
        case CPub::e_Man:
            it->SetMan().SetCit().SetTitle().Set().push_back(title);
            m_QualsChangedCount++;
            break;
        case CPub::e_Medline:
        case CPub::e_Journal:
        case CPub::e_Pat_id:
        case CPub::e_Equiv:
        case CPub::e_Pmid:
        default:
            break;
        }
    }
}

void CMacroFunction_SetPubTitle::x_SetTitleInSubmitBlock(CSubmit_block& submit_block, const string& value)
{
    if (submit_block.IsSetCit()) {
        submit_block.SetCit().SetDescr(value);
        m_QualsChangedCount++;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_SetPubAffil
/// SetPubAffil("affil", new_value, existing_text, delimiter, remove_blank)
/// SetPubAffil("city", new_value, existing_text, delimiter, remove_blank)
/// The last two parameters are optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetPubAffil, "SetPubAffil")
void CMacroFunction_SetPubAffil::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());
    if (!const_pubdesc && !const_block)
        return;

    size_t index = 0;
    const string& field = m_Args[index]->GetString();
    const string& newValue = NMacroUtil::GetStringValue(m_Args[++index]);
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    CNcbiOstrstream log;
    if (!newValue.empty()) {
        CObjectInfo oi = m_DataIter->GetEditedObject();
        if (const_pubdesc) {
            CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
            for (auto& it : pubdesc->SetPub().Set()) {
                if (it->IsSetAuthors()) {
                    x_SetAffilField(it->SetAuthors(), field, newValue, existing_text);
                    break;
                }
            }
        }
        else if (const_block) {
            CSubmit_block* submit_block = CTypeConverter<CSubmit_block>::SafeCast(oi.GetObjectPtr());
            if (submit_block->IsSetCit() && submit_block->SetCit().IsSetAuthors()) {
                x_SetAffilField(submit_block->SetCit().SetAuthors(), field, newValue, existing_text);
            }
        }

        if (m_QualsChangedCount) {
            log << m_DataIter->GetBestDescr() << ": set ";
        }
    }
    else if (remove_field) {
        CObjectInfo oi = m_DataIter->GetEditedObject();
        if (const_pubdesc) {
            CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
            for (auto& it : pubdesc->SetPub().Set()) {
                if (it->IsSetAuthors()) {
                    x_RemoveAffilField(it->SetAuthors(), field);
                    break;
                }
            }
        }
        if (m_QualsChangedCount) {
            log << m_DataIter->GetBestDescr() << ": removed ";
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        log << m_QualsChangedCount << " publication ";
        if (field == "affil") {
            log << "institution";
        }
        else if (field == "div") {
            log << "department";
        }
        else if (field == "sub") {
            log << "state";
        }
        else {
            log << field;
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetPubAffil::x_ValidArguments() const
{
    auto arg_nr = m_Args.size();
    if (arg_nr < 3 || arg_nr > 5) {
        return false;
    }

    size_t index = 0;
    if (!m_Args[index]->IsString()) return false;
    NMacroUtil::GetPrimitiveFromRef(m_Args[++index].GetNCObject());
    if (!m_Args[index]->IsString() && !m_Args[index]->IsInt()) {
        return false;
    }
    if (!m_Args[++index]->IsString()) return false;
    if (arg_nr > 3 && (!m_Args[++index]->IsString() && !m_Args[index]->IsBool())) return false;
    if (arg_nr > 4 && !m_Args[++index]->IsBool()) return false;
    return true;
}

void CMacroFunction_SetPubAffil::x_SetAffilField(CAuth_list& auth_list, const string& field, const string& newValue, edit::EExistingText existing_text)
{
    CObjectInfo oi(&auth_list, auth_list.GetThisTypeInfo());
    string field_name = "affil.std." + field;
    CMQueryNodeValue::TObs res_oi;
    if (!SetFieldsByName(&res_oi, oi, field_name)) {
        return;
    }
    _ASSERT(!res_oi.empty());
    string orig_value = res_oi.front().field.GetPrimitiveValueString();
    if (edit::AddValueToString(orig_value, newValue, existing_text)) {
        SetQualStringValue(res_oi.front().field, orig_value);
    }
}

void CMacroFunction_SetPubAffil::x_RemoveAffilField(CAuth_list& auth_list, const string& field)
{
    CObjectInfo oi(&auth_list, auth_list.GetThisTypeInfo());
    string field_name = "affil.std." + field;
    CMQueryNodeValue::TObs res_oi;
    if (!GetFieldsByName(&res_oi, oi, field_name)) {
        return;
    }
    m_QualsChangedCount = CMacroFunction_RemoveQual::s_RemoveFields(m_DataIter, res_oi);
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SetPubAuthor
/// SetPubAuthorName(author_object, field_name, newValue, existingtext_option, delimiter, remove_blank)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetPubAuthor, "SetPubAuthorName")
void CMacroFunction_SetPubAuthor::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());
    if (!const_pubdesc && !const_block)
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    size_t index = 0;
    if (m_Args[index]->AreObjects()) {
        res_oi = m_Args[index]->GetObjects();
    }
    else if (m_Args[index]->IsRef()) {
        x_GetObjectsFromRef(res_oi, index);
    }

    const string& field = m_Args[++index]->GetString();
    const string& newValue = m_Args[++index]->GetString();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    
    if (!newValue.empty()) {
        if (existing_text == edit::eExistingText_add_qual) {
            //create a new publication author and populate the specified field
            if (const_pubdesc) {
                CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
                x_SetAuthorInPubdesc(*pubdesc, field, *m_Args[2], existing_text);
            }
            else if (const_block) {
                CSubmit_block* submit_block = CTypeConverter<CSubmit_block>::SafeCast(oi.GetObjectPtr());
                if (submit_block->IsSetCit()) {
                    x_SetNewAuthor(submit_block->SetCit().SetAuthors(), field, *m_Args[2]);
                }
            }
        }
        else {
            for (auto&& it : res_oi) {
                CObjectInfo obj = it.field;
                if (obj.GetTypeFamily() == eTypeFamilyPrimitive) {
                    if (obj.GetPrimitiveValueType() == ePrimitiveValueString) {
                        string orig_value = obj.GetPrimitiveValueString();
                        if (edit::AddValueToString(orig_value, newValue, existing_text)) {
                            SetQualStringValue(obj, orig_value);
                        }
                    }
                }
                else if (obj.GetTypeFamily() == eTypeFamilyClass) {
                    if (NStr::EqualNocase(obj.GetName(), "Name-std")) {
                        if (field == "consortium") {
                            CObjectInfo parent_oi = it.parent;
                            CPerson_id* person_id = CTypeConverter<CPerson_id>::SafeCast(parent_oi.GetObjectPtr());
                            if (person_id) {
                                person_id->SetConsortium() = newValue;
                                m_QualsChangedCount++;
                            }
                        }
                        else if (field == "first") {
                            CName_std* std_name = CTypeConverter<CName_std>::SafeCast(obj.GetObjectPtr());
                            if (std_name && NMacroUtil::ApplyFirstName(*std_name, newValue, existing_text)) {
                                m_QualsChangedCount++;
                            }
                        }
                        else if (ResolveAndSetSimpleTypeValue(obj, field, *m_Args[2], existing_text)) {
                            m_QualsChangedCount++;
                        }
                    }
                }
            }
        }

        if (m_QualsChangedCount) {
            m_DataIter->SetModified();
            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr() << ": set " << m_QualsChangedCount << " publication author field";
            x_LogFunction(log);
        }
    }
    else if (remove_field) {
        for (auto&& it : res_oi) {
            CObjectInfo obj = it.field;
            if (obj.GetTypeFamily() == eTypeFamilyPrimitive) {
                if (RemoveFieldByName(it)) {
                    m_QualsChangedCount++;
                }
            }
            else if (obj.GetTypeFamily() == eTypeFamilyClass) {
                if (NStr::EqualNocase(obj.GetName(), "Name-std")) {
                    CMQueryNodeValue::TObs sub_objs;
                    GetFieldsByName(&sub_objs, obj, field);
                    m_QualsChangedCount += CMacroFunction_RemoveQual::s_RemoveFields(m_DataIter, sub_objs);
                }
            }
        }
        if (const_pubdesc) {
            CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
            for (auto& it : pubdesc->SetPub().Set()) {
                CAuth_list& auth_list = it->SetAuthors();
                if (auth_list.IsSetNames() && auth_list.GetNames().IsStd()) {
                    auto& authors = auth_list.SetNames().SetStd();
                    CAuth_list_Base::TNames::TStd::iterator auth_it = authors.begin();
                    while (auth_it != authors.end()) {
                        if ((*auth_it)->IsSetName() &&
                            (*auth_it)->GetName().IsConsortium() &&
                            (*auth_it)->GetName().GetConsortium().empty()) {
                            auth_it = authors.erase(auth_it);
                        }
                        else {
                            ++auth_it;
                        }
                    }
                }
            }
        }
        if (m_QualsChangedCount) {
            m_DataIter->SetModified();
            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr() << ": removed " << m_QualsChangedCount << " publication author field";
            x_LogFunction(log);
        }

    }
}

bool CMacroFunction_SetPubAuthor::x_ValidArguments() const
{
    auto arg_nr = m_Args.size();
    if (arg_nr > 6 || arg_nr < 4) {
        return false;
    }

    size_t index = 0;
    if (!m_Args[index]->IsRef() && !m_Args[index]->AreObjects()) {
        return false;
    }
    NMacroUtil::GetPrimitiveFromRef(m_Args[2].GetNCObject());
    index = 4;
    for (size_t i = 1; i < index; ++i) {
        if (!m_Args[i]->IsString()) {
            return false;
        }
    }
    if (arg_nr > 4 && (!m_Args[index]->IsString() && !m_Args[index]->IsBool())) return false;
    if (arg_nr > 5 && !m_Args[++index]->IsBool()) return false;
    return true;
}

void CMacroFunction_SetPubAuthor::x_SetAuthorInPubdesc(CPubdesc& pubdesc, const string& field, const CMQueryNodeValue& value, edit::EExistingText existing_text)
{
    for (auto& it : pubdesc.SetPub().Set()) {
        CAuth_list* auth_list = nullptr;
        if (it->IsSetAuthors()) {
            auth_list = &(it->SetAuthors());
        }
        else {
            try {
                auth_list = &(it->SetAuthors());
            }
            catch (const CException&) { /* can not set the authors */ }
        }
        if (auth_list) {
            x_SetNewAuthor(*auth_list, field, value);
        }
    }
}

void CMacroFunction_SetPubAuthor::x_SetNewAuthor(CAuth_list& auth_list, const string& field, const CMQueryNodeValue& value)
{
    CRef<CAuthor> new_author(new CAuthor);
    CObjectInfo oi(new_author, new_author->GetThisTypeInfo());
    string field_name;
    if (field == "consortium") {
        field_name = "name." + field;
    }
    else {
        field_name = "name.name." + field;
    }
    if (ResolveAndSetSimpleTypeValue(oi, field_name, value, edit::eExistingText_replace_old)) {
        auth_list.SetNames().SetStd().push_back(new_author);
        m_QualsChangedCount++;
    }
}

///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_SetPubAuthorMI
/// SetPubAuthorMI(author_object, newValue, existingtext_option, delimiter, remove_blank)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetPubAuthorMI, "SetPubAuthorMI")
void CMacroFunction_SetPubAuthorMI::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());
    if (!const_pubdesc && !const_block)
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    if (type == CMQueryNodeValue::eObjects) {
        res_oi = m_Args[0]->GetObjects();
    }
    else if (type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(res_oi, 0);
    }

    size_t index = 1;
    const string& newValue = m_Args[index]->GetString();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    if (existing_text == edit::eExistingText_add_qual) {
        return;
    }

    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr();
    
    if (!newValue.empty()) {
        for (auto&& it : res_oi) {
            CObjectInfo obj = it.field;
            if (obj.GetTypeFamily() == eTypeFamilyPrimitive) {
                if (obj.GetPrimitiveValueType() == ePrimitiveValueString) {
                    string orig_value = obj.GetPrimitiveValueString();
                    if (edit::AddValueToString(orig_value, newValue, existing_text)) {
                        SetQualStringValue(obj, orig_value);
                    }
                }
            }
            else if (obj.GetTypeFamily() == eTypeFamilyClass && NStr::EqualNocase(obj.GetName(), "Name-std")) {
                CName_std* std_name = CTypeConverter<CName_std>::SafeCast(obj.GetObjectPtr());
                if (std_name && NMacroUtil::ApplyMiddleInitial(*std_name, newValue, existing_text)) {
                    m_QualsChangedCount++;
                }
            }
        }

        if (m_QualsChangedCount) {
            log << ": set ";
        }
    }
    else if (remove_field) {
        for (auto&& it : res_oi) {
            CObjectInfo obj = it.field;
            if (obj.GetTypeFamily() == eTypeFamilyClass && NStr::EqualNocase(obj.GetName(), "Name-std")) {
                CName_std* std_name = CTypeConverter<CName_std>::SafeCast(obj.GetObjectPtr());
                if (std_name && NMacroUtil::RemoveMiddleInitial(*std_name)) {
                    m_QualsChangedCount++;
                }
            }
        }

        if (m_QualsChangedCount) {
            log << ": removed ";
        }
    }
    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        log << m_QualsChangedCount << " publication author middle initial";
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetPubAuthorMI::x_ValidArguments() const
{
    auto arg_nr = m_Args.size();
    if (arg_nr > 5 || arg_nr < 3) {
        return false;
    }
    size_t index = 0;
    if (!m_Args[index]->IsRef() && !m_Args[index]->AreObjects()) {
        return false;
    }
    NMacroUtil::GetPrimitiveFromRef(m_Args[++index].GetNCObject());
    if (!m_Args[index]->IsString() || !m_Args[++index]->IsString())
        return false;

    if (arg_nr > 3 && (!m_Args[++index]->IsString() && !m_Args[index]->IsBool())) return false;
    if (arg_nr > 4 && !m_Args[++index]->IsBool()) return false;
    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_AddPubAuthors
/// AddAuthor(last_name, first_name, middle initial, suffix, existingtext_option)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_AddPubAuthor, "AddAuthor")
void CMacroFunction_AddPubAuthor::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());
    if (!const_pubdesc && !const_block)
        return;
    
    // do not add author if the last name is empty
    if (m_Args[0]->GetString().empty()) {
        return;
    }
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(m_Args[4]->GetString(), kEmptyStr);

    CObjectInfo oi = m_DataIter->GetEditedObject();
    if (const_pubdesc) {
        CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
        x_AddAuthorInPubdesc(*pubdesc, existing_text);
    }
    else if (const_block) {
        CSubmit_block* submit_block = CTypeConverter<CSubmit_block>::SafeCast(oi.GetObjectPtr());
        if (submit_block->IsSetCit()) {
            x_AddAuthor(submit_block->SetCit().SetAuthors(), existing_text);
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": added " << m_QualsChangedCount << " publication author";
        x_LogFunction(log);
    }
}

bool CMacroFunction_AddPubAuthor::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 5) {
        return false;
    }

    for (auto& it : m_Args) {
        if (it->GetDataType() != CMQueryNodeValue::eString) {
            return false;
        }
    }
    return true;
}

void CMacroFunction_AddPubAuthor::x_AddAuthorInPubdesc(CPubdesc& pubdesc, edit::EExistingText existing_text)
{
    for (auto& it : pubdesc.SetPub().Set()) {
        CAuth_list* auth_list = nullptr;
        if (it->IsSetAuthors()) {
            auth_list = &(it->SetAuthors());
        }
        else {
            try {
                auth_list = &(it->SetAuthors());
            }
            catch (const CException&) { /* can not set the authors */ }
        }
        if (auth_list) 
            x_AddAuthor(*auth_list, existing_text);
    }
}

void CMacroFunction_AddPubAuthor::x_AddAuthor(CAuth_list& auth_list, edit::EExistingText existing_text)
{
    CRef<CAuthor> author(new CAuthor);
    CName_std& std_name = author->SetName().SetName();
    std_name.SetLast(m_Args[0]->GetString());
    if (!m_Args[1]->GetString().empty()) {
        std_name.SetFirst(m_Args[1]->GetString());
    }

    const string middle_init = m_Args[2]->GetString();
    string initials = (std_name.IsSetFirst()) ? CMacroFunction_AuthorFix::s_GetFirstNameInitials(std_name.GetFirst()) : kEmptyStr;
    if (!middle_init.empty()) {
        initials.append(middle_init);
    }
    // don't add another period if one is already there
    if (!NStr::EndsWith(initials, ".")) {
        initials.append(".");
    }
    if (!initials.empty()) {
        std_name.SetInitials(initials);
    }
    if (!m_Args[3]->GetString().empty()) {
        std_name.SetSuffix(m_Args[3]->GetString());
    }

    if (existing_text == edit::eExistingText_replace_old) {
        auth_list.ResetNames();
        auth_list.SetNames().SetStd().push_back(author);
        m_QualsChangedCount++;
    }
    else if (existing_text == edit::eExistingText_append_none) {
        auth_list.SetNames().SetStd().push_back(author);
        m_QualsChangedCount++;
    }
    else if (existing_text == edit::eExistingText_prefix_none) {
        auto& names = auth_list.SetNames().SetStd();
        names.insert(names.begin(), author);
        m_QualsChangedCount++;
    }
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_AddAuthorList
/// AddAuthorList(author_list, remove_blank)
/// The only option for it is to overwrite existing values
/// The last parameter is optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_AddAuthorList, "AddAuthorList")
void CMacroFunction_AddAuthorList::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());
    if (!const_pubdesc && !const_block)
        return;

    size_t index = 0;
    const string& newValue = m_Args[index]->GetString();
    bool remove_field = (++index < m_Args.size()) ? m_Args[index]->GetBool() : false;
    
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr();

    if (!newValue.empty()) {
        if (const_pubdesc) {
            CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
            x_AddAuthorListInPubdesc(*pubdesc);
        }
        else if (const_block) {
            CSubmit_block* submit_block = CTypeConverter<CSubmit_block>::SafeCast(oi.GetObjectPtr());
            if (submit_block->IsSetCit()) {
                m_QualsChangedCount = NMacroUtil::ApplyAuthorNames(submit_block->SetCit().SetAuthors(), m_Args[0]->GetString());
            }
        }

        if (m_QualsChangedCount) {
            log << ": added ";
        }
    }
    else if (remove_field) {
        if (const_pubdesc) {
            CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
            for (auto& it : pubdesc->SetPub().Set()) {
                if (it->IsSetAuthors()) {
                    CAuth_list& auth_list = it->SetAuthors();
                    m_QualsChangedCount += (Int4)auth_list.GetNameCount();
                    auth_list.ResetNames();
                    auth_list.SetNames().SetStr().push_back("?");
                }
            }
        }
        if (m_QualsChangedCount) {
            log << ": removed ";
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        log << m_QualsChangedCount << " author names";
        x_LogFunction(log);
    }
}

bool CMacroFunction_AddAuthorList::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 2)
        return false;

    size_t index = 0;
    NMacroUtil::GetPrimitiveFromRef(m_Args[index].GetNCObject());
    if (!m_Args[index]->IsString())
        return false;
    if (++index < m_Args.size() && !m_Args[index]->IsBool())
        return false;

    return true;
}

void CMacroFunction_AddAuthorList::x_AddAuthorListInPubdesc(CPubdesc& pubdesc)
{
    for (auto& it : pubdesc.SetPub().Set()) {
        CAuth_list* auth_list = nullptr;
        if (it->IsSetAuthors()) {
            auth_list = &(it->SetAuthors());
        }
        else {
            try {
                auth_list = &(it->SetAuthors());
            }
            catch (const CException&) { /* can not set the authors */ }
        }
        if (auth_list) {
            // Note that Suffix is only be parsed correctly when four names are specified
            m_QualsChangedCount = NMacroUtil::ApplyAuthorNames(*auth_list, m_Args[0]->GetString());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SetPubJournal
/// SetPubJournal(newValue, existingtext_option, delimiter, remove_blank)
/// The last two parameters are optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetPubJournal, "SetPubJournal")
void CMacroFunction_SetPubJournal::TheFunction()
{
    // can't set journal in the submit-block
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    if (!const_pubdesc)
        return;

    size_t index = 0;
    const string& newValue = m_Args[index]->GetString();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr();

    if (!newValue.empty()) {
        x_SetJournalInPubdesc(*pubdesc, newValue, existing_text);
        if (m_QualsChangedCount) {
            log << ": set ";
        }
    }
    else if (remove_field) {
        x_RemoveJournalInPubdesc(*pubdesc);
        if (m_QualsChangedCount) {
            log << ": removed ";
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        log << m_QualsChangedCount << " publication journal ";
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetPubJournal::x_ValidArguments() const
{
    auto arg_nr = m_Args.size();
    if (arg_nr < 2 || arg_nr > 4) {
        return false;
    }

    size_t index = 0;
    NMacroUtil::GetPrimitiveFromRef(m_Args[index].GetNCObject());
    if (!m_Args[index]->IsString()) {
        return false;
    }
    if (!m_Args[++index]->IsString()) return false;
    if (arg_nr > 2 && (!m_Args[++index]->IsString() && !m_Args[index]->IsBool())) return false;
    if (arg_nr > 3 && !m_Args[++index]->IsBool()) return false;
    return true;
}

void CMacroFunction_SetPubJournal::x_SetJournalInPubdesc(
    CPubdesc& pubdesc, const string& newValue, edit::EExistingText existing_text)
{
    CTitle* title = nullptr;
    for (auto& it : pubdesc.SetPub().Set()) {
        switch (it->Which()) {
        case CPub::e_Gen:
            title = &it->SetGen().SetJournal();
            break;
        case CPub::e_Article:
            title = &it->SetArticle().SetFrom().SetJournal().SetTitle();
            break;
        case CPub::e_Journal:
            title = &it->SetJournal().SetTitle();
        default:
            break;
        }
    }

    if (title) {
        // if we find a Name, change that
        m_QualsChangedCount = 0;
        if (title->IsSet()) {
            for (auto& it : title->Set()) {
                if (it->IsName()) {
                    string orig_value = it->GetName();
                    if (edit::AddValueToString(orig_value, newValue, existing_text)) {
                        it->SetName(orig_value);
                        m_QualsChangedCount++;
                    }
                }
            }

            if (m_QualsChangedCount == 0) {
                // if we find a not_set, change it to a Name
                for (auto& it : title->Set()) {
                    if (it->Which() == CTitle::C_E::e_not_set) {
                        it->SetName(newValue);
                        m_QualsChangedCount++;
                    }
                }
            }
        }

        // otherwise add a Name
        if (m_QualsChangedCount == 0) {
            CRef<CTitle::C_E> this_title(new CTitle::C_E());
            this_title->SetName(newValue);
            title->Set().push_back(this_title);
            m_QualsChangedCount++;
        }
    }
}

void CMacroFunction_SetPubJournal::x_RemoveJournalInPubdesc(CPubdesc& pubdesc)
{
    for (auto& it : pubdesc.SetPub().Set()) {
        switch (it->Which()) {
        case CPub::e_Gen:
            if (it->GetGen().IsSetJournal()) {
                it->SetGen().ResetJournal();
                m_QualsChangedCount++;
            }
            break;
        case CPub::e_Article:
            if (it->GetArticle().IsSetFrom() &&
                it->GetArticle().GetFrom().IsJournal() &&
                it->GetArticle().GetFrom().GetJournal().IsSetTitle()) {
                it->SetArticle().SetFrom().SetJournal().ResetTitle();
                m_QualsChangedCount++;
            }
            break;
        case CPub::e_Journal:
            if (it->GetJournal().IsSetTitle()) {
                it->SetJournal().ResetTitle();
                m_QualsChangedCount++;
            }
        default:
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SetPubVolIssuePages
/// SetPubVolume(newValue, existing_text, delimiter, remove_blank)
/// SetPubIssue(newValue, existing_text, delimiter, remove_blank)
/// SetPubPages(newValue, existing_text, delimiter, remove_blank)
/// The last two parameters are optional
///
const char* CMacroFunction_SetPubVolIssuePages::sm_FuncVolume = "SetPubVolume";
const char* CMacroFunction_SetPubVolIssuePages::sm_FuncIssue = "SetPubIssue";
const char* CMacroFunction_SetPubVolIssuePages::sm_FuncPages = "SetPubPages";

void CMacroFunction_SetPubVolIssuePages::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    if (!const_pubdesc)
        return;

    size_t index = 0;
    const string& newValue = m_Args[index]->GetString();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr();
    if (!newValue.empty()) {
        CObjectInfo oi = m_DataIter->GetEditedObject();
        CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
        x_SetVolIssuePagesInPubdesc(*pubdesc, newValue, existing_text);

        if (m_QualsChangedCount) {
            log << ": set ";
        }
    }
    else if (remove_field) {
        CObjectInfo oi = m_DataIter->GetEditedObject();
        CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
        x_RemoveVolIssuePagesInPubdesc(*pubdesc);

        if (m_QualsChangedCount) {
            log << ": removed ";
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        log << m_QualsChangedCount << " publication " << m_Field;
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetPubVolIssuePages::x_ValidArguments() const
{
    auto arg_nr = m_Args.size();
    if (arg_nr < 2 || arg_nr > 4) {
        return false;
    }

    size_t index = 0;
    NMacroUtil::GetPrimitiveFromRef(m_Args[index].GetNCObject());
    if (!m_Args[index]->IsString()) {
        return false;
    }
    if (!m_Args[++index]->IsString()) return false;
    if (arg_nr > 2 && (!m_Args[++index]->IsString() && !m_Args[index]->IsBool())) return false;
    if (arg_nr > 3 && !m_Args[++index]->IsBool()) return false;
    return true;
}

static CMQueryNodeValue::TObs s_GetPubVolIssuePagesDateObjects(CPubdesc& pubdesc, const string& field)
{
    CMQueryNodeValue::TObs objs;

    for (auto& it : pubdesc.SetPub().Set()) {
        CObjectInfo pub_oi(it.GetNCPointer(), it->GetThisTypeInfo());
        CObjectInfo pub_var = pub_oi.GetCurrentChoiceVariant().GetVariant();
        switch (it->Which()) {
        case CPub::e_Gen:
            SetFieldsByName(&objs, pub_var, field);
            break;
        case CPub::e_Sub:
            if (field == "date.std") {
                SetFieldsByName(&objs, pub_var, field);
            }
            break;
        case CPub::e_Article:
            if (it->GetArticle().IsSetFrom()) {
                switch (it->GetArticle().GetFrom().Which()) {
                case CCit_art::TFrom::e_Book:
                    SetFieldsByName(&objs, pub_var, "from.book.imp." + field);
                    break;
                case CCit_art::TFrom::e_Journal:
                    SetFieldsByName(&objs, pub_var, "from.journal.imp." + field);
                    break;
                case CCit_art::TFrom::e_Proc:
                    SetFieldsByName(&objs, pub_var, "from.proc.book.imp." + field);
                    break;
                default:
                    break;
                }
            }
            break;
        case CPub::e_Book:
        case CPub::e_Journal:
            SetFieldsByName(&objs, pub_var, "imp." + field);
            break;
        case CPub::e_Man:
            SetFieldsByName(&objs, pub_var, "cit.imp." + field);
            break;
        case CPub::e_Proc:
            SetFieldsByName(&objs, pub_var, "book.imp." + field);
            break;
        default:
            break;
        }
    }
    return objs;
}

void CMacroFunction_SetPubVolIssuePages::x_SetVolIssuePagesInPubdesc(CPubdesc& pubdesc, const string& newValue, edit::EExistingText existing_text)
{
    CMQueryNodeValue::TObs objs = s_GetPubVolIssuePagesDateObjects(pubdesc, m_Field);
    for (auto& it : objs) {
        CObjectInfo obj = NMacroUtil::GetPrimitiveObjInfo(it.field);
        if (obj.GetPrimitiveValueType() == ePrimitiveValueString) {
            string orig_value = obj.GetPrimitiveValueString();
            if (edit::AddValueToString(orig_value, newValue, existing_text)) {
                (SetQualStringValue(obj, orig_value));
            }
        }
    }
}

void CMacroFunction_SetPubVolIssuePages::x_RemoveVolIssuePagesInPubdesc(CPubdesc& pubdesc)
{
    CMQueryNodeValue::TObs objs = s_GetPubVolIssuePagesDateObjects(pubdesc, m_Field);
    m_QualsChangedCount = CMacroFunction_RemoveQual::s_RemoveFields(m_DataIter, objs);
}

///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_SetPubDate
/// SetPubDate(year, month, date)
/// The only option is to overwrite
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetPubDate, "SetPubDate")
void CMacroFunction_SetPubDate::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());
    if (!const_pubdesc && !const_block)
        return;

    const string date_field = "date.std";
    CObjectInfo oi = m_DataIter->GetEditedObject();
    if (const_pubdesc) {
        CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
        CMQueryNodeValue::TObs objs = s_GetPubVolIssuePagesDateObjects(*pubdesc, date_field);
        x_AddDate(objs);
    }
    else if (const_block) {
        CSubmit_block* submit_block = CTypeConverter<CSubmit_block>::SafeCast(oi.GetObjectPtr());
        if (submit_block->IsSetCit()) {
            CMQueryNodeValue::TObs objs;
            CObjectInfo sub_oi(&(submit_block->SetCit()), submit_block->SetCit().GetThisTypeInfo());
            SetFieldsByName(&objs, sub_oi, date_field);
            x_AddDate(objs);
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": set publication date";
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetPubDate::x_ValidArguments() const
{
    if (m_Args.size() != 3) return false;
    for (auto& it : m_Args) {
        if (it->GetDataType() != CMQueryNodeValue::eInt)
            return false;
    }
    return true;
}

void CMacroFunction_SetPubDate::x_AddDate(CMQueryNodeValue::TObs& objs)
{
    if (objs.empty()) return;
    CObjectInfoMI mem = objs.front().field.BeginMembers();
    auto it = m_Args.begin();

    while (mem.Valid() && it != m_Args.end()) {
        if ((*it)->GetInt() > 0) {
            CObjectInfo tmp(objs.front().field.SetClassMember(mem.GetMemberIndex()));
            if (SetSimpleTypeValue(tmp, **it)) {
                m_QualsChangedCount++;
            }
        }
        ++mem;
        ++it;
    }
}

///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_SetPubDateField
///SetPubDateField(date_field, new_value, remove_blank)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetPubDateField, "SetPubDateField")
void CMacroFunction_SetPubDateField::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());
    if (!const_pubdesc && !const_block)
        return;

    size_t index = 1;
    string newValue;
    if (m_Args[index]->IsInt()) {
        newValue = NStr::Int8ToString(m_Args[index]->GetInt());
    }
    else if (m_Args[index]->IsString()) {
        newValue = m_Args[index]->GetString();
        if (m_Args[0]->GetString() != "season" && !newValue.empty()) {
            m_Args[index]->SetInt(NStr::StringToInt8(newValue));
        }
    }
    bool remove_field = (++index < m_Args.size()) ? m_Args[index]->GetBool() : false;

    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr();

    const string date_field = "date.std";
    CObjectInfo oi = m_DataIter->GetEditedObject();
    if (const_pubdesc) {
        CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
        CMQueryNodeValue::TObs objs = s_GetPubVolIssuePagesDateObjects(*pubdesc, date_field);
        if (!newValue.empty()) {
            x_AddDateField(objs);
            if (m_QualsChangedCount) {
                log << ": set ";
            }
        }
        else if (remove_field) {
            x_RemoveDateField(objs);
            if (m_QualsChangedCount) {
                log << ": removed ";
            }
        }
    }
    else if (const_block) {
        CSubmit_block* submit_block = CTypeConverter<CSubmit_block>::SafeCast(oi.GetObjectPtr());
        if (submit_block->IsSetCit()) {
            CMQueryNodeValue::TObs objs;
            CObjectInfo sub_oi(&(submit_block->SetCit()), submit_block->SetCit().GetThisTypeInfo());
            if (!newValue.empty()) {
                SetFieldsByName(&objs, sub_oi, date_field);
                x_AddDateField(objs);
                if (m_QualsChangedCount) {
                    log << ": set ";
                }
            }
            else if (remove_field) {
                GetFieldsByName(&objs, sub_oi, date_field);
                x_RemoveDateField(objs);
                if (m_QualsChangedCount) {
                    log << ": removed ";
                }
            }
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        log << m_QualsChangedCount << " publication " + m_Args[0]->GetString();
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetPubDateField::x_ValidArguments() const
{
    if (m_Args.size() != 2 && m_Args.size() != 3) {
        return false;
    }

    size_t index = 0;
    if (!m_Args[index]->IsString()) return false;
    NMacroUtil::GetPrimitiveFromRef(m_Args[++index].GetNCObject());
    if (!m_Args[index]->IsString() && !m_Args[index]->IsInt()) {
        return false;
    }
    if (++index < m_Args.size() && !m_Args[index]->IsBool()) return false;
    return true;
}

void CMacroFunction_SetPubDateField::x_AddDateField(CMQueryNodeValue::TObs& objs)
{
    if (objs.empty()) return;
    CMQueryNodeValue::TObs obj_fields;
    SetFieldsByName(&obj_fields, objs.front().field, m_Args[0]->GetString());

    if (!obj_fields.empty() && SetSimpleTypeValue(obj_fields.front().field, *m_Args[1])) {
        m_QualsChangedCount++;
    }
}

void CMacroFunction_SetPubDateField::x_RemoveDateField(CMQueryNodeValue::TObs& objs)
{
    if (objs.empty()) return;
    CMQueryNodeValue::TObs obj_fields;
    GetFieldsByName(&obj_fields, objs.front().field, m_Args[0]->GetString());
    m_QualsChangedCount = CMacroFunction_RemoveQual::s_RemoveFields(m_DataIter, obj_fields);
}

///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_SetPubCitation
/// SetCitation(newValue, existingtext_option, delimiter, remove_blank)
///

DEFINE_MACRO_FUNCNAME(CMacroFunction_SetPubCitation, "SetPubCitation")
void CMacroFunction_SetPubCitation::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    if (!const_pubdesc)
        return;

    size_t index = 0;
    const string& newValue = m_Args[index]->GetString();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());

    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr();

    if (!newValue.empty()) {
        CMQueryNodeValue::TObs objs;
        for (auto& it : pubdesc->SetPub().Set()) {
            CObjectInfo pub_oi(it.GetNCPointer(), it->GetThisTypeInfo());
            CObjectInfo pub_var = pub_oi.GetCurrentChoiceVariant().GetVariant();
            switch (it->Which()) {
            case CPub::e_Gen:
                SetFieldsByName(&objs, pub_var, "cit");
                break;
            default:
                break;
            }
        }

        for (auto& it : objs) {
            CObjectInfo obj = NMacroUtil::GetPrimitiveObjInfo(it.field);
            if (obj.GetPrimitiveValueType() == ePrimitiveValueString) {
                string orig_value = obj.GetPrimitiveValueString();
                if (edit::AddValueToString(orig_value, newValue, existing_text)) {
                    (SetQualStringValue(obj, orig_value));
                }
            }
        }

        if (m_QualsChangedCount) {
            log << ": set ";
        }
    }
    else if (remove_field) {
        CMQueryNodeValue::TObs objs;
        for (auto& it : pubdesc->SetPub().Set()) {
            CObjectInfo pub_oi(it.GetNCPointer(), it->GetThisTypeInfo());
            CObjectInfo pub_var = pub_oi.GetCurrentChoiceVariant().GetVariant();
            switch (it->Which()) {
            case CPub::e_Gen:
                GetFieldsByName(&objs, pub_var, "cit");
                break;
            default:
                break;
            }
        }
        m_QualsChangedCount = CMacroFunction_RemoveQual::s_RemoveFields(m_DataIter, objs);
        if (m_QualsChangedCount) {
            log << ": removed ";
        }

    }
    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        log << m_QualsChangedCount << " publication citation";
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetPubCitation::x_ValidArguments() const
{
    auto arg_nr = m_Args.size();
    if (arg_nr < 2 || arg_nr > 4) {
        return false;
    }
    size_t index = 0;
    NMacroUtil::GetPrimitiveFromRef(m_Args[index].GetNCObject());
    if (!m_Args[index]->IsString() || !m_Args[++index]->IsString()) {
        return false;
    }
    if (arg_nr > 2 && (!m_Args[++index]->IsString() && !m_Args[index]->IsBool())) return false;
    if (arg_nr > 3 && !m_Args[++index]->IsBool()) return false;
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_SetSerialNumber
/// SetPubSerialNumber(number, existing_text_option, delimiter, remove_blank)
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetSerialNumber, "SetPubSerialNumber")
void CMacroFunction_SetSerialNumber::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    if (!const_pubdesc)
        return;

    size_t index = 0;
    string newValue;
    if (m_Args[index]->IsString()) {
        newValue = m_Args[0]->GetString();
    }
    else if (m_Args[index]->IsInt()) {
        newValue = NStr::Int8ToString(m_Args[index]->GetInt(), NStr::fConvErr_NoThrow);
        if (errno != 0)
            return;
    }

    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());

    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr();

    if (!newValue.empty()) {
        CMQueryNodeValue::TObs objs;
        for (auto& it : pubdesc->SetPub().Set()) {
            CObjectInfo pub_oi(it.GetNCPointer(), it->GetThisTypeInfo());
            CObjectInfo pub_var = pub_oi.GetCurrentChoiceVariant().GetVariant();
            switch (it->Which()) {
            case CPub::e_Gen:
                SetFieldsByName(&objs, pub_var, "serial-number");
                break;
            default:
                break;
            }
        }

        for (auto& it : objs) {
            CObjectInfo obj = NMacroUtil::GetPrimitiveObjInfo(it.field);
            if (obj.GetPrimitiveValueType() == ePrimitiveValueInteger) {
                string orig_value = NStr::IntToString(obj.GetPrimitiveValueInt(), NStr::fConvErr_NoThrow);
                if (errno != 0) {
                    continue;
                }
                if (edit::AddValueToString(orig_value, newValue, existing_text)) {
                    int new_int_value = NStr::StringToInt(orig_value, NStr::fConvErr_NoThrow);
                    if (errno == 0 && new_int_value != obj.GetPrimitiveValueInt()) {
                        obj.SetPrimitiveValueInt(new_int_value);
                        m_QualsChangedCount++;
                    }
                }
            }
        }
        if (m_QualsChangedCount) {
            log << ": set ";
        }
    }
    else if (remove_field) {
        CMQueryNodeValue::TObs objs;
        for (auto& it : pubdesc->SetPub().Set()) {
            CObjectInfo pub_oi(it.GetNCPointer(), it->GetThisTypeInfo());
            CObjectInfo pub_var = pub_oi.GetCurrentChoiceVariant().GetVariant();
            switch (it->Which()) {
            case CPub::e_Gen:
                GetFieldsByName(&objs, pub_var, "serial-number");
                break;
            default:
                break;
            }
        }
        m_QualsChangedCount = CMacroFunction_RemoveQual::s_RemoveFields(m_DataIter, objs);
        if (m_QualsChangedCount) {
            log << ": removed ";
        }

    }
    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        log << m_QualsChangedCount << " publication serial number";
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetSerialNumber::x_ValidArguments() const
{
    auto arg_nr = m_Args.size();
    if (arg_nr > 4 || arg_nr < 2) {
        return false;
    }
    size_t index = 0;
    NMacroUtil::GetPrimitiveFromRef(m_Args[index].GetNCObject());
    if (!m_Args[index]->IsInt() && !m_Args[index]->IsString())
        return false;

    if (!m_Args[++index]->IsString()) return false;
    if (arg_nr > 2 && (!m_Args[++index]->IsString() && !m_Args[index]->IsBool())) return false;
    if (arg_nr > 3 && !m_Args[++index]->IsBool()) return false;
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_SetPubStatus
/// SetPubStatus(value)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetPubStatus, "SetPubStatus")
void CMacroFunction_SetPubStatus::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    if (!const_pubdesc)
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    if (const_pubdesc) {
        CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
        if (s_SetStatus(*pubdesc, m_Args[0]->GetString()))
            m_QualsChangedCount++;
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": set publication status";
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetPubStatus::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eString);
}

bool CMacroFunction_SetPubStatus::s_SetStatus(CPubdesc& pubdesc, const string& value)
{
    if (NStr::EqualNocase(value, "Any") || NMacroUtil::StringsAreEquivalent(value, "Submitter block")) {
        return false;
    }

    bool modified = false;
    for (auto& it : pubdesc.SetPub().Set()) {
        switch (it->Which()) {
        case CPub::e_Gen: {
            CCit_gen& genref = it->SetGen();
            if (genref.IsSetCit() && NStr::EqualNocase(genref.GetCit(), value)) {
                // do nothing
            }
            else {
                if (NStr::EqualNocase(value, "unpublished")) {
                    genref.SetCit("unpublished");
                    modified = true;
                }
                else if (NStr::EqualNocase(value, "published")) {
                    genref.ResetCit();
                    modified = true;
                }
            }
            break;
        }
        case CPub::e_Article:
            if (it->GetArticle().IsSetFrom()) {
                switch (it->GetArticle().GetFrom().Which()) {
                case CCit_art::TFrom::e_Book:
                    if (it->GetArticle().GetFrom().GetBook().IsSetImp()) {
                        modified = s_SetStatus(it->SetArticle().SetFrom().SetBook().SetImp(), value);
                    }
                    break;
                case CCit_art::TFrom::e_Journal:
                    if (it->GetArticle().GetFrom().GetJournal().IsSetImp()) {
                        modified = s_SetStatus(it->SetArticle().SetFrom().SetJournal().SetImp(), value);
                    }
                    break;
                case CCit_art::TFrom::e_Proc:
                    if (it->GetArticle().GetFrom().GetProc().IsSetBook()
                        && it->GetArticle().GetFrom().GetProc().GetBook().IsSetImp()) {
                        modified = s_SetStatus(it->SetArticle().SetFrom().SetProc().SetBook().SetImp(), value);
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case CPub::e_Book:
            if (it->GetBook().IsSetImp()) {
                modified = s_SetStatus(it->SetBook().SetImp(), value);
            }
            break;
        case CPub::e_Journal:
            if (it->GetJournal().IsSetImp()) {
                modified = s_SetStatus(it->SetJournal().SetImp(), value);
            }
            break;
        case CPub::e_Man:
            if (it->GetMan().IsSetCit() && it->GetMan().GetCit().IsSetImp()) {
                modified = s_SetStatus(it->SetMan().SetCit().SetImp(), value);
            }
            break;
        default:
            break;
        }
    }
    return modified;
}

bool CMacroFunction_SetPubStatus::s_SetStatus(CImprint& imp, const string& value)
{
    string current_status;
    if (imp.IsSetPrepub()) {
        if (imp.GetPrepub() == CImprint::ePrepub_in_press) {
            current_status = "In Press";
        }
        else {
            current_status = "Unpublished";
        }
    }
    else {
        current_status = "Published";
    }
    if (NStr::EqualNocase(value, current_status))
        return false;

    bool modified = false;
    if (NStr::EqualNocase(value, "unpublished")) {
        imp.SetPrepub(CImprint::ePrepub_other);
        modified = true;
    }
    else if (NStr::EqualNocase(value, "published")) {
        imp.ResetPrepub();
        modified = true;
    }
    else if (NMacroUtil::StringsAreEquivalent(value, "In Press")) {
        imp.SetPrepub(CImprint::ePrepub_in_press);
        modified = true;
    }
    return modified;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SetPubPMID
/// SetPMID(pmid_value, remove_blank);
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetPubPMID, "SetPMID")
void CMacroFunction_SetPubPMID::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    if (!const_pubdesc) return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
    _ASSERT(pubdesc);
    
    string newStrValue;
    Int8 new_pmid;
    size_t index = 0;
    if (m_Args[index]->IsString()) {
        newStrValue = m_Args[index]->GetString();
        new_pmid = NStr::StringToInt8(newStrValue, NStr::fConvErr_NoThrow);
    }
    else if (m_Args[index]->GetInt()) {
        new_pmid = m_Args[index]->GetInt();
        newStrValue = NStr::Int8ToString(new_pmid);
    }
    bool remove_field = (++index < m_Args.size()) ? m_Args[index]->GetBool() : false;

    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr();

    if (!newStrValue.empty()) {
        bool has_pmid = false;
        for (auto& it : pubdesc->SetPub().Set()) {
            if (it->IsPmid()) {
                has_pmid = true;
                it->SetPmid().Set(TEntrezId(new_pmid));
                m_QualsChangedCount++;
                break;
            }
        }

        // don't add pmids to cit-subs
        for (auto& it : pubdesc->GetPub().Get()) {
            if (it->IsSub()) {
                return;
            }
        }

        if (!has_pmid) {
            CRef<CPub> new_pub(new CPub());
            new_pub->SetPmid().Set(TEntrezId(new_pmid));
            pubdesc->SetPub().Set().push_back(new_pub);
            m_QualsChangedCount++;
        }

        // update the cit-art
        bool has_art_pmid = false;
        for (auto& it : pubdesc->SetPub().Set()) {
            if (it->IsArticle()) {
                if (it->SetArticle().IsSetIds()) {
                    EDIT_EACH_ARTICLEID_ON_CITART(id, it->SetArticle()) {
                        if ((*id)->IsPubmed()) {
                            has_art_pmid = true;
                            (*id)->SetPubmed().Set(TEntrezId(new_pmid));
                            m_QualsChangedCount++;
                        }
                    }
                }

                if (!has_art_pmid) {
                    CRef<CArticleId> art_id(new CArticleId);
                    art_id->SetPubmed().Set(TEntrezId(new_pmid));
                    it->SetArticle().SetIds().Set().push_back(art_id);
                    m_QualsChangedCount++;
                }
            }
        }
        if (m_QualsChangedCount) {
            log << ": applied pmid " << new_pmid;
        }
    }
    else if (remove_field) {
        CPub_equiv::Tdata::iterator pub_it = pubdesc->SetPub().Set().begin();
        while (pub_it != pubdesc->SetPub().Set().end()) {
            auto& pub_elem = (*pub_it);
            if (pub_elem->IsPmid()) {
                pub_it = pubdesc->SetPub().Set().erase(pub_it);
                m_QualsChangedCount++;
                continue;
            }
            else if (pub_elem->IsArticle() && pub_elem->GetArticle().IsSetIds()) {
                EDIT_EACH_ARTICLEID_ON_CITART(id, pub_elem->SetArticle()) {
                    if ((*id)->IsPubmed()) {
                        ERASE_ARTICLEID_ON_CITART(id, pub_elem->SetArticle());
                        m_QualsChangedCount++;
                    }
                }
                if (pub_elem->GetArticle().GetIds().Get().empty()) {
                    pub_elem->SetArticle().ResetIds();
                }
            }
            ++pub_it;
        }
        if (m_QualsChangedCount) {
            log << ": removed pmid";
        }
    }
    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetPubPMID::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 2) {
        return false;
    }
    NMacroUtil::GetPrimitiveFromRef(m_Args[0].GetNCObject());
    if (!m_Args[0]->IsInt() && !m_Args[0]->IsString())
        return false;
    if (m_Args.size() == 2 && !m_Args[1]->IsBool())
        return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ApplyPmidToEntry
/// ApplyPmidToEntry(pmid) 
/// The macro checks all references in the record (usually a seq-entry containing a single bioseq).
/// If the new pmid already exists in the entry, it does nothing. If the new pmid does not exist in the entry, 
/// it applies it as a new pubdesc descriptor containing this single pmid pub. This is to be done even if there is another pmid.
/// At the end it does a pub lookup.
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ApplyPmidToEntry, "ApplyPmidToEntry")
const vector<SArgMetaData> CMacroFunction_ApplyPmidToEntry::sm_Arguments 
{ SArgMetaData("pmid_field", CMQueryNodeValue::eInt, false)};

void CMacroFunction_ApplyPmidToEntry::TheFunction()
{
    // the iterator should iterate over TSEntry
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
    if (!entry) {
        return;
    }

    // we assume that the TSEntry has only one nucleotide sequence
    size_t count = 0;
    CBioseq_CI b_iter(m_DataIter->GetSEH(), CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    for (; b_iter; ++b_iter) {
        ++count;
    }

    Int8 new_pmid = m_Args[0]->GetInt();
    if (count != 1) {
        CNcbiOstrstream log;
        log << "Pmid " << new_pmid << " was not applied as the record has " << count << " nucleotide sequences. ";
        return;
    }

    set < Int8 > orig_pmids;
    for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_Pub); desc_it; ++desc_it) {
        const CPubdesc& pub = desc_it->GetPub();
        for (auto& it : pub.GetPub().Get()) {
            if (it->IsPmid()) {
                orig_pmids.insert(it->GetPmid());
            }
            if (it->IsArticle() && it->GetArticle().IsSetIds()) {
                FOR_EACH_ARTICLEID_ON_CITART(id, it->GetArticle()) {
                    if ((*id)->IsPubmed()) {
                        orig_pmids.insert((*id)->GetPubmed());
                    }
                }
            }
            if (it->IsMedline() && it->GetMedline().IsSetPmid()) {
                orig_pmids.insert(it->GetMedline().GetPmid());
            }
        }
    }

    // don't update if the new_pmid is already in the entry
    if (orig_pmids.find(new_pmid) != orig_pmids.end()) {
        return;
    }

    CRef<CPub> new_pub = CMacroFunction_LookupPub::s_GetArticleFromEntrezById(int(new_pmid));
    if (new_pub) {
        CRef<CPubdesc> new_pubdesc(new CPubdesc);
        new_pubdesc->SetPub().Set().push_back(new_pub);
        CRef<CPub> pmid_pub(new CPub());
        pmid_pub->SetPmid().Set(TEntrezId(new_pmid));
        new_pubdesc->SetPub().Set().push_back(pmid_pub);
        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->SetPub(*new_pubdesc);

        CRef<CCmdComposite> add_cmd(new CCmdComposite("Create new pmid pub"));
        add_cmd->AddCommand(*CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetSeq_entry_Handle(), *new_desc)));
        if (add_cmd) {
            m_DataIter->RunCommand(add_cmd, m_CmdComposite);

            string best_id;
            CWriteUtil::GetBestId(bsh.GetAccessSeq_id_Handle(), m_DataIter->GetSEH().GetScope(), best_id);

            CNcbiOstrstream log;
            log << "Applied " << new_pmid << " pmid to " << best_id;
            x_LogFunction(log);
        }
    }
    else { // lookup failed
        CNcbiOstrstream log;
        log << new_pmid << " pmid failed to look up";
        x_LogFunction(log);
    }

}

bool CMacroFunction_ApplyPmidToEntry::x_ValidArguments() const
{
    return (m_Args.size() == 1) && (m_Args[0]->GetDataType() == CMQueryNodeValue::eInt);
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ApplyDOIToEntry
/// ApplyDOIToEntry(doi_name);
/// The macro checks all references in the record (usually a seq-entry containing a single bioseq).
/// If the new doi number already exists in the entry, it does nothing. Otherwise, it looks up the doi_name and
/// and adds the reference to the entry.
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ApplyDOIToEntry, "ApplyDOIToEntry")
const vector<SArgMetaData> CMacroFunction_ApplyDOIToEntry::sm_Arguments
{ SArgMetaData("doi_field", CMQueryNodeValue::eString, false)};

void CMacroFunction_ApplyDOIToEntry::TheFunction()
{
    // the iterator should iterate over TSEntry
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
    if (!entry) {
        return;
    }

    // we assume that the TSEntry has only one nucleotide sequence
    size_t count = 0;
    CBioseq_CI b_iter(m_DataIter->GetSEH(), CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    for (; b_iter; ++b_iter) {
        ++count;
    }

    const string& new_doi = m_Args[0]->GetString();
    if (count != 1) {
        CNcbiOstrstream log;
        log << "DOI " << new_doi << " was not applied as the record has " << count << " nucleotide sequences. ";
        x_LogFunction(log);
        return;
    }

    set <string> orig_dois;
    for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_Pub); desc_it; ++desc_it) {
        const CPubdesc& pub = desc_it->GetPub();
        for (auto& it : pub.GetPub().Get()) {
            if (it->IsArticle() && it->GetArticle().IsSetIds()) {
                FOR_EACH_ARTICLEID_ON_CITART(id, it->GetArticle()) {
                    if ((*id)->IsDoi()) {
                        orig_dois.insert((*id)->GetDoi());
                    }
                }
            }
        }
    }

    // don't update if the new_doi is already in the entry
    if (orig_dois.find(new_doi) != orig_dois.end()) {
        return;
    }

    pair<CRef<CPubdesc>, string> new_pubdesc_str = CDoiLookup::GetPubFromCrossRef(new_doi);
    CRef<CPubdesc> new_pubdesc = new_pubdesc_str.first;
    if (new_pubdesc) {
        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->SetPub(*new_pubdesc);

        CRef<CCmdComposite> add_cmd(new CCmdComposite("Create new pubdesc based on doi"));
        add_cmd->AddCommand(*CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetSeq_entry_Handle(), *new_desc)));
        if (add_cmd) {
            m_DataIter->RunCommand(add_cmd, m_CmdComposite);

            string best_id;
            CWriteUtil::GetBestId(bsh.GetAccessSeq_id_Handle(), m_DataIter->GetSEH().GetScope(), best_id);

            CNcbiOstrstream log;
            log << "Applied publication with " << new_doi << " doi to " << best_id;
            x_LogFunction(log);
        }
    }
    else
    {
        CNcbiOstrstream log;
        log << "Problem resolving DOI " << new_doi << " " << new_pubdesc_str.second;
        x_LogFunction(log);
    }
}

bool CMacroFunction_ApplyDOIToEntry::x_ValidArguments() const
{
    return (m_Args.size() == 1) && (m_Args[0]->IsString());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemovePubAuthors
/// RemovePubAuthors() - removes the author name list (considered to be one field)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemovePubAuthors, "RemoveAuthors")
void CMacroFunction_RemovePubAuthors::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());
    if (!const_pubdesc && !const_block)
        return;

    bool modified = false;
    CObjectInfo oi = m_DataIter->GetEditedObject();
    if (const_pubdesc) {
        CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());
        for (auto& it : pubdesc->SetPub().Set()) {
            if (it->IsSetAuthors()) {
                CAuth_list& auth_list = it->SetAuthors();
                auth_list.ResetNames();
                auth_list.SetNames().SetStr().push_back("?");
                modified = true;
            }
            break;
        }
    }
    else if (const_block) {
        CSubmit_block* submit_block = CTypeConverter<CSubmit_block>::SafeCast(oi.GetObjectPtr());
        if (submit_block->IsSetCit() && submit_block->SetCit().IsSetAuthors()) {
            submit_block->SetCit().SetAuthors().ResetNames();
            submit_block->SetCit().SetAuthors().SetNames().SetStr().push_back("?");
            modified = true;
        }
    }

    if (modified) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": removed publication authors";
        x_LogFunction(log);
    }
}

bool CMacroFunction_RemovePubAuthors::x_ValidArguments() const
{
    return m_Args.empty();
}


///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_RemovePubAuthorMI
/// RemovePubAuthorMI(author_object)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemovePubAuthorMI, "RemovePubAuthorMI")
void CMacroFunction_RemovePubAuthorMI::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());
    if (!const_pubdesc && !const_block)
        return;

    CMQueryNodeValue::TObs res_oi;
    if (m_Args[0]->GetDataType() == CMQueryNodeValue::eObjects) {
        res_oi = m_Args[0]->GetObjects();
    }
    else if (m_Args[0]->GetDataType() == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(res_oi, 0);
    }

    if (res_oi.empty()) {
        return;
    }

    for(auto&& it : res_oi) {
        // it can be either CName_std object or a string
        if (it.field.GetTypeFamily() == eTypeFamilyPrimitive) {
            return;
        }
        else {
            CName_std* std_name = CTypeConverter<CName_std>::SafeCast(it.field.GetObjectPtr());
            if (std_name && NMacroUtil::RemoveMiddleInitial(*std_name)) {
                m_QualsChangedCount++;
            }
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": removed " << m_QualsChangedCount << " publication author middle initial";
        x_LogFunction(log);
    }
}

bool CMacroFunction_RemovePubAuthorMI::x_ValidArguments() const
{
    if (m_Args.size() != 1) {
        return false;
    }

    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    return (type == CMQueryNodeValue::eObjects || type == CMQueryNodeValue::eRef);
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SetPubField
/// SetPub_Date(year, month, day, season, hour, minute, second); 
/// SetPub_Affil(affil_field, new_value);
/// All parameters except the first one is optional

/// DEPRECATED functions
const char* CMacroFunction_SetPubAffil_Depr::sm_FunctionName = "SetPub_Affil";
const char* CMacroFunction_SetPubDate_Depr::sm_FunctionName = "SetPub_Date";

void CMacroFunction_SetPubField::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    // it will not set new field in the submitter block
    if (!GetFieldsByName(&res_oi, oi, "pub..") || res_oi.size() != 1)
        return;

    CObjectInfo current_oi = res_oi.front().field;
    if (current_oi.GetTypeFamily() == eTypeFamilyPointer) {
        current_oi = current_oi.GetPointedObject();
    }

    if (current_oi.GetTypeFamily() != eTypeFamilyContainer)
        return;

    // res_oi should be a container of pointers, each of them pointing to a Pub field
    CObjectInfoEI elem = current_oi.BeginElements();
    while (elem.Valid()) {
        CObjectInfo pub(elem.GetElement().GetPointedObject());  // the Pub field
        x_SetPubFieldInPub(pub);
        ++elem;
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": set new value to " << m_QualsChangedCount << " publication fields";
        x_LogFunction(log);
    }
}

void CMacroFunction_SetPubField::x_SetPubFieldInPub(CObjectInfo& pub_oi)
{
    CPub* pub = static_cast<CPub*>(pub_oi.GetObjectPtr());
    if (!pub)
        return;

    CObjectInfo pub_var = pub_oi.GetCurrentChoiceVariant().GetVariant();

    switch (m_PubField) {
    case (ePubField_Date):
        return x_SetObjects_Date(*pub, pub_var);
    case ePubField_Affil:
        return x_SetObjects_Affil(*pub, pub_var);
    }
}

void CMacroFunction_SetPubField::x_SetObjects_Date(CPub& pub, CObjectInfo& pub_var)
{
    if (m_PubField != ePubField_Date)
        return;

    switch (pub.Which()) {
    case (CPub::e_Gen):
    case (CPub::e_Sub): {
        CMQueryNodeValue::TObs res_oi;
        if (!SetFieldsByName(&res_oi, pub_var, "date.std")) {
            return;
        }

        _ASSERT(!res_oi.empty());
        if (res_oi.front().field.GetTypeFamily() != eTypeFamilyClass)
            return;

        CObjectInfoMI mem = res_oi.front().field.BeginMembers();
        auto it = m_Args.begin();
        for (; mem.Valid() && it != m_Args.end(); ++mem, ++it) {
            CObjectInfo tmp(res_oi.front().field.SetClassMember(mem.GetMemberIndex()));
            if (SetSimpleTypeValue(tmp, **it)) {
                m_QualsChangedCount++;
            }
        }
    }
    default:
        // not handling other type of publications for now
        break;
    }
}

void CMacroFunction_SetPubField::x_SetObjects_Affil(objects::CPub& pub, CObjectInfo& pub_var)
{
    if (m_PubField != ePubField_Affil)
        return;

    switch (pub.Which()) {
    case (CPub::e_Gen):
    case (CPub::e_Sub): {

        string field_name("authors.affil.std.");
        field_name.append(m_Args[0]->GetString());
        CMQueryNodeValue::TObs res_oi;
        if (!SetFieldsByName(&res_oi, pub_var, field_name)) {
            return;
        }

        _ASSERT(!res_oi.empty());
        if (SetSimpleTypeValue(res_oi.front().field, *m_Args[1])) {
            m_QualsChangedCount++;
        }
    }
    default:
        // not handling other type of publications for now
        break;
    }
}

bool CMacroFunction_SetPubField::x_ValidArguments() const
{
    vector<CMQueryNodeValue::EType> valid_types;

    switch (m_PubField) {
    case ePubField_Date: {
        if (m_Args.size() < 1 || m_Args.size() > 6)
            return false;

        for (size_t i = 0; i < m_Args.size(); ++i) {
            if (i == 3 && m_Args[i]->GetDataType() != CMQueryNodeValue::eString) {
                return false;
            }
            else if (m_Args[i]->GetDataType() != CMQueryNodeValue::eInt) {
                return false;
            }
        }
        break;
    }
    case ePubField_Affil: {
        if (m_Args.size() != 2)
            return false;
        return (m_Args[0]->GetDataType() == CMQueryNodeValue::eString)
            && (m_Args[1]->GetDataType() == CMQueryNodeValue::eString);
    }
    }

    return true;
}


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
