#ifndef GUI_OBJUTILS___ALIGNMENT_CONF__HPP
#define GUI_OBJUTILS___ALIGNMENT_CONF__HPP

/* $Id: alignment_conf.hpp 41774 2018-09-28 21:16:42Z evgeniev $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
  */


#include <gui/gui.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/utils/rgba_color.hpp>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CAlignmentConfig: the data structure holding alignment rendering parameters

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAlignmentConfig : public CObject
{
public:
    enum ELabelPosition {
        ePos_Above,   ///< above the rendered bar
        ePos_Side,    ///< always on 5' side
        ePos_NoLabel  ///< no label      
    };

    enum ECompactMode {
        eNormal,        ///< no compact mode
        eCompact,       ///< compact mode
        eExtremeCompact ///< extreme compact mode
    };

    enum EHideSraAlignments {
        eHide_None          = 0x0,
        eHide_Duplicates    = 0x1,
        eHide_BadReads      = 0x2,
        eHide_Both          = 0x3
    };

    enum EUnalignedTailsMode {
        eTails_Hide,
        eTails_ShowGlyph,
        eTails_ShowSequence
    };

    CAlignmentConfig()
        : m_LabelPos(ePos_Side)
        , m_UnalignedTailsMode(eTails_ShowGlyph)
        , m_BarHeight(6)
        , m_ShowLabel(true)
        , m_ShowIdenticalBases(true)
        , m_ShowUnalignedTailsForTrans2GenomicAln(true)
        , m_CompactMode(eNormal)
        , m_Dirty(false)
    {}

public:
    /// @name Colors.
    /// @{
    CRgbaColor  m_BG;
    CRgbaColor  m_FG;
    CRgbaColor  m_Label;
    CRgbaColor  m_Sequence;
    CRgbaColor  m_SeqMismatch;
    CRgbaColor  m_TailColor;
    CRgbaColor  m_Insertion;
    CRgbaColor  m_Gap;
    CRgbaColor  m_Intron;
    CRgbaColor  m_NonConsensus;
    CRgbaColor 	m_SmearColorMin;
    CRgbaColor 	m_SmearColorMax;
    CRgbaColor 	m_UnalignedFG;
    CRgbaColor 	m_UnalignedSequence;
    /// @}

    CGlTextureFont   m_LabelFont;
    CGlTextureFont   m_SeqFont;
    
    ELabelPosition  m_LabelPos;

    EUnalignedTailsMode  m_UnalignedTailsMode;

    int         m_BarHeight;
    bool        m_ShowLabel;
    bool        m_ShowIdenticalBases;
    bool        m_ShowUnalignedTailsForTrans2GenomicAln;

    /// Alignment layout compact mode.
    /// It is a run-time setting depending on the number 
    /// of total alignments shown on screen. 
    ECompactMode    m_CompactMode;

    /// Dirty flag indicates any unsaved changes
    mutable bool    m_Dirty;
};


///////////////////////////////////////////////////////////////////////////////
/// CMatePairConfig: the data structure holding alignment rendering parameters

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CMatePairConfig : public CObject
{
public:


    CMatePairConfig() 
        : m_ShowLabel(true)
        , m_Dirty(false)
    {}

    /// @name Colors.
    /// @{
    CRgbaColor m_BG;
    CRgbaColor m_FGDistance;
    CRgbaColor m_FGLink;
    CRgbaColor m_FGNo;
    CRgbaColor m_FGNonUnique;
    CRgbaColor m_FGOrientation;
    CRgbaColor m_FGCoAlign;
    CRgbaColor m_FGContraAlign;
    CRgbaColor m_Label;
    CRgbaColor m_SeqDistance;
    CRgbaColor m_SeqMismatchDistance;
    CRgbaColor m_SeqMismatchNo;
    CRgbaColor m_SeqMismatchNonUnique;
    CRgbaColor m_SeqMismatchOrientation;
    CRgbaColor m_SeqNo;
    CRgbaColor m_SeqNonUnique;
    CRgbaColor m_SeqOrientation;

    //CRgbaColor m_SeqCoAlign;
    //CRgbaColor m_SeqContraAlign;
    //CRgbaColor m_SeqMismatchCoAlign;
    //CRgbaColor m_SeqMismatchContraAlign;
    /// @}

    CGlTextureFont  m_LabelFont;
    bool            m_ShowLabel;

    /// Dirty flag indicates any unsaved changes
    mutable bool    m_Dirty;
};

END_NCBI_SCOPE

#endif // GUI_OBJUTILS___ALIGNMENT_CONF__HPP
