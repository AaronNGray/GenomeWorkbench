#ifndef GUI_WIDGETS_WX___PREVIEW_SETUP_WIDGET__HPP
#define GUI_WIDGETS_WX___PREVIEW_SETUP_WIDGET__HPP

/*  $Id: preview_setup_widget.hpp 33050 2015-05-19 16:06:18Z falkrb $
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
 * CGlPreviewSetupWidget is a display widget for an image that allows the
 * the image to be shown with overlapping tiles of either fixed or
 * dynamic size ratio.  The widget also will write (image) names on 
 * the overlapping tiles with an associated numbering scheme.
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/widgets/wx/glcanvas.hpp>
#include <gui/widgets/wx/image_grabber.hpp>
#include <gui/utils/vect2.hpp>

#include <wx/panel.h>


BEGIN_NCBI_SCOPE

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_TILE_PREVIEW, 1)
END_DECLARE_EVENT_TYPES()


class CImage;
class CGlTexture;

class NCBI_GUIWIDGETS_WX_EXPORT CGlPreviewSetupWidget 
    : public CGLCanvas
    , public IImageGrabberProgress
{
    DECLARE_EVENT_TABLE();

public:
    CGlPreviewSetupWidget(CRef<CImage> img,
                          wxWindow* parent,
                          wxWindowID id,
                          const wxPoint& pos = wxDefaultPosition,
                          const wxSize& size = wxDefaultSize,
                          long style = 0,
                          int *attribList = NULL);
    virtual ~CGlPreviewSetupWidget();

    /// Set the number of partitions for display on the widget
    void SetPartitions(int p);   

    /// Return the number of partitions of the image in x and y
    CVect2<int> GetPartitions() const 
        { return CVect2<int>(m_PartitionsX, m_PartitionsY);}

    /// Get width and height of image
    int GetImageWidth() const;
    int GetImageHeight() const;

    /// Set/Get desired width/height ratio for image when it is subdivided
    void SetAspectRatio(float r);
    float GetAspectRatio() const { return m_AspectRatio; }

    /// Set numbering format for image names, e.g. img{x_y} vs img{1..n}
    void SetNumberingFormat(IImageGrabber::eImageNumberingFormat fmt);

    /// Set the name to assign to the image(s).  This allows a user to see
    /// the file names that will be used for the saved images
    void SetImageBaseName(const std::string& n)  { m_BaseImageName = n; }

    /// Set to true to rotate the image 90 (to better match paper size)
    void SetRotated(bool b) { m_Rotated = b; }
    bool GetRotated() const { return m_Rotated; }

    /// Enable/disable zoom in x && y
    void SetZoomBehavior(bool zoomx, bool zoomy);
    bool GetZoomX() const { return m_ZoomX; }
    bool GetZoomY() const { return m_ZoomY; }

    /// Updated number of images saved and refresh the image
    void RefreshImage(int x_idx, int y_idx);

    /// @name IImageGrabberProgress implementation
    /// @{
    /// Set GL context to this window
    virtual void SetGLContext();

    /// Implement interface for IImageGrabberProgress which shows user
    /// interactive progress in saving (forces immediate redraw)
    virtual void ImageSaved(int x_idx, int y_idx);

    /// Set an image to be displayed in the upper-left of the window.
    /// Image should be a single tile from the composite
    virtual void SetPreviewSubImage(CRef<CImage> img);

    /// Update reference image (allows image to reflect level of zoom based
    /// on curreint tiling).
    virtual void SetReferenceImage(CGlTexture* refimg);

    /// Get locations of individual tiles so that they can be updated
    vector<CTileOrigin> GetTileLocations(int& w, int& h) 
        { w=m_RectWidth; h=m_RectHeight; return m_TileOrigins; }
    /// @}

    virtual void OnMouseDown(wxMouseEvent& event);
    virtual void OnMouseDoubleClick(wxMouseEvent& event);
    
protected:


    /// Draw the image, its overlapping tiling, and text (image named, saved
    /// status) for those tiles.
    void x_Render();

    /// Image to display in setup widget
    CGlTexture*  m_ReferenceImage;

    /// Size of reference image initially passed to widget
    CVect2<int> m_RefImageSize;

    /// Expanded image of a single tile of the composite image
    CGlTexture*  m_ReferenceSubImage;

    /// If true, show the image rotated by 90 degrees
    bool m_Rotated;

    /// When subdividing image, the desired ratio of width to height (w/h)
    float m_AspectRatio;

    /// Base name of image for display in (sub)image windows
    std::string m_BaseImageName;

    /// Numbering format for saved images
    IImageGrabber::eImageNumberingFormat m_NumberingFormat;

    /// The number of subdivisions of the image along the x and y axes
    int m_PartitionsX;
    int m_PartitionsY;

    /// The current lower-left hand corners of all the tiles (as of last draw)
    vector<CTileOrigin> m_TileOrigins;
    /// The width and height of the current tiles (as displayed on this widget)
    float m_RectWidth, m_RectHeight;

    /// The number of images saved so far, indexed by row and column.  
    /// Allows us to render saved tiles differently from unsaved.
    int m_SavedX;
    int m_SavedY;

    /// We may only allow zoom in X or Y so these are true if zoom applies to that direction
    bool m_ZoomX;
    bool m_ZoomY;
};

END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___PREVIEW_SETUP_WIDGET__HPP

