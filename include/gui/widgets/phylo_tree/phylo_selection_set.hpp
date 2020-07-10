#ifndef GUI_WIDGETS_PHY_TREE___PHYLO_SELECTION_SET__HPP
#define GUI_WIDGETS_PHY_TREE___PHYLO_SELECTION_SET__HPP

/*  $Id: phylo_selection_set.hpp 36928 2016-11-15 14:28:10Z falkrb $
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
*
*/
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/utils/rgba_color.hpp>
#include <objects/biotree/Node.hpp>
#include <objects/biotree/BioTreeContainer.hpp>
#include <objects/general/User_object.hpp>


BEGIN_NCBI_SCOPE

class CPhyloTree;

/*******************************************************************************
* CPhyloSelectionSet
* 
* A named set of selected nodes.  Allows user to save a selection
* set for later viewing without having to re-create it through a query
* or hand-slection.
******************************************************************************/
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloSelectionSet {
public:
    CPhyloSelectionSet(const vector<objects::CNode::TId>& sel, const string& name, int cluster_id = 0)
        : m_SelectedIDs(sel)
        , m_SelectionName(name)
        , m_SelectionColor(0.0f, 0.0f, 0.0f, 1.0f)
        , m_Selected(false)
        , m_ClusterID(cluster_id) {}

    CPhyloSelectionSet()
        : m_Selected(false)
        , m_ClusterID(-1) {}

    /// call RemoveDuplicateIDs first which sorts and removes dups
    bool operator==(const CPhyloSelectionSet& rhs) const;

    /// remove duplicates form m_SelectedIDs. should be called before operator==
    void RemoveDuplicateIDs();

    void InitFromUserObject(CRef< objects::CUser_object > sel_set);
    CRef< objects::CUser_object > SaveToUserObject() const;

    vector<objects::CNode::TId>& GetSelectionSet() { return m_SelectedIDs; }
    const vector<objects::CNode::TId>& GetSelectionSet() const { return m_SelectedIDs; }
    void UpdateSelectionSet(const vector<objects::CNode::TId>& sel) { m_SelectedIDs = sel; }

    string GetName() const { return m_SelectionName; }
    void SetName(const string& n) { m_SelectionName = n; }

    CRgbaColor GetColor() const { return m_SelectionColor; }
    void SetColor(const CRgbaColor& c) { m_SelectionColor = c; }

    bool GetSelected() const { return m_Selected; }
    void SetSelected(bool b) { m_Selected = b; }

    int GetClusterID() const { return m_ClusterID; }
    void SetClusterID(int cid) { m_ClusterID = cid; }


protected:
    vector<objects::CNode::TId> m_SelectedIDs;
    string                 m_SelectionName;
    CRgbaColor             m_SelectionColor;
    bool                   m_Selected;
    int                    m_ClusterID;
};


/*******************************************************************************
* CPhyloSelectionSetMgr
*
* Manages a set of CPhyloSelectionSet objects
******************************************************************************/
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloSelectionSetMgr {
public:
    typedef std::map<int, size_t> TClusterToColorMap;

public:
    CPhyloSelectionSetMgr() {}

    bool operator==(const CPhyloSelectionSetMgr& rhs) const;

    void Clear() { m_SelectionSets.clear(); }
    void InitFromUserObject(CPhyloTree* tree_model,
                            const objects::CBioTreeContainer_Base::TUser& uo);
    void SaveToUserObject(objects::CBioTreeContainer_Base::TUser& uo);

    /// Remove duplicate IDs from individual sets
    void RemoveDuplicateIDs();

    vector<CPhyloSelectionSet>& GetSets() { return m_SelectionSets; }
    const vector<CPhyloSelectionSet>& GetSets() const { return m_SelectionSets; }

    void SetSelectionSetProperty(CPhyloTree* tree_model);
   
    /// Returns size_t(-1) if the name set_name is not an existing selection set
    size_t GetSelectionSet(const string& set_name);

    /// Add the new set set_name and return it. If it already exists, just return it.
    CPhyloSelectionSet& AddSet(const string& set_name);
    CPhyloSelectionSet& AddSet(const string& set_name, const CRgbaColor& c);

    // Get cluster ids from selection sets
    map<int, size_t> GetClusterToSelectionMap();

    CRgbaColor PickBestColor(const vector<CRgbaColor>& current_colors);

    /// Update all cluster ids using the starting-id as the lowest value
    void RenumberClusterIDs(int start_id);

protected:

    void x_SyncSelectionSets(CPhyloTree* tree_model);

    vector<CPhyloSelectionSet> m_SelectionSets;
};

END_NCBI_SCOPE

#endif //GUI_WIDGETS_PHY_TREE___PHYLO_SELECTION_SET__HPP

