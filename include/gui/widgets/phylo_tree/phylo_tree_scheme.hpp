#ifndef GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_SCHEME__HPP
#define GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_SCHEME__HPP
/*  $Id: phylo_tree_scheme.hpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/objutils/registry.hpp>

class wxArrayString;

BEGIN_NCBI_SCOPE

class CRgbaGradColorTable;


//"GBPlugins.PhyloTreeView"
static const string kPhyloBaseKey("GBPlugins.PhyloTreeView"); 

class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloTreeScheme :  public CObject
{
public:
	typedef enum {
		eColored,
		eClusters,
		eMonochrome
	} TColoration;

    typedef enum {
        eTree,
        eNode,
        eLine,
        eLabel
    } TPhyloTreeObject;

    typedef enum {
        eBgColor,
        eColor,
        eSelColor,
        eTraceColor,
        eSharedColor,
        eTipSelColor
    } TPhyloTreeColors;

    typedef enum {
        eLeftMargin,
        eTopMargin,
        eRightMargin,
        eBottomMargin,
        eNodeSize,
        eLeafNodeSize,
        eLineWidth,
        eMaxNodeSize,
        eMinNodeSize,
        eMaxNumChildren,
        eMinNumChildren
    } TPhyloTreeSizes;

    typedef enum {
        eLabelsForLeavesOnly,
        eLabelsVisible,      
        eLabelsHidden
    } TLabelsVisibility;

    // Determines if labels going past the edge are truncated or not
    typedef enum {
        eLabelsTruncated,
        eLabelsFull
    } TLabelTruncation;

    typedef enum {
        eSimpleLabels,
        eFancyLabels
    } TLabelStyle;

    typedef enum {
        eAlNone,
        eAlPhylip,
        eAlPaml
    } TAutoLabels;

    typedef enum {
        eNormalSelection,
        eHighlightSelection
    } TSelectionVisibility;

    typedef enum {eZoomXY, eZoomX, eZoomY}  TZoomBehavior;

public:
    CPhyloTreeScheme();
    virtual ~CPhyloTreeScheme(void) {}

    // interface functions
    void SetMargins(const GLdouble & left, const GLdouble & top,
                    const GLdouble & right, const GLdouble & bottom)
    {
        m_TopMargin     = top;
        m_BottomMargin  = bottom;
        m_LeftMargin    = left;
        m_RightMargin   = right;
    }

    void GetMargins(GLdouble & left, GLdouble & top,
                    GLdouble & right, GLdouble & bottom) const
    {
        top = m_TopMargin;
        bottom = m_BottomMargin;
        left = m_LeftMargin;
        right = m_RightMargin;
    }

    string & SetLabelFormat(void) { return m_LabelFormat; }
    string & SetTooltipFormat(void) { return m_TooltipFormat; }

    void SetMaxLabelLength(int len) { m_MaxLabelLength = len; }
    int GetMaxLabelLength() const { return m_MaxLabelLength; }

    const TLabelsVisibility & GetLabelVisibility(void) const {
       return m_LabelVisibility;
    }
    void SetLabelVisibility(const TLabelsVisibility & lv) {
        m_LabelVisibility = lv;
    }

    TSelectionVisibility GetSelectionVisibility() const { 
        return m_SelectionVisibility; 
    }
    void SetSelectionVisibility(const TSelectionVisibility sv) {
        m_SelectionVisibility = sv; 
    }

    // If true (the default) all selected nodes are shown as selected
    bool GetShowAllSelected() const { return m_ShowAllSelected; }
    void SetShowAllSelected(bool show_all) { m_ShowAllSelected = show_all; }

    float GetNonSelectedAlpha() const { return m_NonSelectedAlpha; }    
    void SetNonSelectedAlpha(float a) { m_NonSelectedAlpha = a; }

    TLabelTruncation& GetLabelTruncation() { return m_LabelTruncation; }
    void SetLabelTruncation(TLabelTruncation lt) { m_LabelTruncation = lt; }

    const TLabelStyle & GetLabelStyle(void) const { return m_LabelStyle; }
    void  SetLabelStyle(const TLabelStyle & ls) { m_LabelStyle = ls; }

	const TColoration & GetColoration(void) const { return m_Coloration; }
	void  SetColoration(const TColoration & col) { m_Coloration = col; }

    const TAutoLabels & GetAutoLabels(void) const { return m_AutoLabels; }
    void  SetAutoLabels(const TAutoLabels & al) { m_AutoLabels = al; }

	const CGlTextureFont & GetFont(void) const { return m_Font; }
	void SetFont(const CGlTextureFont & font)  { m_Font = font; }

    bool GetBoaNodes() const { return m_bBoaNodes; }
    void SetBoaNodes(bool x_bnodes) { m_bBoaNodes = x_bnodes; }

    bool GetSolidColorLeaves() const { return m_SolidColorLeaves; }
    void SetSolidColorLeaves(bool b) { m_SolidColorLeaves = b; }

    // zoom behavior
    void            SetZoomBehavior(const string& renderer, TZoomBehavior zb);
    TZoomBehavior   GetZoomBehavior(const string& renderer);

    CRgbaColor & SetColor(TPhyloTreeObject obj, TPhyloTreeColors col);    
    short GetColorIdx(TPhyloTreeObject obj, TPhyloTreeColors col); 

    GLdouble& SetSize(TPhyloTreeSizes obj);
    GLdouble SetSize(TPhyloTreeSizes obj) const;
	GLdouble GetLineWidth() const { return m_LineWidth; }
    GLdouble GetNodeSize() const { return m_NodeSize; }
    GLdouble GetLeafNodeSize() const { return m_LeafNodeSize; }

    /// size range for variable-sized collapsed nodes (so min node size here may
    /// be greater than node size)  
    GLdouble GetMaxNodeSize() const { return m_MaxNodeSize; }
    GLdouble GetMinNodeSize() const { return m_MinNodeSize; }

    /// collapsed node sizes are scaled from min to max values based on number of children
    /// so we need to have that range to do our computation
    GLdouble GetMinNumChildren() const { return m_MinNumChildren; }
    GLdouble GetMaxNumChildren() const { return m_MaxNumChildren; }
    void SetMinNumChildren(GLdouble min_num_children) { m_MinNumChildren = min_num_children; }
    void SetMaxNumChildren(GLdouble max_num_children) { m_MaxNumChildren = max_num_children; }

    /// And we scale the length of the collapsed node marker based on the 
    /// max. distance of any child of a collapsed node from the collapsed node
    /// itself so we need that distance too
    GLdouble GetMaxBranchDist() const { return m_MaxBranchDist; }
    void SetMaxBranchDist(GLdouble bd) { m_MaxBranchDist = bd; }

    void SetLayoutIdx(int lidx) { m_LastLayout = lidx; }
    int GetLayoutIdx() const { return m_LastLayout; }

    /// Make sure all current colors used by the scheme are in
    /// 'color_table' and add the ones that are not.
    void UpdateColorTable(CRgbaGradColorTable* color_table);

    virtual bool    LoadCurrentSettings();
    virtual bool    SaveCurrentSettings() const;

    void GetDefaultBroadcastProperties(wxArrayString& wxprops);

protected:
    /// themes
    string  m_CurrTheme;
    string  m_ThemeKey;
    string  m_DefaultTheme;
    string  m_CurrColorTheme;
    string  m_CurrSizeTheme;
    string  m_CurrGlobalTheme;

    /// margins
    GLdouble m_TopMargin;
    GLdouble m_BottomMargin;
    GLdouble m_LeftMargin;
    GLdouble m_RightMargin;

    /// sizes
    GLdouble m_LineWidth;
    GLdouble m_NodeSize;
    GLdouble m_LeafNodeSize;
    GLdouble m_MaxNodeSize;
    GLdouble m_MinNodeSize;
    GLdouble m_MinNumChildren;
    GLdouble m_MaxNumChildren;

    // Max distance of any node from root (in current tree). Min is 0.
    GLdouble m_MaxBranchDist;

    /// colors
    CRgbaColor m_BgColor;
    CRgbaColor m_LineColor;
    CRgbaColor m_LineSelColor;
    CRgbaColor m_LineTraceColor;
    CRgbaColor m_LineSharedColor;
    CRgbaColor m_NodeColor;
    CRgbaColor m_NodeSelColor;
    CRgbaColor m_NodeTraceColor;
    CRgbaColor m_NodeSharedColor;
    CRgbaColor m_NodeTipSelColor;
    CRgbaColor m_LabelColor;
    CRgbaColor m_LabelSelColor;
    CRgbaColor m_LabelTraceColor;
    CRgbaColor m_LabelSharedColor;

    /// monochrome colors
    CRgbaColor m_BgColorMono;
    CRgbaColor m_LineColorMono;
    CRgbaColor m_LineSelColorMono;
    CRgbaColor m_LineTraceColorMono;
    CRgbaColor m_LineSharedColorMono;
    CRgbaColor m_NodeColorMono;
    CRgbaColor m_NodeSelColorMono;
    CRgbaColor m_NodeTraceColorMono;
    CRgbaColor m_NodeSharedColorMono;
    CRgbaColor m_NodeTipSelColorMono;
    CRgbaColor m_LabelColorMono;
    CRgbaColor m_LabelSelColorMono;
    CRgbaColor m_LabelTraceColorMono;
    CRgbaColor m_LabelSharedColorMono;

    /// Color indices in color table
    short m_BgColorIdx;
    short m_LineColorIdx;
    short m_LineSelColorIdx;
    short m_LineTraceColorIdx;
    short m_LineSharedColorIdx;
    short m_NodeColorIdx;
    short m_NodeSelColorIdx;
    short m_NodeTraceColorIdx;
    short m_NodeSharedColorIdx;
    short m_NodeTipSelColorIdx;
    short m_LabelColorIdx;
    short m_LabelSelColorIdx;
    short m_LabelTraceColorIdx;
    short m_LabelSharedColorIdx;

    short m_BgColorMonoIdx;
    short m_LineColorMonoIdx;
    short m_LineSelColorMonoIdx;
    short m_LineTraceColorMonoIdx;
    short m_LineSharedColorMonoIdx;
    short m_NodeColorMonoIdx;
    short m_NodeSelColorMonoIdx;
    short m_NodeTraceColorMonoIdx;
    short m_NodeSharedColorMonoIdx;
    short m_NodeTipSelColorMonoIdx;
    short m_LabelColorMonoIdx;
    short m_LabelSelColorMonoIdx;
    short m_LabelTraceColorMonoIdx;
    short m_LabelSharedColorMonoIdx;

    /// label format
    string   m_LabelFormat;

    /// max label display length (characters). -1=>No max len
    int m_MaxLabelLength;

    /// tooltip format
    string   m_TooltipFormat;

    /// label visibility
    TLabelsVisibility m_LabelVisibility;

    /// visibility of selected nodes vs. non selected nodes
    TSelectionVisibility m_SelectionVisibility;

    /// If true (the default) all selected nodes are shown as selected.
    /// Otherwise only the current node (m_TreeModel->SetCurrentNode()) will
    /// show as selected (and only if the current node is selected- it
    /// doesn't have to be).
    bool m_ShowAllSelected;
    
    /// alpha value of non-selected glyphs when visibility is eHighlightSelection
    float m_NonSelectedAlpha;

    /// Label truncation (how are labels clipped at edges)
    TLabelTruncation m_LabelTruncation;

    /// labels style
    TLabelStyle      m_LabelStyle;

    /// auto-labeling
    TAutoLabels m_AutoLabels;

    /// zoom behavior by renderer (string is GetDescription() from renderer)
    std::map<string,TZoomBehavior>  m_ZoomBehavior;

	TColoration m_Coloration;

    CGlTextureFont m_Font;

    /// boa - node size behavior
    bool m_bBoaNodes;

    /// Solid color leaf nodes (if true)
    bool m_SolidColorLeaves;

    // Last layout selected by user
    int m_LastLayout;

    void x_DefaultScheme(void);
    CRgbaColor & x_SetColorMono(TPhyloTreeObject obj, TPhyloTreeColors col);
    short x_GetColorMonoIdx(TPhyloTreeObject obj, TPhyloTreeColors col);

    /// Return a read view of the registry given a base and 2 subkeys
    CRegistryReadView x_GetReadView(CGuiRegistry& gui_reg,
                                          const string& base_key,
                                          const string& curr_key,
                                          const string& def_key1="") const;
    /// Return a read-write view of the registry given a base and 2 subkeys
    CRegistryWriteView x_GetWriteView(CGuiRegistry& gui_reg,
                                                    const string& base_key,
                                                    const string& curr_key,
                                                    const string& def_key1="") const;
    /// Load a color from the registry
    bool x_UserFieldToColor(const CRegistryReadView& view,
                            const string& key, 
                            CRgbaColor& c);
    void x_ColorToUserField(CRegistryWriteView& view,
                            const string& key,
                            const CRgbaColor& c) const;

    /// Check if current color index corresponds to the correct color
    /// in the color table and, if not, add it.
    void x_UpdateColorIndex(CRgbaGradColorTable* color_table, 
                            const CRgbaColor& current_color,
                            short& current_index);
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_SCHEME__HPP
