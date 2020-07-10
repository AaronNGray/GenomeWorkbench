/*  $Id: phylo_tree_scheme.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_exception.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/config/settings_set.hpp>
#include <gui/opengl/glfont.hpp>
#include <gui/opengl/glcolortable.hpp>
#include <gui/types.hpp>

#include <objects/general/User_field.hpp>


BEGIN_NCBI_SCOPE

CPhyloTreeScheme::CPhyloTreeScheme()
 : m_CurrTheme("")
 , m_ThemeKey("Theme")
 , m_DefaultTheme("Default")
 , m_BgColorIdx(-1)
 , m_LineColorIdx(-1)
 , m_LineSelColorIdx(-1)
 , m_LineTraceColorIdx(-1)
 , m_LineSharedColorIdx(-1)
 , m_NodeColorIdx(-1)
 , m_NodeSelColorIdx(-1)
 , m_NodeTraceColorIdx(-1)
 , m_NodeSharedColorIdx(-1)
 , m_NodeTipSelColorIdx(-1)
 , m_LabelColorIdx(-1)
 , m_LabelSelColorIdx(-1)
 , m_LabelTraceColorIdx(-1)
 , m_LabelSharedColorIdx(-1)
 , m_BgColorMonoIdx(-1)
 , m_LineColorMonoIdx(-1)
 , m_LineSelColorMonoIdx(-1)
 , m_LineTraceColorMonoIdx(-1)
 , m_LineSharedColorMonoIdx(-1)
 , m_NodeColorMonoIdx(-1)
 , m_NodeSelColorMonoIdx(-1)
 , m_NodeTraceColorMonoIdx(-1)
 , m_NodeSharedColorMonoIdx(-1)
 , m_NodeTipSelColorMonoIdx(-1)
 , m_LabelColorMonoIdx(-1)
 , m_LabelSelColorMonoIdx(-1)
 , m_LabelTraceColorMonoIdx(-1)
 , m_LabelSharedColorMonoIdx(-1)
 , m_LastLayout(-1)
{
    x_DefaultScheme();
}

void CPhyloTreeScheme::x_DefaultScheme(void)
{
    m_LabelFormat   = "$(label)";
    m_MaxLabelLength = -1;
    m_TooltipFormat = "Label: $(label) \nDistance: $(dist)";
    m_TopMargin = m_LeftMargin =  m_RightMargin = 10;
    m_BottomMargin = 40; // extra room for scale marker
    
    m_LineWidth = 3.0;
    m_NodeSize  = 3.0;
    m_LeafNodeSize = 3.0;
    m_MinNodeSize = m_NodeSize;
    m_MaxNodeSize = m_MinNodeSize + 3.0;
    m_MinNumChildren = 1.0;
    m_MaxNumChildren = 300.0;
    m_MaxBranchDist = 0.0f;
    m_bBoaNodes = true;
    m_SolidColorLeaves = false;

    m_LabelVisibility = eLabelsForLeavesOnly;
    m_LabelTruncation = eLabelsTruncated;
    m_LabelStyle      = eFancyLabels;
    m_AutoLabels      = eAlNone;
	m_Coloration	  = eClusters;
    m_SelectionVisibility = eNormalSelection;
    m_ShowAllSelected = true;

    m_NonSelectedAlpha = 0.1f;

    m_BgColor.Set(1.0f, 1.0f, 1.0f, 1.0f);


    // line, edge, and label sel colors..
    // selected to be reasonably color-blind friendly
    m_LineColor.Set(0.6f, 0.6f, 0.6f, 1.0f); // 153,153,153
    m_LineSelColor.Set(0.0f, 0.0f, 1.0f, 1.0f); // 0,0,255
    m_LineSharedColor.Set(0.45f, 0.45f, 1.0f, 1.0f); // 115,115,255
    m_LineTraceColor.Set(0.4f, 0.9f, 0.02f, 1.0f);  // 102,229,5
    
    m_NodeColor.Set(0.4f, 0.4f, 0.4f, 1.0f);  // 102,102,102
    m_NodeSelColor.Set(1.0f, 0.35f, 0.35f, 1.0f);  // 255,90,90
    m_NodeSharedColor.Set(0.4f, 0.4f, 0.9f, 1.0f);  // 100,100,229
    m_NodeTraceColor.Set(0.3f, 0.8f, 0.02f, 1.0f); // 76,204,4
    m_NodeTipSelColor.Set(1.0f, 0.0f, 0.0f, 0.25f);
    
    m_LabelColor.Set(0.0f, 0.0f, 0.0f, 1.0f);
    m_LabelSharedColor.Set(0.0f, 0.0f, 0.0f, 1.0f);
    m_LabelTraceColor.Set(0.0f, 0.0f, 0.0f, 1.0f);
    m_LabelSelColor.Set(1.0f, 0.35f, 0.35f, 1.0f);  // 255,90,90

    // monochrome colors
    m_BgColorMono.Set(1.0f, 1.0f, 1.0f, 1.0f);

    m_LineColorMono.Set(0.6f, 0.6f, 0.6f, 1.0f);
    m_LineSelColorMono.Set(0.85f, 0.85f, 0.85f, 1.0f);
    m_LineSharedColorMono.Set(0.3f, 0.3f, 0.3f, 1.0f);
    m_LineTraceColorMono.Set(0.3f, 0.3f, 0.3f, 1.0f);

    m_NodeColorMono.Set (0.3f, 0.3f, 0.3f, 1.0f);
    m_NodeSelColorMono.Set(0.5f, 0.5f, 0.5f, 1.0f);
    m_NodeSharedColorMono.Set(0.8f, 0.8f, 0.8f, 1.0f);
    m_NodeTraceColorMono.Set(0.1f, 0.1f, 0.1f, 1.0f);
    m_NodeTipSelColorMono.Set(0.4f, 0.4f, 0.4f, 0.25f);

    m_LabelColorMono.Set(0.0f, 0.0f, 0.0f, 1.0f);
    m_LabelSharedColorMono.Set(0.0f, 0.0f, 0.0f, 1.0f);
    m_LabelTraceColorMono.Set(0.0f, 0.0f, 0.0f, 1.0f);
    m_LabelSelColorMono.Set(0.2f, 0.2f, 0.2f, 1.0f);

    m_Font.SetFontSize(8);
    m_Font.SetFontFace(CGlTextureFont::eFontFace_Helvetica);

    m_LastLayout = 0;
}

void CPhyloTreeScheme::SetZoomBehavior(const string& renderer, TZoomBehavior zb)
{
    m_ZoomBehavior[renderer] = zb;
}

CPhyloTreeScheme::TZoomBehavior CPhyloTreeScheme::GetZoomBehavior(const string& renderer)
{
    if (m_ZoomBehavior.count(renderer) > 0)
        return m_ZoomBehavior[renderer];

    return eZoomXY;
}

CRgbaColor& CPhyloTreeScheme::SetColor(TPhyloTreeObject obj,
                                       TPhyloTreeColors col)
{
    if (GetColoration() == eMonochrome)
        return x_SetColorMono(obj, col);


    switch (obj) {
    case eNode:
        {{
             switch (col) {
             case eColor:        return m_NodeColor;
             case eSelColor:     return m_NodeSelColor;
             case eTraceColor:   return m_NodeTraceColor;
             case eSharedColor:  return m_NodeSharedColor;
             case eTipSelColor:  return m_NodeTipSelColor;
             default: break;
             }
             break;
         }}

    case eLine:
        {{
             switch (col) {
             case eColor:        return m_LineColor;
             case eSelColor:     return m_LineSelColor;
             case eTraceColor:   return m_LineTraceColor;
             case eSharedColor:  return m_LineSharedColor;
             default: break;
             }
             break;
         }}
    case eLabel:
        {{
             switch (col) {
             case eColor:        return m_LabelColor;
             case eSelColor:     return m_LabelSelColor;
             case eTraceColor:   return m_LabelTraceColor;
             case eSharedColor:  return m_LabelSharedColor;
             default: break;
             }
             break;
         }}
    default:
        break;
    }

    // bgColor is one for all elements
    if (col == eBgColor) {
        return m_BgColor;
    }

    // throw error
    NCBI_THROW(CPhyloTreeException, eNoValue,
               "Unable to get/set specified value");
}

short CPhyloTreeScheme::GetColorIdx(TPhyloTreeObject obj,
                                    TPhyloTreeColors col)
{
    if (GetColoration() == eMonochrome)
        return x_GetColorMonoIdx(obj, col);


    switch (obj) {
    case eNode:
        {{
             switch (col) {
             case eColor:        return m_NodeColorIdx;
             case eSelColor:     return m_NodeSelColorIdx;
             case eTraceColor:   return m_NodeTraceColorIdx;
             case eSharedColor:  return m_NodeSharedColorIdx;
             case eTipSelColor:  return m_NodeTipSelColorIdx;
             default: break;
             }
             break;
         }}

    case eLine:
        {{
             switch (col) {
             case eColor:        return m_LineColorIdx;
             case eSelColor:     return m_LineSelColorIdx;
             case eTraceColor:   return m_LineTraceColorIdx;
             case eSharedColor:  return m_LineSharedColorIdx;
             default: break;
             }
             break;
         }}
    case eLabel:
        {{
             switch (col) {
             case eColor:        return m_LabelColorIdx;
             case eSelColor:     return m_LabelSelColorIdx;
             case eTraceColor:   return m_LabelTraceColorIdx;
             case eSharedColor:  return m_LabelSharedColorIdx;
             default: break;
             }
             break;
         }}
    default:
        break;
    }

    // bgColor is one for all elements
    if (col == eBgColor) {
        return m_BgColorIdx;
    }

    // throw error
    NCBI_THROW(CPhyloTreeException, eNoValue,
               "Unable to get/set specified value");
}

CRgbaColor& CPhyloTreeScheme::x_SetColorMono(TPhyloTreeObject obj,
                                           TPhyloTreeColors col)
{

    switch (obj) {
    case eNode:
        {{
             switch (col) {
             case eColor:        return m_NodeColorMono;
             case eSelColor:     return m_NodeSelColorMono;
             case eTraceColor:   return m_NodeTraceColorMono;
             case eSharedColor:  return m_NodeSharedColorMono;
             case eTipSelColor:  return m_NodeTipSelColorMono;
             default: break;
             }
             break;
         }}

    case eLine:
        {{
             switch (col) {
             case eColor:        return m_LineColorMono;
             case eSelColor:     return m_LineSelColorMono;
             case eTraceColor:   return m_LineTraceColorMono;
             case eSharedColor:  return m_LineSharedColorMono;
             default: break;
             }
             break;
         }}
    case eLabel:
        {{
             switch (col) {
             case eColor:        return m_LabelColorMono;
             case eSelColor:     return m_LabelSelColorMono;
             case eTraceColor:   return m_LabelTraceColorMono;
             case eSharedColor:  return m_LabelSharedColorMono;
             default: break;
             }
             break;
         }}
    default:
        break;
    }

    // bgColor is one for all elements
    if (col == eBgColor) {
        return m_BgColorMono;
    }

    // throw error
    NCBI_THROW(CPhyloTreeException, eNoValue,
               "Unable to get/set specified value");
}

short CPhyloTreeScheme::x_GetColorMonoIdx(TPhyloTreeObject obj,
                                          TPhyloTreeColors col)
{

    switch (obj) {
    case eNode:
        {{
             switch (col) {
             case eColor:        return m_NodeColorMonoIdx;
             case eSelColor:     return m_NodeSelColorMonoIdx;
             case eTraceColor:   return m_NodeTraceColorMonoIdx;
             case eSharedColor:  return m_NodeSharedColorMonoIdx;
             case eTipSelColor:  return m_NodeTipSelColorMonoIdx;
             default: break;
             }
             break;
         }}

    case eLine:
        {{
             switch (col) {
             case eColor:        return m_LineColorMonoIdx;
             case eSelColor:     return m_LineSelColorMonoIdx;
             case eTraceColor:   return m_LineTraceColorMonoIdx;
             case eSharedColor:  return m_LineSharedColorMonoIdx;
             default: break;
             }
             break;
         }}
    case eLabel:
        {{
             switch (col) {
             case eColor:        return m_LabelColorMonoIdx;
             case eSelColor:     return m_LabelSelColorMonoIdx;
             case eTraceColor:   return m_LabelTraceColorMonoIdx;
             case eSharedColor:  return m_LabelSharedColorMonoIdx;
             default: break;
             }
             break;
         }}
    default:
        break;
    }

    // bgColor is one for all elements
    if (col == eBgColor) {
        return m_BgColorMonoIdx;
    }

    // throw error
    NCBI_THROW(CPhyloTreeException, eNoValue,
               "Unable to get/set specified value");
}

void CPhyloTreeScheme::x_UpdateColorIndex(CRgbaGradColorTable* color_table, 
                                          const CRgbaColor& current_color,
                                          short& current_index)
{
    // Don't know which colors may have chnaged. Only add new elements to
    // the color array as needed.
    if (current_index < 0 || current_index >= (short)color_table->GetSize()) {
        // If the current index is -1 or out-of-bounds, just add the color:
        current_index = (short)color_table->AddColor(current_color);        
    }
    else {
        // See if color at current index matches intended color. If not, update.
        CRgbaColor table_color = color_table->GetColor((int)current_index);

        if (!(table_color == current_color)) {
            current_index = (short)color_table->AddColor(current_color);           
        }
    }
}

void CPhyloTreeScheme::UpdateColorTable(CRgbaGradColorTable* color_table)
{
    x_UpdateColorIndex(color_table, m_BgColor, m_BgColorIdx);
    x_UpdateColorIndex(color_table, m_LineColor, m_LineColorIdx);
    x_UpdateColorIndex(color_table, m_LineSelColor, m_LineSelColorIdx);
    x_UpdateColorIndex(color_table, m_LineTraceColor, m_LineTraceColorIdx);
    x_UpdateColorIndex(color_table, m_LineSharedColor, m_LineSharedColorIdx);
    x_UpdateColorIndex(color_table, m_NodeColor, m_NodeColorIdx);
    x_UpdateColorIndex(color_table, m_NodeSelColor, m_NodeSelColorIdx);
    x_UpdateColorIndex(color_table, m_NodeTraceColor, m_NodeTraceColorIdx);
    x_UpdateColorIndex(color_table, m_NodeSharedColor, m_NodeSharedColorIdx);
    x_UpdateColorIndex(color_table, m_NodeTipSelColor, m_NodeTipSelColorIdx);
    x_UpdateColorIndex(color_table, m_LabelColor, m_LabelColorIdx);
    x_UpdateColorIndex(color_table, m_LabelSelColor, m_LabelSelColorIdx);
    x_UpdateColorIndex(color_table, m_LabelTraceColor, m_LabelTraceColorIdx);
    x_UpdateColorIndex(color_table, m_LabelSharedColor, m_LabelSharedColorIdx);

    x_UpdateColorIndex(color_table, m_BgColorMono, m_BgColorMonoIdx);
    x_UpdateColorIndex(color_table, m_LineColorMono, m_LineColorMonoIdx);
    x_UpdateColorIndex(color_table, m_LineSelColorMono, m_LineSelColorMonoIdx);
    x_UpdateColorIndex(color_table, m_LineTraceColorMono, m_LineTraceColorMonoIdx);
    x_UpdateColorIndex(color_table, m_LineSharedColorMono, m_LineSharedColorMonoIdx);
    x_UpdateColorIndex(color_table, m_NodeColorMono, m_NodeColorMonoIdx);
    x_UpdateColorIndex(color_table, m_NodeSelColorMono, m_NodeSelColorMonoIdx);
    x_UpdateColorIndex(color_table, m_NodeTraceColorMono, m_NodeTraceColorMonoIdx);
    x_UpdateColorIndex(color_table, m_NodeSharedColorMono, m_NodeSharedColorMonoIdx);
    x_UpdateColorIndex(color_table, m_NodeTipSelColorMono, m_NodeTipSelColorMonoIdx);
    x_UpdateColorIndex(color_table, m_LabelColorMono, m_LabelColorMonoIdx);
    x_UpdateColorIndex(color_table, m_LabelSelColorMono, m_LabelSelColorMonoIdx);
    x_UpdateColorIndex(color_table, m_LabelTraceColorMono, m_LabelTraceColorMonoIdx);
    x_UpdateColorIndex(color_table, m_LabelSharedColorMono, m_LabelSharedColorMonoIdx);
}

GLdouble & CPhyloTreeScheme::SetSize(TPhyloTreeSizes obj)
{
    switch (obj) {
    case eLeftMargin:   return m_LeftMargin;
    case eTopMargin:    return m_TopMargin;
    case eRightMargin:  return m_RightMargin;
    case eBottomMargin: return m_BottomMargin;
    case eNodeSize:     return m_NodeSize;
    case eLeafNodeSize: return m_LeafNodeSize;
    case eMinNodeSize:  return m_MinNodeSize;
    case eMaxNodeSize:  return m_MaxNodeSize;
    case eLineWidth:    return m_LineWidth;
    case eMinNumChildren: return m_MinNumChildren;
    case eMaxNumChildren: return m_MaxNumChildren;
    default:            break;
    };

    // throw error
    NCBI_THROW(CPhyloTreeException, eNoValue,
        "Unable to get/set specified value");
}

GLdouble CPhyloTreeScheme::SetSize(TPhyloTreeSizes obj) const
{
    switch (obj) {
    case eLeftMargin:   return m_LeftMargin;
    case eTopMargin:    return m_TopMargin;
    case eRightMargin:  return m_RightMargin;
    case eBottomMargin: return m_BottomMargin;
    case eNodeSize:     return m_NodeSize;
    case eLeafNodeSize: return m_LeafNodeSize;
    case eMinNodeSize:  return m_MinNodeSize;
    case eMaxNodeSize:  return m_MaxNodeSize;
    case eLineWidth:    return m_LineWidth;
    case eMinNumChildren: return m_MinNumChildren;
    case eMaxNumChildren: return m_MaxNumChildren;
    default:            break;
    };

    // throw error
    NCBI_THROW(CPhyloTreeException, eNoValue,
        "Unable to get/set specified value");
}

CRegistryReadView
CPhyloTreeScheme::x_GetReadView(CGuiRegistry& gui_reg,
                                const string& base_key,
                                const string& curr_key,
                                const string& def_key1) const
{
    vector <string> keys;
    if ( !def_key1.empty()) {
        keys.push_back(def_key1);
    }
    if (curr_key != def_key1) {
        keys.push_back(curr_key);
    }
    return gui_reg.GetReadView(base_key, keys);
}

CRegistryWriteView
CPhyloTreeScheme::x_GetWriteView(CGuiRegistry& gui_reg,
                                     const string& base_key,
                                     const string& curr_key,
                                     const string& def_key1) const
{
    vector <string> keys;
    if ( !def_key1.empty()) {
        keys.push_back(def_key1);
    }
    if (curr_key != def_key1) {
        keys.push_back(curr_key);
    }
    return gui_reg.GetWriteView(base_key, keys);
}

bool CPhyloTreeScheme::x_UserFieldToColor(const CRegistryReadView& view,
                                          const string& key, 
                                          CRgbaColor& c)
{
    CConstRef<objects::CUser_field> ref(view.GetField(key));
    if (!ref) {
        return false;
    }

    const objects::CUser_field& f = *ref;

    switch(f.GetData().Which()) {
    case objects::CUser_field::TData::e_Ints:
        {{
             vector<int> cvi = f.GetData().GetInts();
             if (cvi.size() > 2 ) {
                 vector<int>::const_iterator cv_it = cvi.begin();
                 c.SetRed  (static_cast<unsigned char>(*cv_it++));
                 c.SetGreen(static_cast<unsigned char>(*cv_it++));
                 c.SetBlue (static_cast<unsigned char>(*cv_it++));
                 if (cv_it != cvi.end()) {
                     c.SetAlpha(static_cast<unsigned char>(*cv_it));
                 }
             } else {
                 return false;
             }
             break;
         }}
    case objects::CUser_field::TData::e_Reals:   {{
         vector<double> cvd = f.GetData().GetReals();
         if (cvd.size() > 2 ) {
             vector<double>::const_iterator cv_it = cvd.begin();
             c.SetRed  (static_cast<float>(*cv_it++));
             c.SetGreen(static_cast<float>(*cv_it++));
             c.SetBlue (static_cast<float>(*cv_it++));
             if (cv_it != cvd.end()) {
                 c.SetAlpha(static_cast<float>(*cv_it));
             }
         } else {
             return false;
         }
         break;
     }}
    case objects::CUser_field::TData::e_Str: {{
         string cs = f.GetData().GetStr();
         c.FromString(cs);
         break;
     }}
    default:
        return false;
    }

    return true;
}


void CPhyloTreeScheme::x_ColorToUserField(CRegistryWriteView& view,
                                          const string& key,
                                          const CRgbaColor& c) const
{
    objects::CUser_field& f = *view.SetField(key);

    // store the color as a vector of ints.
    vector<int> val;
    val.push_back(c.GetRedUC());
    val.push_back(c.GetGreenUC());
    val.push_back(c.GetBlueUC());
    val.push_back(c.GetAlphaUC());

    f.SetData().SetInts() = val;
}

bool CPhyloTreeScheme::LoadCurrentSettings()
{
    try {
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(kPhyloBaseKey);

    m_LastLayout = view.GetInt("LastLayout", m_LastLayout);

    // check  for a preset theme
    if ( m_CurrTheme.empty()  ||
        !view.HasField(m_ThemeKey + "." + m_CurrTheme) ) {
        m_CurrTheme = view.GetString("CurrentTheme", m_DefaultTheme);
    }

    // load theme settings
    view = x_GetReadView(gui_reg, kPhyloBaseKey + "." + m_ThemeKey,
        m_CurrTheme, m_DefaultTheme);
    if (m_CurrColorTheme.empty()) {
        m_CurrColorTheme = view.GetString("ColorTheme", "Color");
    }
    if (m_CurrSizeTheme.empty()) {
        m_CurrSizeTheme = view.GetString("SizeLevel", "Normal");
    }
    if (m_CurrGlobalTheme.empty()) {
        m_CurrGlobalTheme = view.GetString("GlobalSettings", "Default");
    }

    // Get key for sizes and font
    view = x_GetReadView(gui_reg, 
                         kPhyloBaseKey, 
                         "Size." + m_CurrSizeTheme, 
                         "Size.Normal");

    // load sizes
    m_LeftMargin = view.GetReal("LeftMargin", m_LeftMargin);
    m_RightMargin = view.GetReal("RightMargin", m_RightMargin);
    m_TopMargin = view.GetReal("TopMargin", m_TopMargin);
    m_BottomMargin = view.GetReal("BottomMargin", m_BottomMargin);
    m_LineWidth = view.GetReal("LineWidth", m_LineWidth);
    m_NodeSize = view.GetReal("NodeSize", m_NodeSize);
    m_LeafNodeSize = view.GetReal("LeafNodeSize", m_LeafNodeSize);
    m_MaxNodeSize = view.GetReal("MaxNodeSize", m_MaxNodeSize);
    
    // For now, this is ignored.  If we want it to be diff from m_NodeSize,
    // Need to add it to UI too!
    m_MinNodeSize = m_NodeSize;

    // Variable size nodes turned on automatically if node sizes vary   
    m_bBoaNodes = view.GetBool("ResizableCollapsed", m_bBoaNodes);
     
    // load font
    m_LabelFormat = view.GetString("LabelFormat", m_LabelFormat);
    m_MaxLabelLength = view.GetInt("MaxLabelLength", m_MaxLabelLength);

    CGlTextureFont::EFontFace font_face = CGlTextureFont::FaceFromString(
                                    view.GetString("LabelFontFace", "Helvetica"));
    int font_size = view.GetInt("LabelFontSize", 10);

    if (font_face != CGlTextureFont::eFontFace_LastFont) {                                 
        m_Font.SetFontSize(font_size);
        m_Font.SetFontFace(font_face);
    }
    
    // Get key for colors
    // could make black/white a theme instead of separate...
    view = x_GetReadView(gui_reg, 
                         kPhyloBaseKey, 
                         "Color." + m_CurrColorTheme, 
                         "Color.Color");

    // Get selected coloration
    int tc = view.GetInt("Coloration", -1);
    if (tc != -1) {
        SetColoration((TColoration)tc);
    }

    // load colors
    x_UserFieldToColor(view, "LineColor", m_LineColor);
    x_UserFieldToColor(view,"LineSelColor",  m_LineSelColor);
    x_UserFieldToColor(view, "LineTraceColor",  m_LineTraceColor);
    x_UserFieldToColor(view, "LineSharedColor",  m_LineSharedColor);

    x_UserFieldToColor(view, "NodeColor", m_NodeColor);
    x_UserFieldToColor(view, "NodeSelColor", m_NodeSelColor);
    x_UserFieldToColor(view, "NodeTraceColor", m_NodeTraceColor);
    x_UserFieldToColor(view, "NodeSharedColor", m_NodeSharedColor);
    x_UserFieldToColor(view, "NodeTipSelColor", m_NodeTipSelColor);

    x_UserFieldToColor(view, "LabelColor", m_LabelColor);
    x_UserFieldToColor(view, "LabelSelColor", m_LabelSelColor);
    x_UserFieldToColor(view, "LabelTraceColor", m_LabelTraceColor);
    x_UserFieldToColor(view, "LabelSharedColor", m_LabelSharedColor);

    x_UserFieldToColor(view, "LineColorMono", m_LineColorMono);
    x_UserFieldToColor(view, "LineSelColorMono", m_LineSelColorMono);
    x_UserFieldToColor(view, "LineTraceColorMono", m_LineTraceColorMono);
    x_UserFieldToColor(view, "LineSharedColorMono", m_LineSharedColorMono);

    x_UserFieldToColor(view, "NodeColorMono", m_NodeColorMono);
    x_UserFieldToColor(view, "NodeSelColorMono", m_NodeSelColorMono);
    x_UserFieldToColor(view, "NodeTraceColorMono", m_NodeTraceColorMono);
    x_UserFieldToColor(view, "NodeSharedColorMono", m_NodeSharedColorMono);
    x_UserFieldToColor(view, "NodeTipSelColorMono", m_NodeTipSelColorMono);

    x_UserFieldToColor(view, "LabelColorMono", m_LabelColorMono);
    x_UserFieldToColor(view, "LabelSelColorMono", m_LabelSelColorMono);
    x_UserFieldToColor(view, "LabelTraceColorMono", m_LabelTraceColorMono);
    x_UserFieldToColor(view, "LabelSharedColorMono", m_LabelSharedColorMono);
       
    m_NonSelectedAlpha = (float)view.GetReal("NonSelectedAlpha", m_NonSelectedAlpha);

    view = x_GetReadView(gui_reg, kPhyloBaseKey, "Zoom");   
    m_ZoomBehavior.clear();
    // Find default zoom for each named renderer (only renderers the user has
    // previously used will be here and we have to retrieve the key names here
    // since they are the names of the renderers
    CRegistryReadView::TKeys keys;
    view.GetTopKeys(keys);
    ITERATE(CRegistryReadView::TKeys, iter, keys) {
        int zoombehavior = view.GetInt((*iter).key);
        if (zoombehavior >= 0 && zoombehavior <= int(eZoomY))
            m_ZoomBehavior[(*iter).key] = TZoomBehavior(zoombehavior);
    }

    }
    catch (std::exception&)
    {
        return false;
    }

    return true;
}

bool CPhyloTreeScheme::SaveCurrentSettings() const
{
 
    try {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(kPhyloBaseKey);
        // Save last layout user switched to
        view.Set("LastLayout", m_LastLayout);
        view.Set("CurrentTheme", m_CurrTheme);

        // save theme settings
        view = x_GetWriteView(gui_reg, kPhyloBaseKey + "." + m_ThemeKey,
                              m_CurrTheme, m_DefaultTheme);
        view.Set("ColorTheme", m_CurrColorTheme);
        view.Set("SizeLevel", m_CurrSizeTheme);
        view.Set("GlobalSettings", m_CurrGlobalTheme);

        // Get key for sizes and font
        view = x_GetWriteView(gui_reg, 
                                  kPhyloBaseKey, 
                                  "Size." + m_CurrSizeTheme,
                                  "Size.Normal");

        // save size settings
        view.Set("LeftMargin", m_LeftMargin);
        view.Set("RightMargin", m_RightMargin);
        view.Set("TopMargin", m_TopMargin);
        view.Set("BottomMargin", m_BottomMargin);
        view.Set("NodeSize", m_NodeSize);
        view.Set("LeafNodeSize", m_LeafNodeSize);
        view.Set("MaxNodeSize", m_MaxNodeSize);
        view.Set("LineWidth", m_LineWidth);
        view.Set("ResizableCollapsed", m_bBoaNodes);

        // save font settings
        view.Set("LabelFormat", m_LabelFormat);
        view.Set("MaxLabelLength", m_MaxLabelLength);

        view.Set("LabelFontFace", 
                 CGlTextureFont::FaceToString(m_Font.GetFontFace()));
        view.Set("LabelFontSize", (int)m_Font.GetFontSize());               

        // save color settings
        // could make black/white a theme instead of separate...
        view = x_GetWriteView(gui_reg,
                                  kPhyloBaseKey, 
                                  "Color." + m_CurrColorTheme, 
                                  "Color.Color");     

        view.Set("Coloration", (int)m_Coloration);

        x_ColorToUserField(view, "LineColor", m_LineColor);
        x_ColorToUserField(view, "LineSelColor", m_LineSelColor);
        x_ColorToUserField(view, "LineTraceColor", m_LineTraceColor);
        x_ColorToUserField(view, "LineSharedColor", m_LineSharedColor);

        x_ColorToUserField(view, "NodeColor", m_NodeColor);
        x_ColorToUserField(view, "NodeSelColor", m_NodeSelColor);
        x_ColorToUserField(view, "NodeTraceColor", m_NodeTraceColor);
        x_ColorToUserField(view, "NodeSharedColor", m_NodeSharedColor);
        x_ColorToUserField(view, "NodeTipSelColor", m_NodeTipSelColor);

        x_ColorToUserField(view, "LabelColor", m_LabelColor);
        x_ColorToUserField(view, "LabelSelColor", m_LabelSelColor);
        x_ColorToUserField(view, "LabelTraceColor", m_LabelTraceColor);
        x_ColorToUserField(view, "LabelSharedColor", m_LabelSharedColor);

        x_ColorToUserField(view, "LineColorMono", m_LineColorMono);
        x_ColorToUserField(view, "LineSelColorMono", m_LineSelColorMono);
        x_ColorToUserField(view, "LineTraceColorMono", m_LineTraceColorMono);
        x_ColorToUserField(view, "LineSharedColorMono", m_LineSharedColorMono);

        x_ColorToUserField(view, "NodeColorMono", m_NodeColorMono);
        x_ColorToUserField(view, "NodeSelColorMono", m_NodeSelColorMono);
        x_ColorToUserField(view, "NodeTraceColorMono", m_NodeTraceColorMono);
        x_ColorToUserField(view, "NodeSharedColorMono", m_NodeSharedColorMono);
        x_ColorToUserField(view, "NodeTipSelColorMono", m_NodeTipSelColorMono);

        x_ColorToUserField(view, "LabelColorMono", m_LabelColorMono);
        x_ColorToUserField(view, "LabelSelColorMono", m_LabelSelColorMono);
        x_ColorToUserField(view, "LabelTraceColorMono", m_LabelTraceColorMono);
        x_ColorToUserField(view, "LabelSharedColorMono", m_LabelSharedColorMono);    

        view.Set("NonSelectedAlpha", m_NonSelectedAlpha);

        view = x_GetWriteView(gui_reg, kPhyloBaseKey, "Zoom");

        // Save zoom for each separate renderer as a separate key
        std::map<string,TZoomBehavior>::const_iterator iter;
        for (iter=m_ZoomBehavior.begin(); iter != m_ZoomBehavior.end(); ++iter) {
            view.Set((*iter).first, int((*iter).second));
        }
    }
    catch (std::exception&)
    {
        return false;
    }

    return true;    
}

void CPhyloTreeScheme::GetDefaultBroadcastProperties(wxArrayString& wxprops)
{
    wxprops.clear();    

    wxprops.Add(wxT("seq-id"));
    wxprops.Add(wxT("fullasm_id"));
    wxprops.Add(wxT("asm_accession"));
    wxprops.Add(wxT("tax-id"));
    wxprops.Add(wxT("taxid"));
    wxprops.Add(wxT("label"));
}



END_NCBI_SCOPE
