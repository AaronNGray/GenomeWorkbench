/*  $Id: phylo_tree_query_macro_fns.cpp 39133 2017-08-04 16:09:53Z asztalos $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *      Implementation for tree query execution
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_query_macro_fns.hpp>
#include <gui/widgets/phylo_tree/phylo_selection_set.hpp>
#include <gui/widgets/phylo_tree/tree_model.hpp>

#include <gui/objutils/macro_ex.hpp>



BEGIN_NCBI_SCOPE
using namespace macro;


// Standard javascript events.
// This list is taken from: http://help.dottoro.com/ljfvvdnm.php.
string ITreeMacroFunction::s_JSFilterdKeywords[] = {
    // Java events
    "getClass", 
    "JavaPackage", 
    "JavaObject", 
    "JavaArray", 
    "javaClass", 
    "java", 

    // Character input and clipboard events
    "onbeforecopy",
    "onbeforecut",
    "onbeforepaste",
    "oncopy",
    "oncut",
    "oninput",
    "onkeydown",
    "onkeypress",
    "onkeyup",
    "onpaste",
    "textInput",

    // Load, unload and state events
    "onabort",
    "onbeforeunload",
    "onhashchange",
    "onload",
    "onoffline",
    "ononline",
    "onreadystatechange",
    "onreadystatechange",
    "onreadystatechange",
    "onstop",
    "onunload",

    //Form Events
    "onreset",
    "onsubmit",

    // Mouse Events and scrolling
    "onclick",
    "oncontextmenu",
    "ondblclick",
    "onlosecapture",
    "onmouseenter",
    "onmousedown",
    "onmouseleave",
    "onmousemove",
    "onmouseout",
    "onmouseover",
    "onmouseup",
    "onmousewheel",
    "onscroll",

    //Move and drag events:
    "onmovestart",
    "onmoveend",
    "onmove",
    "ondragstart",
    "ondragleave",
    "ondragenter",
    "ondragover",
    "ondragend",
    "ondrag",
    "ondrop",

    //Resize events:
    "onresizestart",
    "onresizeend",
    "onresize",

    //Activation and focus events:
    "onactivate",
    "onbeforeactivate",
    "onbeforedeactivate",
    "onbeforeeditfocus",
    "onblur",
    "ondeactivate",
    "onfocusout",
    "onfocusin",
    "onfocus",

    //Selection events:
    "oncontrolselect",
    "onselectionchange",
    "onselectstart",

    //Print and help events:
    "onafterprint",
    "onbeforeprint",
    "onhelp",

    //Error events:
    "onerrorupdate",
    "onerror",

    //DataBound events:
    "onafterupdate",
    "onbeforeupdate",
    "oncellchange",
    "ondataavailable",
    "ondatasetchanged",
    "ondatasetcomplete",
    "onrowenter",
    "onrowexit",
    "onrowsdelete",
    "onrowsinserted",

    //Marquee events:
    "onbounce",
    "onfinish",
    "onstart",

    //Change events:
    "onchange",
    "onfilterchange",
    "onpropertychange",

    //Search event:
    "onsearch",

    //Cross-document communication event:
    "onmessage",

    //Additional events:
    "CheckboxStateChange",
    "DOMActivate",
    "DOMAttrModified",
    "DOMCharacterDataModified",
    "DOMFocusIn",
    "DOMFocusOut",
    "DOMMouseScroll",
    "DOMNodeInserted",
    "DOMNodeInsertedIntoDocument",
    "DOMNodeRemovedFromDocument",
    "DOMNodeRemoved",
    "DOMSubtreeModified",
    "dragdrop",
    "dragexit",
    "draggesture",
    "overflow",
    "overflowchanged",
    "RadioStateChange",
    "underflow"
};

std::pair<string, string> ITreeMacroFunction::s_JSSwapKeywords[] = {
    { "<", " less "},
    { "&lt", " less "},
    { "&LT", " less "},
    { "%3c", " less "},
    { "%3C", " less "},
    { "&#60", " less "},
    { "&#x3c", " less "},
    { "&#x3C", " less "},
    { "&#x003c", " less "},
    { "&#x003C", " less "},
    { "&#x00003c", " less "},
    { "&#x00003C", " less "},
    { ">", " greater " },
    { "&gt", " greater "},
    { "&GT", " greater "},
    { "%3e", " greater "},
    { "%3E", " greater "},
    { "&#62", " greater "},
    { "&#x3e", " greater "},
    { "&#x3E", " greater "},
    { "&#x003e", " greater "},
    { "&#x003E", " greater "},
    { "&#x00003e", " greater "},
    { "&#x00003E", " greater "}
};

// JSFilter
// Find and replace lists of words in a 'blacklist' from 'str'
// 
// This works with two lists. For the first list we replace each
// word in the list with a string of x's of the same length and
// a blank on either end so  'onmove' becomes ' xxxx ', 'java' becomes ' xx '
// We could just delete the string but this approach shows the caller where
// strings were removed And prevents the removal from generating new strings,
// e.g. if we remove 'onmove' from 'ononmovemove' we still get 'onmove'.
//
// The second list is a list of replacement values. We want to maintain
// The < and > values but change them to english to prevent valid html
// from being injected. I have tried to include all valid encoding variations.
//
// This is a brute force approach to the problem. There are almost
// certainly more efficient approaches for removing a large set of
// pre-defined, seldom-occuring strings from an arbitrary string value.
string ITreeMacroFunction::JSFilter(const string& str)
{
    string result = str;

    // Loop through all the keywords to be filtered out, replacing
    // all instances with a same-length string of the form ' xxx.. '.
    
    // Note that the lexical values of some handlers occur as substrings
    // in other handlers, e.g. ondrag and ondragstart. For a more complete
    // result, we should put the longer terms ahead of the shorter ones.
    // If we don't terms like 'ondragstart' will get converted to
    // ' xxxx start' instead of ' xxxxxxxxx ' 
    for (string& keyword : s_JSFilterdKeywords) {
        size_t pos = NStr::Find(result, keyword, NStr::eNocase);
        while (pos != NPOS) {
            string substitution(keyword.size(), 'x');
            substitution[0] = ' ';
            substitution[substitution.length()-1] = ' ';
            result.replace(pos, substitution.length(), substitution);

            pos = NStr::Find(result, keyword, NStr::eNocase);
        }
    }

    for (std::pair<string,string>& keyword : s_JSSwapKeywords) {
        NStr::ReplaceInPlace(result, keyword.first, keyword.second);
    }

    return result;
}

void CMacroFunction_SetColor::TheFunction() 
{
    if (m_Args.size() == 1) {
        string color_str = m_Args[0]->GetString();

        try {
            CRgbaColor c(color_str);
            color_str = "[" + c.ToString(true, true) + "]";
        }
        // If color string not valid:
        catch (const CException&) {
            NCBI_THROW(CMacroExecException, eWrongArguments,
                "Cannot convert: " + color_str + " to a color");
        }
        m_Node->GetValue().SetFeature(
            m_Tree->GetFeatureDict(), "$NODE_COLOR", color_str);
        m_Node->GetValue().InitFeatures(
            m_Tree->GetFeatureDict(), m_Tree->GetColorTable());
    }
    else {
        NCBI_THROW(CMacroExecException, eWrongArguments,
            "SetColor requires 1 argument but received: " +
            NStr::NumericToString(m_Args.size()));
    }
}

void CMacroFunction_SetSize::TheFunction() 
{
    // Check type - throw exception/ log error?
    if (m_Args.size() == 1) {
        int radius;
        if (m_Args[0]->GetDataType() == QueryValueType::eInt ||
            m_Args[0]->GetDataType() == QueryValueType::eStringInt ||
            m_Args[0]->GetDataType() == QueryValueType::eFieldInt) {
                radius = m_Args[0]->GetInt();
        }
        else if (m_Args[0]->GetDataType() == QueryValueType::eFloat ||
                 m_Args[0]->GetDataType() == QueryValueType::eStringFloat ||
                 m_Args[0]->GetDataType() == QueryValueType::eFieldFloat) {
                     radius = int(m_Args[0]->GetDouble());
        }
        else {
            string data_type = QueryValueType::GetTypeAsString(m_Args[0]->GetDataType());

            NCBI_THROW(CMacroExecException, eWrongArguments,
                "SetSize expected an integer but was passed a: " + data_type);
        }

        string str_radius = NStr::IntToString(radius);

        m_Node->GetValue().SetFeature(
            m_Tree->GetFeatureDict(), "$NODE_SIZE", str_radius);
        m_Node->GetValue().InitFeatures(
            m_Tree->GetFeatureDict(), m_Tree->GetColorTable());
    }
    else {
        NCBI_THROW(CMacroExecException, eWrongArguments,
            "SetSize requires 1 argument but received: " +
            NStr::NumericToString(m_Args.size()));
    }
}

void CMacroFunction_SetMarker::TheFunction() 
{
    if (m_Args.size() == 1) {
        string marker_str = m_Args[0]->GetString();

        m_Node->GetValue().SetFeature(
            m_Tree->GetFeatureDict(), "marker", marker_str);
        m_Node->GetValue().InitFeatures(
            m_Tree->GetFeatureDict(), m_Tree->GetColorTable());
    }
    else if (m_Args.size() == 2) {
        string color_str = m_Args[0]->GetString();
        int radius = m_Args[1]->GetInt();
        string radius_str = NStr::IntToString(radius);
        string marker_str = color_str + " size=" + radius_str;

        m_Node->GetValue().SetFeature(
            m_Tree->GetFeatureDict(), "marker", marker_str);
        m_Node->GetValue().InitFeatures(
            m_Tree->GetFeatureDict(), m_Tree->GetColorTable());
    }
}

void CMacroFunction_SetBounding::TheFunction() 
{
    if (m_Args.size() == 1) {
        string boundary_str = m_Args[0]->GetString();

        m_Node->GetValue().SetFeature(
            m_Tree->GetFeatureDict(), "$NODE_BOUNDED", boundary_str);
        m_Node->GetValue().InitFeatures(
            m_Tree->GetFeatureDict(), m_Tree->GetColorTable());
    }
}



void CMacroFunction_SetProperty::TheFunction()
{
    if (m_Args.size() == 2) {
        string feature_name;
        if (m_Args[0]->GetDataType() == QueryValueType::eString ||
            m_Args[0]->GetDataType() == QueryValueType::eFieldString) {

            feature_name = m_Args[0]->GetString();
            if (feature_name == "") {                
                NCBI_THROW(CMacroExecException, eWrongArguments,
                    "Blank feature name passed to SetProperty: ");
            }
        }
        else {
            string data_type = QueryValueType::GetTypeAsString(m_Args[0]->GetDataType());

            NCBI_THROW(CMacroExecException, eWrongArguments,
                "IsNull expected a string but was passed a: " + data_type);
        }

        m_Args[1]->PromoteTo(QueryValueType::eString);

        // Guard against possible java/javascript attacks (filter any event handlers
        // and <,> symbols from the text)
        string filtered_property = ITreeMacroFunction::JSFilter(m_Args[1]->GetString());
        m_Node->GetValue().SetFeature(
            m_Tree->GetFeatureDict(), feature_name, filtered_property);
        m_Node->GetValue().InitFeatures(
            m_Tree->GetFeatureDict(), m_Tree->GetColorTable());
    }
    else {
        NCBI_THROW(CMacroExecException, eWrongArguments,
            "SetProperty requires 2 arguments but received: " +
            NStr::NumericToString(m_Args.size()));
    }
}

class CVisitorAccumulateProperty
{
public:
    CVisitorAccumulateProperty(const string& feature_name, bool include_node, CPhyloTree::TTreeIdx main_node_idx)
        : m_FeatName(feature_name)
        , m_IncludeNode(include_node)
        , m_SumInt(0)
        , m_SumDbl(0.0)
        , m_SumIsInteger(true)
        , m_MainNode(main_node_idx) {}

    ETreeTraverseCode operator()(CPhyloTree&  tree, CPhyloTree::TTreeIdx node_idx, int delta)
    {
        if ((delta == 0 || delta == 1) && (m_IncludeNode || node_idx != m_MainNode)) {
            CPhyloTreeNode& node = tree[node_idx];

            string feature_value;
            auto feat_list = node.GetValue().GetBioTreeFeatureList();
            bool has_value = feat_list.GetFeatureValue(tree.GetFeatureDict().GetId(m_FeatName), feature_value);
            if (has_value && !feature_value.empty()) {
                try {
                    Int8 value = NStr::StringToInt8(feature_value, NStr::fConvErr_NoThrow);
                    if (errno == 0) {
                        m_SumInt += value;
                    }
                    else {
                        double value = NStr::StringToDouble(feature_value);
                        m_SumIsInteger = false;
                        m_SumDbl += value;
                    }
                }
                catch (const CException& e) {
                    NCBI_THROW(CMacroExecException, eInvalidData, 
                        "Summation of existing property values failed: " + e.GetMsg());
                }
            }
        }
            
        return eTreeTraverse;
    }

    bool IsSumInteger() const { return m_SumIsInteger; }
    Int8 GetInteger() const { return m_SumInt; }
    double GetDouble() const { return m_SumDbl; }
private:
    string m_FeatName;
    bool m_IncludeNode;

    Int8 m_SumInt;
    double m_SumDbl;
    bool m_SumIsInteger;
    CPhyloTree::TTreeIdx m_MainNode;

};

void CMacroFunction_SubtreeSum::TheFunction()
{
    if (m_Args.size() != 2) {
        NCBI_THROW(CMacroExecException, eWrongArguments,
            "SubtreeSum requires 2 arguments but received: " +
            NStr::NumericToString(m_Args.size()));
    }

    if (m_Args[0]->GetDataType() != QueryValueType::eString &&
        m_Args[0]->GetDataType() != QueryValueType::eFieldString) {

        string data_type = QueryValueType::GetTypeAsString(m_Args[0]->GetDataType());
        NCBI_THROW(CMacroExecException, eWrongArguments,
            "SubtreeSum expected a string but was passed a: " + data_type);
    } 

    const string& existing_feature = m_Args[0]->GetString();

    if (existing_feature.empty()) {
        NCBI_THROW(CMacroExecException, eWrongArguments,
            "Blank existing feature name passed to SubtreeSum: ");
    }

    if (!m_Tree->GetFeatureDict().HasFeature(existing_feature)) {
        NCBI_THROW(CMacroExecException, eWrongArguments,
            existing_feature + " is not a property of the tree nodes.");
    }

    bool include_node = false;
    if (m_Args[1]->GetDataType() == QueryValueType::eBool ||
        m_Args[1]->GetDataType() == QueryValueType::eFieldBool) {

        include_node = m_Args[1]->GetBool();
    }
    else {
        string data_type = QueryValueType::GetTypeAsString(m_Args[1]->GetDataType());
        NCBI_THROW(CMacroExecException, eWrongArguments,
            "SubtreeSum expected a boolean but was passed a: " + data_type);
    }

    auto current_node_idx = m_Tree->FindNodeById(m_Node->GetValue().GetId());
    CVisitorAccumulateProperty visitor = TreeDepthFirst(*m_Tree, current_node_idx, 
        CVisitorAccumulateProperty(existing_feature, include_node, current_node_idx));

    string new_value;
    if (visitor.IsSumInteger()) {
        new_value = NStr::Int8ToString(visitor.GetInteger());
    }
    else {
        double sum_double = visitor.GetDouble();
        sum_double += static_cast<double>(visitor.GetInteger());
        new_value = NStr::DoubleToString(sum_double);
    }

    m_Result->SetString(new_value);
}

void CMacroFunction_CollapseNode::TheFunction()
{
    // Only collapse if node has children and they are hidden
    if (m_Node->NumChildren() > 0 && 
        m_Node->GetValue().GetDisplayChildren() == CPhyloNodeData::eShowChildren) {
            m_Node->GetValue().SetFeature(m_Tree->GetFeatureDict(),
                "$NODE_COLLAPSED", "1");
            m_Node->GetValue().InitFeatures(
                m_Tree->GetFeatureDict(), m_Tree->GetColorTable());
    }
}

void CMacroFunction_ExpandNode::TheFunction()
{
    // Don't expand it (update $NODE_COLLAPSED property) unless the node is 
    // currently collapsed
    if (m_Node->GetValue().GetDisplayChildren() == CPhyloNodeData::eHideChildren) {
        m_Node->GetValue().SetFeature(m_Tree->GetFeatureDict(),
            "$NODE_COLLAPSED", "0");
        m_Node->GetValue().InitFeatures(
            m_Tree->GetFeatureDict(), m_Tree->GetColorTable());
    }
}

void CMacroFunction_AddToSelectionSet::TheFunction()
{
    CPhyloSelectionSetMgr& sel_sets = m_Tree->GetSelectionSets();

    if (m_Args.size() != 1 && m_Args.size() != 2) {
        NCBI_THROW(CMacroExecException, eWrongArguments,
            "AddToSelectionSet requires 1 argument (name) or 2 (name and color) arguments but received: " +
            NStr::NumericToString(m_Args.size()));
    }

    string set_name;
    if (m_Args[0]->GetDataType() != QueryValueType::eString &&
        m_Args[0]->GetDataType() != QueryValueType::eFieldString) {
            string data_type = QueryValueType::GetTypeAsString(m_Args[0]->GetDataType());

            NCBI_THROW(CMacroExecException, eWrongArguments,
                "AddToSelectionSet expected first parameter to be a string but was passed a: " + data_type);
    }
    else {
        set_name = m_Args[0]->GetString();
        if (set_name == "") {
            NCBI_THROW(CMacroExecException, eWrongArguments,
                "Blank set name passed to AddToSelectionSet: ");
        }
    }

    if (m_Args.size() == 2) {
        if (m_Args[1]->GetDataType() != QueryValueType::eString &&
            m_Args[1]->GetDataType() != QueryValueType::eFieldString) {
            string data_type = QueryValueType::GetTypeAsString(m_Args[0]->GetDataType());

            NCBI_THROW(CMacroExecException, eWrongArguments,
                "AddToSelectionSet expected second parameter to be a color string but was passed a: " + data_type);
        }

        CRgbaColor c;

        try {
            c.FromString(m_Args[1]->GetString());
        }
        // If color string not valid:
        catch (const CException&) {
            NCBI_THROW(CMacroExecException, eWrongArguments,
                "Cannot convert: " + m_Args[1]->GetString() + " to a color");
        }

        CPhyloSelectionSet& s = sel_sets.AddSet(set_name, c);
        x_AddSetInfo(s);
    }
    else {
        CPhyloSelectionSet& s = sel_sets.AddSet(set_name);
        x_AddSetInfo(s);
    }
}

void CMacroFunction_AddToSelectionSet::x_AddSetInfo(CPhyloSelectionSet& s)
{
    s.GetSelectionSet().push_back(m_Node->GetValue().GetId());
    s.SetSelected(true);
}

void CMacroFunction_DeleteNode::TheFunction()
{
    if (!m_Args.empty()) {
        return;
    }

    auto parent_idx = (m_Node->HasParent()) ? m_Node->GetParent() : CPhyloTree::Null();
    if (parent_idx == CPhyloTree::Null()) {
        // the root node can not be removed
        return;
    }

    for (auto&&  it = m_Node->SubNodeBegin(); it != m_Node->SubNodeEnd(); it++)  {
        m_Tree->AddChild(parent_idx, *it);
    }

    auto current_node_idx = m_Tree->FindNodeById(m_Node->GetValue().GetId());
    m_Node->ClearConnections();
    m_Tree->RemoveChild(parent_idx, current_node_idx);
    m_Node->GetValue().SetId(CPhyloNodeData::TID(-1));
}

void CMacroFunction_IsLeaf::TheFunction() 
{
    bool is_leaf = m_Node->IsLeaf();
    m_Result->SetBool(is_leaf);
    m_Result->SetValue(is_leaf);
    m_Result->SetDataType(QueryValueType::eBool);
}

void CMacroFunction_NumChildren::TheFunction()
{
    m_Result->SetInt(m_Node->NumChildren());
    m_Result->SetDataType(QueryValueType::eInt);
}

void CMacroFunction_BranchDepth::TheFunction()
{
    m_Result->SetInt(m_Result->GetBranchDepth());
    m_Result->SetDataType(QueryValueType::eInt);
}

void CMacroFunction_MaxChildBranchDepth::TheFunction()
{
    m_Result->SetInt(m_Result->GetMaxChildBranchDepth());
    m_Result->SetDataType(QueryValueType::eInt);
}

void CMacroFunction_IsNull::TheFunction()
{
    if (m_Args.size() == 1) {
        string feature_name;
        bool is_null = true;

        if (m_Args[0]->GetDataType() == QueryValueType::eString ||           
            m_Args[0]->GetDataType() == QueryValueType::eFieldString) {

            feature_name = m_Args[0]->GetString();
            if (m_Tree->GetFeatureDict().HasFeature(feature_name)) {
                string feat_value;
                is_null = !m_Node->GetValue().GetBioTreeFeatureList().GetFeatureValue(
                    m_Tree->GetFeatureDict().GetId(feature_name), feat_value);
            }
        }
        else {
            string data_type = QueryValueType::GetTypeAsString(m_Args[0]->GetDataType());

            NCBI_THROW(CMacroExecException, eWrongArguments,
                "IsNull expected a string but was passed a: " + data_type);
        }

        m_Result->SetBool(is_null);
        m_Result->SetValue(is_null);
        m_Result->SetDataType(QueryValueType::eBool);
    }
    else {
        NCBI_THROW(CMacroExecException, eWrongArguments,
            "IsNull requires 1 argument but received: " +
            NStr::NumericToString(m_Args.size()));
    }
}

void CMacroFunction_Date::TheFunction()
{
    if (m_Args.size() == 1 || m_Args.size() == 2) {
        if (m_Args[0]->GetDataType() == QueryValueType::eString ||
            m_Args[0]->GetDataType() == QueryValueType::eFieldString) {

            string format = "";            
            if (m_Args.size() == 2) {
                if (m_Args[1]->GetDataType() == QueryValueType::eString ||
                    m_Args[1]->GetDataType() == QueryValueType::eFieldString) {
                        format = m_Args[1]->GetString();
                }
                else {
                    string data_type = QueryValueType::GetTypeAsString(m_Args[0]->GetDataType());
                    NCBI_THROW(CMacroExecException, eWrongArguments,
                        "Date expected a date format string but was passed a: " + data_type);
                }
            }

            CTimeFormat tf;
            if (format != "")
                tf.SetFormat(format, CTimeFormat::fFormat_Simple | CTimeFormat::fMatch_Weak);
            else
                tf.SetFormat("M/D/Y h:m:s", CTimeFormat::fFormat_Simple | CTimeFormat::fMatch_Weak);

            string str_date = m_Args[0]->GetString();
            // If the string is blank we have essentially a null value.  generally we would 
            // want the full query to evaluate to false, but that is up to the user who needs
            // to check with IsLeaf or IsNull or  not field="".
            if (str_date == "") {
                m_Result->SetInt(0);
                m_Result->SetDataType(QueryValueType::eInt);
            }
            else {
                // Get time in seconds since midnight (00:00:00), January 1, 1970 UTC 
                // This may be a 64 bit type.
                CTime date(str_date, tf);
                time_t t = date.GetTimeT();

                // 32 bit types will work until 2038.  If it is 2037 and you are reading this,
                // please make the necessary changes.
                m_Result->SetInt((int)t);
                m_Result->SetDataType(QueryValueType::eInt);
            }
        }
        else {
            string data_type = QueryValueType::GetTypeAsString(m_Args[0]->GetDataType());
            NCBI_THROW(CMacroExecException, eWrongArguments,
                "Date expected a date string but was passed a: " + data_type);
        }
    }
    else {
        NCBI_THROW(CMacroExecException, eWrongArguments,
            "Date requires 1 or 2 arguments but received: " +
            NStr::NumericToString(m_Args.size()));
    }
}


END_NCBI_SCOPE
