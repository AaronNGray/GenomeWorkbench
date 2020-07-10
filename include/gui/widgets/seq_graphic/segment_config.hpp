#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_MAP_CONFIG__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_MAP_CONFIG__HPP

/*  $Id: segment_config.hpp 36548 2016-10-06 18:53:00Z shkeda $
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
 * Authors:  Vlad Lebedev, Liangshou Wu, Victor Joukov
 *
 * File Description:
 *   CSegmentGlyph -- utility class to layout sequence segments and
 *                    hold CSeq_id_Handle objects
 */


#include <corelib/ncbiobj.hpp>
#include <gui/opengl/gltexturefont.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///   CSegmentConfig
///
class CSegmentConfig : public CObject
{
public:
    enum ESequenceQuality {
        eGap = 0,
        eFinished,
        eWgs,
        eOther,
        eDraft,
        eDefault = eOther
    };

    enum ELabelPosition {
        ePos_Above,   ///< above the rendered bar
        ePos_Inside,  ///< inside the rendered bar
        ePos_Side,    ///< always on 5' side
        ePos_NoLabel  ///< no label      
    };

    typedef ESequenceQuality TSegMapQuality;

    CSegmentConfig()
        : m_BarHeight(10)
        , m_LabelPos(ePos_Inside)
        {}

    CRgbaColor  m_BG;
    CRgbaColor  m_FGExc;
    CRgbaColor  m_FGInc_F;
    CRgbaColor  m_FGInc_D;
    CRgbaColor  m_FGInc_W;
    CRgbaColor  m_FGInc_U;
    CRgbaColor  m_FGGap;
    CRgbaColor  m_Label;
    CRgbaColor  m_SelLabel;

    CGlTextureFont   m_LabelFont;
    int             m_BarHeight;
    ELabelPosition  m_LabelPos;
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_MAP_CONFIG__HPP
