/*  $Id: seq_text_feature_list.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
 * Authors:  Colleen Bollin
 *
 * File Description:
 *    Implements an Fl_Choice populated with options for feature display in the
 *    Sequence Text Viewer
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_text/seq_text_feature_list.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


CSeqTextFeatureList::CSeqTextFeatureList(int x, int y,int w, int h, const char *label) :
    CChoice (x, y, w, h, label)
{
    item_names.clear();
    item_names.push_back("None");
    add ("None");

    vector<string> names;

    CSeqFeatData::GetFeatList()->GetDescriptions(names, true);

    ITERATE(vector<string>, iter, names) {
        if (*iter == "All") {
            continue;
        }
        item_names.push_back(iter->c_str());
        add(iter->c_str() );
    }
    value(3);
}


void CSeqTextFeatureList::SetAvailableFeatures(CSeqTextDataSource::TAvailableSubtypeVector& avail_subtypes)
{
    int num_options = 1, cds_position = 0;
    unsigned int i;
    //bool has_cds = false;
    CFeatListItem config_item;

    clear();

    item_names.clear();
    item_names.push_back("None");

    add ("None");

    const CFeatList* cfg_list = CSeqFeatData::GetFeatList();

    for (i = CSeqFeatData::eSubtype_bad; i < CSeqFeatData::eSubtype_any; i++) {
        if (avail_subtypes [i]) {
            cfg_list->GetItemBySubtype (i, config_item);
            string desc = config_item.GetDescription();
            if (desc == "CDS") {
                cds_position = num_options;
            }
            item_names.push_back(desc.c_str());
            add (desc.c_str());
            num_options ++;
        }
    }
}


bool CSeqTextFeatureList::GetTypeSubType(int& type, int& subtype) const
{
    string desc = text();
    return CSeqFeatData::GetFeatList()->GetTypeSubType(desc, type, subtype);
}


int CSeqTextFeatureList::SetSubType(int subtype)
{
    unsigned int i;
    int position = 0, actual_subtype = 0;
    CFeatListItem config_item;

    CSeqFeatData::GetFeatList()->GetItemBySubtype (subtype, config_item);
    string desc = config_item.GetDescription();

    for (i=1; i < item_names.size() && position == 0; i++) {
       if (NStr::EqualNocase(desc, item_names[i])) {
           position = i;
           actual_subtype = subtype;
       }
    }
    value(position);
    return actual_subtype;
}


END_NCBI_SCOPE
