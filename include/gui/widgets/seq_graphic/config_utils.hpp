#ifndef GUI_WIDGETS_SEQ_GRAPHIC___CONFIG_UTILS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___CONFIG_UTILS__HPP

/*  $Id: config_utils.hpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *      A set of utitlity methods for creating views and getting global keys.
 *
 */

#include <gui/objutils/registry.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/glbitmapfont.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <objects/general/User_field.hpp>
#include <gui/gui.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGConfigUtils
{
public:
    /// @static read/readwrite view creation helper methods.
    /// @{
    /// Create a read view with polymorphic data structure.
    /// It returns a read view that overlays several named sub-sections in a
    /// given section. Note that the sections listed are presumed to be in
    /// decreasing order of importance, so curr_key will overload def_key1
    /// which in turn will overload key_2 and so on.
    /// Example:
    /// For a given settings as below:
    ///      [Base.Section.Default]
    ///         BGColor = white
    ///         FGColor = black
    ///      [Base.Section.Gene]
    ///         FGColor = red
    /// The read view created with parameters:
    ///      GetReadView(reg, "Base.Section", "Gene", "Default") has the
    /// effect that settings in Gene section will override the corresponding
    /// settings in the Default section. So the FGColor will be red instead of
    /// black.
    ///
    static CRegistryReadView GetReadView(
        const CGuiRegistry& reg, const string& base_key,
        const string& curr_key, const string& def_key1 = "",
        const string& def_key2 = "", const string& def_key3 = "");

    static CRegistryWriteView GetWriteView(
        CGuiRegistry& reg, const string& base_key,
        const string& curr_key, const string& def_key1 = "",
        const string& def_key2 = "", const string& def_key3 = "");

    /// Create a read view specifically for 'Color' section.
    /// The method is used for constructing necessary parameters to create
    /// a read view for 'Color' section with polymothic data structure.
    /// For a given example settings as below:
    ///      [Base.Section.GeneModel.Default]
    ///         ShowLabel = true
    ///      [Base.Section.GeneModel.Default.Color.Color] ; default settings
    ///         BGColor = white
    ///         FGColor = green
    /// base_key is Base.Section.GeneModel.Default in this example, and key
    /// is the current color setting key which is "Color".
    static CRegistryReadView GetColorReadView(
        const CGuiRegistry& reg, const string& base_key,
        const string& sect, const string& key,
        const string& def_sect = "");

    static CRegistryWriteView GetColorRWView(
        CGuiRegistry& reg, const string& base_key,
        const string& sect, const string& key,
        const string& def_sect = "");

    static CRegistryReadView GetSizeReadView(
        const CGuiRegistry& reg, const string& base_key,
        const string& sect, const string& key,
        const string& def_sect = "");

    static CRegistryWriteView GetSizeRWView(
        CGuiRegistry& reg, const string& base_key,
        const string& sect, const string& key,
        const string& def_sect = "");

    static CRegistryReadView GetTrackProfileReadView(
        const CGuiRegistry& reg, const string& base_key,
        const string& key);

    static CRegistryWriteView GetTrackProfileRWView(
        CGuiRegistry& reg, const string& base_key,
        const string& key);

    static CRegistryReadView GetLabelPosReadView(
        const CGuiRegistry& reg, const string& base_key,
        const string& sect, const string& key,
        const string& def_sect = "");

    static CRegistryWriteView GetLabelPosRWView(
        CGuiRegistry& reg, const string& base_key,
        const string& sect, const string& key,
        const string& def_sect = "");
    /// @}

    static string ComposeColorKey(const string& base, const string& key);
    static string ComposeSizeKey(const string& base, const string& key);
    static string ComposeDecoKey(const string& base, const string& key);

    typedef pair<string, string> TKeyValuePair;
    typedef map<string, string>  TKeyValuePairs;

    static void ParseProfileString(const string& profile_str,
                                   TKeyValuePairs& settings);

    static string ComposeProfileString(const TKeyValuePairs& settings);

    /// @name color accessor and mutator.
    /// @{
    static void GetColor(const CRegistryReadView& view,
        const string& key, CRgbaColor& color);

    static void SetColor(CRegistryWriteView& view,
        const string& key, const CRgbaColor& color);
    /// @}

    /// @name font accessor and mutator.
    /// @{
    static void GetFont(const CRegistryReadView& view,
        const string& face_key, const string& size_key, CGlTextureFont& f);
    static void SetFont(CRegistryWriteView& view,
        const string& face_key, const string& size_key,
        const CGlTextureFont& f);

    static void GetTexFont(const CRegistryReadView& view,
        const string& face_key, const string& size_key, CGlTextureFont& f);
    static void SetTexFont(CRegistryWriteView& view,
        const string& face_key, const string& size_key,
        const CGlTextureFont& f);
    /// @}

    /// @name five typical keys and default style names.
    /// @{
    static const string& ColorKey();
    static const string& SizeKey();
    static const string& DecoKey();
    static const string& TrackProfileKey();
    static const string& LayoutKey();
    static const string& LabelPositionKey();
    static const string& GlobalSettingStyleKey();
    static const string& UserSettingPrefix();

    static const string& DefColorTheme();
    static const string& DefSizeLevel();
    static const string& DefDecoStyle();
    static const string& DefTrackProfile();
    static const string& DefLayout();
    static const string& DefLabelPosition();
    static const string& DefGlobalSettingStyle();
    /// @}

private:
    static bool x_UserFieldToColor(const objects::CUser_field& f, CRgbaColor& c);
    static void x_ColorToUserField(const CRgbaColor& c, objects::CUser_field& f);
    
    static string m_CommonDefaultStyleName;
};

