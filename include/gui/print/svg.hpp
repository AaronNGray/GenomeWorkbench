#ifndef GUI_UTILS__SVG_HPP
#define GUI_UTILS__SVG_HPP

/*  $Id: svg.hpp 45024 2020-05-09 02:03:16Z evgeniev $
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
 * Authors:  Vladislav Evgeniev
 *
 * File Description:
 *      SVG serialization
 */

#include <ostream>
#include <vector>
#include <string>
#include <sstream> 

#include <corelib/ncbistd.hpp>
#include <gui/utils/vect2.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/gltexturefont.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(svg)

class StartTag
{
public:
    StartTag(std::string const& tag, unsigned indent = 0) : m_Tag(tag), m_Indent(indent) {}

    friend std::ostream& operator<<(std::ostream& ostrm, StartTag const& tag)
    {
        for (unsigned i = 0; i < tag.m_Indent; ++i) {
            ostrm << "\t";
        }
        ostrm << '<' << tag.m_Tag << ' ';
        return ostrm;
    }

private:
    std::string m_Tag;
    unsigned    m_Indent;
};

class EndTag
{
public:
    EndTag(std::string const& tag, unsigned indent = 0) : m_Tag(tag), m_Indent(indent) {}

    friend std::ostream& operator<<(std::ostream& ostrm, EndTag const& tag)
    {
        for (unsigned i = 0; i < tag.m_Indent; ++i) {
            ostrm << "\t";
        }
        ostrm << "</" << tag.m_Tag << ">\n";
        return ostrm;
    }

private:
    std::string m_Tag;
    unsigned    m_Indent;
};

inline std::string EmptyElementEnd()
{
    return "/>\n";
}

template <typename T>
class Attribute
{
public:
    Attribute(std::string const& name, T const& value, std::string const& units = "") : m_Name(name), m_Value(value), m_Units(units) {}
    friend std::ostream& operator<<(std::ostream& ostrm, Attribute const& attr)
    {
        ostrm  << attr.m_Name << "=\"" << std::fixed << std::setprecision(2) << attr.m_Value << attr.m_Units << "\" ";
        return ostrm;
    }
private:
    std::string m_Name;
    T           m_Value;
    std::string m_Units;
};

class ISerializeable
{
public:
    virtual ~ISerializeable() = default;
    virtual void Serialize(std::ostream& ostrm) const = 0;
};

inline std::ostream& operator<<(std::ostream& ostrm, ISerializeable const& object)
{
    object.Serialize(ostrm);
    return ostrm;
}

class Color : public ISerializeable
{
public:
    Color() = default;
    Color(CRgbaColor const& color, std::string const& color_tag, std::string const& opacity_tag) :
        m_Color(color.ToHtmlString()), m_Opacity(color.GetAlpha()), m_ColorTag(color_tag), m_OpacityTag(opacity_tag) {}
    bool IsEmpty() const
    {
        return m_Color.empty();
    }
    virtual void Serialize(std::ostream& ostrm) const
    {
        if (m_Color.empty())
            return;
        ostrm << Attribute<std::string>(m_ColorTag, m_Color);
        if (1.0 == m_Opacity)
            return;
        ostrm << Attribute<float>(m_OpacityTag, m_Opacity);
    }
private:
    std::string m_Color;
    float       m_Opacity;
    std::string m_ColorTag;
    std::string m_OpacityTag;
};

class ReferencePrimitive : public ISerializeable
{
public:
    ReferencePrimitive(std::string const& id) : m_Id(id) {}
    virtual ~ReferencePrimitive() = default;
    virtual void Serialize(std::ostream& ostrm) const
    {
        if (!m_Id.empty())
            ostrm << Attribute<std::string>("id", m_Id);
    }
private:
    std::string m_Id;
};

class Image : public ReferencePrimitive
{
public:
    Image(std::string const& href, std::string const& id = std::string(), CVect2<float> const& origin = { 0.0, 0.0 }, unsigned width = 32, unsigned height = 32) :
        ReferencePrimitive(id), m_Origin(origin), m_Href(href), m_Width(width), m_Height(height) {}
    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << StartTag("image", 3)
            << Attribute<float>("x", m_Origin.X())
            << Attribute<float>("y", m_Origin.Y())
            << Attribute<unsigned>("width", m_Width)
            << Attribute<unsigned>("height", m_Height)
            << Attribute<std::string>("href", m_Href);
        ReferencePrimitive::Serialize(ostrm);
        ostrm << EmptyElementEnd();
    }
