#ifndef GUI_WIDGETS_PHY_TREE___PHYLO_TREE_RADIAL__HPP
#define GUI_WIDGETS_PHY_TREE___PHYLO_TREE_RADIAL__HPP

/*  $Id: phylo_tree_radial.hpp 42826 2019-04-18 21:45:11Z katargir $
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

#include <gui/widgets/phylo_tree/phylo_tree_render.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>


BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloRadial : public IPhyloTreeRender
{
public:
    CPhyloRadial();
    CPhyloRadial(double w, double h);
    virtual ~CPhyloRadial() {}
    string  GetDescription(void);
    
    bool    SupportsRotatedLabels() const { return true; }

    /// Allocate less spaces for labels in circular views since they appear on both sides
    /// of nodes, thus using up more space.
    virtual float   GetLabelViewPct() const { return m_LabelViewPct / 2.0f; }

protected:
    void  x_Calculate(CPhyloTree* tree);
    void  x_Layout(CPhyloTreeDataSource& ds);
    virtual void x_SetGlRenderOptions(CPhyloTreeDataSource& ds);
    void  x_Normalize(TTreeIdx node_idx);
    void  x_RenderVbo(CPhyloTreeDataSource& ds);
    void  x_DrawTreeVbo(vector<CVect2<float> >& edge_node_coords,
                        vector<CVect4<unsigned char> >& edge_node_colors,
                        vector<CVect2<float> >& sel_edge_node_coords,
                        vector<CVect4<unsigned char> >& sel_edge_node_colors,
                        vector<float>& color_coords,
                        CPhyloTree* tree);
    TModelUnit x_GetVerticalSeparationFactor() const { return TModelUnit(1.5); }
};

END_NCBI_SCOPE

#endif //GUI_WIDGETS_PHY_TREE___PHYLO_TREE_RADIAL__HPP