inline
CRegistryReadView CSGConfigUtils::GetColorReadView(
    const CGuiRegistry& reg, const string& base_key,
    const string& sect, const string& key,
    const string& def_sect)
{
    return GetReadView(reg, base_key,
        (sect.empty() ? "" : sect + ".") + ColorKey() + "." + key,
        (sect.empty() ? "" : sect + ".") + ColorKey() + "." + DefColorTheme(),
        def_sect.empty() ? "" : def_sect + "." + ColorKey() + "." + key, 
        def_sect.empty() ? "" : def_sect + "." + ColorKey() + "." + DefColorTheme());
}

inline
CRegistryWriteView CSGConfigUtils::GetColorRWView(
    CGuiRegistry& reg, const string& base_key,
    const string& sect, const string& key,
    const string& def_sect)
{
    return GetWriteView(reg, base_key,
        (sect.empty() ? "" : sect + ".") + ColorKey() + "." + key,
        (sect.empty() ? "" : sect + ".") + ColorKey() + "." + DefColorTheme(),
        def_sect.empty() ? "" : def_sect + "." + ColorKey() + "." + key, 
        def_sect.empty() ? "" : def_sect + "." + ColorKey() + "." + DefColorTheme());
}

inline
CRegistryReadView CSGConfigUtils::GetSizeReadView(
    const CGuiRegistry& reg, const string& base_key,
    const string& sect, const string& key,
    const string& def_sect)
{
    return GetReadView(reg, base_key,
        (sect.empty() ? "" : sect + ".") + SizeKey() + "." + key,
        (sect.empty() ? "" : sect + ".") + SizeKey() + "." + DefSizeLevel(),
        def_sect.empty() ? "" : def_sect + "." + SizeKey() + "." + key, 
        def_sect.empty() ? "" : def_sect + "." + SizeKey() + "." + DefSizeLevel());
}

inline
CRegistryWriteView CSGConfigUtils::GetSizeRWView(
    CGuiRegistry& reg, const string& base_key,
    const string& sect, const string& key,
    const string& def_sect)
{
    return GetWriteView(reg, base_key,
        (sect.empty() ? "" : sect + ".") + SizeKey() + "." + key,
        (sect.empty() ? "" : sect + ".") + SizeKey() + "." + DefSizeLevel(),
        def_sect.empty() ? "" : def_sect + "." + SizeKey() + "." + key, 
        def_sect.empty() ? "" : def_sect + "." + SizeKey() + "." + DefSizeLevel());
}

inline
CRegistryReadView CSGConfigUtils::GetTrackProfileReadView(
    const CGuiRegistry& reg, const string& base_key, const string& key)
{
    return GetReadView(reg, base_key + "." + TrackProfileKey(),
        key, DefTrackProfile());
}

