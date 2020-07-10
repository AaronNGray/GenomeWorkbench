#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_MAP_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_MAP_DS__HPP

/*  $Id: segment_map_ds.hpp 44596 2020-01-29 15:41:36Z filippov $
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
#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>

BEGIN_NCBI_SCOPE

class ICache;

const int kSegmentLoadLimit = 100;

///////////////////////////////////////////////////////////////////////////////
/// CSGSegmentMapDS
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGSegmentMapDS : public CSGGenBankDS
{
public:
    /// It is not technically correct to name a specific segment
    /// level as config/scaffold and component because segment map
    /// is just a representation of sequence relationship at different
    /// level of sequence assembly. 
    /// Assumption: For all chromosome sequences, contigs/scaffolds are
    /// at level 0, and components are at level 1. For any other sequences,
    /// there will be only one level of segments which are components. And
    /// the components are at level 0.
    enum ESegmentLevel {
        eInvalid = -2,
        eAdaptive = -1, ///< show either contigs or components based on range
        eContig = 0,
        eComponent = 1
    };

    CSGSegmentMapDS(objects::CScope& scope, const objects::CSeq_id& id);

    void LoadSegmentMap(const TSeqRange& range, int cutoff, TJobToken token);

    /// Used in sequence track for showing segment color.
    void LoadSegmentSmear(const TSeqRange& range, TModelUnit scale, const objects::CSeqVector* seq_vec = nullptr);

    void LoadSegmentMapSeqIDs(CSeqGlyph::TObjects& objs, TJobToken token);

    void LoadSwitchPoints();

    /// Get total number of segment map levels.
    int GetSegmentMapLevels(const TSeqRange& range) const;

    /// Query if there is segment maps for given level.
    /// @param level if it is -1, it checks all the way down until
    /// either finding one or reaching the bottom.
    bool HasSegmentMap(int level, const TSeqRange& range) const;
    bool HasComponent(const TSeqRange& range) const;
    bool HasScaffold(const TSeqRange& range) const;

    void GetAnnotNames(objects::SAnnotSelector& sel,
        const TSeqRange& range, TAnnotNameTitleMap& names) const;

    void SetAnnot(const string& annot);
    const string& GetAnnot() const;

    void SetChromosome(bool flag);
    bool IsChromosome() const;

    /// Set which segment level to show.
    /// @param level is the conceptual segment level. It needs
    /// to be converted to the real segment level. If the level
    /// is the actual segment level, call SetDepth() directly.
    void SetSegmentLevel(ESegmentLevel level);

    /// Set cache client to use
    static void SetICacheClient(ICache* pCache);
    /// Enables/disables the segment map
    static void SetEnabled(bool enabled = true);
    /// Indicates if the segment map is enabled 
    static bool GetEnabled();

private:
    /// Convert conceptual segment level to real segment depth.
    /// For all chromosome sequences, contigs/scaffolds are at level 0,
    /// and components are at level 1. For any other sequences, there will
    /// be only one level of segments (components) which are at level 0.
    /// -1 means 'Adaptive'.
    int x_ConceptualToRealDepth(ESegmentLevel level) const;

private:
    /// Flag to indicate if the sequence is a chromosome sequence.
    bool   m_IsChromosome;

    /// The named annotation storing segment map data.
    /// Used in sequence track (not segment map tracks, such as
    /// components or scaffolds)
    string m_Annot;

    /// Cache, used to store segment smear
    static ICache*  m_Cache;
    /// Indicates if the segment map is enabled
    static bool     m_Enabled;
};

///////////////////////////////////////////////////////////////////////////////
/// CSGFeatureDSType
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGSegmentMapDSType :
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
/// CSGFeatureDS inline methods
///
inline
void CSGSegmentMapDS::SetAnnot(const string& annot)
{ m_Annot = annot; }


inline
const string& CSGSegmentMapDS::GetAnnot() const
{ return m_Annot; }

inline
void CSGSegmentMapDS::SetChromosome(bool flag)
{ m_IsChromosome = flag; }

inline
bool CSGSegmentMapDS::IsChromosome() const
{ return m_IsChromosome; }


END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_MAP_DS__HPP */
