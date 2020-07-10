#ifndef GUI_OPENGL___GLDRAWSCALE__HPP
#define GUI_OPENGL___GLDRAWSCALE__HPP

/*  $Id: gldrawscale.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
* Author: Philip Johnson
*
* File Description: 'gldrawscale' -- contains class to nicely draw a scale
* on screen (ie. sequence coordinates).
*
* ---------------------------------------------------------------------------
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui.hpp>
#include <gui/opengl/gltypes.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class IGlFont;


//class CGlDrawScale implements a routine to nicely draw a scale (numbers +
//tickmarks) for a generic coordinate system.  Intervals between marked
//numbers are normalized and, depending on the options supplied at
//construction, the numbers are abbreviated using the SI symbols (ie, 'k'
//== 10^3).

class NCBI_GUIOPENGL_EXPORT CGlDrawScale {
public:
    class CCoordConverter {
    public:
        virtual ~CCoordConverter(void) {};
        virtual int ToGl (int) const = 0;
    };

    enum EAbbrevType {
        eNoAbbrev,
        eCommas,
        eUseSISymbols,
        eUseScientificNotation
    };

    CGlDrawScale(IGlFont &font, EAbbrevType useAbbrev);

    //PRE : conversion routine to be used when going from user coordinates
    //to modelview coordinates
    //POST: conversion routine set
    void SetConverter(CCoordConverter *cc) {
        m_CoordConverter = cc;
    };

    //PRE : width in pixels; left-most user coordinate, right-most user
    //coordinate
    //POST: if converter not set, scale drawn in the following box: left,
    //-0.5, right, [font-height]; if it is set, scale drawn in: cnv(left),
    //-0.5, cnv(right), [font-height]
    void Draw(int width, int left, int right) const;

private:
    unsigned int x_Exp10(unsigned int x) const;
    void x_Normalize(int &num) const;
    string x_GenerateLabel(int num, EAbbrevType type) const;
    unsigned int x_CalcMaxLabelSize(int left, int right,
                                    EAbbrevType abbrev) const;

    IGlFont &m_Font;
    EAbbrevType m_UseAbbrev;
    CCoordConverter *m_CoordConverter;

    TModelUnit m_MaxDigitWidth;
    TModelUnit m_MaxSIPrefixWidth;

    static const char* sm_SISymbols;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GLDRAWSCALE__HPP
