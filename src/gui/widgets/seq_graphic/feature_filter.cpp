/*  $Id:
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *      Implementation for feature filter query execution
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiobj.hpp>
#include <corelib/ncbistd.hpp>
#include <gui/widgets/seq_graphic/feature_filter.hpp>
#include <objects/seqfeat/Variation_ref.hpp>
#include <objects/seqfeat/Phenotype.hpp>
#include <objects/seqfeat/Clone_ref.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>


BEGIN_NCBI_SCOPE

static const char* kVariantQuality = "variant_quality";
static const char* kPilot = "pilot";
static const char* kClinicalAssertion = "clinical_assertion";
static const char* kConcordant = "concordant";
static const char* kDiscordant = "discordant";
static const char* kSamplesetType = "sampleset_type";
static const char* kValidationStatus = "validation_status";

///////////////////////////////////////////////////////////////////////////////
/// class CFeatureFilter implementation
///
CFeatureFilter::CFeatureFilter() : m_NeedFiltering(false)
{
}


void CFeatureFilter::Init(const string& filter)
{
    if (filter.empty()) return;

    AddId("len", CQueryParseNode::eIntConst);
    AddId("label", CQueryParseNode::eString);
    AddId(kVariantQuality, CQueryParseNode::eString);
    AddId(kPilot, CQueryParseNode::eString);
    AddId("allele_type", CQueryParseNode::eString);
    AddId("variant_type", CQueryParseNode::eString);
    AddId(kClinicalAssertion, CQueryParseNode::eString);
    AddId(kConcordant, CQueryParseNode::eBoolConst);
    AddId(kDiscordant, CQueryParseNode::eBoolConst);
    AddId(kSamplesetType, CQueryParseNode::eString);
    AddId(kValidationStatus, CQueryParseNode::eString);

    string filter_str = filter;
    NStr::ReplaceInPlace(filter_str, " eq ", "=");
    NStr::ReplaceInPlace(filter_str, " gt ", ">");
    NStr::ReplaceInPlace(filter_str, " ge ", ">=");
    NStr::ReplaceInPlace(filter_str, " lt ", "<");
    NStr::ReplaceInPlace(filter_str, " le ", "<=");
    try {
        // If query tree parsing is case sensitive, keywords like AND and LIKE
        // have to be uppercase.  For tree queries, cas sensitivity will just
        // refer to string comparisons for each node.
        m_Qtree.Parse(filter_str.c_str(), CQueryParseTree::eCaseInsensitive,
            CQueryParseTree::eSyntaxCheck);

        //CNcbiOstrstream  strstrm;
        //m_Qtree.Print(strstrm);

        //// Logging merges lines (even if i call 
        //// UnsetDiagPostFlag(eDPF_PreMergeLines/eDPF_MergeLines); 
        //LOG_POST(Info << "Parsed Query: " << filter_str);
        //vector<string> arr;
        //string s = (string)CNcbiOstrstreamToString(strstrm);
        //NStr::Split(s, "\n", arr);
        //for (size_t i=0; i<arr.size(); ++i) {
        //    LOG_POST(Info << arr[i]);
        //}

        // do preprocessing/initialization/validation ob query tree
        CSGQueryNodePreprocessor pre_exec(&m_IdDict);
        TreeDepthFirstTraverse(*m_Qtree.GetQueryTree(), pre_exec);

        m_NeedFiltering = true;

    } catch (CFeatFilterQueryException& e) {
        LOG_POST(Info << "Error parsing query: " << e.GetMsg());
        return;
    }

    // Logical operators:
    AddFunc(CQueryParseNode::eAnd, new CSGQueryFunctionLogic());
    AddFunc(CQueryParseNode::eOr, new CSGQueryFunctionLogic());
    AddFunc(CQueryParseNode::eSub, new CSGQueryFunctionLogic());
    AddFunc(CQueryParseNode::eXor, new CSGQueryFunctionLogic());
    AddFunc(CQueryParseNode::eNot, new CSGQueryFunctionLogic());

    // Constants:
    AddFunc(CQueryParseNode::eIntConst, new CSGQueryFunctionValue());
    AddFunc(CQueryParseNode::eFloatConst, new CSGQueryFunctionValue());
    AddFunc(CQueryParseNode::eBoolConst, new CSGQueryFunctionValue());
    AddFunc(CQueryParseNode::eString, new CSGQueryFunctionValue());

    // Comparison operators:
    AddFunc(CQueryParseNode::eEQ, new CSGQueryFunctionCompare());
    AddFunc(CQueryParseNode::eGT, new CSGQueryFunctionCompare());
    AddFunc(CQueryParseNode::eGE, new CSGQueryFunctionCompare());
    AddFunc(CQueryParseNode::eLT, new CSGQueryFunctionCompare());
    AddFunc(CQueryParseNode::eLE, new CSGQueryFunctionCompare());
    AddFunc(CQueryParseNode::eIn, new CSGQueryFunctionCompare());
    AddFunc(CQueryParseNode::eBetween, new CSGQueryFunctionCompare());
    AddFunc(CQueryParseNode::eLike, new CSGQueryFunctionCompare());

}


bool CFeatureFilter::Pass(const objects::CMappedFeat* feat)
{
    m_EvalFeat = feat;
    try {
        Evaluate(m_Qtree);
        CQueryParseTree::TNode* top_node = m_Qtree.GetQueryTree();
        CSGQueryNodeValue*  v =
            dynamic_cast<CSGQueryNodeValue*>((*top_node)->GetUserObject());
        _ASSERT(v);
        if (v->GetDataType() == CQueryParseNode::eBoolConst  &&
            v->m_Bool) {
            return true;
        }

    } catch (CQueryParseException&) {
        // Could avoid flooding log since many may be identical, but
        // maybe the last one is the one you need to see (for debugging...)
        // _TRACE("Query execution error: " << e.GetMsg());
    } catch (CFeatFilterQueryException& ex) {
        // Could avoid flooding log since many may be identical, but
        // maybe the last one is the one you need to see (for debugging...)
        LOG_POST(Info << "Query execution error: " << ex.GetMsg());
    }


    return false;
}


void CFeatureFilter::AddId(const string& id, CQueryParseNode::EType type)
{
    m_IdDict[id] = type;
}


const TIdentifierDict* CFeatureFilter::GetIdDictionary() const
{
    return &m_IdDict;
}


bool CFeatureFilter::NeedFiltering() const
{
    return m_NeedFiltering;
}


bool CFeatureFilter::ResolveIdentifier(const std::string& identifier,
                                       bool& value)
{
    if(!m_EvalFeat) {
        NCBI_THROW(CFeatFilterQueryException, eNullFeature, 
            "Error: NULL feature evaluation");

    }
    const objects::CSeq_feat& feat = m_EvalFeat->GetOriginalFeature();

    if (feat.GetData().IsClone()) {
        if (identifier == kConcordant) {
            if (feat.GetData().GetClone().IsSetConcordant()) {
                value = feat.GetData().GetClone().GetConcordant();
            } else {
                value = false;
            }
        } else if (identifier == kDiscordant) {
            if (feat.GetData().GetClone().IsSetConcordant()) {
                value = !feat.GetData().GetClone().GetConcordant();
            } else {
                value = false;
            }
        }
    } else {
        return false;
    }

    return true;
}


bool CFeatureFilter::ResolveIdentifier(const std::string& identifier,
                                       Int8& value)
{
    if (identifier == "len") {
        value = 0;
        if(!m_EvalFeat) {
            NCBI_THROW(CFeatFilterQueryException, eNullFeature, 
                "Error: NULL feature evaluation");
        }
        const objects::CSeq_loc& loc = m_EvalFeat->GetLocation();
        if (loc.GetId()) {
            value = loc.GetTotalRange().GetLength();
        }
        return true;
    }
    return false;
}


bool CFeatureFilter::ResolveIdentifier(const std::string& identifier,
                                       std::string& value)
{
    if(!m_EvalFeat) {
        NCBI_THROW(CFeatFilterQueryException, eNullFeature, 
            "Error: NULL feature evaluation");
    }
    const objects::CSeq_feat& feat = m_EvalFeat->GetOriginalFeature();
    if (identifier == kVariantQuality) {
        value = feat.GetNamedQual("Variant Quality");
        if (value.empty()) {
            value = "high";
        }
    } else if (identifier == kPilot) {
        value = feat.GetNamedQual("Pilot");
    } else if (identifier == kClinicalAssertion  &&
        feat.GetData().IsVariation()) {
        const objects::CVariation_ref& var = feat.GetData().GetVariation();
        ITERATE (objects::CVariation_ref::TPhenotype, pnt_iter, var.GetPhenotype()) {
            if ((*pnt_iter)->CanGetClinical_significance()) {
                switch ((*pnt_iter)->GetClinical_significance()) {
                    case objects::CPhenotype::eClinical_significance_non_pathogenic:
                        value = "benign";
                        break;
                    case objects::CPhenotype::eClinical_significance_pathogenic:
                        value = "pathogenic";
                        break;
                    case objects::CPhenotype::eClinical_significance_probable_pathogenic:
                        value = "likely_pathogenic";
                        break;
                    case objects::CPhenotype::eClinical_significance_probable_non_pathogenic:
                        value = "likely_benign";
                        break;
                    case objects::CPhenotype::eClinical_significance_unknown:
                        value = "likely_unknown";
                        break;
                    case objects::CPhenotype::eClinical_significance_untested:
                        value = "not_tested";
                        break;
                    case objects::CPhenotype::eClinical_significance_other:
                    default:
                        value = "other";
                        break;
                }
                break;
            }
        }
        if (value.empty()) {
            value = "other";
        }
    } else if (identifier == kSamplesetType  &&
        feat.GetData().IsVariation()) {
        value = feat.GetNamedQual("sampleset_type");
    } else if (identifier == kValidationStatus  &&  feat.GetData().IsVariation()) {
        value = "other";
        if (feat.IsSetExts()) {
            const objects::CSeq_feat::TExts& exts = feat.GetExts();
            ITERATE (objects::CSeq_feat::TExts, iter, exts) {
                if ( (*iter)->GetType().IsStr()  &&
                    NStr::EqualNocase((*iter)->GetType().GetStr(), "Validation")  &&
                    (*iter)->GetFieldRef("Status")  &&
                    (*iter)->GetFieldRef("Status")->GetData().IsStr()) {
                        value = (*iter)->GetFieldRef("Status")->GetData().GetStr();
                        break;
                }
            }
        }
    } else {
        return false;
    }
    return true;
}
    

bool CFeatureFilter::HasIdentifier(const std::string& id)
{
    return m_IdDict.count(id) > 0;
}

///////////////////////////////////////////////////////////////////////////////
/// class CSGQueryNodeValue implementation
///
bool CSGQueryNodeValue::PromoteTo(CQueryParseNode::EType type)
{
    // we only do data type converstion to int, float, bool and string, and 
    // not data field node
    if (type > CQueryParseNode::eString  ||
        type < CQueryParseNode::eIntConst ||
        IsDataField()) {
            return false;
    }

    bool success = true;
    CQueryParseNode::EType src_type = (*m_Node)->GetType();
    switch (type) {
        case CQueryParseNode::eIntConst:
            {{
                switch (src_type) {
                    case CQueryParseNode::eIntConst:
                        m_Int = (*m_Node)->GetInt();
                        break;
                    case CQueryParseNode::eFloatConst:
                        m_Int = (Int8)(*m_Node)->GetDouble();
                        break;
                    case CQueryParseNode::eBoolConst:
                        m_Int = (Int8)(*m_Node)->GetBool();
                        break;
                    default:
                        // treat it as a string
                        try {
                            m_Int = NStr::StringToInt8((*m_Node)->GetStrValue());
                        } catch (CException&) {
                            // fail to convert
                            success = false;
                        }
                        break;
                }
            }}
            break;
        case CQueryParseNode::eFloatConst:
            {{
                switch (src_type) {
                    case CQueryParseNode::eIntConst:
                        m_Double = (double)(*m_Node)->GetInt();
                        break;
                    case CQueryParseNode::eFloatConst:
                        m_Double = (*m_Node)->GetDouble();
                        break;
                    case CQueryParseNode::eBoolConst:
                        // no bool to double
                        success = false;
                        break;
                    default:
                        // treat it as a string
                        try {
                            m_Double = NStr::StringToDouble((*m_Node)->GetStrValue());
                        } catch (CException&) {
                            // fail to convert
                            success = false;
                        }
                        break;
                }
            }}
            break;
        case CQueryParseNode::eBoolConst:
            {{
                switch (src_type) {
                    case CQueryParseNode::eIntConst:
                        m_Bool = (bool)(*m_Node)->GetInt();
                        break;
                    case CQueryParseNode::eFloatConst:
                        // no double to bool
                        success = false;
                        break;
                    case CQueryParseNode::eBoolConst:
                        m_Bool = (*m_Node)->GetBool();
                        break;
                    default:
                        // treat it as a string
                        try {
                            m_Bool = NStr::StringToBool((*m_Node)->GetStrValue());
                        } catch (CException&) {
                            // fail to convert
                            success = false;
                        }
                        break;
                }
            }}
            break;
        case CQueryParseNode::eString:
        default:
            {{
                try {
                    switch (src_type) {
                        case CQueryParseNode::eIntConst:
                            m_String = NStr::Int8ToString((*m_Node)->GetInt());
                            break;
                        case CQueryParseNode::eFloatConst:
                            m_String = NStr::DoubleToString((*m_Node)->GetDouble());
                            break;
                        case CQueryParseNode::eBoolConst:
                            m_String = NStr::BoolToString((*m_Node)->GetBool());
                            break;
                        default:
                            m_String = (*m_Node)->GetStrValue();
                            break;
                    }
                } catch (CException&) {
                    // fail to convert
                    success = false;
                }
            }}
            break;

    }

    if ( !success ) {
        SetDataType(CQueryParseNode::eNotSet);
    } else {
        SetDataType(type);
    }

    return success;
}


///////////////////////////////////////////////////////////////////////////////
/// class CSGQueryNodePreprocessor implementation
///
CSGQueryNodePreprocessor::CSGQueryNodePreprocessor(TIdentifierDict* d) 
    : m_Dictionary(d) 
{}
          
ETreeTraverseCode
CSGQueryNodePreprocessor::operator()(CTreeNode<CQueryParseNode>& tr, int delta)
{
    if (delta == 0 || delta == 1) {
        // If node has children, we skip it and process on the way back
        if (!tr.IsLeaf()) {
            return eTreeTraverse;
        }
    }

    CSGQueryNodeValue* v = new CSGQueryNodeValue(&tr);
    tr->SetUserObject(v);
    CQueryParseNode::EType node_type = tr->GetType();

    if (tr.IsLeaf()) {
        // leaf node will be initialized by it parent if it is not
        // an verifed identifier node
        const string& id = tr->GetOriginalText();
        TIdentifierDict::const_iterator iter = m_Dictionary->find(id);
        if (iter != m_Dictionary->end()) {
            v->SetDataType(iter->second);
            v->SetDataField(true);
        }
        return eTreeTraverse;
    }

    bool valid = false;
    int child_num = tr.CountNodes();

    // all no-leaf node is in bool type
    v->SetDataType(CQueryParseNode::eBoolConst);

    if (node_type == CQueryParseNode::eAnd  ||
        node_type == CQueryParseNode::eOr   ||
        node_type == CQueryParseNode::eSub  ||
        node_type == CQueryParseNode::eXor  ||
        node_type == CQueryParseNode::eNot) {
        // verify having correct number of child nodes
        if ( (node_type == CQueryParseNode::eNot  &&  child_num != 1)  ||
             (node_type != CQueryParseNode::eNot  &&  child_num != 2) ) {
                NCBI_THROW(CFeatFilterQueryException, eWrongArgumentCount,
                    "Incorrect query node count");
        }

        CTreeNode<CQueryParseNode>::TNodeList_I iter = tr.SubNodeBegin();
        while (iter != tr.SubNodeEnd()) {
            CQueryParseNode& sub_qnode = (*iter)->GetValue();
            IQueryParseUserObject* uo = sub_qnode.GetUserObject();
            CSGQueryNodeValue* sub_v = dynamic_cast<CSGQueryNodeValue*>(uo);
            _ASSERT(sub_v);

            if ( !sub_v->IsValid() ) {
                // no need to do further check at all.
                continue;
            }

            // All child nodes must be eBoolConst. If not and it is leaf node 
            // with const value, we will do data conversion. Otherwise, we 
            // mark it as invalid.
            if ((*iter)->IsLeaf()  &&  !sub_v->IsDataField()) {
                sub_v->PromoteTo(CQueryParseNode::eBoolConst);
            }

            if (sub_v->GetDataType() != CQueryParseNode::eBoolConst) {
                sub_v->SetValid(false);
            } else {
                // For logic node, it if valid if there is at least one of its 
                // child nodes is valid. We simply ingnore it other invlid nodes
                // during query execution.
                valid = true;
            }
            ++iter;
        }
    } else if (node_type == CQueryParseNode::eLike ||
        node_type == CQueryParseNode::eBetween ||
        node_type == CQueryParseNode::eIn      ||
        node_type == CQueryParseNode::eEQ      ||
        node_type == CQueryParseNode::eGT      ||
        node_type == CQueryParseNode::eGE      ||
        node_type == CQueryParseNode::eLT      ||
        node_type == CQueryParseNode::eLE) {

        // assume valid first, but if any child node is invalid,
        // it will be marked as invalid
        valid = true;

        // verify having correct number of child nodes
        if ( (node_type == CQueryParseNode::eBetween  &&  child_num != 3)  ||
             (node_type == CQueryParseNode::eIn  &&  child_num < 2)  ||
             (( (node_type >= CQueryParseNode::eEQ  &&
                node_type <= CQueryParseNode::eLE)  ||
                node_type == CQueryParseNode::eLike ) && child_num != 2)) {
                valid = false;
                NCBI_THROW(CFeatFilterQueryException, eWrongArgumentCount,
                    "Incorrect query node count");
        }

        // verify all child has the same type
        CTreeNode<CQueryParseNode>::TNodeList_I iter = tr.SubNodeBegin();
        CSGQueryNodeValue* primary_v =
            dynamic_cast<CSGQueryNodeValue*>((*iter)->GetValue().GetUserObject());
        _ASSERT(primary_v);

        if ( !primary_v->IsDataField() ) {
            NCBI_THROW(CFeatFilterQueryException, eIncorrectNodeType,
                "The first argument has to be an indentifier");
        }

        CQueryParseNode::EType p_type = primary_v->GetDataType();
        if (node_type == CQueryParseNode::eLike  &&
            p_type != CQueryParseNode::eString) {
            valid = false;
        }

        if (node_type == CQueryParseNode::eBetween  &&
            p_type == CQueryParseNode::eBoolConst) {
            valid = false;
        }

        while (valid  &&  ++iter != tr.SubNodeEnd()) {
            CQueryParseNode& sub_qnode = (*iter)->GetValue();
            IQueryParseUserObject* uo = sub_qnode.GetUserObject();
            CSGQueryNodeValue* sub_v = dynamic_cast<CSGQueryNodeValue*>(uo);
            _ASSERT(sub_v);

            // All other child nodes must use p_type. If not, we will do data
            // conversion.
            if ((*iter)->IsLeaf()) {
                sub_v->PromoteTo(p_type);
                // all other sub_node can't be a data field
                sub_v->SetDataField(false);
            }

            if (sub_v->GetDataType() != p_type) {
                valid = false;
            }
        }

        // mark all children as invlid if the parent node is invalid
        if ( !valid ) {
            for (iter = tr.SubNodeBegin(); iter != tr.SubNodeEnd(); ++iter) {
                CQueryParseNode& sub_qnode = (*iter)->GetValue();
                IQueryParseUserObject* uo = sub_qnode.GetUserObject();
                CSGQueryNodeValue* sub_v = dynamic_cast<CSGQueryNodeValue*>(uo);
                _ASSERT(sub_v);

                sub_v->SetValid(false);
            }
        }
    }

    v->SetValid(valid);
   
    return eTreeTraverse;
}


///////////////////////////////////////////////////////////////////////////////
/// class CSGQueryFunctionValue
///
void CSGQueryFunctionValue::Evaluate(CQueryParseTree::TNode& node)
{
    CSGQueryNodeValue* v =
        dynamic_cast<CSGQueryNodeValue*>(node->GetUserObject());
    _ASSERT(v);
    
   if (v->IsValid()  &&   v->IsDataField()) {
        const string& id = node->GetStrValue();
        // udpate the data field
        switch (v->GetDataType()) {
            case CQueryParseNode::eIntConst:
                m_QExec->ResolveIdentifier(id, v->m_Int);
                break;
            case CQueryParseNode::eFloatConst:
                m_QExec->ResolveIdentifier(id, v->m_Double);
                break;
            case CQueryParseNode::eBoolConst:
                m_QExec->ResolveIdentifier(id, v->m_Bool);
                break;
            case CQueryParseNode::eString:
                m_QExec->ResolveIdentifier(id, v->m_String);
                break;
            default:
                break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
/// class CSGQueryFunctionLogic
///
void CSGQueryFunctionLogic::Evaluate(CQueryParseTree::TNode& node)
{
    CSGQueryNodeValue* v =
        dynamic_cast<CSGQueryNodeValue*>(node->GetUserObject());
    _ASSERT(v);
    
    if ( !v->IsValid() ) {
        return;
    }

    v->m_Bool = false;

    CQueryFunctionBase::TArgVector args;
    this->MakeArgVector(node, args);
    CQueryParseNode::EType node_type = node->GetType();

    // check if it is a binary operator:
    if (node_type == CQueryParseNode::eAnd ||
        node_type == CQueryParseNode::eOr ||
        node_type == CQueryParseNode::eSub ||
        node_type == CQueryParseNode::eXor) {

        // Since this is a logical operator, both arguments must be the
        // boolean result of a previous expression or promotable to
        // boolean.  No need to do type promotion for this.

        CSGQueryNodeValue* sub_v1 =
            dynamic_cast<CSGQueryNodeValue*>((*args[0])->GetUserObject());
        CSGQueryNodeValue* sub_v2 =
            dynamic_cast<CSGQueryNodeValue*>((*args[1])->GetUserObject());
        _ASSERT(sub_v1);
        _ASSERT(sub_v2);

        // preprocessor gurantees at least one of them is valid
        if ( !sub_v1->IsValid() ) {
            v->m_Bool = sub_v2->m_Bool;
        } else if ( !sub_v2->IsValid() ) {
            v->m_Bool = sub_v1->m_Bool;
        } else {
            switch (node_type) {
                case CQueryParseNode::eAnd:
                    v->m_Bool = sub_v1->m_Bool && sub_v2->m_Bool;
                    break;
                case CQueryParseNode::eOr:
                    v->m_Bool = sub_v1->m_Bool || sub_v2->m_Bool;
                    break;    
                case CQueryParseNode::eSub:
                    v->m_Bool = sub_v1->m_Bool && !sub_v2->m_Bool;
                    break;
                case CQueryParseNode::eXor:
                    v->m_Bool = sub_v1->m_Bool != sub_v2->m_Bool;
                    break;
                default:
                    break;
            }
        }

        if (node->IsNot()) {
            v->m_Bool = !v->m_Bool;
        }

    } else if (node_type == CQueryParseNode::eNot) {
        CSGQueryNodeValue* sub_v =
            dynamic_cast<CSGQueryNodeValue*>((*args[0])->GetUserObject());
        _ASSERT(sub_v);

        v->m_Bool = !sub_v->m_Bool;

    } else {        
        NCBI_THROW(CFeatFilterQueryException, eInvalidQuery, 
            "Error: Unexpected logical operand:" + node->GetOriginalText());
    }
}


///////////////////////////////////////////////////////////////////////////////
/// class CSGQueryFunctionCompare
///
void CSGQueryFunctionCompare::Evaluate(CQueryParseTree::TNode& node)
{
    CSGQueryNodeValue* v =
        dynamic_cast<CSGQueryNodeValue*>(node->GetUserObject());
    _ASSERT(v);
    
    if ( !v->IsValid() ) {
        return;
    }

    v->m_Bool = false;

    CQueryFunctionBase::TArgVector args;
    this->MakeArgVector(node, args);   

    // get operation type
    CQueryParseNode::EType node_type = node->GetType();

    // handle all the binary comparisons
    if (node_type == CQueryParseNode::eEQ ||
        node_type == CQueryParseNode::eGT ||
        node_type == CQueryParseNode::eGE ||
        node_type == CQueryParseNode::eLT ||
        node_type == CQueryParseNode::eLE ||
        node_type == CQueryParseNode::eLike) {

        CSGQueryNodeValue* sub_v1 =
            dynamic_cast<CSGQueryNodeValue*>((*args[0])->GetUserObject());
        CSGQueryNodeValue* sub_v2 =
            dynamic_cast<CSGQueryNodeValue*>((*args[1])->GetUserObject());
        _ASSERT(sub_v1);
        _ASSERT(sub_v2);

        CQueryParseNode::EType value_type = sub_v1->GetDataType();
        switch (node_type) {
            case CQueryParseNode::eEQ:
                if (value_type == CQueryParseNode::eBoolConst) {
                    v->m_Bool = sub_v1->m_Bool == sub_v2->m_Bool;
                } else if (value_type == CQueryParseNode::eIntConst) {
                    v->m_Bool = sub_v1->m_Int == sub_v2->m_Int;
                } else if (value_type == CQueryParseNode::eFloatConst) {
                    v->m_Bool = sub_v1->m_Double == sub_v2->m_Double;
                } else if (value_type == CQueryParseNode::eString) {
                    v->m_Bool = NStr::EqualNocase(sub_v1->m_String, sub_v2->m_String);
                }
                if (node->IsNot())
                    v->m_Bool = !v->m_Bool;
                break;

            case CQueryParseNode::eLike:
                if (value_type == CQueryParseNode::eString) {
                    v->m_Bool = NStr::MatchesMask(sub_v1->m_String.c_str(), 
                        sub_v2->m_String.c_str(), NStr::eNocase);
                }
                if (node->IsNot()) {
                    v->m_Bool = !v->m_Bool;
                }
                break;

            case CQueryParseNode::eGT:
                if (value_type == CQueryParseNode::eBoolConst) {
                    v->m_Bool = sub_v1->m_Bool > sub_v2->m_Bool;
                } else if (value_type == CQueryParseNode::eIntConst) {
                    v->m_Bool = sub_v1->m_Int > sub_v2->m_Int;
                } else if (value_type == CQueryParseNode::eFloatConst) {
                    v->m_Bool = sub_v1->m_Double > sub_v2->m_Double;
                } else if (value_type == CQueryParseNode::eString) {
                    int result = NStr::Compare(sub_v1->m_String.c_str(), 
                        sub_v2->m_String.c_str(), NStr::eNocase);
                    v->m_Bool = result > 0;
                }
                break;

            case CQueryParseNode::eGE:
                if (value_type == CQueryParseNode::eBoolConst) {
                    v->m_Bool = sub_v1->m_Bool >= sub_v2->m_Bool;
                } else if (value_type == CQueryParseNode::eIntConst) {
                    v->m_Bool = sub_v1->m_Int >= sub_v2->m_Int;
                } else if (value_type == CQueryParseNode::eFloatConst) {
                    v->m_Bool = sub_v1->m_Double >= sub_v2->m_Double;
                } else if (value_type == CQueryParseNode::eString) {
                    int result = NStr::Compare(sub_v1->m_String.c_str(), 
                        sub_v2->m_String.c_str(), NStr::eNocase);
                    v->m_Bool = result >= 0;
                }
                break;

            case CQueryParseNode::eLT:
                if (value_type == CQueryParseNode::eBoolConst) {
                    v->m_Bool = sub_v1->m_Bool < sub_v2->m_Bool;
                } else if (value_type == CQueryParseNode::eIntConst) {
                    v->m_Bool = sub_v1->m_Int < sub_v2->m_Int;
                } else if (value_type == CQueryParseNode::eFloatConst) {
                    v->m_Bool = sub_v1->m_Double < sub_v2->m_Double;
                } else if (value_type == CQueryParseNode::eString) {
                    int result = NStr::Compare(sub_v1->m_String.c_str(), 
                        sub_v2->m_String.c_str(), NStr::eNocase);
                    v->m_Bool = result < 0;
                }
                break;

            case CQueryParseNode::eLE:
                if (value_type == CQueryParseNode::eBoolConst) {
                    v->m_Bool = sub_v1->m_Bool  <= sub_v2->m_Bool;
                } else if (value_type == CQueryParseNode::eIntConst) {
                    v->m_Bool = sub_v1->m_Int <= sub_v2->m_Int;
                } else if (value_type == CQueryParseNode::eFloatConst) {
                    v->m_Bool = sub_v1->m_Double <= sub_v2->m_Double;
                } else if (value_type == CQueryParseNode::eString) {
                    int result = NStr::Compare(sub_v1->m_String.c_str(), 
                        sub_v2->m_String.c_str(), NStr::eNocase);
                    v->m_Bool = result <= 0;
                }
                break;

            default:
                // We already check for this wih previous 'if' so should not happen 
                break;
        }

    } else if (node_type == CQueryParseNode::eBetween) { // handle the 'between' operator
        // Set is_between to true if the value is eiter >=sub_v2 and <=res3.

        CSGQueryNodeValue* sub_v1 =
            dynamic_cast<CSGQueryNodeValue*>((*args[0])->GetUserObject());
        CSGQueryNodeValue* sub_v2 =
            dynamic_cast<CSGQueryNodeValue*>((*args[1])->GetUserObject());
        CSGQueryNodeValue* sub_v3 =
            dynamic_cast<CSGQueryNodeValue*>((*args[2])->GetUserObject());
        _ASSERT(sub_v1);
        _ASSERT(sub_v2);
        _ASSERT(sub_v3);

        CQueryParseNode::EType value_type = sub_v1->GetDataType();

        bool is_between = false;
        if (value_type == CQueryParseNode::eIntConst) {
            is_between = sub_v1->m_Int >= sub_v2->m_Int;
        } else if (value_type == CQueryParseNode::eFloatConst) {
            is_between = sub_v1->m_Double >= sub_v2->m_Double;
        } else if (value_type == CQueryParseNode::eString) {
            int result = NStr::Compare(sub_v1->m_String.c_str(), 
                                       sub_v2->m_String.c_str(), 
                                       NStr::eNocase);
            is_between = (result >= 0);
        }

        // If we passed first test (sub_v1>=sub_v2), try second test (sub_v1<=sub_v3):
        if (is_between) {
            if (value_type == CQueryParseNode::eIntConst) {
                is_between = sub_v1->m_Int <= sub_v3->m_Int;
            } else if (value_type == CQueryParseNode::eFloatConst) {
                is_between = sub_v1->m_Double <= sub_v3->m_Double;
            } else if (value_type == CQueryParseNode::eString) {
                int result = NStr::Compare(sub_v1->m_String.c_str(), 
                                           sub_v3->m_String.c_str(), 
                                           NStr::eNocase);
                is_between = (result <= 0);
            }
        }
        v->m_Bool = is_between; 

    } else if (node_type == CQueryParseNode::eIn) { // handle the 'In' operator
        // Get underlying data and data type. Since there may be any number of operands,
        // iterate through them and check each time if they need to be promoted.
        // This allows the user to mix types between the parms, e.g. : dist IN (0.57, "", 0.56)
        // without throwing an exception.
        CSGQueryNodeValue* sub_v1 =
            dynamic_cast<CSGQueryNodeValue*>((*args[0])->GetUserObject());
        _ASSERT(sub_v1);
        CQueryParseNode::EType value_type = sub_v1->GetDataType();

        bool found = false;
        for (size_t i = 1; i < args.size() && !found;  ++i) {
            CSGQueryNodeValue* sub_v2 =
                dynamic_cast<CSGQueryNodeValue*>((*args[i])->GetUserObject());
            _ASSERT(sub_v2);

            if (value_type == CQueryParseNode::eBoolConst) {
                found = (sub_v1->m_Bool == sub_v2->m_Bool);
            } else if (value_type == CQueryParseNode::eIntConst) {
                found = (sub_v1->m_Int == sub_v2->m_Int);
            } else if (value_type == CQueryParseNode::eFloatConst) {
                found = (sub_v1->m_Double == sub_v2->m_Double);
            } else if (value_type == CQueryParseNode::eString) {                   
                int result = NStr::Compare(sub_v1->m_String.c_str(), 
                    sub_v2->m_String.c_str(), 
                    NStr::eNocase);
                found = (result == 0);
            }
        }
        v->m_Bool = found;

    } else {
        NCBI_THROW(CFeatFilterQueryException, eInvalidQuery, 
            "Error - Unhandled comparison operator: " + node->GetOriginalText());
    }
}


END_NCBI_SCOPE



