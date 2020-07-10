/*  $Id: add_flu_comments.cpp 31592 2014-10-24 19:21:41Z filippov $
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


#include <objmgr/scope.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/packages/pkg_sequence_edit/add_flu_comments.hpp>


BEGIN_NCBI_SCOPE


string CAddFluComments::ConcatIds(const TNumStrSet &ids)
{
    vector<string> id_vec;
    pair<string,string> range;
    pair<long,long> range_num;
    for (TNumStrSet::const_iterator id = ids.begin(); id != ids.end(); ++id)
    {
        long num_id = NStr::StringToULong(*id, NStr::fAllowLeadingSymbols | NStr::fConvErr_NoThrow);
        if (num_id == 0)
        {
            id_vec.push_back(*id);
            range_num.first = 0;
            range_num.second = 0;
            range.first.clear();
            range.second.clear();
            continue;
        }
      
        if (!range.first.empty())
        {
            if (num_id == range_num.second+1)
            {
                range.second = *id;
                range_num.second = num_id;
                continue;
            }

            if ( !range.second.empty())
            {
                if (range_num.second > range_num.first+1)
                    id_vec.push_back(range.first+"-"+range.second);
                else
                {
                    id_vec.push_back(range.first);
                    id_vec.push_back(range.second);
                }

            }
            else
            {
                id_vec.push_back(range.first);
            }
        }
        range.first = *id;
        range.second.clear();
        range_num.first = num_id;
        range_num.second = num_id;
    }

    if (!range.first.empty())
    {
        if ( !range.second.empty())
        {
            if (range_num.second > range_num.first+1)
                id_vec.push_back(range.first+"-"+range.second);
            else
            {
                    id_vec.push_back(range.first);
                    id_vec.push_back(range.second);
            }
            
        }
        else
        {
            id_vec.push_back(range.first);
        }
    }
    return NStr::Join(id_vec,", ");
}

CRef<objects::CSeqdesc> CAddFluComments::CreateNewComment(const TNumStrSet &ids, const string &taxname)
{
    CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );

    string id_size = NStr::NumericToString(ids.size());
    string id_list = ConcatIds(ids);
    string new_comment = "GenBank Accession Numbers "+id_list+" represent sequences from the "+id_size+" segments of "+taxname;
    new_desc->SetComment(new_comment);

    return new_desc;
}

string CAddFluComments::GetBestLabel(const CBioseq &seq)
{
    string id_str;
    if (!seq.GetId().empty()) 
    {
        const CSeq_id* wid = FindBestChoice(seq.GetId(), CSeq_id::BestRank).GetPointer();
        if (wid) 
            wid->GetLabel(&id_str,CSeq_id::eContent, CSeq_id::fLabel_GeneralDbIsContent);
    }
    return id_str;
}

void CAddFluComments::apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
{
    CRef<CCmdComposite> composite(new CCmdComposite(title)); 

    map<string,TNumStrSet> taxname_to_ids;
    objects::CBioseq_CI b_iter(tse,CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) 
    {
        string taxname;
        FindBioSource(b_iter->GetSeq_entry_Handle(), taxname);
        if (taxname.empty() && b_iter->GetSeq_entry_Handle().HasParentEntry())
            FindBioSource(b_iter->GetSeq_entry_Handle().GetParentEntry(), taxname);

        string id_str = GetBestLabel(*b_iter->GetBioseqCore());     
        if (!taxname.empty() && !id_str.empty())
            taxname_to_ids[taxname].insert(id_str);        
    }

    objects::CBioseq_CI b_iter2(tse, CSeq_inst::eMol_na);
    for ( ; b_iter2 ; ++b_iter2 ) 
    {
        string taxname;
        FindBioSource(b_iter2->GetSeq_entry_Handle(), taxname);
        if (taxname.empty() && b_iter2->GetSeq_entry_Handle().HasParentEntry())
            FindBioSource(b_iter2->GetSeq_entry_Handle().GetParentEntry(), taxname);
        if (!taxname.empty() && taxname_to_ids.find(taxname) != taxname_to_ids.end() && taxname_to_ids[taxname].size() > 1)
        {
            CRef<objects::CSeqdesc> new_desc = CreateNewComment(taxname_to_ids[taxname],taxname);
            composite->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(b_iter2->GetParentEntry(), *new_desc)) );
        }
    }
    cmdProcessor->Execute(composite.GetPointer());
}


void CAddFluComments::AddBioSource(const CBioSource& biosource, string &taxname)
{
    if (biosource.IsSetTaxname() && !biosource.GetTaxname().empty())
    {
        taxname = biosource.GetTaxname();
    }
}

void CAddFluComments::GetDesc(const CSeq_entry& se, string & taxname)  
{

    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) {
        if ((*it)->IsSource()) {
            AddBioSource ((*it)->GetSource(), taxname);
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQDESC_ON_SEQSET (it, se) {
            if ((*it)->IsSource()) {
                AddBioSource ((*it)->GetSource(), taxname);
            }
        }   
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            GetDesc (**it, taxname);
        }
    }
}


void  CAddFluComments::FindBioSource(objects::CSeq_entry_Handle tse, string &taxname)
{
    if (!tse)
        return;
    GetDesc (*(tse.GetCompleteSeq_entry()), taxname);
    CFeat_CI feat (tse, SAnnotSelector(CSeqFeatData::e_Biosrc));
    while (feat) {
        AddBioSource (feat->GetData().GetBiosrc(), taxname); 
        ++feat;
    }
}

END_NCBI_SCOPE
