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
 * Authors:  Vladislav Evgeniev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/wx/svg_grabber.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/print/svg_renderer.hpp>
#include <gui/print/print_media.hpp>

#include <util/image/image.hpp>

#include <corelib/ncbifile.hpp>

BEGIN_NCBI_SCOPE

CSVGGrabber::CSVGGrabber(IVectorGraphicsRenderer& pane)
  : m_Pane(pane) 
  , m_OutputStream(NULL)
{
}


void CSVGGrabber::SetOutputStream(CNcbiOstream* ostr)
{
    if (ostr) {
        m_OutputStream = ostr;
    }
}

IImageGrabber::eCaptureResult 
CSVGGrabber::GrabImages(int tex_size, IImageGrabberProgress* progress)
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

    const TVPRect& viewport = m_Pane.GetViewportRect();
    // Set the size of the rendered area (full viewport)
    CMedia m("", 
        viewport.Width(),
        viewport.Height() + m_TitleHeight,
        CUnit::ePdfUnit);
    
    // to have margins we have to pass in a viewport that fits within them
    // (e.g. 20,20,20,20 = vp(20, 20, width-40, height-40) but for now
    // we are using the screens viewport size only
    m_PrintOptions->SetMarginBottom(0);
    m_PrintOptions->SetMarginLeft(0);
    m_PrintOptions->SetMarginRight(0);
    m_PrintOptions->SetMarginTop(0);

    m_PrintOptions->SetMedia(m);

    m_PrintOptions->DisableGouraudShading(m_DisableGouraudShaded);


    //***************************************************

    if (m_TitleHeight>0.0f) 
        m_Pane.AddTitle(true);

    _ASSERT(m_SVGRenderer);
    m_SVGRenderer->Initialize(TVPRect(viewport.Left(), viewport.Bottom(), viewport.Right(), viewport.Top() + m_TitleHeight));
    m_SVGRenderer->SetSimplified(m_PrintOptions->GetGouraudShadingDisabled());

    m_Pane.RenderVectorGraphics((int)m_PrintOptions->GetMedia().GetWidth(), (int)m_PrintOptions->GetMedia().GetHeight());
    
    m_SVGRenderer->Write(*m_OutputStream);
    
    m_SVGRenderer->Finalize();
    m_PrintOptions.release();

    return eSuccess;
}

void CSVGGrabber::x_BeginCaptures(int /*buffer_size*/)
{
    m_ModelViewScaler = 1.0f;
    m_GuideWidthX = 0;
    m_GuideWidthY = 0;
   

    m_PrintOptions.reset(new CPrintOptions());

    m_PrintOptions->SetPageOrientation(CPrintOptions::ePortrait);
    string fname = CDir::ConcatPath(m_Directory, m_BaseImageName + "." + m_ImageFormat);
    m_PrintOptions->SetFilename(fname.c_str());
    m_PrintOptions->SetMarginBottom(20);
    m_PrintOptions->SetMarginLeft(20);
    m_PrintOptions->SetMarginRight(20);
    m_PrintOptions->SetMarginTop(20);
    m_PrintOptions->SetPagesWide(1);
    m_PrintOptions->SetPagesTall(1);

    m_PrintOptions->SetOutputFormat(CPrintOptions::eSvg);
}

void CSVGGrabber::x_EndCaptures()
{
}

void CSVGGrabber::x_InitRenderer()
{
    CIRef<IRender>  mgr = CGlResMgr::Instance().GetRenderer(eRenderSVG);
    if (mgr.IsNull()) {
        mgr.Reset(new CSVGRenderer());
        CGlResMgr::Instance().AddRenderer(mgr);
    }
    CSVGRenderer* svg = dynamic_cast<CSVGRenderer*>(mgr.GetPointerOrNull());
    _ASSERT(svg);
    m_SVGRenderer.Reset(svg);
    m_SVGRenderer->ResetState();
    CGlResMgr::Instance().SetCurrentRenderer(mgr);
}

END_NCBI_SCOPE
