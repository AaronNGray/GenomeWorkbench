/*  $Id: table_data_biotreecontainer.cpp 32508 2015-03-13 20:22:07Z evgeniev $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <algorithm>
#include <assert.h>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/table_data_base.hpp>
#include <gui/objutils/table_selection.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/biotree_selection.hpp>
#include <gui/objutils/obj_event.hpp>
#include <gui/objutils/registry.hpp>

#include <objects/biotree/BioTreeContainer.hpp>
#include <objects/biotree/FeatureDictSet.hpp>
#include <objects/biotree/FeatureDescr.hpp>
#include <objects/biotree/NodeSet.hpp>
#include <objects/biotree/Node.hpp>
#include <objects/biotree/NodeFeatureSet.hpp>
#include <objects/biotree/NodeFeature.hpp>

#include <objmgr/util/sequence.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CTableDataBioTreeContainer : public CObject, public CTableDataBase
{
public:
    static CTableDataBioTreeContainer* CreateObject(SConstScopedObject& object, ICreateParams* params);

    friend class CTableSelectionBioTreeContainer;

    virtual void       LoadData() {}
    virtual ColumnType GetColumnType(size_t col) const;
    virtual string     GetColumnLabel(size_t col) const;

    virtual size_t GetRowsCount() const;
    virtual size_t GetColsCount() const;

    virtual void GetStringValue(size_t row, size_t col, string& value) const;
    virtual long   GetIntValue(size_t row, size_t col) const;
    virtual double GetRealValue(size_t row, size_t col) const;
    virtual SConstScopedObject GetObjectValue(size_t row, size_t col) const;
    CRef<CScope> GetScope() { return m_Scope; }
    CConstRef<CNode> GetNode(size_t row) const;
    int FindRow (int nodeId) const;

    void SelectSeqIds(const CSelectionEvent::TIds& ids, CFeatureDescr::TId seqidFeatId, vector<size_t>& selectedNodes) const;
    void SelectTaxIds(const CSelectionEvent::TTaxIds& tids, CFeatureDescr::TId taxidFeatId, vector<size_t>& selectedNodes) const;
    void SelectFeatures(const CBioTreeSelection &selection, CFeatureDescr::TId srcFeatId, CFeatureDescr::TId dstFeatId, vector<size_t>& selectedNodes) const;

private:
    struct ColInfo {
        ColInfo() : m_Name(""), m_Type(kNone) {}
        ColInfo(const string& n) : m_Name(n), m_Type(kNone) {}
        ColInfo(const string& n, ITableData::ColumnType t) 
            : m_Name(n), m_Type(t) {}

        string m_Name;
        ITableData::ColumnType m_Type;
    };

    void Init();

    string x_GetNodeFeature(int feature_id, const CNode& node) const;

    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;
    vector<pair<int, ColInfo> > m_FeatureDescr;
    vector<CRef<CNode> > m_Nodes;
    typedef map<CNode::TId,size_t>  TNodeIdRowMap;
    TNodeIdRowMap   m_NodeIdRows;
};

class CTableSelectionBioTreeContainer : public CObject, public ITableSelection
{
public:
    typedef vector<string>  TStringVector;

    static CTableSelectionBioTreeContainer* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual void GetRows(const ITableData& table, const CSelectionEvent& evt, vector<size_t>& rows) const;
    virtual void GetSelection(const ITableData& table, const vector<size_t>& rows, CSelectionEvent& evt) const;

private:
    bool x_StringToSeqId(const string &seq_id, CRef<CSeq_id> &id) const;
};

void initCTableDataBioTreeContainer()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(ITableData).name(),
            CBioTreeContainer::GetTypeInfo(),
            new CObjectInterfaceFactory<CTableDataBioTreeContainer>());

    CInterfaceRegistry::RegisterFactory(
            typeid(ITableSelection).name(),
            CBioTreeContainer::GetTypeInfo(),
            new CObjectInterfaceFactory<CTableSelectionBioTreeContainer>());
}

bool StringToSeqId(const string &seq_id, CRef<CSeq_id> &id)
{
    // 1. make an attempt to convert "as-is":
    try {
        id->Set(seq_id);
        return true;
    }
    catch(CException& )
    {
    }

    // 2. try ad-hoc GI extraction (for misformed seq-id strings like: "gi|55823257|ref|YP_141698.1"
    try {
        string gi_str;
        CSeqUtils::GetGIString(seq_id, &gi_str);
        if (!gi_str.empty()) {
            id->Set(gi_str);
            return true;
        }
    }
    catch(CException& )
    {
    }
    
    // 3. Take it as local
    try {
        string lcl_sid = "lcl|";
        lcl_sid.append(seq_id);
        id->Set(lcl_sid);
        return true;
    }
    catch(CException& )
    {
    }
    return false;
}

CTableDataBioTreeContainer* CTableDataBioTreeContainer::CreateObject(SConstScopedObject& object, ICreateParams*)
{
    CTableDataBioTreeContainer* table_data = new CTableDataBioTreeContainer();
    table_data->m_Object = object.object;
    table_data->m_Scope  = object.scope;
    table_data->Init();
    return table_data;
}

void CTableDataBioTreeContainer::Init()
{
    const CBioTreeContainer& biotree = dynamic_cast<const CBioTreeContainer&>(*m_Object);
    const CFeatureDictSet::Tdata& fdict = biotree.GetFdict().Get();
    ITERATE(CFeatureDictSet::Tdata, it, fdict) {
        ITableData::ColumnType t = kNone;

        if (NStr::EqualNocase((*it)->GetName(), "seq-id"))
            t = kObject;

        m_FeatureDescr.push_back(pair<int, ColInfo>((*it)->GetId(), 
            ColInfo((*it)->GetName(),t)));
    }

    const CNodeSet::Tdata& nodes = biotree.GetNodes().Get();
    m_Nodes.resize(nodes.size());

    size_t i = 0;
    ITERATE(CNodeSet::Tdata, it, nodes) {
        // Guess types - look for ints and reals (with blank == 0) so that numbers
        // can have sorting and querying based on numeric rather than string semantics.
        // If a field as any non-numbers (other than blanks), call it a string. No 
        // attempt is made here to look for other IDs (only id is decided by name).
        for (size_t j=0; j<m_FeatureDescr.size(); ++j) {          
            string val = x_GetNodeFeature(m_FeatureDescr[j].first, (*it).GetObject());
            if (val == "")
                continue;

            // kObject was decided by field name and once a string always a string
            // (if we find a string value for a column, no future ints or floats
            //  will change it's type back).
            if (m_FeatureDescr[j].second.m_Type == kObject ||
                m_FeatureDescr[j].second.m_Type == kString)
                continue;

            try {
                // No exception converting to int - it's an int. Can
                // only change to int from 'none' (all values must be ints)
                NStr::StringToInt(val);
                if (m_FeatureDescr[j].second.m_Type == kNone)
                    m_FeatureDescr[j].second.m_Type = kInt;
            }
            catch (CStringException&) {
                try {
                    // No exception converting to double - it's a real number.
                    // only change to real from kInt or kNone.
                    NStr::StringToDouble(val.c_str());
                    if (m_FeatureDescr[j].second.m_Type == kNone ||
                        m_FeatureDescr[j].second.m_Type == kInt) {
                            m_FeatureDescr[j].second.m_Type = kReal;
                    }
                }
                catch (CStringException&) {
                    // It's just a string probably, but
                    // leave it as a number for range errors (overflow/underflow)
                    double d = NStr::StringToDoublePosix(val.c_str());
                    if (errno != ERANGE) {
                        m_FeatureDescr[j].second.m_Type = kString;
                    }
                }
            }
        }


        m_Nodes[i++] = *it;
        if ((*it)->CanGetId())
            m_NodeIdRows[(*it)->GetId()] = i-1;
    }
}

ITableData::ColumnType CTableDataBioTreeContainer::GetColumnType(size_t col) const
{    
    if (col < 2)
        return kInt;

    if (col - 2 < m_FeatureDescr.size())
        return m_FeatureDescr[col-2].second.m_Type;

    return kNone;
}

string CTableDataBioTreeContainer::GetColumnLabel(size_t col) const
{
    if (col == 0)
        return "id";
    else if (col == 1)
        return "parent";

    if (col - 2 < m_FeatureDescr.size())
        return m_FeatureDescr[col - 2].second.m_Name;

    return "";
}

size_t CTableDataBioTreeContainer::GetRowsCount() const
{
    return m_Nodes.size();
}

size_t CTableDataBioTreeContainer::GetColsCount() const
{
    return 2 + m_FeatureDescr.size();
}
 
void CTableDataBioTreeContainer::GetStringValue(size_t row, size_t col, string& value) const
{
    value.resize(0);
    if (row >= m_Nodes.size())
        return ;

    if (col < 2) {
        value = NStr::SizetToString(GetIntValue(row, col));
    }
    else {
        if (col - 2 < m_FeatureDescr.size()) {
            try {
                const CNode& node = *m_Nodes[row];
                value = x_GetNodeFeature(m_FeatureDescr[col - 2].first, node);
            }
            catch (const std::exception& e) {
                LOG_POST(Error << "CTableDataBioTreeContainer: " << e.what());
            }
        }
    }
}

long CTableDataBioTreeContainer::GetIntValue(size_t row, size_t col) const
{
    long value = 0;
    if (row >= m_Nodes.size())
        return value;

    try {
        const CNode& node = *m_Nodes[row];
        if (col == 0)
            value = node.GetId();
        else if (col == 1) {
            value = node.CanGetParent() ? node.GetParent() : -1;
        }
        else {
            string strval = x_GetNodeFeature(m_FeatureDescr[col - 2].first, node);
            if (NStr::IsBlank(strval))
                return 0;
            value = NStr::StringToLong(strval);
        }
    }
    catch (const std::exception& e) {
        LOG_POST(Error << "CTableDataBioTreeContainer: " << e.what());
    }

    return value;
}

double CTableDataBioTreeContainer::GetRealValue(size_t row, size_t col) const
{
    double value = 0.0;

    if (row >= m_Nodes.size())
        return value;

    if (col < 2)
        return value;

    const CNode& node = *m_Nodes[row];
    string strval = x_GetNodeFeature(m_FeatureDescr[col - 2].first, node);

    try {
        if (NStr::IsBlank(strval))
            return 0.0;
        value = NStr::StringToDouble(strval.c_str());
    }
    catch (const std::exception& e) {
        value = NStr::StringToDoublePosix(strval.c_str());
        if (errno != ERANGE)
            LOG_POST("Value range error: " << strval);
        else
            LOG_POST(Error << "CTableDataBioTreeContainer: " << e.what());
    }

    return value;
}

SConstScopedObject CTableDataBioTreeContainer::GetObjectValue(size_t row, size_t col) const
{
    SConstScopedObject value;

    if (row >= m_Nodes.size())
        return value;

    if (col >= 2 && NStr::EqualNocase(m_FeatureDescr[col - 2].second.m_Name, "Seq-id")) {
        const CNode& node = *m_Nodes[row];
        string seq_id = x_GetNodeFeature(m_FeatureDescr[col - 2].first, node);
        if (seq_id.empty())
            return value;

        CRef<CSeq_id> id(new CSeq_id());

        bool sid_res_status = false;
        // 1. make an attempt to convert "as-is":
        try {
            id->Set(seq_id);

            CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( *id );
            idh = sequence::GetId( idh, *m_Scope, sequence::eGetId_Best );

            if( idh ){
                CRef<CSeq_id> id_copy( new CSeq_id() );
                id_copy->Assign( *idh.GetSeqId() );
                value.object.Reset(id_copy.GetPointer());
                value.scope = m_Scope;
            }
            sid_res_status = true;
        }
        catch(CException& )
        {
        }

        // 2. try ad-hoc GI extraction (for misformed seq-id strings like: "gi|55823257|ref|YP_141698.1"
        if (!sid_res_status) {
            try {
                string gi_str;
                CSeqUtils::GetGIString(seq_id, &gi_str);
                if (!gi_str.empty()) {
                    id->Set(gi_str);
                    sid_res_status = true;
                }
            }
            catch(CException& )
            {
            }
        }

        // 3. Take it as local
        if (!sid_res_status) {
            try {
                string lcl_sid = "lcl|";
                lcl_sid.append(seq_id);
                id->Set(lcl_sid);
                sid_res_status = true;
            }
            catch(CException& )
            {
            }
        }

        if (sid_res_status) {
            value.object.Reset(id.GetPointer());
            value.scope = m_Scope;
        }
    } // if "seq-id"


    return value;
}

CConstRef<CNode> CTableDataBioTreeContainer::GetNode(size_t row) const
{
    if (row >= m_Nodes.size())
        return CConstRef<CNode>();

    return m_Nodes[row];
}

int CTableDataBioTreeContainer::FindRow (int nodeId) const
{
    TNodeIdRowMap::const_iterator node = m_NodeIdRows.find(nodeId);
    return (node != m_NodeIdRows.end()) ? (int)node->second : -1;
}

void CTableDataBioTreeContainer::SelectSeqIds(const CSelectionEvent::TIds& ids, CFeatureDescr::TId seqidFeatId, vector<size_t>& selectedNodes) const
{
    ITERATE(CSelectionEvent::TIds, it_id, ids) {
        const CSeq_id& id = **it_id;
        
        for (size_t i=0; i<m_Nodes.size(); ++i) {
            const CNode& node = *m_Nodes[i];

            string feat_value = x_GetNodeFeature(seqidFeatId, node);
            if (feat_value.empty())
                continue;
            CRef<CSeq_id> id2(new CSeq_id());
            if (!StringToSeqId(feat_value, id2))
                continue;

            if (CSeq_id::e_YES != id.Compare(*id2))
                continue;

            selectedNodes.push_back(i);
        }
    }
}

void CTableDataBioTreeContainer::SelectTaxIds(const CSelectionEvent::TTaxIds& tids, CFeatureDescr::TId taxidFeatId, vector<size_t>& selectedNodes) const
{
    for (size_t i=0; i<m_Nodes.size(); ++i) {
        const CNode& node = *m_Nodes[i];

        string tax_id = x_GetNodeFeature(taxidFeatId, node);
        if (tax_id.empty())
            continue;
        
        CTaxIdSelSet::TTaxId id = (CTaxIdSelSet::TTaxId)NStr::StringToUInt(tax_id);
        if (!tids.IsSelected(id))
            continue;

        selectedNodes.push_back(i);
    }

}

void CTableDataBioTreeContainer::SelectFeatures(const CBioTreeSelection &selection, CFeatureDescr::TId srcFeatId, CFeatureDescr::TId dstFeatId, vector<size_t>& selectedNodes) const
{
    vector<string> selected_feature_values;
    selected_feature_values.reserve(selection.GetNodeIds().size());
    // Iterate over the selection tree and collect all the selected values. 
    ITERATE(CBioTreeSelection::TNodeIdsVector, iter, selection.GetNodeIds()) {                
        string feat_val = selection.GetFeatureValue(*iter, srcFeatId);
        if (feat_val.empty())
            continue;
        selected_feature_values.push_back(feat_val);
    }

    // Iterate over the tree to be updated and select all those nodes 
    // that have the feature dstFeatId equal to one of the selected values
    ITERATE(vector<string>, itFeatValue, selected_feature_values) {                
 
        for (size_t i=0; i<m_Nodes.size(); ++i) {
            const CNode& node = *m_Nodes[i];

            string feat_value = x_GetNodeFeature(dstFeatId, node);
            if (feat_value.empty())
                continue;
            if (*itFeatValue != feat_value)
                continue;
            
            selectedNodes.push_back(i);
        }
    }
}

string CTableDataBioTreeContainer::x_GetNodeFeature(int feature_id, const CNode& node) const
{
    string feature_value;

    if (node.CanGetFeatures()) {
        const CNodeFeatureSet::Tdata& features = node.GetFeatures().Get();
        ITERATE(CNodeFeatureSet::Tdata, it, features) {
            if ((*it)->GetFeatureid() == feature_id) {
                feature_value = (*it)->GetValue();
                break;
            }
        }
    }

    return feature_value;
}

CTableSelectionBioTreeContainer* CTableSelectionBioTreeContainer::CreateObject(SConstScopedObject&, ICreateParams*)
{
    return new CTableSelectionBioTreeContainer();
}

void CTableSelectionBioTreeContainer::GetRows (const ITableData& table, const CSelectionEvent& evt, vector<size_t>& rows) const
{
    enum SelectionMode {
        selectNone      =   0x0,
        selectNodeIds   =   0x1,
        selectSeqIds    =   0x2,
        selectTaxIds    =   0x4,
        selectBothIds   =   0x6,
        selectFeatures  =   0x8
    };

    unsigned selectionMode = selectNone;
    const CTableDataBioTreeContainer& bc = dynamic_cast<const CTableDataBioTreeContainer&>(table);
    const CBioTreeContainer& biotree = dynamic_cast<const CBioTreeContainer&>(*bc.m_Object);
    const CBioTreeSelection* bioTreeSelection = 0;
    TConstObjects objs;
    evt.GetIndexed("tree", objs);
    if (!objs.empty()) {
        bioTreeSelection = dynamic_cast<const CBioTreeSelection*>(objs[0].GetNonNullPointer());
    }
    else {
        selectionMode = selectSeqIds | selectTaxIds;
    }
        
    // Feature Id in the source (selection) tree
    CFeatureDescr::TId  srcFeatId = -1;
    // Feature Id in the destination (data source) tree
    CFeatureDescr::TId  dstFeatId = -1;

    if (selectNone == selectionMode) {
        assert(bioTreeSelection);
        if ((&biotree == bioTreeSelection->GetBioTree().GetNonNullPointer()) && CSelectionEvent::sm_TreeBroadcastOneToOne) {
            selectionMode = selectNodeIds;
        }
        else {
            // Helper object, used to search the destination (data source) tree 
            auto_ptr<CBioTreeSelection> dstTree(new CBioTreeSelection(CConstRef<CBioTreeContainer>(&biotree)));
            // The name of the feature, used to select node
            string  feat_name;

            ITERATE(vector<string>, it, CSelectionEvent::sm_TreeBroadcastProperties) {
                srcFeatId = bioTreeSelection->GetFeatureId(*it);
                if ((CFeatureDescr::TId)-1 == srcFeatId)
                    continue;

                dstFeatId = dstTree->GetFeatureId(*it);
                if ((CFeatureDescr::TId)-1 == dstFeatId)
                    continue;

                // The feature was found in both trees
                feat_name = *it;
                break;
            }
            if (feat_name == "seq-id") 
                selectionMode = selectSeqIds;
            else if (feat_name == "tax-id" || feat_name == "taxid")
                selectionMode = selectTaxIds;
            else 
                selectionMode = selectFeatures;
        }
    }

    switch(selectionMode)
    {
        case selectNodeIds :
            // Convert node ids to row numbers
            {
                const CBioTreeSelection::TNodeIdsVector &nodeIds = bioTreeSelection->GetNodeIds();
                rows.reserve(nodeIds.size());
                ITERATE(CBioTreeSelection::TNodeIdsVector, nodeId, nodeIds) {
                    int row = bc.FindRow((int)(*nodeId));
                    if (-1 == row) 
                        continue;
                    rows.push_back(row);
                }
            }
            break;
        case selectSeqIds :
            bc.SelectSeqIds(evt.GetIds(), dstFeatId, rows);
            break;
        case selectTaxIds :
            bc.SelectTaxIds(evt.GetTaxIDs(), dstFeatId, rows);
            break;
        case selectBothIds :
            bc.SelectSeqIds(evt.GetIds(), dstFeatId, rows);
            bc.SelectTaxIds(evt.GetTaxIDs(), dstFeatId, rows);
            break;
        case selectFeatures :
            bc.SelectFeatures(*bioTreeSelection, srcFeatId, dstFeatId, rows);
            break;
        default:
            break;
    }
}

void CTableSelectionBioTreeContainer::GetSelection (const ITableData& table, const vector<size_t>& rows, CSelectionEvent& evt) const
{
    const CTableDataBioTreeContainer& bc = dynamic_cast<const CTableDataBioTreeContainer&>(table);
    const CBioTreeContainer& biotree = dynamic_cast<const CBioTreeContainer&>(*bc.m_Object);
    CRef<CBioTreeSelection> bioTreeSelection(new CBioTreeSelection(CConstRef<CBioTreeContainer>(&biotree)));
    CBioTreeSelection::TNodeIdsVector &nodeIds = bioTreeSelection->GetNodeIds();
    nodeIds.reserve(rows.size());
    ITERATE(vector<size_t>, rowNum, rows) {
        CConstRef<CNode> node = bc.GetNode(*rowNum);
        if (node.IsNull())
            continue;
        if (!node->CanGetId())
            continue;
        nodeIds.push_back(node->GetId());
    }
    if (!nodeIds.empty())
        evt.AddIndexed("tree", *bioTreeSelection);

    // Broadcast Seq-id objects
    CFeatureDescr::TId seqidFeatId = bioTreeSelection->GetFeatureId("seq-id");
    if ((CFeatureDescr::TId)-1 != seqidFeatId) {
        TConstObjects objs;
        ITERATE(CBioTreeSelection::TNodeIdsVector, iter, bioTreeSelection->GetNodeIds()) {                
            string feat_val = bioTreeSelection->GetFeatureValue(*iter, seqidFeatId);
            if (feat_val.empty())
                continue;
            CRef<CSeq_id> id(new CSeq_id());
            if (StringToSeqId(feat_val, id))
                objs.push_back(CConstRef<CObject>(id));            
        }
        
        if (!objs.empty())
            evt.AddObjectSelection(objs);   
    }

    // Broadcast tax-ids
    CFeatureDescr::TId taxidFeatId = bioTreeSelection->GetFeatureId("tax-id");
    if ((CFeatureDescr::TId)-1 == taxidFeatId) 
        taxidFeatId = bioTreeSelection->GetFeatureId("taxid"); // Common misspelling

    if ((CFeatureDescr::TId)-1 != taxidFeatId) {
        
        ITERATE(CBioTreeSelection::TNodeIdsVector, iter, bioTreeSelection->GetNodeIds()) {                
            string feat_val = bioTreeSelection->GetFeatureValue(*iter, taxidFeatId);
            if (feat_val.empty())
                continue;
            try {
                CTaxIdSelSet::TTaxId id = (CTaxIdSelSet::TTaxId)NStr::StringToUInt(feat_val);
                evt.AddTaxIDSelection(id);
            }
            catch(const CException&) {
            }
        }

    }
}

END_NCBI_SCOPE