private:
    CVect2<float>   m_Origin;
    std::string     m_Href;
    unsigned        m_Width;
    unsigned        m_Height;
};

class Pattern : public ReferencePrimitive
{
public:
    Pattern(std::string const& id, Image const& image, unsigned width = 32, unsigned height = 32, std::string const& pattern_units = string("userSpaceOnUse")) :
        ReferencePrimitive(id), m_Image(image), m_PatternUnits(pattern_units), m_Width(width), m_Height(height) {}

    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << StartTag("pattern", 2)
            << Attribute<std::string>("patternUnits", m_PatternUnits)
            << Attribute<unsigned>("width", m_Width)
            << Attribute<unsigned>("height", m_Height);
        ReferencePrimitive::Serialize(ostrm);
        ostrm << ">\n" << m_Image << EndTag("pattern", 2);

    }
private:
    Image      m_Image;
    std::string m_PatternUnits;
    unsigned    m_Width;
    unsigned    m_Height;
};

class LinearGradient : public ReferencePrimitive
{
public:
    enum EType { eHorizontal, eVertical };
    LinearGradient(std::string const& id, std::vector<CRgbaColor> const& colors, EType type = eVertical) :
        ReferencePrimitive(id), m_Colors(colors), m_Type(type) {}

    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << StartTag("linearGradient", 2);
        ReferencePrimitive::Serialize(ostrm);
        if (eVertical == m_Type) {
            ostrm << Attribute<string>("gradientTransform", "rotate(90)");
        }
        ostrm << ">\n";
        double step = 100.0 / double(m_Colors.size() - 1);
        double offset = 0.0;
        for (auto const& color : m_Colors) {
            ostrm << StartTag("stop", 3)
                << Attribute<int>("offset", int(offset), "%")
                << Color(color, "stop-color", "stop-opacity")
                << EmptyElementEnd();
            offset += step;
        }
        ostrm << EndTag("linearGradient", 2);
    }
private:
    std::vector<CRgbaColor> const& m_Colors;
    EType       m_Type;
};

class ClipRect : public ReferencePrimitive
{
public:
    ClipRect(std::string const& id, int x, int y, unsigned width, unsigned height) : 
        ReferencePrimitive(id), m_X(x), m_Y(y), m_Width(width), m_Height(height) {}
    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << StartTag("clipPath", 2);
        ReferencePrimitive::Serialize(ostrm);
        ostrm << ">\n";
        ostrm << StartTag("rect", 3)
            << Attribute<int>("x", int(m_X))
            << Attribute<int>("y", int(m_Y))
            << Attribute<unsigned>("width", int(m_Width))
            << Attribute<unsigned>("height", int(m_Height))
            << EmptyElementEnd();
        ostrm << EndTag("clipPath", 2);
    }
private:
    int         m_X;
    int         m_Y;
    unsigned    m_Width;
    unsigned    m_Height;
};

class Fill : public ISerializeable
{
public:
    Fill() = default;
    Fill(CRgbaColor const& color) : m_Color(color, "fill", "fill-opacity") { }
    Fill(std::string const& refid) : m_RefId("url('#" + refid + "')") { }
    virtual void Serialize(std::ostream& ostrm) const
    {
        if (!m_RefId.empty()) {
            ostrm << Attribute<std::string>("fill", m_RefId);
        }
        else {
            ostrm << m_Color;
        }
    }
private:
    std::string m_RefId;
    Color       m_Color;
};

