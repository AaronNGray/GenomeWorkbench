/*  $Id: phylo_tree_node.cpp 43643 2019-08-13 15:20:19Z katargir $
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

#include <ncbi_pch.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_boundary_shapes.hpp>
#include <gui/opengl/glcolortable.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/objutils/utils.hpp>

#include <cmath>


#define PHYLO_FEAT_LABEL      "label"
#define PHYLO_FEAT_DIST       "dist"
#define PHYLO_FEAT_SEQID      "seq-id"
#define PHYLO_FEAT_TAXID      "tax-id"
#define PHYLO_FEAT_CLUSTERID  "cluster-id"
#define PHYLO_NODE_MARKER     "marker"


#define PHYLO_NODE_COLOR      "$NODE_COLOR"
#define PHYLO_EDGE_COLOR      "$EDGE_COLOR"
#define EDGE_GRADIENT         "$EDGE_GRADIENT"
#define PHYLO_NODE_COLLAPSED  "$NODE_COLLAPSED"
#define PHYLO_NODE_BOUNDED    "$NODE_BOUNDED"

#define PHYLO_LABEL_COLOR     "$LABEL_COLOR"
#define PHYLO_LABEL_BGCOLOR   "$LABEL_BG_COLOR"

#define PHYLO_NODE_SIZE       "$NODE_SIZE"
#define PHYLO_SELECT_CLUSTERS "$SEL_CLUSTERS"

#define MAX_PROPERTIES       255


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
// CPhyloTreeNode
///////////////////////////////////////////////////////////////////////////////


CPhyloTreeNode::CPhyloTreeNode()
{       
}

CPhyloTreeNode::CPhyloTreeNode(int x_id, double x_dist, const string & x_label)
{
    GetValue().Set(x_id, x_dist, x_label);
}

void  CPhyloTreeNode::ExpandCollapse(CBioTreeFeatureDictionary& dict,
                                     CPhyloNodeData::TDisplayChildren chds)
{
    GetValue().SetDisplayChildren(chds);

    // Sync expand/collapse feature value:
    if (GetValue().GetDisplayChildren() == CPhyloNodeData::eHideChildren)
        GetValue().SetFeature(dict, PHYLO_NODE_COLLAPSED, "1");
    else // expanded is default, so no need to save it explicitly
        GetValue().RemoveFeature(dict, PHYLO_NODE_COLLAPSED);
}

bool CPhyloTreeNode::CanExpandCollapse(CPhyloNodeData::TDisplayChildren chds)
{
    // don't do expand/cpllapse on root
    if (!HasParent()) {
        return false;
    }
    else if (chds == CPhyloNodeData::eShowChildren) {
        return !Expanded();
    }
    else if (chds == CPhyloNodeData::eHideChildren) {
        return !IsLeafEx();
    }
    return true;
}


void  CPhyloTreeNode::SetLabel(CBioTreeFeatureDictionary& dict,
                               const string & label)
{
    GetValue().SetLabel(label);
    GetValue().Sync(dict);
}

///////////////////////////////////////////////////////////////////////////////
// CPhyloNodeData
///////////////////////////////////////////////////////////////////////////////

const CPhyloNodeData::TColor CPhyloNodeData::TNoColor = "";

SExtendedNodeParms::SExtendedNodeParms(const SExtendedNodeParms& rhs)
{
	m_MarkerColors = rhs.m_MarkerColors;
	m_MarkerSize = rhs.m_MarkerSize;
	m_SelClusters = rhs.m_SelClusters;

	m_Boundary = NULL;

	if (rhs.m_Boundary != NULL) {
		m_Boundary = new CSubtreeBoundary(*rhs.m_Boundary);
	}
}

SExtendedNodeParms& SExtendedNodeParms::operator=(const SExtendedNodeParms& rhs)
{
	m_MarkerColors = rhs.m_MarkerColors;
	m_MarkerSize = rhs.m_MarkerSize;
	m_SelClusters = rhs.m_SelClusters;

	delete m_Boundary;
	m_Boundary = NULL;

	if (rhs.m_Boundary != NULL) {
		m_Boundary = new CSubtreeBoundary(*rhs.m_Boundary);
	}

	return *this;
}

SExtendedNodeParms::~SExtendedNodeParms()
{ 
	delete m_Boundary; 
	m_Boundary = NULL; 
}


CPhyloNodeData::CPhyloNodeData(const CPhyloNodeData& rhs)
	: m_ExtendedParms(NULL)
{
    *this = rhs;
}

CPhyloNodeData& CPhyloNodeData::operator=(const CPhyloNodeData& rhs)
{
    m_SeqID = rhs.m_SeqID;
    m_Features = rhs.m_Features;
	m_NumLeaves = rhs.m_NumLeaves;
    m_PamlCounter = rhs.m_PamlCounter;
	m_NumLeavesEx = rhs.m_NumLeavesEx;
    m_ClusterID = rhs.m_ClusterID;
    m_ID = rhs.m_ID;
    m_Label = rhs.m_Label;
    m_Distance = rhs.m_Distance;
    m_DistanceFromRoot = rhs.m_DistanceFromRoot;
    m_MaxChildDistance = rhs.m_MaxChildDistance;
    m_XY = rhs.m_XY;
    m_IDX = rhs.m_IDX;
    m_Angle = rhs.m_Angle;
    m_DrawAngle = rhs.m_DrawAngle;
    m_LabelRect = rhs.m_LabelRect;
    m_NodeOffset = rhs.m_NodeOffset;
    m_NodePixelOffset = rhs.m_NodePixelOffset;
    m_Selected = rhs.m_Selected;
    m_ColorType = rhs.m_ColorType;
    m_Children = rhs.m_Children;
    m_NodeWidth = rhs.m_NodeWidth;
    m_NodeEdColorIdx = rhs.m_NodeEdColorIdx;
	m_NodeFgColorIdx = rhs.m_NodeFgColorIdx;
    m_ClusterColorIdx = rhs.m_ClusterColorIdx;
    m_LabelFgColorIdx = rhs.m_LabelFgColorIdx;
    m_LabelBgColorIdx = rhs.m_LabelBgColorIdx;

	m_Size = rhs.m_Size;
	m_EdgeColorGradient = rhs.m_EdgeColorGradient;

    delete m_ExtendedParms;
    m_ExtendedParms = NULL;
    if (rhs.m_ExtendedParms != NULL) {
        m_ExtendedParms = new SExtendedNodeParms(*rhs.m_ExtendedParms);
	}
    
    m_NodeColor = rhs.m_NodeColor;
    m_FgColor = rhs.m_FgColor;

    return *this;
}

CPhyloNodeData::~CPhyloNodeData()
{
    delete m_ExtendedParms;
    m_ExtendedParms = NULL;
}

bool CPhyloNodeData::HasSelClusters() const 
{
	if (!x_HasExtendedParms())
		return false;

    return (m_ExtendedParms->m_SelClusters.size() > 0);
}

CPhyloNodeData::TSelClusterIDs& CPhyloNodeData::GetSelClusters() 
{ 
    if (!x_HasExtendedParms())
        m_ExtendedParms = new SExtendedNodeParms();

    return m_ExtendedParms->m_SelClusters;
}

void CPhyloNodeData::SetSelClusters(const TSelClusterIDs& sc) 
{ 
    if (!x_HasExtendedParms())
        m_ExtendedParms = new SExtendedNodeParms();

    m_ExtendedParms->m_SelClusters = sc;
}

bool CPhyloNodeData::HasNodeMarker() const
{
    if (!x_HasExtendedParms())
		return false;

    return (m_ExtendedParms->m_MarkerColors.size() > 0);
}

vector<CRgbaColor>& CPhyloNodeData::GetMarkerColors() 
{ 
    if (!x_HasExtendedParms())
        m_ExtendedParms = new SExtendedNodeParms();

    return m_ExtendedParms->m_MarkerColors;
}

float CPhyloNodeData::GetMarkerSize() const 
{ 
    if (!x_HasExtendedParms())
		return 0.0f;
	else
        return m_ExtendedParms->m_MarkerSize;
}

void CPhyloNodeData::SetMarkerSize(float f) 
{ 
    if (!x_HasExtendedParms())
        m_ExtendedParms = new SExtendedNodeParms();

    m_ExtendedParms->m_MarkerSize = f;
}

void CPhyloNodeData::SetMarkerColors(const string& str)
{
	// marker colors should be in format:
	// [r1,g1,b1,a1][r2,g2,b2,a2]....[rn,gn,bn,an], n = (1..n)

	string remainder = str;

	string marker_size_str = x_GetParameter(str, "size");
	float marker_size = 2.0f;
	if (marker_size_str != "") {
		marker_size = NStr::StringToDouble(marker_size_str, NStr::fConvErr_NoThrow);
		if (marker_size <= 0.0f) marker_size = 2.0f;
	}
	SetMarkerSize(marker_size);

	string::size_type idx = remainder.find_first_of(']');

	while (idx != string::npos) {
		string color = remainder.substr(0, idx + 1);
		size_t start_idx = remainder.find_first_of('[');
		if (start_idx == string::npos)
			return;
		color = color.substr(start_idx, color.length() - start_idx);

        if (!x_HasExtendedParms())
            m_ExtendedParms = new SExtendedNodeParms();

        m_ExtendedParms->m_MarkerColors.push_back(CRgbaColor(color));

		remainder = remainder.substr(idx + 1, remainder.size() - (idx + 1));
		idx = remainder.find_first_of(']');
	}
}

string CPhyloNodeData::GetMarkerColorsAsString()
{
    if (!x_HasExtendedParms())
		return "";

	string c;

    for (size_t i = 0; i<m_ExtendedParms->m_MarkerColors.size(); ++i) {
		// Convert to string format [r,g,b] where r,g,b are 0..255.
        c += string("[") + m_ExtendedParms->m_MarkerColors[i].ToString(false, true) + std::string("] ");
	}

	return c;
}

CPhyloNodeData::TBoundingState CPhyloNodeData::GetBoundedDisplay() const
{ 
    if (!x_HasExtendedParms())
		return eUnBounded;

    return (m_ExtendedParms->m_Boundary == NULL) ? eUnBounded : eBounded;
}

CSubtreeBoundary* CPhyloNodeData::GetSubtreeBoundary()
{
    if (!x_HasExtendedParms())
        m_ExtendedParms = new SExtendedNodeParms();

    if (m_ExtendedParms->m_Boundary == NULL)
        m_ExtendedParms->m_Boundary = new CSubtreeBoundary();

    return m_ExtendedParms->m_Boundary;
}

void CPhyloNodeData::DeleteSubtreeBoundary()
{
    if (!x_HasExtendedParms())
		return;

    delete m_ExtendedParms->m_Boundary;
    m_ExtendedParms->m_Boundary = NULL;
}

void CPhyloNodeData::SetFeature(CBioTreeFeatureDictionary& dict,
                                const string & name, 
                                const string & value)
{    
    TBioTreeFeatureId   id;

    if (dict.HasFeature(name)){
        id = dict.GetId(name);
    }
    else { // find smallest available id
        for (id=0; id < MAX_PROPERTIES; id++){
            if (!dict.HasFeature(id)) {
                dict.Register(id, name);
                break;
            }
        }
        if (id==MAX_PROPERTIES) {
             NCBI_THROW(CException, eUnknown,
                        "CPhyloTreeNode::SetFeature - maximum number of features exceeded");
        }
    }
    m_Features.SetFeature(id, value);
}

void CPhyloNodeData::RemoveFeature(CBioTreeFeatureDictionary& dict,
                                   const string & name)
{    
    TBioTreeFeatureId   id;

    if (dict.HasFeature(name)){
        id = dict.GetId(name);
        m_Features.RemoveFeature(id);
    }
}

void CPhyloNodeData::InitFeatures(const CBioTreeFeatureDictionary& dict,
                                  CRgbaGradColorTable* t)
{
    const CBioTreeFeatureList & bfl = m_Features;

    const string& dist = bfl.GetFeatureValue(dict.GetId(PHYLO_FEAT_DIST));
    if (!dist.empty()) {
        try {
            double ddist = NStr::StringToDouble(dist);
            if (ddist < 0.0)
                ddist = 0.0;
            SetDistance(ddist);
        }
        catch(std::exception&) {}
    }

    const string & size = bfl.GetFeatureValue(dict.GetId(PHYLO_NODE_SIZE));
    SetNodeSize(-1.0f); // set to default size
    if (!size.empty()) {
        try {
            //short isize = short(NStr::StringToInt(size));
            float isize = NStr::StringToDouble(size, NStr::fConvErr_NoThrow);
            SetNodeSize(isize);
        }
        catch(std::exception&) {}
    }

	if (HasNodeMarker())
		GetMarkerColors().clear();
    const string & marker = bfl.GetFeatureValue(dict.GetId(PHYLO_NODE_MARKER));
    if (!marker.empty()) {
        try {
            SetMarkerColors(marker);
        }
        catch(std::exception&){}
    }

    // cluster ID if any
    string cid = bfl.GetFeatureValue(dict.GetId(PHYLO_FEAT_CLUSTERID));
    int cluster_id=-1;
    if (!cid.empty()) {
        int sel_id = NStr::StringToInt(cid, NStr::fConvErr_NoThrow);
        if (sel_id  ||  errno == 0)
            cluster_id = sel_id;
    }
    SetClusterID(cluster_id);


    // Clusters of selected nodes, if any
    string sel_clusters = bfl.GetFeatureValue(dict.GetId(PHYLO_SELECT_CLUSTERS));
	if (HasSelClusters())
        m_ExtendedParms->m_SelClusters.clear();
   
    if (!sel_clusters.empty()) {
        vector<CTempString> arr;
        NStr::Split(sel_clusters, " ", arr);
        for (size_t i=0; i<arr.size(); ++i) {        
            int sel_id = NStr::StringToInt(arr[i], NStr::fConvErr_NoThrow);
            if (!sel_id  &&  errno != 0)
                continue;
            GetSelClusters().push_back(sel_id);                      
        }
    }
   
    
    // collapse/expand node
    const string & ce = bfl.GetFeatureValue(dict.GetId(PHYLO_NODE_COLLAPSED));
    if (ce == "1") SetDisplayChildren(CPhyloNodeData::eHideChildren);
    else SetDisplayChildren(CPhyloNodeData::eShowChildren);

    // node (and subtree) has bounding region (or not)
    const string & bounded = bfl.GetFeatureValue(dict.GetId(PHYLO_NODE_BOUNDED));
    if (!bounded.empty()) {        
        GetSubtreeBoundary()->CreateShapes(bounded);
    }
	else {
		DeleteSubtreeBoundary();
	}


    SetNodeFgColorIdx(-1);
    SetNodeEdgeColorIdx(-1);
    SetLabelColorIdx(-1);
    SetLabelBgColorIdx(-1);

    string node_color = bfl.GetFeatureValue(dict.GetId(PHYLO_NODE_COLOR));
    size_t color_idx = -1;
    if (!node_color.empty()) {
        try {
            CRgbaColor c(node_color);

            if (!t->FindColor(c, color_idx)) {
                color_idx = t->AddColor(c);             
            }
            SetNodeFgColorIdx((short)color_idx);
        }
        catch (CException&) {}
    }

    node_color = bfl.GetFeatureValue(dict.GetId(PHYLO_EDGE_COLOR));
    if (!node_color.empty()) {
        try {
            CRgbaColor c(node_color);

            if (!t->FindColor(c, color_idx)) {
                color_idx = t->AddColor(c);              
            }
            SetNodeEdgeColorIdx((short)color_idx);
        }
        catch (CException&) {}
    }
    
    // Get gradient color property 
    node_color = bfl.GetFeatureValue(dict.GetId(EDGE_GRADIENT));
    if (node_color != "") {
        try {
			m_EdgeColorGradient = NStr::StringToDouble(node_color) > 0.0 ? true : false;
        }
        catch (CException&) {}
    }

    node_color = bfl.GetFeatureValue(dict.GetId(PHYLO_LABEL_COLOR));
    if (!node_color.empty()) {
        try {
            CRgbaColor c(node_color);

            if (!t->FindColor(c, color_idx)) {
                color_idx = t->AddColor(c);
            }
            SetLabelColorIdx((short)color_idx);
        }
        catch (CException&) {}
    }
    node_color = bfl.GetFeatureValue(dict.GetId(PHYLO_LABEL_BGCOLOR));
    if (!node_color.empty()) {
        try {
            CRgbaColor c(node_color);

            if (!t->FindColor(c, color_idx)) {
                color_idx = t->AddColor(c);                
            }
            SetLabelBgColorIdx((short)color_idx);
        }
        catch (CException&) {}
    }
}

void CPhyloNodeData::Init(const CBioTreeFeatureDictionary& dict,
                          CRgbaGradColorTable* t)
{   
    const CBioTreeFeatureList &bfl = m_Features;

    const string& sid = bfl.GetFeatureValue(dict.GetId(PHYLO_FEAT_SEQID));
    if (!sid.empty()) {
        try {
            SetSeqID(new objects::CSeq_id(sid));
        }
        catch(CException& )
        {
            // try ad-hoc GI extraction (for misformed seq-id strings like: "gi|55823257|ref|YP_141698.1"
            // (data coming from some other ad-hoc scripts)
            //
            try {
                string gi_str;
                CSeqUtils::GetGIString(sid, &gi_str);
                if (!gi_str.empty()) {
                    SetSeqID(new objects::CSeq_id(gi_str));            
                }
                else {
                    string lcl_sid = "lcl|";
                    lcl_sid.append(sid);
                    SetSeqID(new objects::CSeq_id(lcl_sid));
                }
            }
            catch(std::exception&)
            {
            }

        }
        catch(std::exception&)
        {
        }
    }
    
    InitFeatures(dict, t);
}

void  CPhyloNodeData::Sync(CBioTreeFeatureDictionary& dict)
{
    string tmp;
    NStr::DoubleToString(tmp, GetDistance());

    SetFeature(dict, PHYLO_FEAT_DIST,     tmp);
    SetFeature(dict, PHYLO_FEAT_LABEL,    GetLabel());

    if ( GetDisplayChildren()==CPhyloNodeData::eHideChildren)
        SetFeature(dict, PHYLO_NODE_COLLAPSED, "1");       
    else // expanded is default, so no need to save it explicitly
        RemoveFeature(dict, PHYLO_NODE_COLLAPSED);

    if (GetClusterID() == -1) // No need for empty ids
        RemoveFeature(dict, PHYLO_FEAT_CLUSTERID);

    // Not sure we need to save these if we save all selection
    // info in user-data.
	if (HasSelClusters()) {
        string sel_ids;
        for (size_t i=0; i<GetSelClusters().size(); ++i) {
            string sel_id;
			NStr::IntToString(sel_id, GetSelClusters()[i]);
            if (i>0)
                sel_ids += " ";
            sel_ids += sel_id;                    
        }
        SetFeature(dict, PHYLO_SELECT_CLUSTERS, sel_ids);
    }
    else {
        RemoveFeature(dict, PHYLO_SELECT_CLUSTERS);
    }
}

CBioTreeFeatureList& CPhyloNodeData::GetBioTreeFeatureList() 
{ 
    return m_Features;  
}

void CPhyloNodeData::SetBioTreeFeatureList(const CBioTreeFeatureList& btcfl) 
{ 
    m_Features = btcfl; 
}

int CPhyloNodeData::GetPrimaryCluster() const
{
	if (HasSelClusters())
        return m_ExtendedParms->m_SelClusters[0];

    if (m_ClusterID != -1) {
        return m_ClusterID;
    }

    return -1;
}

int CPhyloNodeData::GetNumClusters() const
{
    // This is for the clustering algo so when there are multiple
    // selection clusters it can set up a marker with each of
    // those.  So return the selection clusters size, unless
    // 0, in which case we return number based on cluster ID.
	if (HasSelClusters())
        return m_ExtendedParms->m_SelClusters.size();

    return (m_ClusterID != -1 ? 1 : 0);
}

void CPhyloNodeData::SetId(TID x_id)    
{ 
    m_ID = x_id;
}

string CPhyloNodeData::x_GetParameter(const string& features, const string& parm)
{
    string parm_lower = parm;
    parm_lower = NStr::ToLower(parm_lower) + "=";

    string features_lower = features;
    features_lower = NStr::ToLower(features_lower);

    size_t idx = features_lower.find(parm_lower);
    if (idx != string::npos) {
        size_t end_idx = features.find_first_of(" ,\t", idx);
        if (end_idx == string::npos)
            end_idx = features.size();

        return features.substr(idx + parm_lower.length(), end_idx-(idx+parm_lower.length()));
    }

    return "";
}

// Get angle after screen is scaled (compressed/stretched) aniostropically
float CPhyloNodeData::GetScaledAngle(const CVect3<float>& scale, float a, bool for_label) const
{
    // Angle is unscaled so multiple x/y by inverse scale and recompute angle
    // to get actual screen angle after scaling
    float cos_a = cosf(a) / scale.X();
    float sin_a = sinf(a) / scale.Y();
    a = atan2f(sin_a, cos_a);

    // rotation for angles >90 and <270 is different since text those
    // labels end (not start) at the node
    if (for_label && cos_a < 0.0f) {
        a = a - M_PI;
    }

    return a;
}

CVect2<float> CPhyloNodeData::GetNodePosEx(const CVect3<float>& scale, 
                                           CPhyloTreeScheme* scheme, 
                                           float& node_size,
                                           float size_scaler) const
{
    CVect2<float> p = m_XY - m_NodeOffset;

    // Get size of node possibly modified if it is collapsed
    node_size = GetDefaultNodeSize(scheme);

    // If not expanded:
    if (GetDisplayChildren() != CPhyloNodeData::eShowChildren) {
        if (m_Angle != 0.0f) {
            float a = GetScaledAngle(scale, m_Angle, false);             

            // Bigger collapsed nodes start at end of branch and extend out, so when drawing
            // a highlight, it may look worse hanging out past the end of the branch.
            p.X() += cosf(a) * node_size * size_scaler * scale.X();
            p.Y() += sinf(a) * node_size * size_scaler * scale.Y();
        }
        else {
            p.X() += node_size * size_scaler * scale.X();
        }
    }

    return p;
}

void CPhyloNodeData::Render(const CGlPane& pane,
    const CGlTextureFont& font,
    CRgbaGradColorTable* color_table,
    const CVect3<float>& scale,
    bool truncate_labels,
    TModelUnit max_len)
{
    if (!m_Visible || m_AlreadyDrawn || m_Text.empty())
        return;

    CGlRect<float> r;
    TModelUnit pos_xmin;
    TModelUnit pos_xmax;
    TModelUnit pos_y;

    r = GetLabelRect();

    pos_xmin = (TModelUnit)(X() + (r.Left() * scale.X()));
    pos_xmax = (TModelUnit)(X() + (r.Right() * scale.X()));
    pos_y = (TModelUnit)(Y() + (r.Bottom() * scale.Y()));

    string text_out = m_Text;

    TModelUnit text_len = 0.0;
    TModelUnit vp_width = (TModelUnit)pane.GetViewport().Width();
    TModelUnit tw = font.TextWidth(m_Text.c_str());

    if (truncate_labels) {
        TModelUnit screen_minx = pane.ProjectX(pos_xmin);
        TModelUnit screen_maxx = pane.ProjectX(pos_xmax);

        if (cos(GetAngle()) >= 0.0) {
            text_len = std::min(tw, vp_width - screen_minx);
        }
        else {
            text_len = std::min(tw, screen_maxx);
        }
    }

    // max_len is in screen (pixel) coordinates
    if (max_len > 0.0) {
        text_len = std::min(text_len, max_len);
    }

    // Trim the string if the target length is > 0 and < the strings
    // current length
    if (text_len > 0.0 && text_len < tw) {
        if (cos(GetAngle()) >= 0.0) {
            text_out = font.Truncate(m_Text.c_str(), text_len);
        }
        else {
            std::reverse(text_out.begin(), text_out.end());
            text_out = font.Truncate(text_out.c_str(), text_len);
            std::reverse(text_out.begin(), text_out.end());

            // Text is trimmed but since it ends at the node we have to move the 
            // start of the text in the direction of the node by the amount
            // that we trimmed:
            TModelUnit new_width = font.TextWidth(text_out.c_str());

            pos_xmin += (tw - new_width)*scale.X();
        }

        if (text_out == "...")
            return;
    }

    IRender& gl = GetGl();

    // Draw label background, if it has one specified
    if (m_LabelBgColorIdx != -1) {
        CRgbaColor c = color_table->GetColor(m_LabelBgColorIdx);
        gl.Color4fv(c.GetColorArray());
       
        TModelUnit text_h = font.GetMetric(IGlFont::eMetric_CharHeight);
        TModelUnit text_w = font.TextWidth(text_out.c_str());

        gl.Enable(GL_BLEND);
        gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        gl.Begin(GL_TRIANGLE_STRIP);
            gl.Vertex2d(pos_xmin - 1.0 * pane.GetScaleX(), pos_y + (text_h + 1.0) * pane.GetScaleY());
            gl.Vertex2d(pos_xmin - 1.0 * pane.GetScaleX(), pos_y - 1.0*pane.GetScaleY());
            gl.Vertex2d(pos_xmin + (text_w + 1.0) * pane.GetScaleX(), pos_y + (text_h + 1.0) * pane.GetScaleY());
            gl.Vertex2d(pos_xmin + (text_w + 1.0) * pane.GetScaleX(), pos_y - 1.0*pane.GetScaleY());
        gl.End();
    } 

    gl.Color4fv(m_FgColor.GetColorArray());
    SetDrawAngle(0.0f);

    font.TextOut(pos_xmin, pos_y, text_out.c_str());
}

void CPhyloNodeData::RenderRotated(const CGlPane& pane,
                                   const CGlTextureFont& font,
                                   CRgbaGradColorTable* color_table,
                                   const CVect3<float>& scale,
                                   float orig_angle,
                                   bool truncate_labels)
{
    if (!m_Visible || m_AlreadyDrawn)
        return;

    float angle;
    CVect4<CVect2<float> > corners = GetRotatedTextBox(scale, angle, orig_angle);

    float angle_degrees = angle * (360.0f / (2.0f*float(M_PI)));

    IRender& gl = GetGl();

    // Draw label background, if it has one specified
    if (m_LabelBgColorIdx != -1) {
        CRgbaColor c = color_table->GetColor(m_LabelBgColorIdx);
        gl.Color4fv(c.GetColorArray());

        gl.Enable(GL_BLEND);
        gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
       
        gl.Begin(GL_TRIANGLE_STRIP);
            gl.Vertex2f(corners[3].X(), corners[3].Y());
            gl.Vertex2f(corners[0].X(), corners[0].Y());
            gl.Vertex2f(corners[2].X(), corners[2].Y());
            gl.Vertex2f(corners[1].X(), corners[1].Y());
        gl.End();
    } 

    string text_out = m_Text;
    if (truncate_labels) {
        // Find bounding box for text rectangle:
        float minx = 1e10f;
        float miny = 1e10f;
        float maxx = -1e10f;
        float maxy = -1e10f;
        for (size_t i = 0; i < 4; ++i) {
            minx = std::min(corners[i].X(), minx);
            miny = std::min(corners[i].Y(), miny);
            maxx = std::max(corners[i].X(), maxx);
            maxy = std::max(corners[i].Y(), maxy);
        }

        // Get viewport (pixel) coordinates of bounding box:
        TModelUnit screen_maxx = pane.ProjectX(maxx);
        TModelUnit screen_minx = pane.ProjectX(minx);
        TModelUnit screen_miny = pane.ProjectY(miny);
        TModelUnit screen_maxy = pane.ProjectY(maxy);

        TVPPoint p1 = pane.Project(corners[0].X(), corners[0].Y());
        TVPPoint p2 = pane.Project(corners[1].X(), corners[1].Y());
        CVect2<TModelUnit> text_box_vec(p2.X() - p1.X(), p2.Y() - p1.Y());
        TModelUnit text_box_len = text_box_vec.Length();

        TModelUnit pct = 1.0;
        TModelUnit vp_width = (TModelUnit)pane.GetViewport().Width();
        TModelUnit vp_height = (TModelUnit)pane.GetViewport().Height();

        // Trim the string if it extends past the viewport's borders of
        // (0,0, vp_width, vp_height)

        // In positive (cos > 0) direction text starts at the node
        if (cos(orig_angle) >= 0.0) {
            // If text extends past vp_width:
            if (screen_minx < vp_width && screen_maxx > vp_width) {
                pct = (vp_width - screen_minx) / (screen_maxx - screen_minx);
            }
            // If text extends past vp_height:
            if (screen_miny < vp_height && screen_maxy > vp_height) {
                pct = std::min(pct, (vp_height - screen_miny) / (screen_maxy - screen_miny));
            }
            // If text textends below y=0:
            else if (screen_maxy > 0.0 && screen_miny < 0.0) {
                pct = std::min(pct, (screen_maxy) / (screen_maxy - screen_miny));
            }

            // Trim text if needed:
            if (pct < 1.0) {
                text_out = font.Truncate(m_Text.c_str(), text_box_len*pct);
            }
        }
        // In negative (cos < 0) direction text ends at node so we truncate text
        // beginning, not the end
        else {
            // If text starts before x=0:
            if (screen_minx < 0.0 && screen_maxx > 0.0) {
                pct = (screen_maxx) / (screen_maxx - screen_minx);
            }
            // If text starts above vp_height:
            if (screen_miny < vp_height && screen_maxy > vp_height) {
                pct = std::min(pct, (vp_height - screen_miny) / (screen_maxy - screen_miny));
            }
            // If text starts below y=0:
            else if (screen_maxy > 0.0 && screen_miny < 0.0) {
                pct = std::min(pct, (screen_maxy) / (screen_maxy - screen_miny));
            }

            // Trim text if needed:
            if (pct < 1.0) {              
                TModelUnit tw = font.TextWidth(m_Text.c_str());
              
                // Reverse the text so that the beginning (left side) is truncated
                // then set width to total width - width of text to left of viewport
                std::reverse(text_out.begin(), text_out.end());
                text_out = font.Truncate(text_out.c_str(), text_box_len*pct);
                std::reverse(text_out.begin(), text_out.end());

                // Text is trimmed but since it ends at the node we have to move the 
                // start of the text in the direction of the node by the amount
                // that we trimmed:
                TModelUnit new_width = font.TextWidth(text_out.c_str());

                CVect2<float> text_dir = corners[1] - corners[0];

                float delta = float((tw-new_width)/tw);
                corners[0] = corners[0] + text_dir * delta;
            }
        }
        if (text_out == "...")
            return;
    }

    gl.Color4fv(m_FgColor.GetColorArray());
    SetDrawAngle(angle);

    font.BeginText();
    font.WriteText(corners[0].X(), corners[0].Y(), text_out.c_str(), TModelUnit(angle_degrees));
    font.EndText();
}

/// Return text box (m_LabelRect) rotated to align with radial edges
CVect4<CVect2<float> > CPhyloNodeData::GetRotatedTextBox(const CVect3<float>& scale, 
                                                         float& angle, float orig_angle) const
{
    CGlRect<float> r = GetLabelRect();
    CVect4<CVect2<float> > result;

    angle = GetScaledAngle(scale, m_Angle);

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    CVect2<float> dir(0.0f, 0.0f);
    float overhang = GetNodeOverhang(CVect2<float>(scale.X(), scale.Y()));
    if (overhang > 0.0f) {
        if (orig_angle < 0.0f)
            overhang *= -1.0f;
        dir.Set(cosf(orig_angle)*overhang*scale.X(), sinf(orig_angle)*overhang*scale.Y());
    }

    // First corner is (left, bottom)
    for (int corner=0; corner<4; ++corner) {
        CGlPoint<float> pos = r.GetCorner(corner);
       
        // multiply corner by standard 2D rotation matrix:
        CVect2<float> rpos((pos.X())*cos_a-pos.Y()*sin_a, (pos.X())*sin_a + pos.Y()*cos_a);
        //CVect2<float> rpos((overhang + pos.X())*cos_a - pos.Y()*sin_a, (overhang + pos.X())*sin_a + pos.Y()*cos_a);
        rpos.X() = X() + scale.X()*rpos.X() + dir.X();
        rpos.Y() = Y() + scale.Y()*rpos.Y() + dir.Y();
        result[corner] = rpos;
    }

    return result;
}

/// Return the text starting point rotated to align with radial edges
CVect2<float> CPhyloNodeData::GetRotatedTextPos(const CVect3<float>& scale,
                                                float& angle) const
{
    CGlRect<float> r = GetLabelRect();

    angle = GetScaledAngle(scale, m_Angle);
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    // Vector from node to lower left corner of text
    CVect2<float> tp(r.Left(), r.Bottom());

    CVect2<float> tpr(tp.X()*cos_a-tp.Y()*sin_a, tp.X()*sin_a + tp.Y()*cos_a);

    // position of text (which will be rotated) is the rotated offset tpr
    CVect2<float> result(X() + tpr.X()*scale.X(),
                         Y() + tpr.Y()*scale.Y());

    return result;
}

CVect2<float> CPhyloNodeData::GetTextOut(CGlPane& pane, 
                                         const CGlTextureFont& font,
                                         const CVect3<float>& scale,
                                         bool truncate_labels,
                                         string& text_out)
{
    CGlRect<float> r;
    CVect2<float> pos;

    r = GetLabelRect();    

    pos.X() = (float)(X() +
        (r.Left() * scale.X()));
    pos.Y() = (float)(Y() + 
        (r.Bottom() * scale.Y()));


    if (truncate_labels) {
        float screen_x = (float)(pane.ProjectX(pos.X()));

        // Label may go off right side of screen so see if it has
        // to be truncated on the right;
        if (screen_x > 0.0f) {
            float w = ((float)pane.GetViewport().Width()-screen_x);
            text_out = font.Truncate(m_Text.c_str(), w);
        }
        // Label starts before left side of screen (definitly clipped)
        // so truncate it on the left
        else {
            text_out = m_Text;
            // Reverse the text so that the beginning (left side) is truncated
            // then set width to total width - width of text to left of viewport
            std::reverse(text_out.begin(), text_out.end());
            text_out = font.Truncate(text_out.c_str(), r.Width()+screen_x);
            std::reverse(text_out.begin(), text_out.end());
            // After truncation, draw text from left edge of viewport
            pos.X() = (float)pane.UnProjectX(0.0);        
        }
    }
    else {
        text_out = m_Text;
    }

    return pos;
}


void CPhyloNodeData::GetLabelRect(const CVect2<float>& scale,                                 
                                  CVect2<float>& lower_left,
                                  CVect2<float>& upper_right,
                                  bool rotated)
{
    CGlRect<float> r = GetLabelRect();

    if (!rotated || r.Width() == 0.0f || r.Height() == 0.0f) {
        lower_left.X() = X() + r.Left()*scale.X();
        lower_left.Y() = Y() + r.Bottom()*scale.Y();

        upper_right.X() = X() + r.Right()*scale.X();
        upper_right.Y() = Y() + r.Top()*scale.Y();
    }
    else {
        lower_left.X() = std::numeric_limits<float>::max();
        lower_left.Y() = std::numeric_limits<float>::max();

        upper_right.X() = -std::numeric_limits<float>::max();
        upper_right.Y() = -std::numeric_limits<float>::max();


        float ang;
        CVect3<float> s3(scale.X(), scale.Y(), 1.0f);
        CVect4<CVect2<float> > corners = GetRotatedTextBox(s3, ang);

        for (int corner=0; corner<4; ++corner) {
            lower_left.X() = std::min(lower_left.X(), corners[corner].X());
            lower_left.Y() = std::min(lower_left.Y(), corners[corner].Y());

            upper_right.X() = std::max(upper_right.X(), corners[corner].X());
            upper_right.Y() = std::max(upper_right.Y(), corners[corner].Y());
        }
    }
}

bool CPhyloNodeData::PointInTextBox(const CVect2<float>& scale, 
                                    CVect2<float> pt, 
                                    bool rotated)
{
    CGlRect<float> r = GetLabelRect();

    // Put the selection position in pixel coordinates relative to the node
    // since that's how text box coordinates are stored.
    pt.X() -= X();
    pt.Y() -= Y();

    pt.X() *= 1.0f/scale.X();
    pt.Y() *= 1.0f/scale.Y();

    if (rotated && m_DrawAngle != 0.0f) {   
        // Rotate the selection point by the opposite angle rather than
        // rotating the whole text box
        float a = -m_DrawAngle;

        // rotate point around node:
        float cos_a = cosf(a);
        float sin_a = sinf(a);
        CVect2<float> rpos(pt.X()*cos_a-pt.Y()*sin_a, pt.X()*sin_a + pt.Y()*cos_a);
        pt = rpos;
    }
 
    // Is point inside text box (using pixel coords)?
    if (r.PtInRect(pt.X(), pt.Y()))
        return true;
    else
        return false;

}

CGlRect<float> CPhyloNodeData::GetLabelRect(const CVect2<float>& scale, bool rotated)
{   
    if (!rotated) {    
        return GetLabelRect();
    }
    else {
        CGlRect<float> r(GetLabelRect());

        CGlRect<float> rr(std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max(),
                          -std::numeric_limits<float>::max(),
                          -std::numeric_limits<float>::max());

        float a = m_Angle;

        // Angle is unscaled so multiple x/y by inverse scale and recompute angle
        // to get actual screen angle after scaling
        float cos_a = cosf(a)*1.0f/scale.X();
        float sin_a = sinf(a)*1.0f/scale.Y();
        a = atan2f(sin_a, cos_a);

        // rotation for angles >90 and <270 is different since text for those
        // labels ends (not starts) at the node
        if (cos_a < 0.0f) {
            a = a-M_PI;
        }

        cos_a = cosf(a);
        sin_a = sinf(a);

        // First corner is (left, bottom)
        for (int corner=0; corner<4; ++corner) {
            CGlPoint<float> pos = r.GetCorner(corner);

            // multiply corner by standard 2D rotation matrix:
            CVect2<float> rpos(pos.X()*cos_a-pos.Y()*sin_a, pos.X()*sin_a + pos.Y()*cos_a);

            rr.SetLeft(std::min(rr.Left(), rpos.X()));
            rr.SetRight(std::max(rr.Right(), rpos.X()));
            rr.SetBottom(std::min(rr.Bottom(), rpos.Y()));
            rr.SetTop(std::max(rr.Top(), rpos.Y()));
        }

        return rr;
    }
}

void CPhyloNodeData::GetBoundingRect(const CVect2<float>& scale,                                     
                                     CVect2<float>& lower_left,
                                     CVect2<float>& upper_right, 
                                     bool rotated)
{
    GetLabelRect(scale, lower_left, upper_right, rotated);

    float deltax = m_NodeSize*scale.X() + std::abs(m_NodeOffset.X());
    float deltay = m_NodeSize*scale.Y() + std::abs(m_NodeOffset.Y());

    lower_left.X() = std::min(lower_left.X(), X()-deltax);
    lower_left.Y() = std::min(lower_left.Y(), Y()-deltay);
    upper_right.X() = std::max(upper_right.X(), X()+deltax);
    upper_right.Y() = std::max(upper_right.Y(), Y()+deltay);
}

void CPhyloNodeData::SetVisible(bool b) 
{
    m_Visible = b; 
}

float CPhyloNodeData::GetMinX(const CVect2<float>& scale)
{
    return (X() + (GetLabelRect().Left() * scale.X()));
}

float CPhyloNodeData::GetDefaultNodeSize(const CPhyloTreeScheme* scheme) const
{
    float       forced_size = GetNodeSize();
    float       def_node_size = 0;

    if (forced_size < 0.0f) {
        if (GetDisplayChildren() == eHideChildren) {
            if (scheme->GetBoaNodes()) {
                double max_child_size = scheme->GetMaxNumChildren();
                double min_child_size = scheme->GetMinNumChildren();
                double this_childs_size = std::min(max_child_size, double(GetNumLeaves() + 2));

                double min_node_size = scheme->GetMinNodeSize();
                double max_node_size = scheme->GetMaxNodeSize();

                def_node_size = (float)(min_node_size + (max_node_size - min_node_size) *
                    (log(this_childs_size) - log(min_child_size)) /
                    (log(max_child_size) - log(min_child_size)));
            }
            else {
                // Make all collapsed nodes a little bigger than regular ones
                def_node_size = (float)scheme->GetNodeSize()*1.5;
            }
        }
        else if (GetNumLeavesEx() > 0) {
            def_node_size = (float)scheme->GetNodeSize();
        }
        else {
            def_node_size = (float)scheme->GetLeafNodeSize();
        }
    }
    else {
        def_node_size = (float)forced_size;
    }

    return def_node_size;
}

float CPhyloNodeData::GetNodeLayoutSize(const CPhyloTreeScheme* scheme) const
{
    // For layout, smallest size is scheme->GetNodeSize(), otherwise parent nodes (of this size)
    // may be forced into overlapping (and 0-sized nodes mess up layout which uses node sizes for spacing)
    float size = GetDefaultNodeSize(scheme);
    if (size < (float)scheme->GetNodeSize())
        size = (float)scheme->GetNodeSize();

    return std::max(size, 1.0f);
}

float CPhyloNodeData::GetNodeWidthScaler(const CPhyloTreeScheme* scheme) const
{
    // If this function returns 0 (tree without distance) caller should
    // use default distance
    m_NodeWidth = 0.0f;
      
    double max_branch_dist = scheme->GetMaxBranchDist();
    if (scheme->GetBoaNodes() &&
        GetDisplayChildren() == eHideChildren &&
        max_branch_dist > 1e-04f) {

        double min_width = scheme->GetMinNodeSize();
        double max_width = scheme->GetMaxNodeSize();

        // Rescale branch distance which goes from 0 to max_branch_dist
        // to be in the range scheme->GetMinNumChildren()..scheme->GetMaxNumChildren()
        // which is the range we use for node size (height) in GetDefaultNodeSize
        double max_dist = scheme->GetMaxNumChildren();
        double min_dist = scheme->GetMinNumChildren();

        float scaled_max_child_dist = (m_MaxChildDistance / max_branch_dist)*(max_dist - min_dist) + min_dist;

        // We multiply by 4 here to get more length than width in our collapsed
        // nodes since width crowds out other things more than length does.
        m_NodeWidth = 4.0f*((float)(min_width + (max_width - min_width) *
          (log(scaled_max_child_dist) - log(min_dist)) /
          (log(max_dist) - log(min_dist))));
    }
    else {
        m_NodeWidth = 2.0f*GetDefaultNodeSize(scheme);
    }

    return m_NodeWidth;
}

float CPhyloNodeData::GetNodeOverhang(const CVect2<float>& scale) const
{
    // Only interested in this for collapsed nodes which vary in size and width
    if (GetDisplayChildren() != CPhyloNodeData::eShowChildren) {
        CVect2<float> pix_offset(m_NodeOffset.X() / scale.X(), m_NodeOffset.Y() / scale.Y());

        pix_offset += m_NodePixelOffset;

        // we subtract abs(m_LabelRect.Left()) becuase this is the distance that we already offset
        // the label from the node, and this overhang is used to consider additional offset.
        float label_dist = std::min(std::abs(m_LabelRect.Left()), std::abs(m_LabelRect.Right()));
        float total_extension = -pix_offset.Length() + m_NodeWidth - label_dist;

        return total_extension > 0.0f ? total_extension : 0.0f;
    }

    return 0.0f;
}

END_NCBI_SCOPE
