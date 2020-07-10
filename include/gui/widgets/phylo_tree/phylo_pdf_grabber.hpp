#ifndef GUI_WIDGETS_PHYLO_TREE__PHYLO_PDF_GRABBER__HPP
#define GUI_WIDGETS_PHYLO_TREE__PHYLO_PDF_GRABBER__HPP

/*  $Id: phylo_pdf_grabber.hpp 29699 2014-01-30 19:10:19Z falkrb $
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
 * Authors: Bob Falk
 *
 * File Description:
 * 
 * This is the subclass of IImageGrabber specialized to work with phylogenetic 
 * tree objects and to save those objects in pdf format.
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/widgets/wx/image_grabber.hpp>

#include <util/image/image.hpp>
#include <gui/utils/vect2.hpp>
#include <gui/opengl/glrect.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/print/pdf.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>

#include <corelib/ncbiobj.hpp>

BEGIN_NCBI_SCOPE

class CImage;
class CPhyloTreePane;

class CPhyloPdfGrabber : public IImageGrabber
{ 
public:
    CPhyloPdfGrabber(CPhyloTreePane& pane);

    virtual ~CPhyloPdfGrabber() {}

    virtual eCaptureResult GrabImages(int tex_size, 
                                      IImageGrabberProgress* p = NULL);

    void SaveTooltips(bool b) { m_SaveTooltips=b; }

protected:
    /// Called to setup values before image capturing starts.
    void x_BeginCaptures(int buffer_size);
    /// Called to restore values (in pane) when capturing stops.
    void x_EndCaptures();

    /// Pane display canvas for the tree we are saving
    CPhyloTreePane&  m_Pane;

    /// We change some rendering options.  These are the previous
    /// values used in m_Pane so we can set them back when were done.
    CGlPane  m_PreviousGlPane;
    CPhyloTreeScheme::TLabelTruncation  m_LabelsTruncated;
    double m_MarginLeft, m_MarginTop, m_MarginRight, m_MarginBottom;

    CRef<CPdf>  m_Pdf;

    /// Ratio scalers for the drawing area that maps to the images. 
    /// These are both 1.0 unless we are forcing the individual images
    /// to a fixed proportion in which case the scale the underlying
    /// image extent accordingly.
    CVect2<float> m_ModelViewScaler;

    /// If true, tooltips will be generated for the pdf as well
    bool m_SaveTooltips;
};


END_NCBI_SCOPE

#endif // GUI_WIDGETS_PHYLO_TREE__PHYLO_PDF_GRABBER__HPP