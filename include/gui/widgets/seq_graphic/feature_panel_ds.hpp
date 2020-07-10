#ifndef GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_PANEL_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_PANEL_DS__HPP

/*  $Id: feature_panel_ds.hpp 42615 2019-03-26 18:58:57Z katargir $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/layout_track.hpp>

BEGIN_NCBI_SCOPE

class CRenderingContext;
class CTrackContainer;


///////////////////////////////////////////////////////////////////////////////
/// CFeaturePanelDS
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CFeaturePanelDS :
    public CSGGenBankDS
{
public:
    typedef ILayoutTrackFactory::TAnnotMetaDataList TAnnotMetaDataList;

    CFeaturePanelDS(objects::CScope& scope, const objects::CSeq_id& id);
    ~CFeaturePanelDS();

    bool IsMetaDataJob(TJobID id) const;
    void DeleteMetaDataJob();
    void ClearMetaDataJobId();

    void CreateTrack(const string& desc, const ILayoutTrackFactory* factory,
        CTrackContainer* parent, CTempTrackProxy* params,
        const ILayoutTrackFactory::SExtraParams& extra_param);

    /// retrieve a list of annotations with their meta data.
    void GetAnnotMetaData(const string& desc);
    void GetAssemblies(const string& seqAcc);

    void SetDSContext(ISGDataSourceContext* ds_context);
    void SetRenderingContext(CRenderingContext* r_cntx);
    void SetAssembly(const string& assembly);
	const string& GetAssembly() const { return m_Assembly; }
    void SetTMSContext(const string& tms_context);
	void SetUseEUtils(bool use);
    virtual bool AllJobsFinished() const;

private:
    /// @name Forbidden methods
    /// @{
    CFeaturePanelDS(const CFeaturePanelDS&);
    CFeaturePanelDS& operator=(const CFeaturePanelDS&);
    /// @}

private:
    CIRef<ISGDataSourceContext> m_DSContext;
    CRenderingContext*          m_RenderingCntx;

    string  m_Assembly;
    string  m_TMSContext;
	bool    m_UseEUtils;

    /// job id for annot meta-data job id.
    TJobID  m_MetaDataJob;
};

///////////////////////////////////////////////////////////////////////////////
/// CFeaturePanelDSType
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CFeaturePanelDSType :
    public CObject,
    public ISGDataSourceType,
    public IExtension
{
public:
    /// create an instance of the layout track type using default settings.
    virtual ISGDataSource* CreateDS(SConstScopedObject& object) const;

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}

    /// check if the data source can be shared.
    virtual bool IsSharable() const;
};


///////////////////////////////////////////////////////////////////////////////
/// CTrackInitJobResult
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTrackInitJobResult : public CObject
{
public:
    CTrackInitJobResult(){}

    ILayoutTrackFactory::TTrackMap  m_Tracks;
    CTrackContainer*                m_ParentTrack;
    CRef<CTempTrackProxy>           m_TrackParams;
    string                          m_Desc;
};

///////////////////////////////////////////////////////////////////////////////
/// CAnnotMetaDataJobResult
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAnnotMetaDataJobResult : public CObject
{
public:
    typedef ILayoutTrackFactory::TAnnotMetaDataList TAnnotMetaDataList;

    CAnnotMetaDataJobResult(){}

    TAnnotMetaDataList  m_Annots;
};

///////////////////////////////////////////////////////////////////////////////
/// CAnnotMetaDataJobResult
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGetAssembliesJobResult : public CObject
{
public:
    CGetAssembliesJobResult() {}

    list<CRef<objects::CGC_Assembly> > m_Assemblies;
};


///////////////////////////////////////////////////////////////////////////////
/// CFeaturePanelDS inline methods
///
inline
bool CFeaturePanelDS::IsMetaDataJob(TJobID id) const
{
    return m_MetaDataJob != -1  &&   m_MetaDataJob == id;
}

inline
void CFeaturePanelDS::SetDSContext(ISGDataSourceContext* ds_context)
{
    m_DSContext.Reset(ds_context);
}

inline
void CFeaturePanelDS::SetRenderingContext(CRenderingContext* r_cntx)
{
    m_RenderingCntx = r_cntx;
}


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_PANEL_DS__HPP
