/*  $Id: phylo_pdf_grabber.cpp 32681 2015-04-10 15:14:16Z falkrb $
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
#include <gui/widgets/phylo_tree/phylo_pdf_grabber.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_pane.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/print/glrenderpdf.hpp>
#include <gui/print/pdf.hpp>
#include <gui/print/print_media.hpp>

#include <util/image/image.hpp>

#include <corelib/ncbifile.hpp>



BEGIN_NCBI_SCOPE

CPhyloPdfGrabber::CPhyloPdfGrabber(CPhyloTreePane& pane) 
: m_Pane(pane) 
, m_SaveTooltips(false)
{
    m_PreviousGlPane = m_Pane.GetPane();
}


IImageGrabber::eCaptureResult 
CPhyloPdfGrabber::GrabImages(int tex_size, IImageGrabberProgress* progress)
{
    // progress was more for the tiled image grabber so you could see images as they were genereated
    // on the dialog.  For PDF we wuld just use the pane to set the context.
    if (progress != NULL)
        progress->SetGLContext();
    else 
        m_Pane.SetContext();

    x_BeginCaptures(tex_size);

    string pdfname = CDir::ConcatPath(m_Directory, m_BaseImageName + "." + m_ImageFormat);
    CNcbiOfstream  pstrm(pdfname.c_str(), ios::out | ios::binary);    
    m_Pdf->SetOutputStream(&pstrm);    


    //***************************************************

    m_Pdf->BeginDocument();
    m_Pdf->BeginPage();
    m_Pane.RenderPdf(m_Pdf, m_SaveTooltips);
    m_Pdf->EndPage();

    m_Pdf->EndDocument();
    x_EndCaptures();

    // Show on dialog widget that the image has been saved
    if (progress != NULL)
        progress->ImageSaved(0,0);

    m_Pdf.ReleaseOrNull();

    return eSuccess;
}

void CPhyloPdfGrabber::x_BeginCaptures(int /*buffer_size*/)
{
    // Save some current rendering values we want to change
    CPhyloTreeScheme& s = m_Pane.GetCurrRenderer()->GetScheme();
    m_LabelsTruncated = s.GetLabelTruncation();
    s.SetLabelTruncation(CPhyloTreeScheme::eLabelsFull);
    
    int viewport_x, viewport_y;

    m_Pdf.Reset(new CPdf());   

    CPrintOptions po;
    po.SetPageOrientation(CPrintOptions::ePortrait);
    string pdfname = CDir::ConcatPath(m_Directory, m_BaseImageName + "." + m_ImageFormat);
    po.SetFilename(pdfname.c_str());

    // Set margins for pdf to 0 since our own rendered image will have its own margins
    // as specified in CPhyloTreeScheme.
    po.SetMarginBottom(CPrintOptions::TPdfUnit(0));
    po.SetMarginLeft(CPrintOptions::TPdfUnit(0));
    po.SetMarginRight(CPrintOptions::TPdfUnit(0));
    po.SetMarginTop(CPrintOptions::TPdfUnit(0));
    po.SetPagesWide(1);
    po.SetPagesTall(1);

    CGlRect< TModelUnit> full_rect = m_Pane.GetPane().GetModelLimitsRect();
    CGlRect< TModelUnit> vis_rect = m_Pane.GetPane().GetVisibleRect();
    TVPRect viewport = m_Pane.GetPane().GetViewport();

    // Use the ratio of visible area to total area to determine
    // the level of zoom, then expand the viewport to accomodate
    // the total view
    double ratiox = vis_rect.Width()/full_rect.Width();
    double ratioy = vis_rect.Height()/full_rect.Height();
    viewport_x = (1.0/ratiox)*double(viewport.Width());
    viewport_y = (1.0/ratioy)*double(viewport.Height());

    float maxdim = std::max(viewport_x, viewport_y);
    if (maxdim > 14000.0f) {
        float scaler = 14000.0f/maxdim;
        float user_unit = 1.0f/scaler;
        po.SetUserUnit(user_unit);
        m_Pane.GetPane().SetOutputUnitScaler(user_unit);

        viewport_x *= scaler;
        viewport_x = std::max(viewport_x, 100);
        
        viewport_y *= scaler;
        viewport_y = std::max(viewport_y, 100);
    }

    CMedia m(m_OutputFormat, viewport_x, viewport_y, CUnit::ePdfUnit);
    po.SetMedia(m);

    CGlRect<int> vp(0, 0, viewport_x, viewport_y);
    m_Pane.GetPane().SetViewport(vp);

    po.SetOutputFormat(CPrintOptions::ePdf);
    po.SetPagesTall(1);
    po.SetPagesWide(1);
    m_Pdf->SetOptions(po);

    m_Pane.SetupHardcopyRender();
}

void CPhyloPdfGrabber::x_EndCaptures()
{
    m_Pane.GetPane() = m_PreviousGlPane;
    m_Pane.GetCurrRenderer()->GetScheme().SetLabelTruncation(m_LabelsTruncated);    
}



END_NCBI_SCOPE