class Stroke : public ISerializeable
{
public:
    enum ELineJoinStyle { eMiteredJoin = 0, eRoundedJoin, eBeveledJoin, eDefaultJoin };
    enum ELineCapStyle { eButtCap = 0, eRoundCap, eSquareCap, eDefaultCap };
public:
    Stroke() = default;
    Stroke(CRgbaColor const& color, double width = 0.0, ELineJoinStyle line_join = eDefaultJoin, ELineCapStyle line_cap = eDefaultCap, unsigned pattern = 0xffff, int factor = 0)
        : m_Color(color, "stroke", "stroke-opacity"), m_Width(width), m_LineJoin(line_join), m_LineCap(line_cap), m_Pattern(pattern), m_Factor(factor) { }
    virtual void Serialize(std::ostream& ostrm) const
    {
        if (m_Color.IsEmpty())
            return;
        if (m_Width > 0.0) {
            ostrm << m_Color << Attribute<double>("stroke-width", m_Width);
        }
        if ((eDefaultJoin != m_LineJoin) && (eMiteredJoin != m_LineJoin)) {
            static vector<std::string> linejoin_values = { "miter", "round", "bevel" };
            ostrm << Attribute<std::string>("stroke-linejoin", linejoin_values[m_LineJoin]);
        }
        if ((eDefaultCap != m_LineCap) && (eButtCap != m_LineCap)) {
            static vector<std::string> linecap_values = { "butt", "round", "square" };
            ostrm << Attribute<std::string>("stroke-linecap", linecap_values[m_LineCap]);
        }
        if (0xffff != m_Pattern) {
            ostrm << Attribute<std::string>("stroke-dasharray", x_ParseStipplePattern());
        }
    }
private:
    string x_ParseStipplePattern() const
    {
        int on = 0;
        int off = 0;
        stringstream result;
        auto OutputPattern = [&result, this](int &count) {
            if (count == 0)
                return;
            result << NStr::NumericToString(m_Factor * count) << ' ';
            count = 0;
        };
        for (int j = 15; j >= 0; --j) {
            if (m_Pattern & (1 << j)) {
                OutputPattern(off);
                ++on;
            }
            else {
                OutputPattern(on);
                ++off;
            }
        }
        OutputPattern(on);
        OutputPattern(off);
        
        return result.str();
    }
private:
    Color       m_Color;
    double      m_Width = 1.0;
    ELineJoinStyle  m_LineJoin = eDefaultJoin;
    ELineCapStyle   m_LineCap = eDefaultCap;
    unsigned        m_Pattern = 0xffff;
    int             m_Factor = 0;
};

class Font : public ISerializeable
{
public:
    Font(CGlTextureFont const& font, int align = 0) : m_Family(CGlTextureFont::FaceToString(font.GetFontFace())), m_Size(font.GetFontSize()), m_Align(align) {}
    Font(std::string const &family, unsigned size = 12, int align = 0) : m_Family(family), m_Size(size), m_Align(align) { }
    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << Attribute<std::string>("font-family", m_Family) << Attribute<unsigned>("font-size", m_Size);
        if (m_Align) {
            switch(m_Align & CGlTextureFont::eAlign_HorizMask) {
                case CGlTextureFont::eAlign_Left:
                    ostrm << Attribute<std::string>("text-anchor", "start");
                    break;
                case CGlTextureFont::eAlign_HCenter:
                    ostrm << Attribute<std::string>("text-anchor", "middle");
                    break;
                case CGlTextureFont::eAlign_Right:
                    ostrm << Attribute<std::string>("text-anchor", "end");
                    break;
            }

            /*switch (m_Align & CGlTextureFont::eAlign_VertMask) {
            case CGlTextureFont::eAlign_Top:
                ostrm << Attribute<unsigned>("dy", m_Size);
                break;
            case CGlTextureFont::eAlign_VCenter:
                ostrm << Attribute<double>("dy", m_Size / 2.0);
                break;
            case CGlTextureFont::eAlign_Bottom:
                ostrm << Attribute<unsigned>("dy", 0);
                break;
            }*/
        }
    }
private:
    std::string m_Family;
    unsigned    m_Size;
    int         m_Align;
};

