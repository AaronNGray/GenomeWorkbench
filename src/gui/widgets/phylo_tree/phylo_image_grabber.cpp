/*  $Id: phylo_image_grabber.cpp 33050 2015-05-19 16:06:18Z falkrb $
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

#include <ncbi_pch.hpp>
#include <gui/widgets/phylo_tree/phylo_image_grabber.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_pane.hpp>

#include <util/image/image.hpp>



BEGIN_NCBI_SCOPE

CPhyloImageGrabber::CPhyloImageGrabber(CPhyloTreePane& pane) 
: m_Pane(pane) 
, m_TileViewportWidth(0)
, m_TileViewportHeight(0)
{
    m_PreviousGlPane = m_Pane.GetPane();
}

void CPhyloImageGrabber::x_BeginCaptures(int buffer_size)
{
    // Save some current rendering values we want to change
    CPhyloTreeScheme& s = m_Pane.GetCurrRenderer()->GetScheme();
    m_LabelsTruncated = s.GetLabelTruncation();
    s.SetLabelTruncation(CPhyloTreeScheme::eLabelsFull);
    s.GetMargins(m_MarginLeft, m_MarginTop, m_MarginRight, m_MarginBottom);

    // Update rendering options. It would be better if these margins explicitly
    // included any boundary widths from subtree boundaries that are included
    // in the graph.  But normally those boundaries will be pretty narrow and
    // this should handle it.
    s.SetMargins(20.0, 20.0, 20.0, 20.0);

    m_ModelViewScaler = 1.0f;
    m_GuideWidthX = 0;
    m_GuideWidthY = 0;
    
    int viewport_x, viewport_y;

    // If the user doesn't ask for a specific aspect ratio, pick an optimal one.
    if (m_TileAspectRatio < 0.0f) {
        float partitions_ratio = ((float)(m_ImagePartitions.Y()+1))/
                                  (float)(m_ImagePartitions.X()+1);
        float tile_aspect = m_ImageAspectRatio*partitions_ratio;
    
        // Set the size of the target-image to which we will render.  
        // If we are not scaling in x or y, ignore aspect ratio (tile into squre images)
        if (s.GetZoomBehavior( m_Pane.GetCurrRenderer()->GetDescription()) != CPhyloTreeScheme::eZoomXY) {
            m_TileViewportWidth = buffer_size;
            m_TileViewportHeight = buffer_size;
        }
        else {
            if (tile_aspect > 1.0f) {        
                m_TileViewportWidth = buffer_size;
                m_TileViewportHeight = (int)(((float)m_TileViewportWidth)/tile_aspect);
            }
            else {
                m_TileViewportHeight = buffer_size;
                m_TileViewportWidth = (int)(((float)m_TileViewportHeight)*tile_aspect);
            }
        }

        if (m_GuidesEnabled) {
            m_GuideWidthX = m_GuideWidth;
            m_GuideWidthY = m_GuideWidth;
            m_TileViewportWidth -= 2 * m_GuideWidthX;
            m_TileViewportHeight -= 2 * m_GuideWidthY;
        }

        // Set the size of the full viewport (sum of all tiles).
        viewport_x = m_TileViewportWidth * (m_ImagePartitions.X()+1);
        viewport_y = m_TileViewportHeight * (m_ImagePartitions.Y()+1);
    }
    else {
        // Since tiles have a fixed ratio in this section of code,
        // the partitioned aspect is the ratio of the tiles summed
        // together (ratio of tiled window)
        float partition_ratio = ((float)(m_ImagePartitions.X()+1))/
                                 (float)(m_ImagePartitions.Y()+1);
        float partitioned_aspect = m_TileAspectRatio*partition_ratio;
       
        // Get the size of the individual tiles (this is the size of the
        // target image to be saved). One dimension is the target image
        // size, and the other (smaller) dimension is sized according to 
        // aspect ratio.
        if (m_TileAspectRatio > 1.0f) {
            m_TileViewportWidth = buffer_size;
            m_TileViewportHeight = (int)(((float)buffer_size) * 1.0f/m_TileAspectRatio);

            // Subtract space, if requested, for print guidelines. to keep imag
            // proportional, we will subtract the base amount from the shorter 
            // dimension and a proportional (larger) amount from the long dimension.
            if (m_GuidesEnabled) {
                m_GuideWidthX = (int)(((float)m_GuideWidth)*m_TileAspectRatio);
                m_GuideWidthY = m_GuideWidth;
            }
        }
        else {
            m_TileViewportHeight = buffer_size;
            m_TileViewportWidth = (int)(((float)buffer_size) * m_TileAspectRatio);

            // Subtract space, if requested, for print guidelines. to keep imag
            // proportional, we will subtract the base amount from the shorter 
            // dimension and a proportional (larger) amount from the long dimension.
            if (m_GuidesEnabled) {
                m_GuideWidthX = m_GuideWidth;
                m_GuideWidthY = (int)(((float)m_GuideWidth)*(1.0f/m_TileAspectRatio));
            }
        }

        if (m_GuidesEnabled) {
            m_TileViewportWidth -= 2 * m_GuideWidthX;
            m_TileViewportHeight -= 2 * m_GuideWidthY;
        }

        // Get size of viewport containing all tiles (sum of tiles)
        viewport_y = m_TileViewportHeight * (m_ImagePartitions.Y()+1);
        viewport_x = m_TileViewportWidth * (m_ImagePartitions.X()+1);        

        // for a fixed aspect ratio, one of the dimensions is going
        // to be longer than it needs to be (will have blank space
        // at the end).  The scaler is the ratio of the actual (extended)
        // viewport size to the size it would be if it matched the aspect
        // ratio of the on-screen tree image.

        // If we are not scaling in x or y, though, just tile the images evenly (given the fixed per-image
        // aspect ration) along the x or y direction (so do nothing here in that case)
        if (s.GetZoomBehavior( m_Pane.GetCurrRenderer()->GetDescription()) == CPhyloTreeScheme::eZoomXY) {      
            if (partitioned_aspect > m_ImageAspectRatio) {            
                m_ModelViewScaler.X() = ((float)viewport_x)/
                    (((float)m_ImageAspectRatio)*(float)viewport_y);
                viewport_x = (int)(m_ImageAspectRatio*(float)viewport_y);
            }
            else {       
                m_ModelViewScaler.Y() = ((float)viewport_y)/
                    (((float)viewport_x)/m_ImageAspectRatio);
                viewport_y = (int)(((float)viewport_x)/m_ImageAspectRatio);
            }
        }
    }   

    // We need the correct (sum of all tiles) viewport size in order
    // to correctly compute the size of the model limits rect 
    CGlRect<int> vp(0, 0, viewport_x, viewport_y);
    m_Pane.GetPane().SetViewport(vp);

    // Compute the size of the model limits rectangle
    m_Pane.SetupHardcopyRender();
}

void CPhyloImageGrabber::x_EndCaptures()
{
    m_Pane.GetPane() = m_PreviousGlPane;

    /// Reset rendering options we may have updated.
    CPhyloTreeScheme& s = m_Pane.GetCurrRenderer()->GetScheme();
    s.SetLabelTruncation(m_LabelsTruncated);
    s.SetMargins(m_MarginLeft, m_MarginTop, m_MarginRight, m_MarginBottom);
}

CPhyloImageGrabber::eCaptureResult 
CPhyloImageGrabber::x_CaptureImage(CVect2<int> partitions, 
                                   CVect2<int> index,
                                   CVect2<size_t>& capture_size)
{
    CGlRect< TModelUnit> pane_rect, full_rect;
    
    full_rect = m_Pane.GetPane().GetModelLimitsRect();

    int vp_width;
    int vp_height;

    double width, height, left, bottom;

    vp_width = m_TileViewportWidth;
    vp_height = m_TileViewportHeight;

    width = ceil(((double)full_rect.Width())/(double)(partitions.X()+1)) *
            m_ModelViewScaler.X();
    height = ceil(((double)full_rect.Height())/(double)(partitions.Y()+1)) *
            m_ModelViewScaler.Y();


    // We add one pixel-equivalent in each direction here
    // to ensure overlap.
    double one_pix_x = width/(double)vp_width;
    double one_pix_y = height/(double)vp_height;

    left =  full_rect.Left() + ((double)(index.X()))*width;         
    bottom = full_rect.Top() - (index.Y() + 1)*height;
    pane_rect.Init(left - one_pix_x, 
                   bottom - one_pix_y, 
                   left + width + one_pix_x, 
                   bottom + height + one_pix_y);

    // Set viewport size to image size and render image
    CGlRect<int> vp(m_GuideWidthX,
                    m_GuideWidthY,
                    vp_width + m_GuideWidthX,
                    vp_height + m_GuideWidthY);

    m_Pane.GetPane().SetViewport(vp);
    m_Pane.GetPane().SetVisibleRect(pane_rect);

    // Only render the scale marker on last image of tree.  Determine
    // this using index and number of partitions.
    bool b = m_Pane.GetCurrRenderer()->GetRenderScale();
    if (index.Y() == partitions.Y() && index.X() == partitions.X())
        m_Pane.GetCurrRenderer()->SetRenderScale(true);
    else 
        m_Pane.GetCurrRenderer()->SetRenderScale(false);

    m_Pane.RenderHardcopy();

    // Don't change renderstate for tree object.
    m_Pane.GetCurrRenderer()->SetRenderScale(b);

    capture_size.X() = vp_width + 2*m_GuideWidthX;
    capture_size.Y() = vp_height + 2*m_GuideWidthY;

    return eSuccess;
}


END_NCBI_SCOPE
