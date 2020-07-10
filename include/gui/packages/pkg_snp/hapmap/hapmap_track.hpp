#ifndef GUI_PACKAGES_SNP_HAPMAP__HAPMAP_TRACK__HPP
#define GUI_PACKAGES_SNP_HAPMAP__HAPMAP_TRACK__HPP

/*  $Id: hapmap_track.hpp 36620 2016-10-14 20:34:56Z katargir $
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
 * Authors:  Melvin Quintos
 *
 * File Description:
 *    This file contains class declarations for CHapmapTrack and 
 *      CHapmapTrackFactory
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/packages/pkg_snp/hapmap/hapmap_ds.hpp>
#include <gui/utils/extension.hpp>
#include <gui/widgets/seq_graphic/data_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/annot_selector.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
//   CHapmapTrack
///////////////////////////////////////////////////////////////////////////////
class CHapmapTrack :
    public CDataTrack
{
    friend class CHapmapTrackFactory;

public:
    CHapmapTrack(CSGHapmapDS* ds, CRenderingContext* r_cntx);
    virtual ~CHapmapTrack();

    /// @name CDataTrack reimplemented methods.
    /// @{
    virtual CRef<CSGGenBankDS>  GetDataSource();
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const;
    /// @}

    /// @name CLayoutTrack reimplemented methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    /// @}

protected:
    virtual void x_OnIconClicked(TIconID id);
    virtual void x_LoadSettings(const string& preset_style,
        const TKeyValuePairs& settings);

    /// @name CDataTrack pure virtual interfaces
    /// @{
    virtual void x_UpdateData();
    virtual void x_OnJobCompleted(CAppJobNotification& notify);
    /// @}

    void x_CreateLayout(const SHapmapJobResult& result);
    void x_UpdateLayout();

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CHapmapTrack(const CHapmapTrack&);
    CHapmapTrack& operator=(const CHapmapTrack&);
    /// @}

    bool x_IsOverviewMode() const;

private:

    enum ELayout {
        eLayout_Adaptive = 260,  ///< All SNPs drawn in one line
        eLayout_Density,         ///< Always show density
        eLayout_Features,        ///< Show features whenever possible
        eLayout_Labels           ///< Show labels and cascade features (CLayeredLayoutPolicy)
    };

    int                     m_eLayout;

    CRef<CSGHapmapDS>       m_DS;
    objects::SAnnotSelector m_FeatSel;

    SIconInfo               m_IconLayout;
    CRef<SHapmapJobResult>  m_JobResult;

    static CTrackTypeInfo   m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
//   CHapmapTrackFactory
///////////////////////////////////////////////////////////////////////////////
class NCBI_GUIWIDGETS_SNP_EXPORT CHapmapTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IExtension
{
public:
    CHapmapTrackFactory(){}

    static const CTrackTypeInfo& GetTypeInfo()
        { return CHapmapTrack::m_TypeInfo; }

    /// @name ILayoutTrackFactory interface implementation
    /// @{
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
    /// @}

    /// @name ITrackConfigurable implementation 
    /// @{
    CRef<objects::CTrackConfigSet>
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

///////////////////////////////////////////////////////////////////////////////
///   CHapmapTrack inline methods
///////////////////////////////////////////////////////////////////////////////
inline /*virtual*/
CRef<CSGGenBankDS> CHapmapTrack::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline /*virtual*/
CConstRef<CSGGenBankDS> CHapmapTrack::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_DS.GetPointer());
}

END_NCBI_SCOPE

#endif  // GUI_PACKAGES_SNP_HAPMAP__HAPMAP_TRACK__HPP
