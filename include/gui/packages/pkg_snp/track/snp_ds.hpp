#ifndef GUI_PACKAGES_SNP_TRACK__SNP_DS__HPP
#define GUI_PACKAGES_SNP_TRACK__SNP_DS__HPP

/*  $Id: snp_ds.hpp 44952 2020-04-25 01:20:25Z rudnev $
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
 *    This file contains class declarations for SSnpJobResult,
 *       CSnpJob, CSGSnpDS and CSGSnpDSType
 *
 */

#include <corelib/ncbimtx.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_vector.hpp>
#include <util/rangemap.hpp>
#include <util/range_coll.hpp>

#include <gui/gui_export.h>
#include <gui/widgets/wx/ui_tool_registry.hpp>
#include <gui/utils/attr_range_coll.hpp>
#include <gui/utils/event_translator.hpp>
#include <gui/utils/extension.hpp>

#include <gui/widgets/seq_graphic/seqgraphic_job.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_data_source.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/layout_track.hpp>
#include <gui/widgets/snp/filter/snp_filter.hpp>



BEGIN_NCBI_SCOPE

struct SSnpFilter;
///////////////////////////////////////////////////////////////////////////////
///   SSnpJobResult
///////////////////////////////////////////////////////////////////////////////
struct NCBI_GUIWIDGETS_SNP_EXPORT SSnpJobResult : public CObject
{
    enum EDataType {
        eHistFromGraphs,
        eHistFromFeats,
        eComment,
        eFeats
    };
    typedef int             TDataType;

    EDataType               eDataType;
    CSeqGlyph::TObjects     listObjs;
};

///////////////////////////////////////////////////////////////////////////////
///   CSnpJob
///////////////////////////////////////////////////////////////////////////////
class CSnpJob : public CSGAnnotJob
{
public:
    struct SParams {
        bool                        bAdaptive;      ///< Only return feat glyphs when not overview
        bool                        bPreferFeat;    ///< Return feat glyphs instead of histogram glyphs
        bool                        isOverview;
        bool                        bAdaptiveSelector;
        TModelUnit                  scale;
        int                         depth;
        CRenderingContext*          r_cntx;
        string                      sAnnotName;
    };

    CSnpJob(const objects::CBioseq_Handle& handle, const TSeqRange& range,
            const SParams& params, SSnpFilter* filter);

    virtual EJobState   x_Execute();

    //!! this is a temporary function for an interim period when SNP filters are taken from
    //!! TMS and recorded as annots
    //!! to avoid creating a selector with annotation containing a filter (which will not load
    //!! anything as a result), such annotations are converted to "SNP"
    //!! this will need to be revisited once TMS/OM bridge will need to be built for SNP_in_VDB
    static string s_AdjustAnnotName(const string& sAnnotName);
private:
    void    x_FetchGraphs(const objects::CSeq_loc &, CSeqGlyph::TObjects*);
    void    x_FetchFtable(const objects::CSeq_loc &, CSeqGlyph::TObjects*);
	void	x_FetchFtableEx(const objects::SAnnotSelector& sel,
							const objects::CSeq_loc &loc, 
							CSeqGlyph::TObjects *glyphs);
    void    x_PreFetchFtable(const objects::SAnnotSelector& sel,
							  const ncbi::objects::CSeq_loc &loc, 
							  CSeqGlyph::TObjects& glyphs);
    void    x_MakeHistogramFromFtable(const CSeqGlyph::TObjects& srcFtable, 
							  CSeqGlyph::TObjects& glyphs);

private:
    SParams                     m_Params;
    SSnpJobResult::EDataType    m_eDataType;
    CRef<SSnpFilter>            m_Filter;

};

///////////////////////////////////////////////////////////////////////////////
// CSGSnpDS
///////////////////////////////////////////////////////////////////////////////
class NCBI_GUIWIDGETS_SNP_EXPORT CSGSnpDS : public CSGGenBankDS
{
public:
    static void GetAnnotNames(SConstScopedObject& object,
                              map<string, string>& annots,
                              const TSeqRange& range,
                              const ILayoutTrackFactory::SExtraParams& params);

public:
    CSGSnpDS(objects::CScope& scope, const objects::CSeq_id& id);

    void LoadData(const TSeqRange& range, 
                        CSnpJob::SParams& p,
                        SSnpFilter* filter);
};


///////////////////////////////////////////////////////////////////////////////
// CSGSnpDSType
///////////////////////////////////////////////////////////////////////////////
class NCBI_GUIWIDGETS_SNP_EXPORT CSGSnpDSType :
    public CObject,
    public ISGDataSourceType,
    public IExtension
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual ISGDataSource* CreateDS(SConstScopedObject& object) const;
    virtual bool IsSharable() const;
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}
};



END_NCBI_SCOPE

#endif  // GUI_PACKAGES_SNP_TRACK__SNP_DS__HPP
