#ifndef GUI_PACKAGES_SNP_LD_TRACK__LD_LINE__HPP
#define GUI_PACKAGES_SNP_LD_TRACK__LD_LINE__HPP

/*  $Id: ld_line.hpp 19738 2009-07-29 20:01:50Z quintosm $
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
 *    CLDBlockLine -- 
 */


#include <corelib/ncbiobj.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/widgets/seq_graphic/glyph_container_track.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_SNP_EXPORT CLDBlockLine : public CGlyphContainer
{
////////////////////////////////////////////
// Public Structs/Classes/Typedefs
////////////////////////////////////////////
public:
    typedef std::list<objects::CMappedFeat> TListFeats;

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
    CLDBlockLine(CRenderingContext* r_cntx);
    /// @}

    /// @name CSeqGlyph virtual methods.
    /// @{
    
    //virtual bool OnLeftDblClick(const TModelPoint& p);
    //virtual void GetTooltip(const TModelPoint& p, string& tt) const;
    //virtual bool IsClickable() const;

//    virtual TSeqRange GetRange(void) const;
    ///@}

//////////////////////////////////////////////
// Protected Methods
//////////////////////////////////////////////
protected:

    /// @name CGlyphContainer override virtual method implementation.
    /// @{
    virtual void x_RenderContent() const;
    /// @}


//////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////
private:
    void     x_DrawLabel() const;

//////////////////////////////////////////////
// Private Data
//////////////////////////////////////////////
private: 

};

///////////////////////////////////////////////////////////////////////////////
/// CLDBlockLine inline method implementation.

/* Empty */

END_NCBI_SCOPE

/* @} */

#endif  // GUI_PACKAGES_SNP_LD_TRACK__LD_LINE__HPP
