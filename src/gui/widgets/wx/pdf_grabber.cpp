/* 
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
#include <gui/widgets/wx/pdf_grabber.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/print/glrenderpdf.hpp>
#include <gui/print/pdf.hpp>
#include <gui/print/print_media.hpp>

#include <util/image/image.hpp>

#include <corelib/ncbifile.hpp>

BEGIN_NCBI_SCOPE

CPdfGrabber::CPdfGrabber(IVectorGraphicsRenderer& pane) 
  : m_Pane(pane) 
  , m_OutputStream(NULL)
{
}


void CPdfGrabber::SetOutputStream(CNcbiOstream* ostr)
{
    if (ostr) {
        m_OutputStream = ostr;
    }
}

IImageGrabber::eCaptureResult 
CPdfGrabber::GrabImages(int tex_size, IImageGrabberProgress* progress)
{
    if (progress != NULL)
        progress->SetGLContext();

    x_BeginCaptures(tex_size);

    x_InitRenderer();
    m_Pane.UpdateVectorLayout();

    auto_ptr<CNcbiOstream> pstrm;
    if ( !m_OutputStream ) {
        string pdfname = CDir::ConcatPath(m_Directory, m_BaseImageName + "." + m_ImageFormat);
        pstrm.reset(new CNcbiOfstream(pdfname.c_str(), ios::out | ios::binary));
        m_OutputStream = pstrm.get();
    }
    m_Pdf->SetOutputStream(m_OutputStream);  


    // Set the size of the rendered area (full viewport)
    CMedia m("", 
        m_Pane.GetViewportRect().Width(), 
        m_Pane.GetViewportRect().Height() + m_TitleHeight,
        CUnit::ePdfUnit);
    
    // to have margins we have to pass in a viewport that fits within them
    // (e.g. 20,20,20,20 = vp(20, 20, width-40, height-40) but for now
    // we are using the screens viewport size only
    m_Pdf->GetOptions().SetMarginBottom(0);
    m_Pdf->GetOptions().SetMarginLeft(0);
    m_Pdf->GetOptions().SetMarginRight(0);
    m_Pdf->GetOptions().SetMarginTop(0);

    m_Pdf->GetOptions().SetMedia(m);

    m_Pdf->GetOptions().DisableGouraudShading(m_DisableGouraudShaded);


    //***************************************************

    m_Pdf->BeginDocument();
    m_Pdf->BeginPage();
    if (m_TitleHeight>0.0f) 
        m_Pane.AddTitle(true);

    m_Pane.RenderVectorGraphics((int)m_Pdf->GetOptions().GetMedia().GetWidth(), (int)m_Pdf->GetOptions().GetMedia().GetHeight());
    m_Pdf->EndPage();

    m_Pdf->EndDocument();
    x_EndCaptures();

    m_Pdf.ReleaseOrNull();

    return eSuccess;
}

void CPdfGrabber::x_BeginCaptures(int /*buffer_size*/)
{
    m_ModelViewScaler = 1.0f;
    m_GuideWidthX = 0;
    m_GuideWidthY = 0;
   

    m_Pdf.Reset(new CPdf());   

    CPrintOptions po;
    po.SetPageOrientation(CPrintOptions::ePortrait);
    string pdfname = CDir::ConcatPath(m_Directory, m_BaseImageName + "." + m_ImageFormat);
    po.SetFilename(pdfname.c_str());
    po.SetMarginBottom(20);
    po.SetMarginLeft(20);
    po.SetMarginRight(20);
    po.SetMarginTop(20);
    po.SetPagesWide(1);
    po.SetPagesTall(1);

    po.SetOutputFormat(CPrintOptions::ePdf);
    m_Pdf->SetOptions(po);
}

void CPdfGrabber::x_EndCaptures()
{
}

void CPdfGrabber::x_InitRenderer()
{
    CIRef<IRender>  mgr = CGlResMgr::Instance().GetRenderer(eRenderPDF);
    CGlRenderPdf* rpdf = NULL;
    if (mgr.IsNull()) {
        rpdf = new CGlRenderPdf();
        mgr.Reset(rpdf);
        CGlResMgr::Instance().AddRenderer(mgr);
        rpdf->SetPdf(m_Pdf);
    } else {
        rpdf = dynamic_cast<CGlRenderPdf*>(mgr.GetPointerOrNull());
        _ASSERT(rpdf);
        rpdf->SetPdf(m_Pdf);
        rpdf->ResetState();
    }
    CGlResMgr::Instance().SetCurrentRenderer(mgr);
}

END_NCBI_SCOPE
