#ifndef __GUI_WIDGETS_ALNMULTI___ROW_DISPLAY_STYLE__HPP
#define __GUI_WIDGETS_ALNMULTI___ROW_DISPLAY_STYLE__HPP

/*  $Id: row_display_style.hpp 44974 2020-04-30 18:27:28Z shkeda $
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

#include <corelib/ncbiobj.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/utils/rgba_color.hpp>

#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/irender.hpp>

#include <objtools/alnmgr/aln_explorer.hpp>
#include <gui/opengl/gltexture.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/seq_graphic/track_config_manager.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CWidgetDisplayStyle is a collection of display properties common for all
/// rows in the CAlnMultiWidget.

class  NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT  CWidgetDisplayStyle
{
public:
    typedef map<int, CRgbaColor>  TColorMap;

    enum    EColorType {
        eInvalid = -1,
        eText,
        eBack,
        eSelectedText,
        eSelectedBack,
        eFocusedBack,
        eFrame,
        eAlignSegs,
        eSequence,
        eAlignSegsNoScores /// this color is used when scoring is enabled but
        /// cannot be displayed because of the scale
    };

    struct  SColumn    {
        string  m_Name;
        int     m_Width;
        int     m_Visible;
    };
    typedef vector<SColumn> TColumns;

    CWidgetDisplayStyle(void);
    CWidgetDisplayStyle(CGlTextureFont::EFontFace force_font);
    virtual ~CWidgetDisplayStyle(void) {};

    virtual const CRgbaColor& GetColor(EColorType type) const;
    virtual void            SetColor(EColorType type, const CRgbaColor& color);

    void    LoadSettings(const CRegistryReadView& view);
    void    SaveSettings(CRegistryWriteView view, IAlnExplorer::EAlignType type);

protected:
    void    ReadColor(EColorType type, const CRegistryReadView& view, const string& key);
    void    x_GetColor(EColorType type, const CRgbaColor& c);

public:
    CGlTextureFont   m_TextFont; /// text font (description, positions etc.)
    CGlTextureFont   m_SeqFont; /// sequence font
    bool             m_ShowIdenticalBases;  // If false, don't draw base letters matching master row
    bool             m_ShowConsensus; // Show consensus if available (sparse align has no consensu track)

    TColumns    m_Columns;

    string  m_DefDNAMethod;     /// default scoring method for DNA alignments
    string  m_DefProteinMethod; /// default scoring method for protein alignments

    string m_DotImage; ///< png file name - image for identical dot (14x14, 16x16)

protected:
    TColorMap   m_ColorMap;
};

////////////////////////////////////////////////////////////////////////////////
/// CRowDisplayStyle is a base class representing a display style for IAlignRow.
///
/// CRowDisplayStyle plays a role of Proxy providing functions for accessing
/// display properties actually stored in a widget.

class  NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT  CRowDisplayStyle
{
public:
    enum EShownElements {
        fShown_Alignment = 8,
        fShown_Track = 16,
        fShown_All = 0xFF
    };
    typedef CWidgetDisplayStyle::EColorType TColorType;

    CRowDisplayStyle();
    virtual ~CRowDisplayStyle(void);

    virtual void    SetWidgetStyle(const CWidgetDisplayStyle* style);

    /// @name Proxy methods
    /// provide access to the Widget display style properties
    /// @{
    virtual const CGlTextureFont&  GetTextFont(void) const;
    virtual const CGlTextureFont&  GetSeqFont(void) const;
    virtual bool GetShowIdenticalBases(void) const;

    virtual const CRgbaColor&   GetColor(TColorType type) const;
    /// @}

    void     SetShownElements(int shown_mask);
    int      GetShownElements() const;

    void SetSeqGraphicConfig(CRef<CSeqGraphicConfig>& config);
    CRef<CSeqGraphicConfig> GetSeqGraphicConfig() const;

    void SetTrackSettings(const CTrackConfigManager::TTrackProxies& track_settings);
    const CTrackConfigManager::TTrackProxies& GetTrackSettings() const;

    void SetTrackConfigKey(const string& config_key);
    const string& GetTrackConfigKey() const;

    void     SetDotTexture(CRef<CGlTexture>& dot_texture);
    CRef<CGlTexture>  GetDotTexture() const;

    // Rendering happens in Browser (not gbench or pdf/svg prinout)
    bool IsWebRendering() const {
        return GetSeqGraphicConfig()->GetCgiMode() && !GetGl().IsPrinterFriendly();
    }

protected:
    const CWidgetDisplayStyle*  m_WidgetStyle;  /// pointer to Widget's style
    int m_ShownElements = fShown_All;
    mutable CRef<CSeqGraphicConfig> m_gConfig;

    CTrackConfigManager::TTrackProxies m_TrackSettings;
    string m_TrackConfigKey = "GBPlugins.MSAFeatPanel";
    
    mutable CRef<CGlTexture> m_DotTexture;
    
    
};

////////////////////////////////////////////////////////////////////////////////
/// CRowStyleCatalog is a collection of CRowDisplayStyle-s assigned to rows
/// in alignment.
/// By default all rows have the same style but can be customized by...

class  NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT  CRowStyleCatalog
{
public:
    typedef int TNumrow;

    CRowStyleCatalog();
    ~CRowStyleCatalog();

    void    SetWidgetStyle(const CWidgetDisplayStyle* style);

    /// assumes ownership
    void    SetDefaultStyle(CRowDisplayStyle* style);

    void    SetRowStyle(TNumrow row, CRowDisplayStyle* style);
    void    SetRowTypeStyle(int type, CRowDisplayStyle* style);

    const CRowDisplayStyle*   GetStyleForRow(TNumrow row) const;
    const CRowDisplayStyle*   GetStyleForRow(TNumrow row, int type) const;

    void    ClearCustomStyles();

protected:
    typedef map<TNumrow, CRowDisplayStyle*> TRowToStyle;
    typedef map<int, CRowDisplayStyle*>     TTypeToStyle;

    const CWidgetDisplayStyle*  m_WidgetStyle;
    CRowDisplayStyle*           m_DefStyle;

    TRowToStyle     m_RowToStyle; /// overriden styles for rows
    TTypeToStyle    m_TypeToStyle; /// overriden styles for row types
};

inline
void CRowDisplayStyle::SetShownElements(int shown_mask)
{
    m_ShownElements = shown_mask;
}

inline
int CRowDisplayStyle::GetShownElements() const
{
    return m_ShownElements;
}


inline
void CRowDisplayStyle::SetSeqGraphicConfig(CRef<CSeqGraphicConfig>& config)
{
    m_gConfig = config;
}

inline
void CRowDisplayStyle::SetTrackSettings(const CTrackConfigManager::TTrackProxies& track_settings)
{
    m_TrackSettings = track_settings;
}

inline
const CTrackConfigManager::TTrackProxies& CRowDisplayStyle::GetTrackSettings() const
{
    return m_TrackSettings;
}

inline
void CRowDisplayStyle::SetTrackConfigKey(const string& config_key)
{
    m_TrackConfigKey = config_key;
}

inline
const string& CRowDisplayStyle::GetTrackConfigKey() const
{
    return m_TrackConfigKey;
}


inline
void CRowDisplayStyle::SetDotTexture(CRef<CGlTexture>& dot_texture)
{
    m_DotTexture = dot_texture;
}

inline
CRef<CGlTexture>  CRowDisplayStyle::GetDotTexture() const
{
    return m_DotTexture;
}




END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___ROW_DISPLAY_STYLE__HPP
