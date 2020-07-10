#ifndef __GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_LOD__HPP
#define __GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_LOD__HPP

/*  $Id: phylo_tree_lod.hpp 38346 2017-04-27 13:24:57Z falkrb $
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
 * File Description: Level Of Detail descriptor
 *
 */

#include <corelib/ncbistl.hpp>

BEGIN_NCBI_SCOPE

class CPhyloTreeScheme;
class CPhyloTreeNode;
class CPhyloTreeNode;

class IPhyloTreeLOD
{
public:
    IPhyloTreeLOD(){}
    virtual ~IPhyloTreeLOD(){}

    virtual TVPUnit DistanceBetweenNodes(void) const       = 0;
    virtual CPhyloTreeScheme & GetScheme(void)             = 0;
    virtual float GetNodeSize(const CPhyloTreeNode * n) const { return 1.0f; }
    virtual float GetDefaultNodeSize(const CPhyloTreeNode * n) const { return 1.0f; }
    virtual float GetNodeLabelDist(const CPhyloTreeNode * n) const { return GetDefaultNodeSize(n); }
    /// This is the height of the node for layout purposes
    virtual float GetNodeLayoutSize(const CPhyloTreeNode* scheme) const { return 1.0f; }
};

enum  EPhyloTreeEditCommand {
    eCmdSomethingEdited = 1872,
    eCmdFeaturesEdited,
    eCmdNodeExpandCollapse,
    eCmdGrpExpandCollapse,
    eCmdTreeLabelSet,
    eCmdLabelFormatChanged,
    eCmdRenderingOptionsChanged,
	eCmdTreeSorted,   
    eEditCmdNone
};

class IPhyloTreeRenderHost
{
public:
    virtual ~IPhyloTreeRenderHost()  {};

    virtual void     HMGH_UpdateLimits(bool force_redraw = false) = 0;
    virtual TVPUnit  HMGH_GetVPPosByY(int y) const = 0;
    /// a topology or selection (color) change (requires update to buffers
    virtual void     HMGH_OnChanged(void) = 0;
    /// a chnage that requires a redraw but no GL buffer updates
    virtual void     HMGH_OnRefresh(void) = 0;
    virtual void     FireCBEvent(void) = 0;
    virtual void     FireEditEvent(EPhyloTreeEditCommand ec) = 0;
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_EXCEPTION__HPP
