#ifndef GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_CONF__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_CONF__HPP

/* $Id: layout_conf.hpp 44130 2019-11-02 00:14:51Z rudnev $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
 */

#include <corelib/ncbistr.hpp>
#include <gui/objects/CheckBox.hpp>
#include <gui/objects/ChoiceItem.hpp>
#include <gui/objects/TextBox.hpp>
#include <gui/objects/Choice.hpp>
#include <gui/objects/HiddenSetting.hpp>
#include <gui/objects/Category.hpp>
#include <gui/objects/RangeControl.hpp>
#include <gui/objects/RangeValue.hpp>
#include <gui/objects/Comment.hpp>
#include <gui/objects/TrackConfig.hpp>
#include <gui/objects/TrackConfigSet.hpp>

#include <map>


BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTrackConfigUtils
{

public:
    //typedef vector< CRef<objects::CTrackConfig> >  TTrackConfigSet;

    static CRef<objects::CChoiceItem> CreateChoiceItem(const string& name,
                                              const string& disp_name,
                                              const string& help,
                                              const string& legend_txt)
    {
        CRef<objects::CChoiceItem> item(new objects::CChoiceItem);
        item->SetName() = name;
        item->SetDisplay_name() = disp_name;
        item->SetHelp() = help;
        item->SetLegend_text() = legend_txt;
        return item;
    }


    static CRef<objects::CChoice> CreateChoice(const string& name,
                                          const string& disp_name,
                                          const string& curr_val,
                                          const string& help)
    {
        CRef<objects::CChoice> choice(new objects::CChoice);
        choice->SetName() = name;
        choice->SetDisplay_name() = disp_name;
        choice->SetCurr_value() = curr_val;
        choice->SetHelp() = help;
        return choice;
    }


    static CRef<objects::CCheckBox> CreateCheckBox(const string& name,
                                                   const string& disp_n,
                                                   const string& help_text,
                                                   const string& legend_txt,
                                                   bool value)
    {
        CRef<objects::CCheckBox> cb(new objects::CCheckBox);
        cb->SetName() = name;
        cb->SetValue() = value;
        cb->SetDisplay_name() = disp_n;
        cb->SetHelp() = help_text;
        cb->SetLegend_text() = legend_txt;
        return cb;
    }

    static CRef<objects::CTextBox> CreateTextBox(const string& name,
                                                const string& disp_n,
                                                const string& help_text,
                                                const string& value)
    {
        CRef<objects::CTextBox> tb(new objects::CTextBox);
        tb->SetName(name);
        tb->SetValue(value);
        tb->SetDisplay_name(disp_n);
        tb->SetHelp(help_text);
        tb->SetLegend_text(help_text);
        return tb;
    }

    static CRef<objects::CHiddenSetting> CreateHiddenSetting(const string& name,
                                                             const string& value)
    {
        CRef<objects::CHiddenSetting> hs(new objects::CHiddenSetting);
        hs->SetName() = name;
        hs->SetValue() = value;
        return hs;
    }


    static CRef<objects::CComment> CreateComment(const string& label,
                                                 const string& pos_str)
    {
        CRef<objects::CComment> comment(new objects::CComment);
        comment->SetLabel() = label;
        comment->SetPos_str() = pos_str;
        return comment;
    }


    static CRef<objects::CCategory> CreateCategory(const string& name,
                                                   const string& disp_name,
                                                   const string& help,
                                                   int order)
    {
        CRef<objects::CCategory> cat(new objects::CCategory);
        cat->SetName() = name;
        cat->SetDisplay_name() = disp_name;
        cat->SetHelp() = help;
        cat->SetOrder() = order;
        return cat;
    }

    static CRef<objects::CRangeControl> CreateRangeControl(const string& name,
                                                      const string& disp_name,
                                                      const string& help,
                                                      const string& value_min,
                                                      const string& value_max,
                                                      bool autoscale = true,
                                                      bool inverse = false)
    {
        CRef<objects::CRangeControl> range(new objects::CRangeControl);
        range->SetName() = name;
        range->SetDisplay_name() = disp_name;
        range->SetHelp() = help;
        range->SetValue().SetMin(value_min);
        range->SetValue().SetMax(value_max);
        range->SetValue().SetAutoscale(autoscale);
        range->SetValue().SetInverse(inverse);
        return range;
    }

    // range=min:max|auto 
    // min - float or 'inf' where inf(inity) indicator of no value
    // max - float or 'inf' where inf(inity) indicator of no value
    // auto - optional flag indicate if autoscale should be apply

    static void DecodeValueRange(const string& value_range, string& range_min, string& range_max, bool& range_auto)
    {
        range_min.clear();
        range_max.clear();
        range_auto = true;
        if (value_range.empty()) 
            return;
        vector<string> vals;    
        NStr::Split(value_range, "|", vals, NStr::fSplit_Tokenize);        
        auto val_size = vals.size();
        if (val_size < 2 || val_size > 3)
            return;
        range_min = vals[0];
        range_max = vals[1];
        range_auto = val_size == 3 && vals[2] == "auto";
    }

    static void EncodeValueRange(const string& range_min, const string& range_max, bool range_auto, string& value_range)
    {
        value_range.clear();
        value_range = range_min.empty() ? "inf" : range_min;
        value_range += "|";
        value_range += range_max.empty() ? "inf" : range_max;
        if (range_auto)
            value_range += "|auto";
    }

};


///////////////////////////////////////////////////////////////////////////////
/// data structure holding configuration for a track.
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CCategoryConfig : public CObject
{
public:
    /// group/subgroup settings from TMS
    /// string is group name, int is group order
    typedef map<string, int> TGroupMap;    

    /// first string is group name, second is subgroups name
    typedef pair<string, string> TSubgroupName;

    /// int is subgroup order
    typedef map<TSubgroupName, int> TSubgroupMap;    
    
    typedef map< string, CRef<objects::CCategory> > TSubcategories;

    struct SCategorySettings
    {
        CRef<objects::CCategory>  m_Category;
        TSubcategories            m_Subcategories;

        void AddSubcategory(CRef<objects::CCategory> sub_cat)
            {
                m_Subcategories[sub_cat->GetName()] = sub_cat;
            }
    };

    typedef map<string, SCategorySettings> TCategories;

    void AddCategory(const SCategorySettings& cat)
    {
        m_Categories[cat.m_Category->GetName()] = cat;
    }
    
    void AddSubcategory(const string& cat_name, CRef<objects::CCategory> sub_cat)
    {
        m_Categories[cat_name].m_Subcategories[sub_cat->GetName()] = sub_cat;
    }

    CRef<objects::CCategory> GetCategory(const string& cat_name)
    {
        CRef<objects::CCategory> cat;
        TCategories::const_iterator iter = m_Categories.find(cat_name);
        if (iter != m_Categories.end()) {
            cat = iter->second.m_Category;
        }
        return cat;
    }

    CRef<objects::CCategory>
        GetSubcategory(const string& cat_name, const string& sub_cat)
    {
        CRef<objects::CCategory> cat;
        TCategories::const_iterator iter = m_Categories.find(cat_name);
        if (iter != m_Categories.end()) {
            TSubcategories::const_iterator s_iter =
                iter->second.m_Subcategories.find(sub_cat);
            if (s_iter != iter->second.m_Subcategories.end()) {
                cat = s_iter->second;
            }
        }
        return cat;
    }

private:

    TCategories m_Categories;
};



END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_CONF__HPP
