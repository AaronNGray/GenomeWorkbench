#ifndef GUI_PACKAGES_SNP_HAPMAP__HAPMAP_DS__HPP
#define GUI_PACKAGES_SNP_HAPMAP__HAPMAP_DS__HPP

/*  $Id: hapmap_ds.hpp 25703 2012-04-24 19:57:50Z wuliangs $
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
 *    This file contains class declarations for CSGHapmapDS and CSGHapmapDSType
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

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
// CSGHapmapDS
///////////////////////////////////////////////////////////////////////////////
class CSGHapmapDS : public CSGGenBankDS
{
public:
    static void GetTrackNames(SConstScopedObject& object,
                              TAnnotNameTitleMap& names,
                              const TSeqRange& range,
                              const ILayoutTrackFactory::SExtraParams& params);

public:
    CSGHapmapDS(objects::CScope& scope, const objects::CSeq_id& id);

    void SetName(const string&);
    void LoadData(const TSeqRange& range,
                  TModelUnit scale,
                  bool preferFeats,
                  bool isOverview);

private:
    string m_Name;

};


///////////////////////////////////////////////////////////////////////////////
// CSGHapmapDSType
///////////////////////////////////////////////////////////////////////////////
class NCBI_GUIWIDGETS_SNP_EXPORT CSGHapmapDSType :
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

///////////////////////////////////////////////////////////////////////////////
///   SHapmapJobResult
///////////////////////////////////////////////////////////////////////////////
struct SHapmapJobResult : public CObject
{
    enum EDataType {
        eHistFromGraphs,
        eHistFromFeats,
        eFeats
    };

    EDataType               eDataType;
    CSeqGlyph::TObjects     listObjs;
};


END_NCBI_SCOPE

#endif  // GUI_PACKAGES_SNP_HAPMAP__HAPMAP_DS__HPP
