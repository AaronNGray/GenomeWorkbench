/*  $Id: seqtest.cpp 16339 2008-03-10 18:33:35Z dicuccio $
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
 * Author:  Denis Vakatov
 *
 * File Description:
 *    CSeqTestApp -- test of OpenGL framework for rendering images as a CGI app
 */

#include <ncbi_pch.hpp>
#include <cgi/cgictx.hpp>
#include <gui/opengl/mesa/glcgi_image.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/glcamera.hpp>

#include <objmgr/object_manager.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>

#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds.hpp>

#include "../seqgraphic_render.hpp"


USING_SCOPE(ncbi);   //using namespace ncbi;
USING_SCOPE(objects);

/////////////////////////////////////////////////////////////////////////////
//  CSeqTestApplication::
//

class CSeqTestApplication : public CGlCgiImageApplication
{
public:
    CSeqTestApplication();

    virtual void Init();
    virtual void Render(CCgiContext& ctx);

private:
    // camrea set-up
    CGlCamera m_Camera;

    CRef<CObjectManager> m_ObjMgr;
    CRef<CScope> m_Scope;

    // data source and renderer
    CRef<CSeqGraphicDataSource> m_DataSource;
};



CSeqTestApplication::CSeqTestApplication()
{
    m_Camera.SetLayout(CGlCamera::ePerspective);
    m_Camera.SetFieldOfView(45.0f);
    m_Camera.SetNearPlane(0.1f);
    m_Camera.SetFarPlane(500.0f);
}


void CSeqTestApplication::Init()
{
    string syb_path = GetEnvironment().Get("SYBASE");
    if (syb_path.empty()) {
        syb_path = "/export/home/sybase/clients/current";
        CDir dir(syb_path);
        if ( !dir.Exists() ) {
            syb_path = "/netopt/Sybase/clients/current";
        }
        SetEnvironment("SYBASE", syb_path);
    }

    CGlCgiImageApplication::Init();
}

void CSeqTestApplication::Render(CCgiContext& ctx)
{
    // retrieve our CGI rendering params
    const CCgiRequest& request  = ctx.GetRequest();
    const TCgiEntries& entries = request.GetEntries();

    //
    // retrieve the seq-id of interest
    //

    TCgiEntries::const_iterator acc_iter = entries.find("id");
    if ( acc_iter == entries.end() ) {
        LOG_POST(Error << "Error: accession not indicated to cgi");
        return;
    }

    CSeq_id id(acc_iter->second);
    m_ObjMgr.Reset(new CObjectManager());
    m_ObjMgr->RegisterDataLoader(*new CGBDataLoader("GenBank", "id1"),
                                 CObjectManager::eDefault);

    m_Scope.Reset(new CScope(*m_ObjMgr));
    m_Scope->AddDefaults();

    if (!m_Scope->GetBioseqHandle(id) ) {
        LOG_POST(Error << "Failed to retrieve sequence for "
                 << acc_iter->second);
        return;
    }

    m_DataSource.Reset(new CSeqGraphicDataSource( m_Scope.GetObject(), id));
    //m_DataSource.Reset(new CTestDataSource(acc_iter->second));


    // Setup configuration
    // Create an empty config and pass it to the widget
    CRef<CSeqGraphicConfig> config( new CSeqGraphicConfig(NULL) );
    config->SetHistogram(CSeqFeatData::e_Imp,
                        CSeqFeatData::eSubtype_STS, true);
    config->SetHistogram(CSeqFeatData::e_Imp,
                        CSeqFeatData::eSubtype_repeat_region, true);

    //
    // and set up our renderer
    //

    // make sure we reflect the whole window, and our sequence
    TVPRect rc(0, 0, m_Width, m_Height);

    CSeqGraphicRenderer renderer;
    renderer.SetConfig(config);
    renderer.SetDataSource(rc, m_DataSource);

    //
    // rendering options
    //

    // rendered width
    TSeqPos from = 0;
    TSeqPos to = m_DataSource->GetSequenceLength();

    TCgiEntries::const_iterator from_iter = entries.find("from");
    if (from_iter != entries.end()) {
        from = NStr::StringToInt(from_iter->second);
    }
    TCgiEntries::const_iterator to_iter = entries.find("to");
    if (to_iter != entries.end()) {
        to = NStr::StringToInt(to_iter->second);
    }

    renderer.ZoomOnRange(TSeqRange(from, to));

    //
    // and draw!
    //
    renderer.draw();
}



/////////////////////////////////////////////////////////////////////////////
//  MAIN
//

int main(int argc, const char* argv[])
{
    int result = CSeqTestApplication().AppMain(argc, argv, 0, eDS_Default, 0);
    _TRACE("back to normal diags");
    return result;
}
