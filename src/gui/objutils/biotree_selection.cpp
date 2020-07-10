/*  $Id: biotree_selection.cpp 32608 2015-03-31 20:04:19Z falkrb $
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
*   Government have not placed any restriction on its use or reproduction.
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
* Author:  Vladislav Evgeniev
*
* File Description: CBioTreeSelection implementatoin
*
*/

#include <ncbi_pch.hpp>

#include <gui/objutils/biotree_selection.hpp>
#include <objects/biotree/FeatureDictSet.hpp>
#include <objects/biotree/NodeSet.hpp>
#include <objects/biotree/NodeFeatureSet.hpp>
#include <objects/biotree/NodeFeature.hpp>
#include <gui/objutils/utils.hpp>

using namespace std;

BEGIN_NCBI_SCOPE

using namespace objects;

CBioTreeSelection::CBioTreeSelection(const CConstRef<objects::CBioTreeContainer> &bioTree) :
    m_BioTreeContainer(bioTree)
{
    if (m_BioTreeContainer.IsNull() || !m_BioTreeContainer->CanGetFdict())
        return;

    // Populate the features map
    const CBioTreeContainer::TFdict &fdict = m_BioTreeContainer->GetFdict();
    if (fdict.CanGet()) {
        ITERATE(CFeatureDictSet::Tdata, it, fdict.Get()) {
            if (!(*it)->CanGetName() || !(*it)->CanGetId())
                continue;
            string feat_name((*it)->GetName());
            m_FeatIdsMap[NStr::ToLower(feat_name)] = (*it)->GetId();
        }
    }
    
    // Populate the nodes map
    const CBioTreeContainer::TNodes &nodes = m_BioTreeContainer->GetNodes();
    if (nodes.CanGet()) {
        ITERATE(CNodeSet::Tdata, itNode, nodes.Get()) {
            if (!(*itNode)->CanGetId())
                continue;

            m_NodesMap[(*itNode)->GetId()] = *itNode;
        }
    }
}

bool CBioTreeSelection::HasFeature(const string& feature) const
{
    string feat_name(feature);
    TStringFeatIdMap::const_iterator featureId = m_FeatIdsMap.find(NStr::ToLower(feat_name));
    return (featureId != m_FeatIdsMap.end()) ? true : false;
}

CFeatureDescr::TId CBioTreeSelection::GetFeatureId(const string& feature) const
{
    string feat_name(feature);
    TStringFeatIdMap::const_iterator featureId = m_FeatIdsMap.find(NStr::ToLower(feat_name));
    return (featureId != m_FeatIdsMap.end()) ? featureId->second : -1;
}

string CBioTreeSelection::GetFeatureValue(CNode::TId nodeId, CFeatureDescr::TId featureId) const
{
    TNodeIdNodeMap::const_iterator node = m_NodesMap.find(nodeId);
    if (node == m_NodesMap.end())
        return "";
            
    return GetFeatureValue(*(node->second),featureId);
}


string CBioTreeSelection::GetFeatureValue(const CNode& node, CFeatureDescr::TId featureId) const
{
    if (!node.CanGetFeatures()) 
        return "";

    const CNode::TFeatures &features = node.GetFeatures();
    if (!features.CanGet())
        return "";

    ITERATE(CNodeFeatureSet::Tdata, itFeature, features.Get()) {
        if (!(*itFeature)->CanGetFeatureid())
            continue;

        if (featureId != (*itFeature)->GetFeatureid())
            continue;

        if (!(*itFeature)->CanGetValue())
            return "";
        else
            return (*itFeature)->GetValue();
    }
        
    return "";
}

END_NCBI_SCOPE
