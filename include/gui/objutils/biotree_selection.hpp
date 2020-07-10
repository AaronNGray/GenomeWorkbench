#ifndef GUI_OBJUTILS___BIOTREE_SELECTION__HPP
#define GUI_OBJUTILS___BIOTREE_SELECTION__HPP

/*  $Id: biotree_selection.hpp 38774 2017-06-16 15:57:03Z katargir $
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
 * Author:  Vladislav Evgeniev
 *
 *
 */

/// @file biotree_selection.hpp
/// Defines CBioTreeSelection, class used to broadcast selected tree nodes between views.

#include <vector>
#include <map>
#include <string>
#include <gui/gui_export.h>
#include <objects/biotree/BioTreeContainer.hpp>
#include <objects/biotree/FeatureDescr.hpp>
#include <objects/biotree/Node.hpp>
#include <objects/seqloc/Seq_id.hpp>

BEGIN_NCBI_SCOPE

// Class used to broadcast selected tree nodes between views.
class NCBI_GUIOBJUTILS_EXPORT CBioTreeSelection : 
    public CObject
{
public:
    /// Vector of node IDs
    typedef std::vector<objects::CNode::TId>  TNodeIdsVector;

    /// Constructor
    /// @param[in] bioTree
    ///     Reference to a CBioTreeContainer.
    /// @param[in] nodesCount
    ///     Number of nodes to be stored
    CBioTreeSelection(const CConstRef<objects::CBioTreeContainer> &bioTree);

    /// Returns the BioTree
    /// @return
    ///     CBioTreeContainer
    CConstRef<objects::CBioTreeContainer> GetBioTree() const { return m_BioTreeContainer; }

    /// Returns const list of selected nodes
    /// @return
    ///     TNodeIdsVector
    const TNodeIdsVector& GetNodeIds() const { return m_NodeIds; }

    /// Returns the list of selected nodes
    /// @return
    ///     TNodeIdsVector
    TNodeIdsVector& GetNodeIds() { return m_NodeIds; }

    /// Returns const list of explicitly selected nodes
    /// @return
    ///     TNodeIdsVector
    const TNodeIdsVector& GetExplicitlySelectedNodeIds() const { return m_ExplicitlySelectedNodeIds; }

    /// Returns the list of explicitly selected nodes
    /// @return
    ///     TNodeIdsVector
    TNodeIdsVector& GetExplicitlySelectedNodeIds() { return m_ExplicitlySelectedNodeIds; }

    /// Checks if the BioTree has the specified feature
    /// @param[in] feature
    ///     Reference to a string, holding the name of the feature.
    /// @return
    ///     true if the tree has the specified feature, false otherwise.
    bool HasFeature(const std::string& feature) const;

    /// Gets the id of the specified feature
    /// @param[in] feature
    ///     Reference to a string, holding the name of the feature.
    /// @return
    ///     TId of the feature if found, false otherwise
    objects::CFeatureDescr::TId GetFeatureId(const std::string& feature) const;
    /// Gets the value of a feature for the specified node id
    /// @param[in] nodeId
    ///     The ID of the node, who's feature's value to return.
    /// @param[in] featureId
    ///     The ID of the feature, who's value to return.
    /// @return
    ///     The value of the feature as string if it was found, empty string otherwise.
    std::string GetFeatureValue(objects::CNode::TId nodeId, objects::CFeatureDescr::TId featureId) const;

private:
    /// Gets the value of a feature for the specified node
    /// @param[in] node
    ///     The node, who's feature's value to return.
    /// @param[in] featureId
    ///     The ID of the feature, who's value to return.
    /// @return
    ///     The value of the feature as string if it was found, empty string otherwise.
    std::string GetFeatureValue(const objects::CNode& node, objects::CFeatureDescr::TId featureId) const;

private:
    /// BioTree 
    CConstRef<objects::CBioTreeContainer>   m_BioTreeContainer; 
    /// Selected nodes
    TNodeIdsVector                          m_NodeIds;
    /// Nodes user actually clicked on or which were selected in a query. This does
    /// not include parents or children of selected nodes and is maintained separately
    /// so that user can iterate over the explicit selection set.
    TNodeIdsVector                          m_ExplicitlySelectedNodeIds;
    typedef std::map<std::string, objects::CFeatureDescr::TId>  TStringFeatIdMap;
    typedef std::map<objects::CNode::TId, const objects::CNode*>             TNodeIdNodeMap;
    //// Map of feature names to ids
    TStringFeatIdMap    m_FeatIdsMap;
    /// Map od node ids to nodes
    TNodeIdNodeMap      m_NodesMap;
};

END_NCBI_SCOPE

#endif  /// GUI_OBJUTILS___BIOTREE_SELECTION__HPP
