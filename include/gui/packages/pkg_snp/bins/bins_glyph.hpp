#ifndef GUI_PACKAGES_SNP_BINS_TRACK__BINS_GLYPH__HPP
#define GUI_PACKAGES_SNP_BINS_TRACK__BINS_GLYPH__HPP

/*  $Id: bins_glyph.hpp 35413 2016-05-04 17:51:26Z evgeniev $
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
 * Authors:  Melivn Quintos
 *
 * File Description:
 *    CBinsGlyph -- Glyph that represents SNP Bin Track format
 *
 */


#include <corelib/ncbiobj.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/scope.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/density_map.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <objtools/snputil/snp_bins.hpp>
#include <objtools/snputil/snp_utils.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_SNP_EXPORT CBinsGlyph : public CSeqGlyph
{
////////////////////////////////////////////
// Public Structs/Classes/Typedefs
////////////////////////////////////////////
public:

    struct SBinMapped {
        CRef<objects::NSnpBins::SBin> obj; // the mapped NSnpBins::SBin object

        SBinMapped() {};
        SBinMapped(int /*c*/) { /* this is only needed to support DensityMap usage */}

        bool operator < (const SBinMapped &rhs) const
        {
            if (rhs.obj.Empty())   return false;
            if (this->obj.Empty()) return true;

            return (this->obj->range.GetFrom() < rhs.obj->range.GetFrom());
        }
        size_t  SequentialNumber;        // consecutive bin number in the density map
    };

    friend SBinMapped operator+(const SBinMapped& val1, const SBinMapped& val2);

    struct accum_study : public binary_functor<SBinMapped> {
        virtual SBinMapped operator() (const SBinMapped& x,
                                       const SBinMapped& y) const
        {
            return x + y;
        }

        virtual binary_functor<SBinMapped>* clone() const
        {
            return new accum_study;
        }
    };

    typedef CDensityMap<SBinMapped>   TDensityMap;

//////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////
public:

    /// @name Static Methods
    /// @{
    static string GenerateBinSignature(const string& title,
                                       const string& annot_name,
                                       TGi gi,
                                       TSeqRange range,
                                       objects::NSnpBins::TBinType BinType);
    /// @}

    /// @name ctors
    /// @{

    CBinsGlyph(const TDensityMap&       values,
               const objects::CSeq_loc& loc,
               objects::CScope *        scope);
    /// @}

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual bool OnLeftDblClick(const TModelPoint& p);
    virtual bool IsClickable() const;

    virtual TSeqRange GetRange(void) const;
    ///@}

    /// @name Non-Inherited Getters/Setters
    /// @{

    /// Access the first density map.
    const TDensityMap& GetDensityMap(void) const;

    // Get range for a particular bin
    TSeqRange GetBinRangeAt(TSeqPos pos) const;
    objects::NSnpBins::TBinType GetType() const { return m_Type; }
    void  SetType(objects::NSnpBins::TBinType t) { m_Type = t; }

    ///@}

    void SetTitle(const string& title);
    const string& GetTitle() const;

    void SetAnnotName(const string& name);
    const string& GetAnnotName() const;


//////////////////////////////////////////////
// Protected Methods
//////////////////////////////////////////////
protected:
    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();
    ///@}


//////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////
private:
    // This populates 'm_NonEmptyBins'.  It is labeled 'const'
    //   but modifies the mutable m_listObjs
    void        x_BuildNonEmptyBinList() const;
    TSeqRange   x_GetBinRange(unsigned int bin) const;
    string      x_GetColorTheme() const;

private:
    string                          m_Title;
    string                          m_AnnotName;
    CConstRef<objects::CSeq_loc>    m_Location;
    mutable TDensityMap             m_Map;
    objects::NSnpBins::TBinType     m_Type;

    mutable bool                    m_bHasObjectList;

    typedef list<SBinMapped*> TMappedBinList;

    // contains only non-empty bins
    mutable TMappedBinList m_NonEmptyBins;
    mutable CRef<objects::CScope>   m_Scope;
};

///////////////////////////////////////////////////////////////////////////////
/// CBinsGlyph inline method implementation.
inline
const CBinsGlyph::TDensityMap& CBinsGlyph::GetDensityMap(void) const
{
    return m_Map;
}

inline
void CBinsGlyph::SetTitle(const string& title)
{ m_Title = title; }

inline
const string& CBinsGlyph::GetTitle() const
{ return m_Title; }

inline
void CBinsGlyph::SetAnnotName(const string& name)
{ m_AnnotName = name; }

inline
const string& CBinsGlyph::GetAnnotName() const
{ return m_AnnotName; }


inline
CBinsGlyph::SBinMapped operator+(const CBinsGlyph::SBinMapped& val1,
                                 const CBinsGlyph::SBinMapped& val2)
{
    if (val1.obj.Empty()) return val2;
    if (val2.obj.Empty()) return val1;

    CBinsGlyph::SBinMapped  result;
    result.obj.Reset(new objects::NSnpBins::SBin);

    ITERATE(objects::NSnpBins::TBinEntryList, iter, val1.obj->m_EntryList) {
        result.obj->m_EntryList.push_back(*iter);
    }

	ITERATE(objects::NSnpBins::TBinEntryList, iter, val2.obj->m_EntryList) {
        result.obj->m_EntryList.push_back(*iter);
    }

    result.obj->count = val1.obj->count + val2.obj->count;
	// we expect the type to be always the same
	NCBI_ASSERT(val1.obj->type == val2.obj->type, "Merged CBinsGlyph::SBinMapped should always have the same types!");
	result.obj->type = val1.obj->type;

	result.obj->m_SigEntry = (objects::NSnpBins::ChooseSignificant(val1.obj->m_SigEntry, val2.obj->m_SigEntry, result.obj->type) == 1)
                                ? val1.obj->m_SigEntry
                                : val2.obj->m_SigEntry;
    // add the ranges together
    result.obj->range = val1.obj->range.CombinationWith(val2.obj->range);

    // signature is calculated during CBinsGlyph::x_BuildObjectList
    return result;
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_PACKAGES_SNP_BINS_TRACK__BINS_GLYPH__HPP