class Primitive : public ISerializeable
{
public:
    Primitive(Fill const &fill = Fill(), Stroke const &stroke = Stroke(), std::string const &clip_path_id = std::string())
        : m_Fill(fill), m_Stroke(stroke) 
    {
        if (!clip_path_id.empty()) {
            m_ClipPathId = "url('#" + clip_path_id + "')";
        }
    }
    virtual void Serialize(std::ostream& ostrm) const
    {
        if (m_ClipPathId.empty())
            return;
        
        ostrm << Attribute<std::string>("clip-path", m_ClipPathId);
    }
protected:
    Fill        m_Fill;
    Stroke      m_Stroke;
    std::string m_ClipPathId;
};

class Circle : public Primitive
{
public:
    Circle(CVect2<float> const& center, double diameter, Fill const & fill, Stroke const & stroke = Stroke(), std::string const &clip_path_id = std::string())
        : Primitive(fill, stroke, clip_path_id), m_Center(center), m_Radius(diameter * 0.5) { }
    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << StartTag("circle", 2)
            << Attribute<float>("cx", m_Center.X())
            << Attribute<float>("cy", m_Center.Y())
            << Attribute<double>("r", m_Radius)
            << m_Fill
            << m_Stroke;
        Primitive::Serialize(ostrm);
        ostrm << EmptyElementEnd();
    }
private:
    CVect2<float>   m_Center;
    double          m_Radius;
};

class Line : public Primitive
{
public:
    Line(CVect2<float> const& start_point, CVect2<float> const& end_point, Stroke const & stroke = Stroke(), std::string const &clip_path_id = std::string())
        : Primitive(Fill(), stroke, clip_path_id), m_StartPoint(start_point), m_EndPoint(end_point) { }
    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << StartTag("line", 2)
            << Attribute<float>("x1", m_StartPoint.X())
            << Attribute<float>("y1", m_StartPoint.Y())
            << Attribute<float>("x2", m_EndPoint.X())
            << Attribute<float>("y2", m_EndPoint.Y())
            << m_Stroke;
        Primitive::Serialize(ostrm);
        ostrm << EmptyElementEnd();
    }
private:
    CVect2<float> m_StartPoint;
    CVect2<float> m_EndPoint;
};

class Polyline : public Primitive
{
public:
    Polyline(vector<CVect2<float>> const& vertices, Fill const& fill = Fill(), Stroke const& stroke = Stroke(), std::string const &clip_path_id = std::string())
        : Primitive(fill, stroke, clip_path_id), m_Vertices(vertices) { }
    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << StartTag("polyline", 2)
            << m_Fill
            << m_Stroke
            << "points=\"";
        for (const auto& vertex : m_Vertices) {
            ostrm << vertex.X() << "," << vertex.Y() << " ";
        }
        ostrm << "\" ";
        Primitive::Serialize(ostrm);
        ostrm << EmptyElementEnd();
    }
private:
    vector<CVect2<float>> m_Vertices;
};

class Polygon : public Primitive
{
public:
    Polygon(vector<CVect2<float>> const& vertices, Fill const& fill = Fill(), Stroke const& stroke = Stroke(), std::string const &clip_path_id = std::string())
        : Primitive(fill, stroke, clip_path_id), m_Vertices(vertices) { }
    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << StartTag("polygon", 2)
            << m_Fill
            << m_Stroke
            << Attribute<string>("shape-rendering", "crispEdges")
            << "points=\"";
        for (const auto& vertex : m_Vertices) {
            ostrm << vertex.X() << "," << vertex.Y() << " ";
        }
        ostrm << "\" ";
        Primitive::Serialize(ostrm);
        ostrm << EmptyElementEnd();
    }
private:
    vector<CVect2<float>> m_Vertices;
};

class Text : public Primitive
{
public:
    Text(CVect2<float> const& origin, std::string const& text, Font const& font, Fill const& fill = Fill(), std::string const &clip_path_id = std::string(), double angle = 0.0)
        : Primitive(fill, Stroke(), clip_path_id), m_Origin(origin), m_Text(text), m_Angle(angle), m_Font(font) { }
    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << StartTag("text", 2)
            << Attribute<float>("x", m_Origin.X())
            << Attribute<float>("y", m_Origin.Y())
            << m_Font;
        if (m_Angle) {
            ostrm << "transform=\"rotate(" << std::fixed << std::setprecision(2) << -m_Angle << ',' << m_Origin.X() << ',' << m_Origin.Y() << ")\" ";
        }
        ostrm << m_Fill
            << m_Stroke;
        Primitive::Serialize(ostrm);
        ostrm << ">" << m_Text << EndTag("text");
    }
