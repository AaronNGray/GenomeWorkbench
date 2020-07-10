/*  $Id: phylo_tree_label.cpp 41823 2018-10-17 17:34:58Z katargir $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_label.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>

#include <gui/opengl/irender.hpp>
#include <gui/opengl/glutils.hpp>

BEGIN_NCBI_SCOPE

CVect2<float> CPhyloTreeLabel::GetNodeLabelOffset(const string& label,
                                                  CPhyloTreeNode* node,
                                                  IPhyloTreeLOD* lod,
                                                  CVect2<float>& pixel_offset) const
{
    pixel_offset.X() = 0.0f;
    pixel_offset.Y() = 0.0f;

    float defNodeSize = (float)lod->GetNodeLabelDist(node);

    const CGlTextureFont* bestFont = &m_SL->GetFont(); 

    // we have place to show it
    IRender& gl = GetGl();

	CVect2<float> label_size((float)gl.TextWidth(bestFont, label.c_str()),
		                     (float)gl.GetMetric(bestFont, IGlFont::eMetric_CharHeight));

    bool bLeft = cosf((**node).GetAngle()) < 0.0f;

	if (!bLeft) {
		// 2 pixels visible distance from node
		pixel_offset.X() = defNodeSize + 2.0f;
		pixel_offset.Y() = -label_size.Y() / 2.0f;
	}
    else {
		pixel_offset.X() = -(defNodeSize + 4.0f + label_size.X());
		pixel_offset.Y() = -label_size.Y() / 2.0f;
    }
	
	return label_size;
}

CVect2<float> CPhyloTreeLabel::GetNodeLabelOffset(const string& label,
                                                  CPhyloTreeNode* node,
                                                  IPhyloTreeLOD* lod,
                                                  const CVect2<float>& label_size,
                                                  CVect2<float>& pixel_offset) const
{
    pixel_offset.X() = 0.0f;
    pixel_offset.Y() = 0.0f;

    float defNodeSize = lod->GetNodeLabelDist(node);

    bool bLeft = cosf((**node).GetAngle()) < 0.0f;

    if (!bLeft) {
        // 2 pixels visible distance from node
        pixel_offset.X() = defNodeSize + 2.0f;
        pixel_offset.Y() = -label_size.Y() / 2.0f;
    }
    else {
        pixel_offset.X() = -(defNodeSize + 4.0f + label_size.X());
        pixel_offset.Y() = -label_size.Y() / 2.0f;
    }

    return label_size;
}

string CPhyloTreeLabel::x_GenerateAutoLabel(const CPhyloTree &tree,
                                            const CPhyloTreeNode &node,
                                            CPhyloTreeScheme::TAutoLabels lbl) const
{
    string  autoLabel;

    switch (lbl){
    case CPhyloTreeScheme::eAlNone:
    {
        autoLabel = "";
        break;
    }
    case CPhyloTreeScheme::eAlPhylip:
    {
        int ilabel;

        if (!node.IsLeaf()) {
            ilabel = (*node).GetPamlCounter() + 1;
        }
        else {		
			int ibase = (tree.GetNumNodes()-1) - tree.GetRoot()->GetNumLeaves();
            ilabel = ibase + node.GetValue().IDX().second + 1;
        }

        autoLabel = NStr::IntToString(ilabel);
        break;
    }
    case CPhyloTreeScheme::eAlPaml:
    {
        int ilabel;

        if (node.IsLeaf()) {
            ilabel = node.GetValue().IDX().second;
        }
        else if (!node.HasParent()) {
			ilabel = node.GetValue().GetNumLeaves() + 1;
        }
        else {
			ilabel = tree.GetRoot()->GetNumLeaves() + (*node).GetPamlCounter() + 1;
        }
        autoLabel = NStr::IntToString(ilabel);
        break;
    }
    default:
        autoLabel = "";
        break;
    }

    return autoLabel;
}

string CPhyloTreeLabel::GetLabel(const CPhyloTree &tree, const CPhyloTreeNode &node) const
{
    string result = (*node).GetLabel();
    NStr::TruncateSpacesInPlace(result);

    // adding auto-label
    string autoLabel = x_GenerateAutoLabel(tree, node, m_SL->GetAutoLabels());
    if (autoLabel.length()) {
        if (result.length()) {
            result = " | " + result;
        }
        result = autoLabel + result;
    }

    if (!node.Expanded()) {
        //label += (" (" + NStr::IntToString((**node).CountSubnodes()) + " nodes)");        
        if (result.length()) {
            result += " | ";
        }
		result += (NStr::IntToString((*node).GetNumLeaves()) + " leaves");
    }

    if (m_SL->GetMaxLabelLength() > 0 && int(result.length()) > m_SL->GetMaxLabelLength()) {
        result = result.substr(0, m_SL->GetMaxLabelLength()) + "...";
    }

    return result;
}

void CPhyloTreeLabel::SetScheme(CPhyloTreeScheme &sl, const CPhyloTree *tree)
{
    m_SL.Reset(&sl);
    SetLabelFormat(sl.SetLabelFormat(), tree);
}

void CPhyloTreeLabel::SetLabelFormat(const string &fmt, const CPhyloTree *tree)
{
    if (!fmt.empty() && (m_Format!=fmt)) {
        m_Format = fmt;
    }

    // May be called before model is loaded. If so it will be called
    // again during layout.
    if (tree == NULL)
        return;

    m_LabelFormat.reset(new CBioTreeFormatLabel(tree->GetFeatureDict(), fmt));
}

string CPhyloTreeLabel::GetLabelForNode(const CPhyloTree &tree, const CPhyloTreeNode &node, const string &format)
{
    string saved_format = m_Format;
    SetLabelFormat(format, &tree);
    string ready_label = m_LabelFormat->FormatLabel((*node).GetBioTreeFeatureList());
    SetLabelFormat(saved_format, &tree);
    return ready_label;
}

string CPhyloTreeLabel::GetToolTipForNode(const CPhyloTree &tree,
                                          const CPhyloTreeNode &node,
                                          const string &format,
                                          const string &line_break)
{    
    std::vector<std::string> arr;
    std::string final_text;

    string ready_label = GetLabelForNode(tree, node, format);

    // Get tip text, and then remove any lines we don't need.
    NStr::Split(ready_label, "\n", arr);
    for (unsigned int i=0; i<arr.size(); ++i) {
        if (arr[i].length() > 0) {
            // This is in the title
            if (arr[i].substr(0, 6) == "label:")
                continue;
            // This is visually obvious
            if (arr[i].substr(0,16) == "$NODE_COLLAPSED:")
                continue;

            // Remove blank features (no text after first ':')...
            std::string::size_type idx1 = arr[i].find_first_of(":");
            if (arr[i].length() > idx1+1) {
                std::string::size_type idx2 = arr[i].find_first_not_of(' ', idx1+1);
                if (idx2 != std::string::npos) {
                    final_text += arr[i];
                    final_text += line_break;  // may be \n or for pdf (to put an escaped \n in the pdf) \\\\n
                }
            }
        }
    }

    return final_text;
}


TVPUnit CPhyloTreeLabel::GetMinVerticalSeparation() const
{
    TVPUnit h = -1;

    const CGlTextureFont* font = &m_SL->GetFont(); 

    IRender& gl = GetGl();
    h = ((TVPUnit) gl.TextHeight(font) + 1);    

    return h;
}

bool CPhyloTreeLabel::IsVisible(IPhyloTreeLOD * lod) const
{
    TModelUnit h = GetMinVerticalSeparation();

    if (h > 0) {
        TVPUnit visibleDistance = lod->DistanceBetweenNodes();
        return visibleDistance >= h;
    }
    
    return false;
}

END_NCBI_SCOPE
