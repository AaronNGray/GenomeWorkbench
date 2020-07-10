/*  $Id: row_display_style.cpp 43363 2019-06-20 14:33:30Z shkeda $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <util/static_map.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/widgets/aln_multiple/row_display_style.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_data_source.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE


CWidgetDisplayStyle::CWidgetDisplayStyle()
:   m_TextFont(CGlTextureFont::eFontFace_Helvetica, 12),
    m_SeqFont(CGlTextureFont::eFontFace_Courier_Bold, 12),
    m_ShowIdenticalBases(false),
    m_ShowConsensus(false)
{
    m_ColorMap[eText] = CRgbaColor(0.0f, 0.0f, 0.0f); // black
    m_ColorMap[eBack] = CRgbaColor(1.0f, 1.0f, 1.0f); //
    m_ColorMap[eSelectedText] = CRgbaColor(1.0f, 1.0f, 0.0f); // yellow
    m_ColorMap[eSelectedBack] = CRgbaColor(0.75f, 0.75f, 0.75f); // gray
    m_ColorMap[eFocusedBack] = CRgbaColor(0.0f, 0.0f, 0.5f); // dark blue
    m_ColorMap[eFrame] = CRgbaColor(0.8f, 0.8f, 0.8f);
    m_ColorMap[eAlignSegs] = CRgbaColor(0.8f, 0.8f, 0.8f); // light  gray
    m_ColorMap[eAlignSegsNoScores] = CRgbaColor(0.75f, 0.75f, 0.75f); // gray
    m_ColorMap[eSequence] = CRgbaColor(0.0f, 0.0f, 0.0f); // black
}

// Option to Set an invalid font to start since opengl (pane) window may not 
// yet be created and we need a valid opengl context.
CWidgetDisplayStyle::CWidgetDisplayStyle(CGlTextureFont::EFontFace force_font)
:   m_TextFont(force_font, 12),
    m_SeqFont(force_font, 12),
    m_ShowIdenticalBases(false),
    m_ShowConsensus(false)
    
{
    m_ColorMap[eText] = CRgbaColor(0.0f, 0.0f, 0.0f); // black
    m_ColorMap[eBack] = CRgbaColor(1.0f, 1.0f, 1.0f); //
    m_ColorMap[eSelectedText] = CRgbaColor(1.0f, 1.0f, 0.0f); // yellow
    m_ColorMap[eSelectedBack] = CRgbaColor(0.75f, 0.75f, 0.75f); // gray
    m_ColorMap[eFocusedBack] = CRgbaColor(0.0f, 0.0f, 0.5f); // dark blue
    m_ColorMap[eFrame] = CRgbaColor(0.8f, 0.8f, 0.8f);
    m_ColorMap[eAlignSegs] = CRgbaColor(0.8f, 0.8f, 0.8f); // light  gray
    m_ColorMap[eAlignSegsNoScores] = CRgbaColor(0.75f, 0.75f, 0.75f); // gray
    m_ColorMap[eSequence] = CRgbaColor(0.0f, 0.0f, 0.0f); // black
}

const CRgbaColor&   CWidgetDisplayStyle::GetColor(EColorType type) const
{
    TColorMap::const_iterator it = m_ColorMap.find(type);
    _ASSERT(it != m_ColorMap.end());
    return it->second;
}


void CWidgetDisplayStyle::SetColor(EColorType type, const CRgbaColor& color)
{
    m_ColorMap[type] = color;
}


static const char* kDefDNAMethodTag = "Default DNA Scoring Method";
static const char* kDefProteinMethodTag = "Default Protein Scoring Method";
static const char* kDotImage = "dot_image";


void CWidgetDisplayStyle::x_GetColor(EColorType type, const CRgbaColor& c)
{
    m_ColorMap[type] = c;
}


void sReadFont(const CRegistryReadView& view, const string& key, CGlTextureFont& font)
{
    static string s;
    s = view.GetString(key, "");
    if( ! s.empty())    {
        CGlTextureFont::EFontFace face = font.GetFontFace();
        unsigned int font_size = font.GetFontSize();

        // Reset font if it fails to read correctly. This was added after transition from
        // CGlBitmap fonts to CGltexture fonts since the stored strings were incompatible
        // and so fonts would not be created successfully from registry.
        if (!font.FromString(s)) {
            font.SetFontFace(face);
            font.SetFontSize(font_size);
        }
    }
}

// copied from gui/widgets/seq_graphic/config_utils.cpp
static void sGetFont(const CRegistryReadView& view,
                     const string& face_key,
                     const string& size_key,
                     CGlTextureFont& f)
{
    CGlTextureFont::EFontFace font_face = CGlTextureFont::FaceFromString(
        view.GetString(face_key, "Helvetica"));
    unsigned int font_size =  (unsigned int)view.GetInt(size_key, 10);
                
    f.SetFontFace(font_face);
    f.SetFontSize(font_size);
}

static void sSetFont(CRegistryWriteView& view,
                             const string& face_key, const string& size_key,
                             const CGlTextureFont& f)
{
    view.Set(face_key, CGlTextureFont::FaceToString(f.GetFontFace()));
    view.Set(size_key, (int)f.GetFontSize());
}


void CWidgetDisplayStyle::ReadColor(EColorType type, const CRegistryReadView& view, const string& key)
{
    static string s;
    s = view.GetString(key, "");
    if( ! s.empty())    {
        m_ColorMap[type].FromString(s);
    }
}

static const char* kColNamesTag = "Column Names";
static const char* kColWidthsTag = "Column Tag";
static const char* kColVisibleTag = "Column Visible";

void CWidgetDisplayStyle::LoadSettings(const CRegistryReadView& view)
{
    m_TextFont.SetFontFace(CGlTextureFont::eFontFace_Helvetica);
    m_TextFont.SetFontSize(12);
//    sReadFont(view, "Text Font", m_TextFont);
    sGetFont(view, "TextFontFace", "TextFontSize", m_TextFont);

    m_SeqFont.SetFontFace(CGlTextureFont::eFontFace_Courier_Bold);
    m_SeqFont.SetFontSize(12);
//    sReadFont(view, "Sequence Font", m_SeqFont);
    sGetFont(view, "SeqFontFace", "SeqFontSize", m_SeqFont);

    m_ShowIdenticalBases = view.GetBool("Show Identical Bases", m_ShowIdenticalBases);
    m_ShowConsensus = view.GetBool("Show Consensus", m_ShowConsensus);

    ReadColor(eText, view, "Text Color");
    ReadColor(eBack, view, "Back Color");
    ReadColor(eSelectedText, view, "Selected Text Color");
    ReadColor(eSelectedBack, view, "Selected Back Color");
    ReadColor(eFocusedBack, view, "Focused Back Color");
    ReadColor(eFrame, view, "Frame Color");
    ReadColor(eAlignSegs, view, "Align Segs Color");
    ReadColor(eAlignSegsNoScores, view, "Align Segs No Scores Color");
    ReadColor(eSequence, view, "Sequence Color");

    // loading column information;
    vector<string>  names;
    vector<int>     widths;
    vector<int>     visible;

    view.GetStringVec(kColNamesTag, names);
    view.GetIntVec(kColWidthsTag, widths);
    view.GetIntVec(kColVisibleTag, visible);

    m_Columns.clear();
    if(names.size() == widths.size() &&  names.size() == visible.size()) {
        size_t n = names.size();
        m_Columns.resize(n);

        for( size_t i = 0;  i < n;  i++  )   {
            SColumn& col = m_Columns[i];
            col.m_Name = names[i];
            // Set a minimal width to 10 pixels.
            // The minimal width will also be enforced when users change 
            // the width, see row_display_style.cpp. We recover any
            // zero-width settings saved before the minimal width is
            // introduced.
            col.m_Width = max(10, widths[i]);
            col.m_Visible = visible[i] > 0;
        }
    }

    m_DefDNAMethod = view.GetString(kDefDNAMethodTag);
    m_DefProteinMethod = view.GetString(kDefProteinMethodTag);
    m_DotImage = view.GetString(kDotImage);
}


void CWidgetDisplayStyle::SaveSettings(CRegistryWriteView view,
                                       IAlnExplorer::EAlignType type)
{

    sSetFont(view, "TextFontFace", "TextFontSize", m_TextFont);
    sSetFont(view, "SeqFontFace", "SeqFontSize", m_SeqFont);

    view.Set("Show Identical Bases", m_ShowIdenticalBases);
    view.Set("Show Consensus", m_ShowConsensus);

    view.Set("Text Color", m_ColorMap[eText].ToString());
    view.Set("Back Color", m_ColorMap[eBack].ToString());
    view.Set("Selected Text Color", m_ColorMap[eSelectedText].ToString());
    view.Set("Selected Back Color", m_ColorMap[eSelectedBack].ToString());
    view.Set("Focused Back Color", m_ColorMap[eFocusedBack].ToString());
    view.Set("Frame Color", m_ColorMap[eFrame].ToString());
    view.Set("Align Segs Color", m_ColorMap[eAlignSegs].ToString());
    view.Set("Align Segs No Scores Color", m_ColorMap[eAlignSegsNoScores].ToString());
    view.Set("Sequence Color", m_ColorMap[eSequence].ToString());

    // saving column information;
    size_t n = m_Columns.size();
    vector<string>  names(n);
    vector<int>     widths(n);
    vector<int>     visible(n);

    for( size_t i = 0;  i < n;  i++  )   {
        const SColumn& col = m_Columns[i];
        names[i] = col.m_Name;
        widths[i] = col.m_Width;
        visible[i] = col.m_Visible ? 1 : 0;
    }

    view.Set(kColNamesTag, names);
    view.Set(kColWidthsTag, widths);
    view.Set(kColVisibleTag, visible);

    // we should not overwrite setting that we do not really have
    switch(type)    {
    case IAlnExplorer::fDNA:
        view.Set(kDefDNAMethodTag, m_DefDNAMethod);
        break;
    case IAlnExplorer::fProtein:
        view.Set(kDefProteinMethodTag, m_DefProteinMethod);
        break;
    default:
        break;
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CRowDisplayStyle
CRowDisplayStyle::CRowDisplayStyle()
:   m_WidgetStyle(NULL)
{
}


CRowDisplayStyle::~CRowDisplayStyle()
{
}


void    CRowDisplayStyle::SetWidgetStyle(const CWidgetDisplayStyle* style)
{
    m_WidgetStyle = style;
}


const CGlTextureFont&  CRowDisplayStyle::GetTextFont(void) const
{
    return m_WidgetStyle->m_TextFont;
}


const CGlTextureFont&  CRowDisplayStyle::GetSeqFont(void) const
{
    return m_WidgetStyle->m_SeqFont;
}

bool CRowDisplayStyle::GetShowIdenticalBases(void) const
{
    return m_WidgetStyle->m_ShowIdenticalBases;
}


const CRgbaColor&   CRowDisplayStyle::GetColor(TColorType type) const
{
    return m_WidgetStyle->GetColor(type);
}

CRef<CSeqGraphicConfig> CRowDisplayStyle::GetSeqGraphicConfig() const
{
    if (m_gConfig == 0) {
        m_gConfig.Reset(new CSeqGraphicConfig(!CSGDataSource::IsBackgroundJob(), &CGuiRegistry::GetInstance(), "Default"));
        m_gConfig->SetBackgroundLoading(CSGDataSource::IsBackgroundJob());
        m_gConfig->LoadSettings();
        m_gConfig->SetLabelPos("No label");
    }
    return m_gConfig;
}

CRowStyleCatalog::CRowStyleCatalog()
:   m_WidgetStyle(NULL),
    m_DefStyle(NULL)
{
}


CRowStyleCatalog::~CRowStyleCatalog()
{
    delete m_DefStyle;
}


void    CRowStyleCatalog::SetWidgetStyle(const CWidgetDisplayStyle* style)
{
    m_WidgetStyle = style;
    if(m_DefStyle) {
        m_DefStyle->SetWidgetStyle(m_WidgetStyle);
    }
}

void    CRowStyleCatalog::SetDefaultStyle(CRowDisplayStyle* style)
{
    m_DefStyle = style;
    if(m_DefStyle) {
        m_DefStyle->SetWidgetStyle(m_WidgetStyle);
    }
}

const CRowDisplayStyle*   CRowStyleCatalog::GetStyleForRow(int row) const
{
    TRowToStyle::const_iterator it = m_RowToStyle.find(row);
    return (it == m_RowToStyle.end()) ? m_DefStyle : it->second;
}


const CRowDisplayStyle*   CRowStyleCatalog::GetStyleForRow(int row, int type) const
{
    TTypeToStyle::const_iterator it_t = m_TypeToStyle.find(type);
    if(it_t == m_TypeToStyle.end()) {
        TRowToStyle::const_iterator it = m_RowToStyle.find(row);
        return (it == m_RowToStyle.end()) ? m_DefStyle : it->second;
    } else  {
        return it_t->second;
    }
}


void    CRowStyleCatalog::SetRowStyle(TNumrow row, CRowDisplayStyle* style)
{
    _ASSERT(style);
    style->SetWidgetStyle(m_WidgetStyle);
    m_RowToStyle[row] = style;
}


void    CRowStyleCatalog::SetRowTypeStyle(int type, CRowDisplayStyle* style)
{
    _ASSERT(style);
    style->SetWidgetStyle(m_WidgetStyle);
    m_TypeToStyle[type] = style;
}


void    CRowStyleCatalog::ClearCustomStyles()
{
    m_RowToStyle.clear();
    m_TypeToStyle.clear();
}


END_NCBI_SCOPE