inline
CRegistryWriteView CSGConfigUtils::GetTrackProfileRWView(
    CGuiRegistry& reg, const string& base_key, const string& key)
{
    return GetWriteView(reg, base_key + "." + TrackProfileKey(),
        key, DefTrackProfile());
}

inline
CRegistryReadView CSGConfigUtils::GetLabelPosReadView(
    const CGuiRegistry& reg, const string& base_key,
    const string& sect, const string& key,
    const string& def_sect)
{
    return GetReadView(reg, base_key,
        (sect.empty() ? "" : sect + ".") + LabelPositionKey() + "." + key,
        (sect.empty() ? "" : sect + ".") + LabelPositionKey() + "." + DefLabelPosition(),
        def_sect.empty() ? "" : def_sect + "." + LabelPositionKey() + "." + key, 
        def_sect.empty() ? "" : def_sect + "." + LabelPositionKey() + "." + DefLabelPosition());
}

inline
CRegistryWriteView CSGConfigUtils::GetLabelPosRWView(
    CGuiRegistry& reg, const string& base_key,
    const string& sect, const string& key,
    const string& def_sect)
{
    return  GetWriteView(reg, base_key,
        (sect.empty() ? "" : sect + ".") + LabelPositionKey() + "." + key,
        (sect.empty() ? "" : sect + ".") + LabelPositionKey() + "." + DefLabelPosition(),
        def_sect.empty() ? "" : def_sect + "." + LabelPositionKey() + "." + key, 
        def_sect.empty() ? "" : def_sect + "." + LabelPositionKey() + "." + DefLabelPosition());
}

inline
string
CSGConfigUtils::ComposeColorKey(const string& base, const string& key)
{
    return base + "." + ColorKey() + "." + key;
}

inline
string
CSGConfigUtils::ComposeSizeKey(const string& base, const string& key)
{
    return base + "." + SizeKey() + "." + key;
}

inline
string
CSGConfigUtils::ComposeDecoKey(const string& base, const string& key)
{
    return base + "." + DecoKey() + "." + key;
}

inline
void CSGConfigUtils::GetColor(const CRegistryReadView& view,
                              const string& key, CRgbaColor& color)
{
    CConstRef<objects::CUser_field> ref(view.GetField(key));
    if (ref) {
        x_UserFieldToColor(*ref, color);
    }
}

inline
void CSGConfigUtils::SetColor(CRegistryWriteView& view,
                              const string& key, const CRgbaColor& color)
{
    CRef<objects::CUser_field> field = view.SetField(key);
    x_ColorToUserField(color, *field);
}

inline
const string& CSGConfigUtils::ColorKey()
{
    static string key = "Color";
    return key;
}

inline
const string& CSGConfigUtils::SizeKey()
{
    static string key = "Size";
    return key;
}

inline
const string& CSGConfigUtils::DecoKey()
{
    static string key = "Decorate";
    return key;
}

inline
const string& CSGConfigUtils::TrackProfileKey()
{
    static string key = "TrackProfile";
    return key;
}

inline
const string& CSGConfigUtils::LayoutKey()
{
    static string key = "Layout";
    return key;
}

inline
const string& CSGConfigUtils::LabelPositionKey()
{   
    static string key = "LabelPosition";
    return key;
}

inline
const string& CSGConfigUtils::GlobalSettingStyleKey()
{
    static string key = "GlobalSettings";
    return key;
}

inline
const string& CSGConfigUtils::UserSettingPrefix()
{
    static string key = "USER###";
    return key;
}

inline
const string& CSGConfigUtils::DefColorTheme()
{
    static string key = "Color";
    return key;
}

inline
const string& CSGConfigUtils::DefSizeLevel()
{
    static string key = "Normal";
    return key;
}

inline
const string& CSGConfigUtils::DefDecoStyle()
{
    return m_CommonDefaultStyleName;
}

inline
const string& CSGConfigUtils::DefTrackProfile()
{
    return m_CommonDefaultStyleName;
}

inline
const string& CSGConfigUtils::DefLayout()
{
    return m_CommonDefaultStyleName;
}

inline
const string& CSGConfigUtils::DefLabelPosition()
{
    return m_CommonDefaultStyleName;
}

inline
const string& CSGConfigUtils::DefGlobalSettingStyle()
{
    return m_CommonDefaultStyleName;
}


END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_SEQ_GRAPHIC___CONFIG_UTILS__HPP */
