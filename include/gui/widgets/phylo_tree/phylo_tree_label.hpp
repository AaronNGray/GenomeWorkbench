#ifndef GUI_WIDGETS_PHY_TREE___PHYLO_TREE_LABEL__HPP
#define GUI_WIDGETS_PHY_TREE___PHYLO_TREE_LABEL__HPP

/*  $Id: phylo_tree_label.hpp 36053 2016-08-02 14:06:39Z evgeniev $
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

#include <memory>
#include <gui/opengl.h>
#include <gui/opengl/glrect.hpp>
#include <gui/opengl/glbitmapfont.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_lod.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>

#include <algo/phy_tree/bio_tree.hpp>
#include <algo/phy_tree/bio_tree_format_label.hpp>


BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloTreeLabel
{
public:
    CPhyloTreeLabel() {}
    CPhyloTreeLabel(const CPhyloTreeLabel& src) {
        m_SL = src.m_SL;
        m_Format = src.m_Format;
        if (src.m_LabelFormat.get())
            m_LabelFormat.reset(new CBioTreeFormatLabel(*src.m_LabelFormat.get()));
    }

    CPhyloTreeLabel& operator=(const CPhyloTreeLabel& src)
    {
        m_SL = src.m_SL;
        m_Format = src.m_Format;
        if (src.m_LabelFormat.get())
            m_LabelFormat.reset(new CBioTreeFormatLabel(*src.m_LabelFormat.get()));
        else
            m_LabelFormat.reset();
        return *this;
    }

    /// Get label rectangle
	CVect2<float> GetNodeLabelOffset(const string& label,
                                     CPhyloTreeNode* node,
                                     IPhyloTreeLOD* lod,
                                     CVect2<float>& pixel_offset) const;
    /// Get label rectangle using the label_size parameter for width and height
    /// instead of computing it (computing text width can be slow)
    CVect2<float> GetNodeLabelOffset(const string& label,
                                     CPhyloTreeNode* node,
                                     IPhyloTreeLOD* lod,
                                     const CVect2<float>& label_size,
                                     CVect2<float>& pixel_offset) const;
    TVPUnit                   GetMinVerticalSeparation() const;
    bool                      IsVisible(IPhyloTreeLOD* lod) const;  

    string GetLabelForNode(const CPhyloTree &tree, const CPhyloTreeNode &node, const string& format);
    string GetToolTipForNode(const CPhyloTree &tree,
                             const CPhyloTreeNode &node, 
                             const string &format,
                             const string &line_break);

    void SetLabelFormat(const string &fmt, const CPhyloTree *tree);
    void SetScheme(CPhyloTreeScheme & sl, const CPhyloTree *tree);

    string GetLabel(const CPhyloTree &tree, const CPhyloTreeNode &node) const;
private:
     // reference to style
    CRef <CPhyloTreeScheme> m_SL;

    // label formatting string
    string m_Format;

    // labels generation
    string x_GenerateAutoLabel(const CPhyloTree &tree,
                               const CPhyloTreeNode &node,
                               CPhyloTreeScheme::TAutoLabels lbl) const;

    std::unique_ptr<CBioTreeFormatLabel> m_LabelFormat;
};

END_NCBI_SCOPE

#endif //  GUI_WIDGETS_PHY_TREE___PHYLO_TREE_LABEL__HPP
