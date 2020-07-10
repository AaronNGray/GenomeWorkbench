#ifndef GUI_PACKAGES_SNP_LD_TRACK__LD_GLYPH__HPP
#define GUI_PACKAGES_SNP_LD_TRACK__LD_GLYPH__HPP

/*  $Id: ld_glyph.hpp 35413 2016-05-04 17:51:26Z evgeniev $
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
 *    CLDBlockGlyph -- 
 */


#include <corelib/ncbiobj.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_SNP_EXPORT CLDBlockGlyph : public CSeqGlyph
{
////////////////////////////////////////////
// Public Structs/Classes/Typedefs
////////////////////////////////////////////
public:
    struct SValue {
        double          score;
        int             popId;
        int             blockId;
    };

//////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////
public:

    /// @name Static Methods
    /// @{
    static string GenerateBinSignature(unsigned int rsid,
                                const string& title,
                                int gi,
                                TSeqRange range
                                );
    /// @}

    /// @name ctors
    /// @{

    CLDBlockGlyph(const objects::CMappedFeat&  feat); // TODO remove num parameter used for debugging
    /// @}

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual bool OnLeftDblClick(const TModelPoint& p);
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual bool IsClickable() const;

    virtual TSeqRange GetRange(void) const;
    ///@}

    /// @name Non-Inherited Getters
    /// @{
    const string& GetName() const;

    ///@}

    void SetName(const string& name);


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
    // This populates 'm_listObjs'.  It is labeled 'const'
    //   but modifies the mutable m_listObjs
    void        x_BuildObjectList() const;
    TSeqRange   x_GetBinRange(unsigned int bin) const;

private:
    string                          m_Name;    
    //objects::CMappedFeat            m_Feature;
    CConstRef<objects::CSeq_loc>                  m_Location;
    SValue                          m_Value;
};

///////////////////////////////////////////////////////////////////////////////
/// CLDBlockGlyph inline method implementation.
inline
void CLDBlockGlyph::SetName(const string& name)
{ m_Name = name; }

inline
const string& CLDBlockGlyph::GetName() const
{ return m_Name; }


END_NCBI_SCOPE

/* @} */

#endif  // GUI_PACKAGES_SNP_LD_TRACK__LD_GLYPH__HPP
