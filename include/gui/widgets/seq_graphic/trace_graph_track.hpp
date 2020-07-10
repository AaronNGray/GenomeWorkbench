#ifndef GUI_WIDGETS_SEQ_GRAPHIC___TRACE_GRAPH_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___TRACE_GRAPH_TRACK__HPP

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
 * Authors:  Andrei Shkeda
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/utils/extension.hpp>
#include <gui/widgets/seq_graphic/data_track.hpp>
#include <gui/widgets/seq_graphic/trace_graph_ds.hpp>
#include <objmgr/annot_selector.hpp>


BEGIN_NCBI_SCOPE

class CTraceGraphConfig;

///////////////////////////////////////////////////////////////////////////////
///   CTraceGraphTrack -- 
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTraceGraphTrack :
    public CDataTrack
{
    friend class CTraceGraphTrackFactory;

public:

    CTraceGraphTrack(CSGTraceGraphDS* ds, CRenderingContext* r_cntx);
    virtual ~CTraceGraphTrack();

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    /// @}



    virtual CRef<CSGGenBankDS>  GetDataSource();
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const;

    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual CHTMLActiveArea* InitHTMLActiveArea(TAreaVector* p_areas) const;

    void SetAnnot(const string& annot);
    const string& GetAnnot() const;

protected:
    /// @name CLayoutTrack virtual methods overloading.
    /// @{
    virtual void x_LoadSettings(const string& preset_style,
                                const TKeyValuePairs& settings);

    /// save part of settings to a profile string.
    /// Only the potential settings changed by users will be saved.
    virtual void x_SaveSettings(const string& preset_style);

    //virtual void x_OnLayoutChanged();

    /// @}

    /// @name CDataTrack pure virtual interfaces
    /// @{
    virtual void x_UpdateData();
    virtual void x_OnJobCompleted(CAppJobNotification& notify);

    /// @}

    /// @name CGlyphContainerpure virtual interfaces
    /// @{
    virtual void x_UpdateBoundingBox();
    virtual void x_RenderContent() const;
    /// @}

    CSGTraceGraphDS& x_GetGraphDS();

    virtual void x_OnIconClicked(TIconID id);
    void x_OnSettingsIconClicked();


private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CTraceGraphTrack(const CTraceGraphTrack&);
    CTraceGraphTrack& operator=(const CTraceGraphTrack&);
    /// @}

private:
    string                  m_AnnotName;
    CRef<CSGTraceGraphDS>   m_DS;
    objects::SAnnotSelector m_Sel;

    static CTrackTypeInfo   m_TypeInfo;
    CRef<CTraceGraphConfig> m_Config;
};

///////////////////////////////////////////////////////////////////////////////
///   CTraceGraphTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTraceGraphTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IExtension
{
public:
    CTraceGraphTrackFactory() {}

    /// create a layout track based on the input objects and extra parameters.
    virtual TTrackMap CreateTracks(
        SConstScopedObject& object,
        ISGDataSourceContext* ds_context,
        CRenderingContext* r_cntx,
        const SExtraParams& params = SExtraParams(),
        const TAnnotMetaDataList& src_annots = TAnnotMetaDataList()) const;

    virtual bool UnderstandLevel() const
    { return true; }

    virtual bool NeedBackgroundInit() const
    { return true; }

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo& GetTypeInfo()
    { return CTraceGraphTrack::m_TypeInfo; }

    /// @name ITrackConfigurable interface implementation
    /// @{
    virtual CRef<objects::CTrackConfigSet>
        GetSettings(const string& profile,
                    const TKeyValuePairs& settings,
                    const CTempTrackProxy* track_proxy) const;
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}
};



///////////////////////////////////////////////
/// Inline methods

inline
const string& CTraceGraphTrack::GetAnnot() const
{
    return m_AnnotName;
}


inline CSGTraceGraphDS& CTraceGraphTrack::x_GetGraphDS()
{
    return *m_DS;
}


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___TRACE_GRAPH_TRACK__HPP
