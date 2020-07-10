#ifndef GUI_WIDGETS_WX___IMAGE_GRABBER__HPP
#define GUI_WIDGETS_WX___IMAGE_GRABBER__HPP

/*  $Id: image_grabber.hpp 44930 2020-04-21 17:07:30Z evgeniev $
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
 * Authors: 
 *
 * File Description:
 *   
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <util/image/image.hpp>
#include <gui/utils/vect2.hpp>
#include <gui/opengl/glrect.hpp>
#include <gui/opengl/gltypes.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/print/print_options.hpp>

#include <gui/opengl.h>

BEGIN_NCBI_SCOPE

class CImage;
class CGlTexture;

///////////////////////////////////////////////////////////////////////////////
/// Interface class for the pdf renderer
///
class NCBI_GUIWIDGETS_WX_EXPORT IVectorGraphicsRenderer
{
public:
    virtual ~IVectorGraphicsRenderer() {}
    virtual const TVPRect& GetViewportRect() const = 0;
    virtual void RenderVectorGraphics(int vp_width, int vp_height) = 0;
    virtual void UpdateVectorLayout() = 0;
    virtual void AddTitle(bool b) = 0;
};

class NCBI_GUIWIDGETS_WX_EXPORT IImageGrabberProgress
{
public:
    struct CTileOrigin {
        CTileOrigin() : m_PixelOrigin(0.0f,0.0f), m_TileIndex(0,0) {}
        CTileOrigin(const CVect2<float>&o, const CVect2<int> idx) 
            : m_PixelOrigin(o)
            , m_TileIndex(idx) {}

        CVect2<float> m_PixelOrigin;
        CVect2<int> m_TileIndex;
    };

public:
    IImageGrabberProgress() {}
    virtual ~IImageGrabberProgress() {}

    virtual void SetGLContext() {}
    virtual void ImageSaved(int /*x_idx*/, int /*y_idx*/) {}

    /// Return texture so that the preview image can be updated. 
    virtual void SetReferenceImage(CGlTexture* refimg) {}

    /// Set an image to be displayed in the center of the widget.
    /// Image should be a single tile from the composite
    virtual void SetPreviewSubImage(CRef<CImage> /*img*/) {}

    /// Get locations (origin and size) of individual image tiles.
    virtual vector<CTileOrigin> GetTileLocations(int& w, int& h) 
        { return vector<CTileOrigin>(); }
};

class  NCBI_GUIWIDGETS_WX_EXPORT IImageGrabber
{ 
public:
    enum eCaptureResult { eSuccess=0, eOpenGLError, eFileError, eError };

    enum eImageNumberingFormat {
        eSequentialFormat,
        eCartesionFormat
    };


public:
    IImageGrabber();
    virtual ~IImageGrabber();

    /// Grab the requested images
    virtual eCaptureResult GrabImages(int tex_size, IImageGrabberProgress* p = NULL);

    /// Grab a single image for preview purposes
    eCaptureResult GrabImage(int tex_size, 
                             CVect2<int> img_idx, 
                             IImageGrabberProgress* p = NULL);

    /// Grab images to update the on-screen image to approximate final output
    eCaptureResult PreviewImages(int tex_size, 
                                 int ref_img_width, int ref_img_height,
                                 IImageGrabberProgress* p = NULL);

    /// Set number of desired image partitions
    void SetPartitions(const CVect2<int>& p) {m_ImagePartitions = p;}
    /// Set file output information
    void SetOutputInfo(const std::string& dir,
                       const std::string& base_name,
                       const std::string& img_format);
    /// Set the numbering format for sequential image names
    void SetNumberingFormat(eImageNumberingFormat fmt) {m_NumberingFormat = fmt;}
    /// Set aspect ratio tiles captured
    void SetTileAspectRatio(float ar) { m_TileAspectRatio = ar; }
    /// Set aspect ratio for image as a whole
    void SetImageAspectRatio(float ar) { m_ImageAspectRatio = ar; }
    /// Set output format on which image aspect is based, e.g. A4, US Letter..
    void SetOutputFormat(const string& f) { m_OutputFormat = f; }
    /// Set to true to put cutting/printing guides on output images
    void SetPrintingGuidesEnabled(bool b) { m_GuidesEnabled = b; }
    /// Set to true force all triangles to be printed flat (no shadingtype 4)
    /// This makes the file compatible with Adobe Illustrator
    void DisableGouraudShading(bool b) { m_DisableGouraudShaded = b; }

    /// Set size for optional pdf title.
    void SetTitleHeight(TModelUnit h) { m_TitleHeight=h; }

    virtual void SetOutputStream(CNcbiOstream* ostr) { };
protected:

    /// Render guides around image edge (cut marks + adjacent image names)
    void x_RenderGuides(CVect2<size_t> capture_size,
                        const std::string& image_name,
                        std::string image_bottom,
                        std::string image_right,
                        std::string image_top,
                        std::string image_left);

    /// Return image filename for a tile index based on the numbering scheme
    std::string x_GetImageName(int x, int y);

    /// Called before first image captured
    virtual void x_BeginCaptures(int /*buffer_size*/) {}
    /// Called after last image captured
    virtual void x_EndCaptures() {}
    /// Must be subclassed to capture an individual image
    virtual eCaptureResult x_CaptureImage(CVect2<int> partitions, 
                                          CVect2<int> index,
                                          CVect2<size_t>& capture_size)
    { return eSuccess; }

    /// Number of partitions (tilings) in x and 1.  If x and y are 0,
    /// then only a single image is saved.
    CVect2<int> m_ImagePartitions;
    /// Target directory for saved images
    std::string m_Directory;  
    /// Image name without its appended number (e.g. img for img1, img2...)
    std::string m_BaseImageName;
    /// Image output format (e.g. jpeg, png...)
    std::string m_ImageFormat;
    /// Numbering format, such as sequential or tiled (x and y)
    eImageNumberingFormat m_NumberingFormat;
    /// If true, printing guides (like where to cut paper) will be added to
    /// image edges
    bool m_GuidesEnabled;
    /// Width of printing guides, if enabled
    static const int m_GuideWidth = 24;
    /// If we are making space for printing guides (margins) on the sides,
    /// those margins may have different sizes (in x and y) if we are 
    /// forcing a proportional output size.  These are those sizes 
    /// (proportional or not)
    int m_GuideWidthX;
    int m_GuideWidthY;


    /// Ratio of width to height (w/h) in captured images. If the ratio is < 0, 
    /// then we subdivide the image evenly.
    float m_TileAspectRatio;
    /// Name of output format, e.g. A4, US Letter..
    string m_OutputFormat;

    /// Aspect ratio of image from which we are generating tiles.
    float m_ImageAspectRatio;

    /// If true, disable gouraud-shaded polys (shadingtype 4)
    bool m_DisableGouraudShaded;

    /// Allow space at the top for a title
    TModelUnit m_TitleHeight = TModelUnit(0);

    CIRef<IRender>  m_SavedRender;
};

class  NCBI_GUIWIDGETS_WX_EXPORT CImageGrabberFactory
{
public:
    using EOutputFormat = CPrintOptions::EOutputFormat;

    static IImageGrabber* CreateImageGrabber(EOutputFormat format, IVectorGraphicsRenderer& pane);

protected:
    CImageGrabberFactory() = default;
    CImageGrabberFactory(const CImageGrabberFactory&) = default;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___IMAGE_GRABBER__HPP

