#ifndef __GUI_WIDGETS_SEQTEXT___SEQ_TEXT_GEOMETRY__HPP
#define __GUI_WIDGETS_SEQTEXT___SEQ_TEXT_GEOMETRY__HPP

/*  $Id: seq_text_geometry.hpp 30100 2014-04-08 10:56:52Z bollin $
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
 * Authors:  Colleen Bollin
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>

#include <gui/opengl/glbitmapfont.hpp>
#include <gui/opengl/glpane.hpp>

#include <gui/widgets/gl/ievent_handler.hpp>

#include <gui/widgets/seq_text/seq_text_defs.hpp>
#include <util/range_coll.hpp>
#include <gui/objutils/object_index.hpp>

// for codon rendering
#include <objects/seqfeat/Cdregion.hpp>
#include <objmgr/feat_ci.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class ISeqTextGeometry

class ISeqTextGeometry
{
public:
    virtual ~ISeqTextGeometry() { }

    virtual void        STG_Redraw() = 0;
    virtual void        STG_OnChanged() = 0;
    virtual TSeqPos     STG_GetSequenceByWindow(int x, int y) = 0;
    virtual TModelPoint STG_GetModelPointBySourcePos(TSeqPos z) = 0;
    virtual TModelPoint STG_GetModelPointBySequencePos(TSeqPos z, bool *found_in_source) = 0;
    virtual TSeqPos     STG_GetSourcePosBySequencePos (TSeqPos z, bool *found_in_source) = 0;
    virtual void        STG_GetVisibleRange (TSeqPos& seq_start, TSeqPos &seq_stop) = 0;
    virtual void        STG_GetLineInfo(TSeqPos &chars_in_line, TSeqPos &lines_in_pane) = 0;
    virtual int         STG_GetLinesInPane () = 0;

    virtual objects::CScope& STG_GetScope() = 0;
    virtual const objects::CSeq_loc* STG_GetDataSourceLoc() = 0;
    virtual TModelUnit  STG_GetTextHeight() = 0;
    virtual void        STG_ReportMouseOverPos(TSeqPos pos) = 0;
    virtual bool        STG_IsPointInRightMargin(int x, int y) = 0;
    virtual void        STG_RenderSelectedFeature(const objects::CSeq_feat& feat, const objects::CSeq_loc& mapped_loc) = 0;
    virtual void        STG_RenderMouseOverFeature(const objects::CSeq_feat& feat) = 0;
    virtual vector<CConstRef<objects::CSeq_feat> >  STG_GetFeaturesAtPosition(TSeqPos pos) = 0;
    virtual void        STG_SetSubtypesForFeature (CSeqTextDefs::TSubtypeVector &subtypes, const objects::CSeq_loc& loc, int subtype, TSeqPos start_offset, TSeqPos stop_offset) = 0;
    virtual void        STG_RenderFeatureExtras(const objects::CMappedFeat& feat) = 0;

    virtual void        STG_SetDefaultCursor() = 0;
    virtual void        STG_SetResizeCursor() = 0;
};


////////////////////////////////////////////////////////////////////////////////
/// class ISequenceTextGraphHost

class ISequenceTextGraphHost
{
public:
    virtual ~ISequenceTextGraphHost() { }
    virtual void STGH_GetSelectedFeatureSubtypes(CSeqTextDefs::TSubtypeVector &subtypes) = 0;
    virtual void STGH_GetMouseOverFeatureSubtypes(CSeqTextDefs::TSubtypeVector &subtypes) = 0;
};

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_SEQTEXT___SEQ_TEXT_GEOMETRY__HPP
