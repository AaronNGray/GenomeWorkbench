/*  $Id: feature_types_dlg.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/seq/feature_types_dlg.hpp>

#include <gui/widgets/fl/browser.hpp>
#include <gui/widgets/fl/dialog_back.hpp>
#include <gui/widgets/fl/choice.hpp>
#include <gui/widgets/fl/return_button.hpp>
#include <gui/widgets/fl/button.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#include "feature_types_dlg_.cpp"

///////////////////////////////////////////////////////////////////////////////
/// CFeatureTypesParser


bool CFeatureTypesParser::StringToFeatTypes(const string& text,
                                            TFeatTypeItemSet& feat_types)
{
    static const string kFeatSeparators = ",;";

    const CFeatList* feat_list = CSeqFeatData::GetFeatList();
    _ASSERT(feat_list);

    m_Text = text;
    m_ErrorFrom = m_ErrorLen = -1;

    feat_types.clear();

    // parse user input in m_FeatTypesCombo and extract features types
    list<string> values;
    vector<SIZE_TYPE>   positions;
    NStr::Split(m_Text, kFeatSeparators, values, NStr::eMergeDelims, &positions);

    int i = 0;
    ITERATE(list<string>, it, values)   {
        string descr = NStr::TruncateSpaces(*it);

        if( ! descr.empty())    {       // skip spaces
            TFeatTypeItem item; //TODO performance overhead inflicted by CFeatList
            if(feat_list->GetItemByDescription(descr, item))   {
                feat_types.insert(item);
                i++;
            } else {
                // error
                string token = *it;
                int offset = token.find(descr);
                m_ErrorFrom = positions[i] + offset;
                m_ErrorLen = descr.size();
                return false;
            }
        }
    }
    return true;
}


void CFeatureTypesParser::GetErrorPosition(int& from, int& to) const
{
    from = m_ErrorFrom;
    to = m_ErrorLen;
}


string CFeatureTypesParser::GetErrorToken() const
{
    _ASSERT(m_ErrorFrom >= 0  &&  m_ErrorFrom < (int) m_Text.size()  &&
            m_ErrorLen >= 0  &&  m_ErrorLen + m_ErrorFrom <= (int) m_Text.size());

    return m_Text.substr(m_ErrorFrom, m_ErrorLen);
}


string CFeatureTypesParser::FeatTypesToString(const TFeatTypeItemSet& feat_types)
{
    string s_feat_types;
    const string s_sep(", ");

    ITERATE(TFeatTypeItemSet, it, feat_types)  {
        s_feat_types += it->GetDescription() + s_sep;
    }
    if(! s_feat_types.empty())   {
        // truncate trailing ", "
        s_feat_types.resize(s_feat_types.size() - s_sep.size());
    }
    return s_feat_types;
}


///////////////////////////////////////////////////////////////////////////////
/// CFeatureTypesBrowser
CFeatureTypesBrowser::CFeatureTypesBrowser()
: CCheckBrowser()
{
    x_Init();
}


CFeatureTypesBrowser::CFeatureTypesBrowser(int x, int y, int w, int h,
                                           const char* label)
: CCheckBrowser(x, y, w, h, label)
{
    x_Init();
}


void CFeatureTypesBrowser::x_Init()
{
    callback(x_Clicked, this);
    when(FL_WHEN_CHANGED);

    const CFeatList* feat_list = CSeqFeatData::GetFeatList();
    ITERATE(CFeatList, ft_it, *feat_list) {
        const CFeatListItem& item = *ft_it;
        string desc = item.GetDescription();
        int feat_type = item.GetType();
        // indent subordinate types
        if (item.GetSubtype() != CSeqFeatData::eSubtype_any  &&
            (feat_type == CSeqFeatData::e_Prot ||
                feat_type == CSeqFeatData::e_Rna  ||
                feat_type == CSeqFeatData::e_Imp)) {
            desc = "    " + desc;
        }
        Add(desc.c_str(), (void *) &item);
    }
}


void CFeatureTypesBrowser::Select(const TFeatTypeItemSet& feat_types)
{
    // iterate by all items
    bool master_check = false;

    for(int i = 1; i <= size(); i++ )   {
        CFeatListItem* item = static_cast<CFeatListItem*>(data(i));
        bool check = (feat_types.find(*item) != feat_types.end());
        if(item->GetSubtype() == CSeqFeatData::eSubtype_any) {
            master_check = check;
        }

        // select item is selected or its master is selected
        SetCheck(i, check || master_check);
    }
}


void CFeatureTypesBrowser::GetSelected(TFeatTypeItemSet& feat_types)
{
    bool master_check = false;
    for(int i = 1; i <= size(); i++ )   {
        bool check = GetCheck(i);
        bool add = check;

        CFeatListItem* item = static_cast<CFeatListItem*>(data(i));
        if(item->GetSubtype() == CSeqFeatData::eSubtype_any) {
            master_check = check;
        } else {
            add = check &&  ! master_check;
        }
        if(add) {
            feat_types.insert(*item);
        }
    }
}


void CFeatureTypesBrowser::x_Clicked(Fl_Widget *w, void *d)
{
    _ASSERT(w);
    ((CFeatureTypesBrowser*) w)->x_OnClicked();
}


void CFeatureTypesBrowser::x_OnClicked()
{
    int check_index = GetLastChecked(); // 1-based
    if(check_index > 0) {
        bool val = GetCheck(check_index);
        CFeatListItem *feat = static_cast<CFeatListItem*>(data(check_index));
        _ASSERT(feat != NULL);

        if(feat->GetSubtype() == CSeqFeatData::eSubtype_any) {
            // checked some master item
            if(feat->GetType() == CSeqFeatData::e_not_set)  {
                // checked "All" item, make all items match this one.
                for (int r = check_index +1; r <= size(); ++r) {
                    SetCheck(r, val);
                }
            } else {
                // checked a fetuare type item that has subitmes (subtypes)
                // make all items with this feature type match this one.
                for (int r = check_index + 1; r <= size(); ++r ) {
                    CFeatListItem *item = static_cast<CFeatListItem*>(data(r));
                    if (item->GetType() == feat->GetType())    {
                        SetCheck(r, val);
                    }
                }
            }
        } else {
            // clicked on a child item
            if( ! val)  {
                for( int r = check_index - 1; r >= 1; r--)   {
                    CFeatListItem *item = static_cast<CFeatListItem*>(data(r));
                    if(item->GetType() == feat->GetType()  &&
                        item->GetSubtype() == CSeqFeatData::eSubtype_any)    {
                        // master item for this feature type
                        SetCheck(r, false);
                        break;
                    } else if(item->GetType() != feat->GetType())   {
                        // master has not been found - there is no master
                        break;
                    }
                }
            }
        }

        if( ! val)   {
            /// an item was unchecked - uncheck "All" as well
            CFeatListItem *item = static_cast<CFeatListItem*>(data(1));
            if(item->GetType() == CSeqFeatData::e_not_set)  {
                SetCheck(1, false);
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CFeatureTypesDlg
CFeatureTypesDlg::CFeatureTypesDlg(const TFeatTypeItemSet& feat_types, const string& title)
{
    m_Window.reset(x_CreateWindow());

    SetTitle(title);
    m_Browser->Select(feat_types);
}


void CFeatureTypesDlg::GetSelected(TFeatTypeItemSet& subtypes)
{
    m_Browser->GetSelected(subtypes);
}


void CFeatureTypesDlg::x_OnCheckAll()
{
    m_Browser->CheckAll(true);
    m_Browser->redraw();
}


void CFeatureTypesDlg::x_OnClearAll()
{
    m_Browser->CheckAll(false);
    m_Browser->redraw();
}


void    CFeatureTypesDlg::x_OnOK()
{
    CDialog::x_OnOK();
}


void    CFeatureTypesDlg::x_OnCancel()
{
    CDialog::x_OnCancel();
}


END_NCBI_SCOPE
