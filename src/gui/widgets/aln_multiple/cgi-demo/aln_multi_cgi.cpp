/*  $Id: aln_multi_cgi.cpp 43735 2019-08-26 13:41:17Z ucko $
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
 * Author:  Andrey Yazhuk
 *
 * File Description:
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistl.hpp>

#include <cgi/cgictx.hpp>
#include <gui/opengl/mesa/glcgi_image.hpp>
#include <gui/opengl/mesa/gloscontext.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/glpane.hpp>

#include <objmgr/object_manager.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>

#include <gui/widgets/aln_multiple/alnmulti_ds_builder.hpp>
#include <gui/widgets/aln_multiple/align_row.hpp>
#include <gui/widgets/aln_multiple/alnvec_multi_ds.hpp>
#include <gui/widgets/aln_multiple/alnmulti_model.hpp>
#include <gui/widgets/aln_multiple/alnmulti_renderer.hpp>

#include <objtools/data_loaders/genbank/gbloader.hpp>

#include <objmgr/bioseq_handle.hpp>


USING_SCOPE(ncbi);   //using namespace ncbi;
//USING_SCOPE(objects);


static const int kColorGradNumber = 32; /// number of gradient colors in alignment


class CAlnMultiCGIException : public CException
{
public:
    enum EErrCode   {
        eInvalidAccession,
        eCannotLoadBioseq,
        eInvalidVisibleRange,
        eInvalidAnchor
    };

    virtual const char* GetErrCodeString(void) const
    {
        switch(GetErrCode())    {
        case eInvalidAccession: return "eInvalidException";
        case eCannotLoadBioseq: return "eCannotLoadBioseq";
        case eInvalidVisibleRange: return "eInvalidVisibleRange";
        case eInvalidAnchor: return "eInvalidAnchor";
        default:    return CException::GetErrCodeString();
        };
    }

    NCBI_EXCEPTION_DEFAULT(CAlnMultiCGIException, CException);
};

/////////////////////////////////////////////////////////////////////////////
//  CAlnMultiCGIApplication::
//

class CAlnMultiCGIApplication : public CGlCgiImageApplication
{
public:
    CAlnMultiCGIApplication();

    virtual void Render(CCgiContext& ctx);
protected:
    virtual void x_PreProcess(CCgiContext& ctx);
private:
    CRef<CObjectManager>    m_ObjMgr;
    CRef<CScope>            m_Scope;
    CBioseq_Handle          m_Handle;

    // Renderer setup
    CRowStyleCatalog    m_StyleCatalog;
    CAlnMultiDSBuilder  m_Builder;
    CRef<IAlnMultiDataSource> m_DataSource;
    CAlnMultiModel      m_Model;
    CAlnMultiRenderer   m_Renderer;

    //parameter
    int m_Start, m_Stop;
};



CAlnMultiCGIApplication::CAlnMultiCGIApplication()
: m_Renderer(TVPRect(0, 0, m_Width, m_Height))
{
    //setup ?
}

//extracting arguments, verifying and loading data
void    CAlnMultiCGIApplication::x_PreProcess(CCgiContext& ctx)
{
    SetEnvironment("NCBI_SERVICE_NAME_ID1",  "ID1LXA");

    // retrieve our CGI rendering params
    const CCgiRequest& request  = ctx.GetRequest();
    const TCgiEntries& entries = request.GetEntries();

    m_Start = m_Stop = -1;

    TCgiEntries::const_iterator it_acc = entries.find("acc");
    TCgiEntries::const_iterator it_start = entries.find("start");
    TCgiEntries::const_iterator it_stop = entries.find("stop");
    TCgiEntries::const_iterator it_anchor = entries.find("anchor");

    if(it_acc == entries.end()) {
        NCBI_THROW(CAlnMultiCGIException, eInvalidAccession, "Accession is not specified.");
    } else {
        string s = string("Accession: ") + it_acc->second;
        LOG_POST(s);

        //extract other parameters
        if(it_start != entries.end()  &&  it_start->second.size()) {
            m_Start = NStr::StringToInt(it_start->second);
        }
        if(it_stop != entries.end()  &&  it_stop->second.size()) {
            m_Stop = NStr::StringToInt(it_stop->second);
        }

        m_ObjMgr = CObjectManager::GetInstance();
        CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);

        m_Scope.Reset(new CScope(*m_ObjMgr));
        m_Scope->AddDefaults();

        CSeq_id id(it_acc->second);
        m_Handle = m_Scope->GetBioseqHandle(id);

        if(m_Handle)    { // setup data source and visible range
            m_Builder.Init(*m_Scope, m_Handle);
            m_DataSource = m_Builder.CreateDataSource();

            // attempt to set anchor
            if(it_anchor != entries.end()  &&  it_anchor->second.size())    {
                CSeq_id anchor_id(it_anchor->second);

                bool b_set = false;
                typedef IAlnMultiDataSource::TNumrow    TNumrow;
                TNumrow n_rows = m_DataSource->GetNumRows();
                for( TNumrow row = 0;  row < n_rows  &&  ! b_set;  row++ ) {
                    if(m_DataSource->CanGetId(row)) {
                        const CSeq_id& id = m_DataSource->GetSeqId(row);
                        if(id.Match(anchor_id)) {
                            m_DataSource->SetAnchor(row);
                            b_set = true;
                        }
                    }
                }
                if(! b_set) {
                    NCBI_THROW(CAlnMultiCGIException, eInvalidAnchor,
                                "Anchor accession is invalid.");
                }

            }

            //calculate visible range
            int aln_start = m_DataSource->GetAlnStart();
            int aln_stop = m_DataSource->GetAlnStop();
            if(m_Start > -1)    {
                if(m_Start < 0  ||  m_Start > aln_stop) {
                    NCBI_THROW(CAlnMultiCGIException, eInvalidVisibleRange,
                               "Visible start out of range.");
                }

            } else m_Start = aln_start;
            if(m_Stop > -1) {
                if(m_Stop < 0  ||  m_Stop > aln_stop)   {
                    NCBI_THROW(CAlnMultiCGIException, eInvalidVisibleRange,
                                "Visible end out of range.");
                }

            } else m_Stop = aln_stop;

            //setup model
            m_StyleCatalog.SetDefaultStyle(new CRowDisplayStyle());
            m_StyleCatalog.SetWidgetStyle(m_Model.GetDisplayStyle());

            //m_Model.EnableScoring(false);
            m_Model.SetStyleCatalog(&m_StyleCatalog);
            m_Model.SetDataSource(m_DataSource.GetPointer());

            m_Renderer.SetContext(&m_Model);
            TModelRect rc_vis = m_Model.GetAlignPort().GetModelLimitsRect();
            size_t h = (size_t) -rc_vis.Height();
            m_Height = min(m_Height, h);
        }
        else {
           string msg = "Cannot load Bioseq for accession \"";
           msg += it_acc->second;
           msg += "\"";
           NCBI_THROW(CAlnMultiCGIException, eCannotLoadBioseq, msg.c_str());
       }

    }
}

void CAlnMultiCGIApplication::Render(CCgiContext& ctx)
{
    if(m_Handle)  {
        m_Renderer.Resize(TVPRect(0, 0, m_Width, m_Height));

        //adjus visible space
        CGlPane& port = m_Model.GetAlignPort();
        port.SetViewport(TVPRect(0, 0, m_Width, m_Height)); ///### this have to be eliminated
        port.ZoomAll();

        // adjust visible range
        TModelRect rc_vis = port.GetVisibleRect();
        rc_vis.SetHorz(m_Start, m_Stop + 1);
        port.SetVisibleRect(rc_vis);


        m_Renderer.Render();
    }
    //debugging trick
    //CImageIO::WriteImage(m_Context->GetBuffer(), "test.png", m_Format);
}



/////////////////////////////////////////////////////////////////////////////
//  MAIN
//

int main(int argc, const char* argv[])
{
    int result = CAlnMultiCGIApplication().AppMain(argc, argv, 0, eDS_Default, 0);
    _TRACE("back to normal diags");
    return result;
}