private:
    CVect2<float>   m_Origin;
    std::string     m_Text;
    double          m_Angle;
    Font            m_Font;
};

class ViewBox : public ISerializeable
{
public:
    ViewBox(int x = 0, int y = 0, unsigned width = 100, unsigned height = 100) : m_X(x), m_Y(y), m_Width(width), m_Height(height) {}
    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << "viewBox=\"" << m_X << ' ' << m_Y << ' ' << m_Width << ' ' << m_Height << "\" ";
    }
    inline int X() const { return m_X; };
    inline int Y() const { return m_Y; };
    inline unsigned Width() const { return m_Width; };
    inline unsigned Height() const { return m_Height; };
private:
    int         m_X;
    int         m_Y;
    unsigned    m_Width;
    unsigned    m_Height;
};

class Style : public ISerializeable
{
public:
    Style(std::string const& style_text = "") : m_Text(style_text) {}
    virtual void Serialize(std::ostream& ostrm) const
    {
        if (m_Text.empty()) {
            return;
        }
        ostrm << StartTag("style", 1)
            << Attribute<std::string>("type", "text/css")
            << ">"
            << m_Text
            << EndTag("style", 1);
    }
private:
    std::string m_Text;
};

class SVG : public Primitive
{
public:
    SVG(ViewBox viewbox, SVG *parent) : m_ViewBox(viewbox), m_Parent(parent) {}
    SVG(ViewBox viewbox, Style style = Style(), std::string const& description = "") : m_ViewBox(viewbox), m_Style(style), m_Description(description) {}

    SVG& operator<<(ReferencePrimitive const& primitive)
    {
        if (nullptr != m_Parent) {
            m_Parent->operator<<(primitive);
            return *this;
        }

        m_Definitions << primitive;
        m_Definitions.flush();
        return *this;
    }

    SVG& operator<<(Primitive const& primitive)
    {
        m_Primitives << primitive;
        m_Primitives.flush();
        return *this;
    }

    virtual void Serialize(std::ostream& ostrm) const
    {
        ostrm << StartTag("svg", 1)
            << Attribute<int>("x", m_ViewBox.X())
            << Attribute<int>("y", m_ViewBox.Y())
            << Attribute<unsigned>("width", m_ViewBox.Width(), "px")
            << Attribute<unsigned>("height", m_ViewBox.Height(), "px")
            << Attribute<std::string>("preserveAspectRatio", "xMinYMin slice")
            << ViewBox(0, 0, m_ViewBox.Width(), m_ViewBox.Height())
            << ">\n"
            << m_Primitives.str()
            << EndTag("svg", 1);
    }

    void WriteToStream(std::ostream& ostr) const
    {
        ostr << "<?xml "
            << Attribute<std::string>("version", "1.0")
            << Attribute<std::string>("standalone", "no")
            << "?>\n<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
            << "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
            << StartTag("svg")
            << Attribute<unsigned>("width", m_ViewBox.Width(), "px")
            << Attribute<unsigned>("height", m_ViewBox.Height(), "px")
            << Attribute<std::string>("xmlns", "http://www.w3.org/2000/svg")
            << Attribute<std::string>("version", "1.1")
            << m_ViewBox
            << ">\n"
            << m_Style;
        if (!m_Description.empty()) {
            ostr << StartTag("desc", 1) << '>'
                << m_Description
                << EndTag("desc");
        }
        ostr << StartTag("defs", 1) << ">\n"
            << m_Definitions.str()
            << EndTag("defs", 1)
            << StartTag("g", 1) << ">\n"
            << m_Primitives.str()
            << EndTag("g", 1)
            << EndTag("svg");
    }
private:
    std::stringstream m_Definitions;
    std::stringstream m_Primitives;
    ViewBox           m_ViewBox;
    Style             m_Style;
    std::string       m_Description;
    SVG               *m_Parent = nullptr;
};

END_SCOPE(svg)

END_NCBI_SCOPE

#endif // GUI_UTILS__SVG_HPP
