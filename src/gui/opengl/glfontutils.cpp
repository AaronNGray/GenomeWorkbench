/*  $Id: glfontutils.cpp 23906 2011-06-21 13:33:59Z wuliangs $
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
 * Authors:  Yury Voronov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/opengl/glfontutils.hpp>

BEGIN_NCBI_SCOPE

void TextOutAligned(
    CGlBitmapFont& font,
    TModelUnit x, TModelUnit y, const char* text, IGlFont::TAlign align,
    TModelUnit scale_x, TModelUnit scale_y
){
    TModelUnit ras_x = 0;
    TModelUnit ras_y = 0;

    TModelUnit text_width = font.TextWidth( text ) *scale_x;
    TModelUnit text_height = font.TextHeight() *scale_y;

    switch( font.GetFontRotate() ){
    case CGlBitmapFont::eFontRotate_0:

        if( (align & IGlFont::eAlign_HorizMask) == IGlFont::eAlign_Left ){
            ras_x = x;

        } else if( (align & IGlFont::eAlign_HorizMask) == IGlFont::eAlign_Right ){
            // right justify
            ras_x = x - text_width;

        } else  { // center
            ras_x = x - text_width *0.5;
        }

        if( (align & IGlFont::eAlign_VertMask) == IGlFont::eAlign_Bottom ){
            ras_y = y;

        } else if( (align & IGlFont::eAlign_VertMask) == IGlFont::eAlign_Top ){
            // skip down enough to keep our text inside
            ras_y = y - text_height;

        } else {
            // if nothing given or eAlign_Center is specified - center
            // vertically
            ras_y = y - text_height *0.5;
        }
        break;

    case CGlBitmapFont::eFontRotate_90:
        // eAlign_Right or eAlign_Center are applied to horz position only if
        // eAlign_Left is not specifed
        if( (align & IGlFont::eAlign_HorizMask) == IGlFont::eAlign_Left ){
            ras_y = y;

        } else if( (align & IGlFont::eAlign_HorizMask) == IGlFont::eAlign_Right ){
            // right justify
            ras_y = y - text_width;

        } else { // center
            ras_y = y - text_width *0.5;
        }

        if( (align & IGlFont::eAlign_VertMask) == IGlFont::eAlign_Bottom ){
            // skip down enough to keep our text inside
            ras_x = x - text_height;

        } else if( (align & IGlFont::eAlign_VertMask) == IGlFont::eAlign_Top ){
            ras_x = x;

        } else {
            // if nothing given or eAlign_Center is specified - center
            // vertically
            ras_x = x - text_height * 0.5;
        }
        break;

    case CGlBitmapFont::eFontRotate_180:
        // eAlign_Right or eAlign_Center are applied to horz position only if
        // eAlign_Left is not specifed
        if( (align & IGlFont::eAlign_HorizMask) == IGlFont::eAlign_Left ){
            ras_x = x; // + w;

        } else if ((align & IGlFont::eAlign_HorizMask) == IGlFont::eAlign_Right)  {
            // right justify
            ras_x = x + text_width;
        } else  { // center
            //ras_x = x + w - (w - text_wid) * 0.5;
            ras_x = x + text_width *0.5;
        }

        if( (align & IGlFont::eAlign_VertMask) == IGlFont::eAlign_Bottom ){
            ras_y = y;

        } else if( (align & IGlFont::eAlign_VertMask) == IGlFont::eAlign_Top ){
            // skip down enough to keep our text inside
            ras_y = y - text_height;

        } else {
            // if nothing given or eAlign_Center is specified - center
            // vertically
            ras_y = y - text_height *0.5;
        }
        break;

    case CGlBitmapFont::eFontRotate_270:
        // eAlign_Right or eAlign_Center are applied to horz position only if
        // eAlign_Left is not specifed
        if( (align & IGlFont::eAlign_HorizMask) == IGlFont::eAlign_Left){
            ras_y = y;

        } else if( (align & IGlFont::eAlign_HorizMask) == IGlFont::eAlign_Right ){
            // right justify
            ras_y = y + text_width;

        } else { // center
            ras_y = y + text_width *0.5;
        }

        if( (align & IGlFont::eAlign_VertMask) == IGlFont::eAlign_Bottom ){
            ras_x = x;

        } else if( (align & IGlFont::eAlign_VertMask) == IGlFont::eAlign_Top ){
            // skip down enough to keep our text inside
            ras_x = x - text_height;

        } else {
            // if nothing given or eAlign_Center is specified - center
            // vertically
            ras_x = x - text_height *0.5;
        }
        break;

    // these are not handled.  Is this intentional?
    case CGlBitmapFont::eFontRotate_90_Base:
    case CGlBitmapFont::eFontRotate_180_Base:
    case CGlBitmapFont::eFontRotate_270_Cap:
    default:
        break;
    }


    font.TextOut( ras_x, ras_y, text );
}

END_NCBI_SCOPE
