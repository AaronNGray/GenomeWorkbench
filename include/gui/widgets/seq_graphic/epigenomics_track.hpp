#ifndef GUI_WIDGETS_SEQ_GRAPHIC___EPIGENOMICS_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___EPIGENOMICS_TRACK__HPP

/*  $Id: epigenomics_track.hpp 36620 2016-10-14 20:34:56Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <gui/utils/extension.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/widgets/seq_graphic/data_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>


BEGIN_NCBI_SCOPE

class CTranslationConfig;

///////////////////////////////////////////////////////////////////////////////
/// CEpigenomicsDS - Epigenomics data source
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CEpigenomicsDS : public CSGGenBankDS
{
public:
    CEpigenomicsDS(objects::CScope& scope, const objects::CSeq_id& id);
    
    void GetAnnotNames(const TSeqRange& range, TAnnotNameTitleMap& annots,
        const vector<string>& annots_allowed);

    void LoadData(const TSeqRange& range, TModelUnit scale,
         bool calc_min = false, bool fixed_scale = false);

    void SetAnnot(const string& annot);
    const string& GetAnnot() const;

private:
    string      m_Annot;
};


///////////////////////////////////////////////////////////////////////////////
/// CEpigenomicsDSType - Epigenomics data source type
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CEpigenomicsDSType :
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
///   CEpigenomicsTrack -- A special track for showing epigenomics map.
///   The data can be stored at multiple scale levels as seq-graph or
///   seq-table.  The graph renderer is differnt from the regular graph
///   renderer. It needs to show max, min and average.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CEpigenomicsTrack :
    public CDataTrack
{
    friend class CEpigenomicsTrackFactory;

public:
    CEpigenomicsTrack(CEpigenomicsDS* ds, CRenderingContext* r_cntx);
    virtual ~CEpigenomicsTrack();

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    virtual string GetFullTitle() const;
    /// @}

    /// @name CDataTrack interface implementation
    /// @{
    virtual CRef<CSGGenBankDS>  GetDataSource();
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const;
    /// @}

protected:
    /// @name CLayoutTrack virtual methods overloading.
    /// @{
    virtual void x_LoadSettings(const string& preset_style,
                                const TKeyValuePairs& settings);

    /// save part of settings to a profile string.
    /// Only the potential settings changed by users will be saved.
    virtual void x_SaveSettings(const string& preset_style);
    virtual void x_OnIconClicked(TIconID id);
    /// @}

    /// @name CDataTrack pure virtual interfaces
    /// @{
    virtual void x_UpdateData();
    virtual void x_OnJobCompleted(CAppJobNotification& notify);
    /// @}

    void x_AddGraphs(const CSGJobResult& result);

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CEpigenomicsTrack(const CEpigenomicsTrack&);
    CEpigenomicsTrack& operator=(const CEpigenomicsTrack&);
    /// @}

    void x_OnSettingsIconClicked();

private:
    string                  m_AnnotDesc; ///< detailed track title
    CRef<CEpigenomicsDS>    m_DS;
    static CTrackTypeInfo   m_TypeInfo;

    /// Superinpose min graph on top of max graph?.
    bool                    m_CalcMin;

    /// Dynamic or fixed graph scale.
    /// By default, the graph scale is a dynamic value which
    /// is updated within the visible range.
    bool                    m_FixedScale;

};

///////////////////////////////////////////////////////////////////////////////
///   CEpigenomicsTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CEpigenomicsTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
    public IExtension
{
public:
    CEpigenomicsTrackFactory(){}

    /// create a layout track based on the input objects and extra parameters.
    virtual TTrackMap CreateTracks(
        SConstScopedObject& object,
        ISGDataSourceContext* ds_context,
        CRenderingContext* r_cntx,
        const SExtraParams& params = SExtraParams(),
        const TAnnotMetaDataList& src_annots = TAnnotMetaDataList()) const;

    virtual void GetMatchedAnnots(
        const TAnnotMetaDataList& src_annots,
        const ILayoutTrackFactory::SExtraParams& params,
        TAnnotNameTitleMap& out_annots) const;

    virtual bool UnderstandLevel() const
    { return true; }

    virtual bool NeedBackgroundInit() const
    { return true; }

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo& GetTypeInfo()
    { return CEpigenomicsTrack::m_TypeInfo; }

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}
};


///////////////////////////////////////////////////////////////////////////////
///   CEpigenomicsDS inline methods
///
inline
void CEpigenomicsDS::SetAnnot(const string& annot)
{ m_Annot = annot; }

inline
const string& CEpigenomicsDS::GetAnnot() const
{ return m_Annot; }


///////////////////////////////////////////////////////////////////////////////
///   CEpigenomicsTrack inline methods
///
inline /*virtual*/
CRef<CSGGenBankDS> CEpigenomicsTrack::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline /*virtual*/
CConstRef<CSGGenBankDS> CEpigenomicsTrack::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_DS.GetPointer());
}


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___EPIGENOMICS_TRACK__HPP
