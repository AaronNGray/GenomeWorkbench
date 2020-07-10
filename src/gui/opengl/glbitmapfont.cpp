/*  $Id: glbitmapfont.cpp 43431 2019-06-28 15:50:46Z katargir $
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
 * Authors:  Mike DiCuccio, Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include "glutbitmap.h"

#include <gui/opengl/glbitmapfont.hpp>
#include <gui/opengl/gldlist.hpp>
#include <gui/opengl/glutils.hpp>

#include <util/static_map.hpp>

#include <math.h>
#include <stdio.h>


///
/// Standard non-rotated fonts; the explicit sc_foo_ptr business works
/// around a bug in ICC 9, which otherwise fails to populate sc_Fonts
/// correctly. :-/
///

#define NCBI_FONT(x) \
    static const BitmapFontRec* const sc_##x##_ptr = &x

// clean fonts
NCBI_FONT(ncbi_clean_6);
NCBI_FONT(ncbi_clean_8);
NCBI_FONT(ncbi_clean_10);
NCBI_FONT(ncbi_clean_12);
NCBI_FONT(ncbi_clean_13);
NCBI_FONT(ncbi_clean_14);
NCBI_FONT(ncbi_clean_15);
NCBI_FONT(ncbi_clean_16);

// courier fonts
NCBI_FONT(ncbi_courier_6);
NCBI_FONT(ncbi_courier_8);
NCBI_FONT(ncbi_courier_10);
NCBI_FONT(ncbi_courier_12);
NCBI_FONT(ncbi_courier_14);
NCBI_FONT(ncbi_courier_18);
NCBI_FONT(ncbi_courier_20);
NCBI_FONT(ncbi_courier_24);

// fixed size fonts
NCBI_FONT(ncbi_fixed_8);
NCBI_FONT(ncbi_fixed_10);
NCBI_FONT(ncbi_fixed_12);
NCBI_FONT(ncbi_fixed_14);
NCBI_FONT(ncbi_fixed_18);
NCBI_FONT(ncbi_fixed_20);

// helvetica fonts
NCBI_FONT(ncbi_helvetica_6);
NCBI_FONT(ncbi_helvetica_8);
NCBI_FONT(ncbi_helvetica_10);
NCBI_FONT(ncbi_helvetica_12);
NCBI_FONT(ncbi_helvetica_14);
NCBI_FONT(ncbi_helvetica_18);
NCBI_FONT(ncbi_helvetica_20);
NCBI_FONT(ncbi_helvetica_24);

// lucida fonts
NCBI_FONT(ncbi_lucida_6);
NCBI_FONT(ncbi_lucida_8);
NCBI_FONT(ncbi_lucida_10);
NCBI_FONT(ncbi_lucida_12);
NCBI_FONT(ncbi_lucida_14);
NCBI_FONT(ncbi_lucida_18);
NCBI_FONT(ncbi_lucida_20);
NCBI_FONT(ncbi_lucida_24);

// times fonts
NCBI_FONT(ncbi_times_6);
NCBI_FONT(ncbi_times_8);
NCBI_FONT(ncbi_times_10);
NCBI_FONT(ncbi_times_12);
NCBI_FONT(ncbi_times_14);
NCBI_FONT(ncbi_times_18);
NCBI_FONT(ncbi_times_20);
NCBI_FONT(ncbi_times_24);


BEGIN_NCBI_SCOPE

typedef SStaticPair<CGlBitmapFont::EFont, const BitmapFontRec *> TFontPair;
static const TFontPair sc_Fonts[] = {
    { CGlBitmapFont::eBitmap5x7,   sc_ncbi_fixed_8_ptr },
    { CGlBitmapFont::eBitmap8x13,  sc_ncbi_fixed_12_ptr },
    { CGlBitmapFont::eBitmap9x15,  sc_ncbi_fixed_14_ptr },
    { CGlBitmapFont::eBitmap10x20, sc_ncbi_fixed_18_ptr },
    { CGlBitmapFont::eBitmap12x24, sc_ncbi_fixed_20_ptr },

    { CGlBitmapFont::eHelvetica6,  sc_ncbi_helvetica_6_ptr },
    { CGlBitmapFont::eHelvetica8,  sc_ncbi_helvetica_8_ptr },
    { CGlBitmapFont::eHelvetica10, sc_ncbi_helvetica_10_ptr },
    { CGlBitmapFont::eHelvetica12, sc_ncbi_helvetica_12_ptr },
    { CGlBitmapFont::eHelvetica14, sc_ncbi_helvetica_14_ptr },
    { CGlBitmapFont::eHelvetica18, sc_ncbi_helvetica_18_ptr },
    { CGlBitmapFont::eHelvetica20, sc_ncbi_helvetica_20_ptr },
    { CGlBitmapFont::eHelvetica24, sc_ncbi_helvetica_24_ptr },

    { CGlBitmapFont::eLucida6,  sc_ncbi_lucida_6_ptr },
    { CGlBitmapFont::eLucida8,  sc_ncbi_lucida_8_ptr },
    { CGlBitmapFont::eLucida10, sc_ncbi_lucida_10_ptr },
    { CGlBitmapFont::eLucida12, sc_ncbi_lucida_12_ptr },
    { CGlBitmapFont::eLucida14, sc_ncbi_lucida_14_ptr },
    { CGlBitmapFont::eLucida18, sc_ncbi_lucida_18_ptr },
    { CGlBitmapFont::eLucida20, sc_ncbi_lucida_20_ptr },
    { CGlBitmapFont::eLucida24, sc_ncbi_lucida_24_ptr },

    { CGlBitmapFont::eCourier6,  sc_ncbi_courier_6_ptr },
    { CGlBitmapFont::eCourier8,  sc_ncbi_courier_8_ptr },
    { CGlBitmapFont::eCourier10, sc_ncbi_courier_10_ptr },
    { CGlBitmapFont::eCourier12, sc_ncbi_courier_12_ptr },
    { CGlBitmapFont::eCourier14, sc_ncbi_courier_14_ptr },
    { CGlBitmapFont::eCourier18, sc_ncbi_courier_18_ptr },
    { CGlBitmapFont::eCourier20, sc_ncbi_courier_20_ptr },
    { CGlBitmapFont::eCourier24, sc_ncbi_courier_24_ptr },

    { CGlBitmapFont::eClean6,  sc_ncbi_clean_6_ptr },
    { CGlBitmapFont::eClean8,  sc_ncbi_clean_8_ptr },
    { CGlBitmapFont::eClean10, sc_ncbi_clean_10_ptr },
    { CGlBitmapFont::eClean12, sc_ncbi_clean_12_ptr },
    { CGlBitmapFont::eClean14, sc_ncbi_clean_13_ptr },
    { CGlBitmapFont::eClean18, sc_ncbi_clean_14_ptr },
    { CGlBitmapFont::eClean20, sc_ncbi_clean_15_ptr },
    { CGlBitmapFont::eClean24, sc_ncbi_clean_16_ptr },

    { CGlBitmapFont::eFixed8,  sc_ncbi_fixed_8_ptr },
    { CGlBitmapFont::eFixed10, sc_ncbi_fixed_10_ptr },
    { CGlBitmapFont::eFixed12, sc_ncbi_fixed_12_ptr },
    { CGlBitmapFont::eFixed14, sc_ncbi_fixed_14_ptr },
    { CGlBitmapFont::eFixed18, sc_ncbi_fixed_18_ptr },
    { CGlBitmapFont::eFixed20, sc_ncbi_fixed_20_ptr },

    { CGlBitmapFont::eTimesRoman6,  sc_ncbi_times_6_ptr },
    { CGlBitmapFont::eTimesRoman8,  sc_ncbi_times_8_ptr },
    { CGlBitmapFont::eTimesRoman10, sc_ncbi_times_10_ptr },
    { CGlBitmapFont::eTimesRoman12, sc_ncbi_times_12_ptr },
    { CGlBitmapFont::eTimesRoman14, sc_ncbi_times_14_ptr },
    { CGlBitmapFont::eTimesRoman18, sc_ncbi_times_18_ptr },
    { CGlBitmapFont::eTimesRoman20, sc_ncbi_times_20_ptr },
    { CGlBitmapFont::eTimesRoman24, sc_ncbi_times_24_ptr }

};
typedef CStaticPairArrayMap<CGlBitmapFont::EFont, const BitmapFontRec *>
    TFontMap;
DEFINE_STATIC_ARRAY_MAP(TFontMap, sc_FontMap, sc_Fonts);


///
/// rotated versions of the above
/// we store a cache of these that is generated on the fly
/// these fonts are only created as needed, and the data structures will
/// only be created once per session
///

struct SRotateKey
{
    /// non-rotated font
    const BitmapFontRec * font;

    /// rotation type
    CGlBitmapFont::EFontRotate rot;
};

struct SRotateKeyLess
{
    bool operator() (const SRotateKey& rk1, const SRotateKey& rk2) const
    {
        if (rk1.font < rk2.font) {
            return true;
        }
        if (rk2.font < rk1.font) {
            return false;
        }
        return rk1.rot < rk2.rot;
    }
};


typedef map<SRotateKey, const BitmapFontRec *, SRotateKeyLess>
    TRotatedFontCache;
static TRotatedFontCache s_RotatedCache;

struct SRotatedFontCachePurger
{
    ~SRotatedFontCachePurger()
    {
        NON_CONST_ITERATE (TRotatedFontCache, iter, s_RotatedCache) {
            //BitmapFontPtr font = iter->second;
            free((void*)iter->second->name);
            for (int i = 0;  i < iter->second->num_chars;  ++i) {
                if (iter->second->ch[i]) {
                    delete [] iter->second->ch[i]->bitmap;
                    delete [] iter->second->ch[i];
                }
            }
            delete [] iter->second->ch;
            delete iter->second;
        }
    }
};
static SRotatedFontCachePurger s_FontCachePurger;



static inline
const BitmapFontRec * const s_FindFontPtr(CGlBitmapFont::EFont font)
{
    TFontMap::const_iterator iter = sc_FontMap.lower_bound(font);
    if (iter->first == font) {
        // exact match
        return iter->second;
    } else if (iter->first != font) {
        // probably off by size
        CGlBitmapFont::EFontFace face =
            static_cast<CGlBitmapFont::EFontFace>
            (font & 0xff);
        CGlBitmapFont::EFontFace this_face =
            static_cast<CGlBitmapFont::EFontFace>
            (iter->first & 0xff);
        if (this_face == face) {
            // faces match, return font
            return iter->second;
        } else {
            // faces don't match
            // we need to jump forward or back to get the right face
            TFontMap::const_iterator back    = iter;
            TFontMap::const_iterator forward = iter;
            for ( ;
                  back != sc_FontMap.begin()  &&
                  forward != sc_FontMap.end();  ) {
                if (back != sc_FontMap.begin()) {
                    this_face = static_cast<CGlBitmapFont::EFontFace>
                        (back->first & 0xff);
                    if (this_face == face) {
                        return back->second;
                    }
                    --back;
                }
                if (forward != sc_FontMap.end()) {
                    this_face =
                        static_cast<CGlBitmapFont::EFontFace>
                        (forward->first & 0xff);
                    if (this_face == face) {
                        return forward->second;
                    }
                    ++forward;
                }
            }
        }
    }

    NCBI_THROW(CException, eUnknown,
               "OpenGL bitmap font font not found");
}


static const BitmapCharRec *s_GetCharPtr(const BitmapFontRec *font_ptr, int c)
{
    if (c  <   font_ptr->first ||
        c  >=  font_ptr->first + font_ptr->num_chars) {
        return NULL;
    }
    return font_ptr->ch[c - font_ptr->first];
}


static TModelUnit s_GetCapHeight(const BitmapFontRec *font_ptr)
{
    const BitmapCharRec *ch = s_GetCharPtr(font_ptr, 'H');
    return ch? ch->height : 0.0;
}


///
/// create a rotated font
///
/// this is limited to creating fonts roted 90, 180, or 270 degrees
///
static const BitmapFontRec * s_CreateRotatedFont
    (const BitmapFontRec * source_font, CGlBitmapFont::EFontRotate rot)
{
    _ASSERT(source_font);
    if (rot == CGlBitmapFont::eFontRotate_0) {
        return source_font;
    }

    /// first, determine the baseline
    /// the baseline may shift depending on our rotation
    /// this ias based on the height of a capital 'M'
    TModelUnit cap_ht = s_GetCapHeight(source_font);


    ///
    /// an OpenGL bitmap is stored as a set of GLubytes, with each bit
    /// corresponding to a pixel.  These are packed to end on even byte boundaries
    /// at the end of a scanline.
    ///
    //CNcbiOfstream ostr("c:/temp/bitmap.txt");
    BitmapCharRec** chars = new BitmapCharRec*[source_font->num_chars];
    for (int charidx = 0;  charidx < source_font->num_chars;  ++charidx) {
        const BitmapCharRec* orig_ch = source_font->ch[charidx];
        BitmapCharRec* new_char = 0;
        if (orig_ch) {
            int i;
            int j;

            /**
            {{
                int row_len = orig_ch->width / 8 + (orig_ch->width % 8 ? 1 : 0);
                ostr << "0x" << hex << charidx << ": '"
                    << (char)charidx << "': "
                    << orig_ch->width << 'x' << orig_ch->height << ": "
                    << orig_ch->xorig << ',' << orig_ch->yorig << ": "
                    << orig_ch->advance
                    << endl;
                for (j = 0;  j < orig_ch->height;  ++j) {
                    for (i = 0;  i < orig_ch->width;  ++i) {
                        int idx = j * row_len + i/8;
                        int bit = i % 8;
                        if (orig_ch->bitmap[idx] & (0x80 >> bit)) {
                            ostr << '*';
                        } else {
                            ostr << ' ';
                        }
                    }
                    ostr << endl;
                }
                ostr << endl;
            }}
            **/

            switch (rot) {
            case CGlBitmapFont::eFontRotate_90_Cap:
            case CGlBitmapFont::eFontRotate_90_Base:
                {{
                    /// 90 degree rotation tilts the image
                    int row_len = (orig_ch->width + 7) / 8;
                    int new_row_len = (orig_ch->height + 7) / 8;
                    GLubyte* data = new GLubyte[new_row_len * orig_ch->width];
                    memset(data, 0, new_row_len * orig_ch->width * sizeof (GLubyte));
                    
                    new_char = new BitmapCharRec;
                    new_char->width = orig_ch->height;
                    new_char->height = orig_ch->width;
                    new_char->advance = orig_ch->advance;
                    new_char->bitmap = data;
                    
                    for(i = 0; i < new_char->height; i++) { // by rows
                        for(j = 0; j < new_row_len; j++) { // by columns
                            GLubyte *b = data + i * new_row_len + j;
                            // dest bit (j*8 + k, i) <-- (i, H - (j*8 + k) - 1), where H = orig_ch->height
                            // source byte = (i/8, H - j*8 - k - 1), bit offset = i % 8
                            int k; // dest bit offset
                            int max_k = min(8, new_char->width - j*8);
                            for(k = 0; k < max_k; k++){
                                int src_row = orig_ch->height - j * 8 - k - 1;
                                int src_bit = i;
                                int src_byte = src_bit / 8 + row_len * src_row;
                                int src_bit_off = src_bit % 8;
                                if(orig_ch->bitmap[src_byte] & (0x80 >> src_bit_off)){
                                    *b |= (0x80 >> k);
                                }
                            }
                       }
                    }
                    
                    // new bitmap origin corner is original's top-left
                    new_char->yorig = orig_ch->xorig;
                    if(rot == CGlBitmapFont::eFontRotate_90_Cap) {
                        new_char->xorig = -orig_ch->yorig - (GLfloat)cap_ht + orig_ch->height;
                    } else {
                        new_char->xorig = -orig_ch->yorig + orig_ch->height;
                    }
                    
                }}
                break;

            case CGlBitmapFont::eFontRotate_180_Base:
            case CGlBitmapFont::eFontRotate_180_Cap:
                {{
                    int row_len = (orig_ch->width + 7)  / 8;
                    GLubyte* data = new GLubyte[row_len * orig_ch->height];
                    memset(data, 0, row_len * orig_ch->height * sizeof (GLubyte));

                    new_char = new BitmapCharRec;
                    new_char->width = orig_ch->width;
                    new_char->height = orig_ch->height;
                    new_char->bitmap = data;

                    for(i = 0; i < new_char->height; i++) { // by rows
                        for(j = 0; j < row_len; j++) { // by columns
                            GLubyte *b = data + i * row_len + j;
                            // dest bit (j*8 + k, i) <-- (W - (j*8 + k) - 1, H - i - 1), 
                            // where H = orig_ch->height, W = orig_ch->width
                            int k;
                            int max_k = min(8, new_char->width - j*8);
                            for(k = 0; k < max_k; k++){
                                int src_row = orig_ch->height - i - 1;
                                int src_bit = orig_ch->width - j * 8 - k - 1;
                                int src_byte = src_bit / 8 + row_len * src_row;
                                int src_bit_off = src_bit % 8;
                                if(orig_ch->bitmap[src_byte] & (0x80 >> src_bit_off)){
                                    *b |= (0x80 >> k);
                                }
                            }
                        }
                    }
                    
                    // new baseline is the same as old one
                    // new bitmap origin corner is original's top-right
                    if(rot == CGlBitmapFont::eFontRotate_180_Base) {
                        new_char->xorig = -orig_ch->xorig + orig_ch->width;
                        new_char->yorig = -orig_ch->yorig + orig_ch->height;
                    } else {
                        new_char->xorig = -orig_ch->xorig + orig_ch->width;
                        new_char->yorig = -orig_ch->yorig + orig_ch->height - (GLfloat)cap_ht;
                    }
                    new_char->advance = -orig_ch->advance;
                    
                }}
                break;

            case CGlBitmapFont::eFontRotate_270_Base:
            case CGlBitmapFont::eFontRotate_270_Cap:
                {{
                    /// 270 degree rotation is NOT just a transpose of the
                    /// original bitmap image
                    int row_len     = (orig_ch->width + 7) / 8;
                    int new_row_len = (orig_ch->height + 7) / 8;
                    GLubyte* data = new GLubyte[new_row_len * orig_ch->width];
                    new_char = new BitmapCharRec;
                    new_char->width = orig_ch->height;
                    new_char->height = orig_ch->width;
                    new_char->bitmap = data;
                    
                    memset(data, 0, new_row_len * new_char->height * sizeof (GLubyte));

                    for(i = 0; i < new_char->height; i++) { // by rows
                        for(j = 0; j < new_row_len; j++) { // by columns
                            GLubyte *b = data + i * new_row_len + j;
                            // dest bit (j*8 + k, i) <-- (W - i - 1, (j*8 + k)), where W = orig_ch->width
                            // source byte = ((W - i - 1)/8, j*8 + k), bit offset = (W - i - 1) % 8
                            int k; // dest bit offset
                            int max_k = min(8, new_char->width - j*8);
                            for(k = 0; k < max_k; k++){
                                int src_row = j * 8 + k;
                                int src_bit = orig_ch->width - i - 1;
                                int src_byte = src_bit / 8 + row_len * src_row;
                                int src_bit_off = src_bit % 8;
                                if(orig_ch->bitmap[src_byte] & (0x80 >> src_bit_off)){
                                    *b |= (0x80 >> k);
                                }
                            }
                       }
                    }
                    
                    if(rot == CGlBitmapFont::eFontRotate_270_Base) {
                        new_char->xorig = orig_ch->yorig;
                        new_char->yorig = -orig_ch->xorig + orig_ch->width;
                    } else {
                        new_char->xorig = orig_ch->yorig + (GLfloat)cap_ht;
                        new_char->yorig = -orig_ch->xorig + orig_ch->width;
                    }
                    new_char->advance = -orig_ch->advance;
                }}
                break;

            default:
                break;
            }

            /**
            {{
                int row_len = new_char->width / 8 + (new_char->width % 8 ? 1 : 0);
                ostr << "0x" << hex << charidx << ": '"
                    << (char)charidx << "': "
                    << new_char->width << 'x' << new_char->height << ": "
                    << new_char->xorig << ',' << new_char->yorig << ": "
                    << new_char->advance
                    << endl;
                for (j = 0;  j < new_char->height;  ++j) {
                    for (i = 0;  i < new_char->width;  ++i) {
                        int idx = j * row_len + i/8;
                        int bit = i % 8;
                        if (new_char->bitmap[idx] & (0x80 >> bit)) {
                            ostr << '*';
                        } else {
                            ostr << ' ';
                        }
                    }
                    ostr << endl;
                }
                ostr << endl;
            }}
            **/

        }
        chars[charidx] = new_char;
    }

    /// initialize our new bitmap structure
    const char* name = NULL;
    switch (rot) {
    case CGlBitmapFont::eFontRotate_90_Cap:
        name = strdup((string(source_font->name) + "-rot90c").c_str());
        break;

    case CGlBitmapFont::eFontRotate_90_Base:
        name = strdup((string(source_font->name) + "-rot90b").c_str());
        break;

    case CGlBitmapFont::eFontRotate_180_Cap:
        name = strdup((string(source_font->name) + "-rot180c").c_str());
        break;

    case CGlBitmapFont::eFontRotate_180_Base:
        name = strdup((string(source_font->name) + "-rot180b").c_str());
        break;

    case CGlBitmapFont::eFontRotate_270_Cap:
        name = strdup((string(source_font->name) + "-rot270c").c_str());
        break;

    case CGlBitmapFont::eFontRotate_270_Base:
        name = strdup((string(source_font->name) + "-rot270b").c_str());
        break;

    default:
        /// can't happen
        _ASSERT(false);
    }

    BitmapFontPtr new_font = new BitmapFontRec;
    new_font->name = name;
    new_font->num_chars = source_font->num_chars;
    new_font->first = source_font->first;
    new_font->ch = chars;
    return new_font;
}


static inline
const BitmapFontRec * const s_GetFont(CGlBitmapFont::EFont font,
                                      CGlBitmapFont::EFontRotate rot)
{
    const BitmapFontRec * font_ptr = s_FindFontPtr(font);
    TRotatedFontCache::iterator iter;

    switch (rot) {
    case CGlBitmapFont::eFontRotate_0:
        return font_ptr;

    default:
    case CGlBitmapFont::eFontRotate_90_Base:
    case CGlBitmapFont::eFontRotate_90_Cap:
    case CGlBitmapFont::eFontRotate_180_Base:
    case CGlBitmapFont::eFontRotate_180_Cap:
    case CGlBitmapFont::eFontRotate_270_Base:
    case CGlBitmapFont::eFontRotate_270_Cap:
        {{
            SRotateKey rk;
            rk.font = font_ptr;
            rk.rot = rot;
            iter = s_RotatedCache.find(rk);
            if (iter == s_RotatedCache.end()) {
                const BitmapFontRec * ptr = s_CreateRotatedFont(font_ptr, rot);
                s_RotatedCache[rk] = ptr;
                iter = s_RotatedCache.find(rk);
            }            
        }}
        break;
    }

    return iter->second;
}

// return the width of a single character
// this is really the *advance* of a character
static TModelUnit s_GetCharAdvance(char c, const BitmapFontRec* font_ptr)
{
    const BitmapCharRec *ch = s_GetCharPtr(font_ptr, c);
    return ch ? ch->advance : 0.0;
}

static TModelUnit s_GetCharWidth(char c, const BitmapFontRec* font_ptr)
{
    const BitmapCharRec *ch = s_GetCharPtr(font_ptr, c);
    if (ch) {
        return ch->width - ch->xorig;
    }
    return 0.0;
}

static TModelUnit s_GetCharBitmapWidth(char c, const BitmapFontRec* font_ptr)
{
    const BitmapCharRec *ch = s_GetCharPtr(font_ptr, c);
    if (ch) {
        return ch->width;
    }
    return 0.0;
}

static TModelUnit s_GetCharDescender(char c, const BitmapFontRec* font_ptr)
{
    const BitmapCharRec *ch = s_GetCharPtr(font_ptr, c);
    return ch ? ch->yorig : 0.0;
}

static TModelUnit s_GetFontDescender(const BitmapFontRec* font_ptr)
{
    float desc = 0.0;
    for (int charidx = 0;  charidx < font_ptr->num_chars;  ++charidx) {
        const BitmapCharRec* ch = font_ptr->ch[charidx];
        desc = max(desc, ch->yorig);
    }
    return TModelUnit(desc);
}



CGlBitmapFont::CGlBitmapFont()
    : m_Font(eBitmap8x13)
    , m_FontRotate(eFontRotate_0)
    , m_Condensed(false)
{
    SetFont(eBitmap8x13);
}


CGlBitmapFont::CGlBitmapFont(EFont font)
    : m_Font(font)
    , m_FontRotate(eFontRotate_0)
    , m_Condensed(false)
{
    SetFont(font);
}


CGlBitmapFont::~CGlBitmapFont()
{
}


CGlBitmapFont& CGlBitmapFont::operator=(const CGlBitmapFont& proto)
{
    m_Font = proto.m_Font;
    m_FontRotate = proto.m_FontRotate;
    m_Condensed = proto.m_Condensed;
    return *this;
}


void CGlBitmapFont::SetFontFace(EFontFace face)
{
    EFont font = static_cast<EFont>((m_Font & 0xff) | face);
    SetFont(font);
}


CGlBitmapFont::EFontFace   CGlBitmapFont::GetFontFace()    const
{
    return (EFontFace)(m_Font & 0xFFFFFF00);
}


void CGlBitmapFont::SetFontSize(EFontSize size)
{
    EFont font = static_cast<EFont>((m_Font & ~0xff) | size);
    SetFont(font);
}


CGlBitmapFont::EFontSize   CGlBitmapFont::GetFontSize() const
{
    return (EFontSize)(m_Font & 0xFF);
}


CGlBitmapFont::EFontRotate CGlBitmapFont::GetFontRotate() const
{
    return m_FontRotate;
}


void CGlBitmapFont::SetFontRotate(EFontRotate rot)
{
    m_FontRotate = rot;
}


void CGlBitmapFont::SetCondensed(bool condensed)
{
    m_Condensed = condensed;
}


bool CGlBitmapFont::IsCondensed(void) const
{
    return m_Condensed;
}


void CGlBitmapFont::SetFont(EFont font)
{
    if (font != m_Font) {
        m_Font = font;
    }
}


CGlBitmapFont::EFont   CGlBitmapFont::GetFont() const
{
    return m_Font;
}


typedef SStaticPair<CGlBitmapFont::EFontFace, const char*>    TFaceElem;
static const TFaceElem sc_FaceTagArray[] = {
    { CGlBitmapFont::eFontFace_Bitmap,      "Bitmap" },
    { CGlBitmapFont::eFontFace_Helvetica,   "Helvetica" },
    { CGlBitmapFont::eFontFace_Lucida,      "Lucida" },
    { CGlBitmapFont::eFontFace_Courier,     "Courier" },
    { CGlBitmapFont::eFontFace_Clean,       "Clean" },
    { CGlBitmapFont::eFontFace_Fixed,       "Fixed" },
    { CGlBitmapFont::eFontFace_TimesRoman,  "Times Roman" }
};
typedef CStaticPairArrayMap<CGlBitmapFont::EFontFace, const char*> TFaceMap;
DEFINE_STATIC_ARRAY_MAP(TFaceMap, sc_FaceMap, sc_FaceTagArray);


typedef SStaticPair<CGlBitmapFont::EFontSize, const char*>    TSizeElem;
static const TSizeElem sc_SizeTagArray[] = {
    { CGlBitmapFont::eFontSize_6,   "6" },
    { CGlBitmapFont::eFontSize_8,   "8" },
    { CGlBitmapFont::eFontSize_10, "10" },
    { CGlBitmapFont::eFontSize_12, "12" },
    { CGlBitmapFont::eFontSize_14, "14" },
    { CGlBitmapFont::eFontSize_18, "18" },
    { CGlBitmapFont::eFontSize_20, "20" },
    { CGlBitmapFont::eFontSize_24, "24" },
};
typedef CStaticPairArrayMap<CGlBitmapFont::EFontSize, const char*> TSizeMap;
DEFINE_STATIC_ARRAY_MAP(TSizeMap, sc_SizeMap, sc_SizeTagArray);


void CGlBitmapFont::GetAllFaces(vector<string>& faces)
{
    faces.clear();
    ITERATE (TFaceMap, iter, sc_FaceMap) {
        if (iter->first != eFontFace_Bitmap)  {
            faces.push_back(iter->second);
        }
    }
}


void     CGlBitmapFont::GetAllSizes(vector<string>& sizes)
{
    sizes.clear();
    ITERATE (TSizeMap, iter, sc_SizeMap) {
        sizes.push_back(iter->second);
    }
}


string CGlBitmapFont::FaceToString(EFontFace face)
{
    TFaceMap::const_iterator iter = sc_FaceMap.find(face);
    if (iter != sc_FaceMap.end()) {
        return iter->second;
    }
    return string();
}


CGlBitmapFont::EFontFace CGlBitmapFont::FaceFromString(const string& str)
{
    ITERATE (TFaceMap, iter, sc_FaceMap) {
        if (str == iter->second) {
            return iter->first;
        }
    }

    return (EFontFace) 0;
}


string CGlBitmapFont::SizeToString(EFontSize size)
{
    TSizeMap::const_iterator iter = sc_SizeMap.find(size);
    if (iter != sc_SizeMap.end()) {
        return iter->second;
    }
    return string();
}


CGlBitmapFont::EFontSize    CGlBitmapFont::SizeFromString(const string& str)
{
    ITERATE (TSizeMap, iter, sc_SizeMap) {
        if (str == iter->second) {
            return iter->first;
        }
    }

    return (EFontSize) 0;
}


CGlBitmapFont::EFontSize    CGlBitmapFont::SizeFromInt(int i)
{
    string s ( NStr::IntToString(i) );
    return SizeFromString(s);
}


string  CGlBitmapFont::ToString() const
{
    string str = FaceToString(GetFontFace());
    str += ",";
    str += SizeToString(GetFontSize());
    return str;
}


void    CGlBitmapFont::FromString(const string& value)
{
    string s_face, s_size;

    bool ok = false;
    if(NStr::SplitInTwo(value, ",", s_face, s_size))    {
        NStr::TruncateSpaces(s_face);
        NStr::TruncateSpaces(s_size);

        EFontFace face = FaceFromString(s_face);
        EFontSize size = SizeFromString(s_size);

        if(face != 0  &&  size != 0)    {
            SetFontFace(face);
            SetFontSize(size);
            ok = true;
        }
    }
    if(! ok)    {
        NCBI_THROW2(CStringException, eConvert, "String cannot be converted to a CGlBitmapFont", 0);
    }
}


const void* CGlBitmapFont::GetFontPtr(const CGlBitmapFont::EFontRotate rot) const
{
    return s_GetFont(m_Font, rot);
}

const void* CGlBitmapFont::GetFontPtr() const
{
    return s_FindFontPtr(m_Font);
}

TModelUnit CGlBitmapFont::GetFontDescender() const
{
    const BitmapFontRec *font_ptr = s_FindFontPtr(m_Font);
    return s_GetFontDescender(font_ptr);
}


template<class TOutputMethod>
void DoTextOut(TOutputMethod& method)
{
    // save a bunch of states
    GLint swapbytes;
    GLint lsbfirst;
    GLint rowlength;
    GLint skiprows;
    GLint skippixels;
    GLint alignment;
    glGetIntegerv(GL_UNPACK_SWAP_BYTES,  &swapbytes);
    glGetIntegerv(GL_UNPACK_LSB_FIRST,   &lsbfirst);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH,  &rowlength);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS,   &skiprows);
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skippixels);
    glGetIntegerv(GL_UNPACK_ALIGNMENT,   &alignment);

    glPixelStorei(GL_UNPACK_SWAP_BYTES,  GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST,   GL_FALSE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,  0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,   0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT,   1);

    // render our text
    method();

    // restore states
    glPixelStorei(GL_UNPACK_SWAP_BYTES, swapbytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, lsbfirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, rowlength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, skiprows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, skippixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
}


class CTextPrinter
{
public:
    CTextPrinter(const char* text, const BitmapFontRec* font_ptr,
                 CGlBitmapFont::EFontRotate rot, bool condensed = false)
        : m_Text(text),
          m_FontPtr(font_ptr),
          m_FontRotate(rot),
          m_Condensed(condensed)
    {
    }


    void operator()()
    {
        GLfloat pos[4];
        glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
        TModelUnit x_adv = 0.0, y_adv = 0.0;
        for (const char* p = m_Text;  p  &&  *p;  ++p) {
            const BitmapCharRec *ch = s_GetCharPtr(m_FontPtr, *p);
            if (ch) {
                TModelUnit  x_orig = ch->xorig, y_orig = ch->yorig;
                switch (m_FontRotate) {
                default:
                case CGlBitmapFont::eFontRotate_0:
                    if(m_Condensed){
                        x_adv = ch->width + 1.0;
                        x_orig = 0.0;
                    } else
                        x_adv = ch->advance;
                    break;

                case CGlBitmapFont::eFontRotate_180_Base:
                case CGlBitmapFont::eFontRotate_180_Cap:
                    if(m_Condensed){
                        x_adv = -ch->width - 1.0;
                        x_orig = ch->width;
                    } else
                        x_adv = ch->advance;
                    break;

                case CGlBitmapFont::eFontRotate_90_Base:
                case CGlBitmapFont::eFontRotate_90_Cap:
                    if(m_Condensed){
                        y_adv = ch->height + 1.0;
                        y_orig = 0.0;
                    } else
                        y_adv = ch->advance;
                    break;

                case CGlBitmapFont::eFontRotate_270_Base:
                case CGlBitmapFont::eFontRotate_270_Cap:
                    if(m_Condensed){
                        y_adv = -ch->height - 1.0;
                        y_orig = ch->height;
                    } else
                        y_adv = ch->advance;
                    break;

                }
                glBitmap(ch->width, ch->height, 
                        (GLfloat)x_orig, (GLfloat)y_orig,
                        (GLfloat)x_adv, (GLfloat)y_adv, ch->bitmap);
            }
        }
    };
protected:
    const char* const m_Text;
    const BitmapFontRec* const m_FontPtr;
    const CGlBitmapFont::EFontRotate m_FontRotate;
    const bool m_Condensed;
};



// TextOut()
// This version performs most of the low-level work.  It simply
// outputs text at a previously determined raster position
void CGlBitmapFont::TextOut(const char* text) const
{
    GLint mode;
    glGetIntegerv(GL_RENDER_MODE, &mode);
    switch (mode) {
    case GL_FEEDBACK:
        {{
            GLfloat pos[4];
            GLfloat col[4];

            // Not sure we should do this (this is the color
            // set after first call to glRasterPos if lighting
            // is enabled, which it may not be.)
            glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
            glGetFloatv(GL_CURRENT_RASTER_COLOR, col);

            const CRgbaColor c(col, 4);

            typedef vector<float> TFList;
            TFList vectext =
                CGlFeedbackFont::EncodeText(pos, c, text, strlen(text));
            ITERATE (TFList, it, vectext) {
                glPassThrough(*it);
            }
        }}
        break;

    default:
        {{
            // first, retrieve our OpenGL font
            CTextPrinter printer(text,
                                 s_GetFont(m_Font, m_FontRotate),
                                 m_FontRotate, m_Condensed);
            DoTextOut(printer);
        }}
        break;
    }
}

class CTextArrayPrinter
{
public:
    CTextArrayPrinter(const char* text,
                      const vector<CRgbaColor*>* colors,
                      TModelUnit x, TModelUnit y, TModelUnit dx, TModelUnit dy,
                      TModelUnit scale_x, TModelUnit scale_y,
                      CGlBitmapFont* font)
        : m_Text(text),
          m_Colors(colors),
          m_X(x),
          m_Y(y),
          m_dX(dx),
          m_dY(dy),
          m_ScaleX(scale_x),
          m_ScaleY(scale_y),
          m_CapHeight(0),
          m_Font(font)
    {
        m_CapHeight = m_Font->TextHeight();
    }

    void operator()()
    {
        const BitmapFontRec * font_ptr
            = s_GetFont(m_Font->GetFont(), m_Font->GetFontRotate());
        TModelUnit k_x = m_ScaleX / 2;
        TModelUnit k_y = m_ScaleY / 2;

        for (const char* p = m_Text;  p  &&  *p;  ++p) {
            const BitmapCharRec *ch = s_GetCharPtr(font_ptr, *p);
            if(ch)  {
                int ind = (int)(p - m_Text);
                TModelUnit pos_x = m_X + ind * m_dX;
                TModelUnit pos_y = m_Y + ind * m_dY;
                TModelUnit off_x;
                TModelUnit off_y;

                off_x = ch->width * k_x; // to current coord system
                off_y = ch->height * k_y; // to current coord system

                if(m_Colors)  {
                    CRgbaColor& c = *(*m_Colors)[ind];
                    glColorC(c);
                }


                switch (m_Font->GetFontRotate()) {
                default:
                case CGlBitmapFont::eFontRotate_0:
                    CTextUtils::SetRasterPosSafe(pos_x - off_x, pos_y);
                    glBitmap(ch->width, ch->height, 0, ch->yorig,
                             ch->advance, 0, ch->bitmap);
                    break;

                case CGlBitmapFont::eFontRotate_90_Base:
                    CTextUtils::SetRasterPosSafe(pos_x - m_CapHeight, pos_y - off_y);
                    glBitmap(ch->width, ch->height, ch->xorig, 0,
                             0, ch->advance, ch->bitmap);
                    break;

                case CGlBitmapFont::eFontRotate_90_Cap:
                    CTextUtils::SetRasterPosSafe(pos_x, pos_y - off_y);
                    glBitmap(ch->width, ch->height, ch->xorig, 0,
                             0, ch->advance, ch->bitmap);
                    break;

                case CGlBitmapFont::eFontRotate_180_Base:
                    CTextUtils::SetRasterPosSafe(pos_x - off_x, pos_y + m_CapHeight);
                    glBitmap(ch->width, ch->height, 0, ch->yorig,
                             ch->advance, 0, ch->bitmap);
                    break;

                case CGlBitmapFont::eFontRotate_180_Cap:
                    CTextUtils::SetRasterPosSafe(pos_x - off_x, pos_y);
                    glBitmap(ch->width, ch->height, 0, ch->yorig,
                             ch->advance, 0, ch->bitmap);
                    break;

                case CGlBitmapFont::eFontRotate_270_Base:
                    CTextUtils::SetRasterPosSafe(pos_x, pos_y + off_y - ch->height);
                    glBitmap(ch->width, ch->height, ch->xorig, 0,
                             0, 0, ch->bitmap);
                    break;

                case CGlBitmapFont::eFontRotate_270_Cap:
                    CTextUtils::SetRasterPosSafe(pos_x + m_CapHeight, pos_y + off_y - ch->height);
                    glBitmap(ch->width, ch->height, ch->xorig, 0,
                             0, 0, ch->bitmap);
                    break;
                }
            }
        }
    };

protected:
    const char* m_Text;
    const vector<CRgbaColor*>* m_Colors;
    TModelUnit  m_X;
    TModelUnit  m_Y;
    TModelUnit  m_dX;
    TModelUnit  m_dY;
    TModelUnit  m_ScaleX;
    TModelUnit  m_ScaleY;
    TModelUnit  m_CapHeight;
    CGlBitmapFont* m_Font;
};


class CFeedbackTextArrayPrinter
{
public:
    CFeedbackTextArrayPrinter(const char* text,
                              TModelUnit x, TModelUnit y, TModelUnit dx, TModelUnit dy,
                              TModelUnit scale_x, TModelUnit scale_y,
                              const BitmapFontRec * const & font_ptr)
        : m_Text(text),
          m_X(x),
          m_Y(y),
          m_dX(dx),
          m_dY(dy),
          m_ScaleX(scale_x),
          m_ScaleY(scale_y),
          m_FontPtr(font_ptr)
    {
    }

    void operator()()
    {
        TModelUnit k_x = m_ScaleX / 2;
        TModelUnit k_y = m_ScaleY / 2;

        for (const char* p = m_Text;  p  &&  *p;  ++p) {
            if (*p  <   m_FontPtr->first ||
                *p  >=  m_FontPtr->first + m_FontPtr->num_chars) {
                continue;
            }
            const BitmapCharRec *ch = m_FontPtr->ch[*p - m_FontPtr->first];
            if (ch ) {
                int ind = (int)(p - m_Text);
                TModelUnit pos_x = m_X + ind * m_dX;
                TModelUnit pos_y = m_Y + ind * m_dY;
                TModelUnit off_x = ch->width * k_x; // to current coord system
                TModelUnit off_y = ch->height * k_y; // to current coord system

                CTextUtils::SetRasterPosSafe(pos_x - off_x, pos_y - off_y);

                GLfloat pos[4];
                GLfloat col[4];

                glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
                glGetFloatv(GL_CURRENT_RASTER_COLOR, col);

                const CRgbaColor c(col, 4);

                typedef vector<float> TFList;
                TFList vectext =
                    CGlFeedbackFont::EncodeText(pos, c, p, 1);
                ITERATE (TFList, it, vectext) {
                    glPassThrough(*it);
                }
            }
        }
    };
protected:
    const char* m_Text;
    TModelUnit  m_X;
    TModelUnit  m_Y;
    TModelUnit  m_dX;
    TModelUnit  m_dY;
    TModelUnit  m_ScaleX;
    TModelUnit  m_ScaleY;
    const BitmapFontRec * const& m_FontPtr;
};


void CGlBitmapFont::ArrayTextOut(TModelUnit x, TModelUnit y, TModelUnit dx, TModelUnit dy,
                                 const char* text,
                                 const vector<CRgbaColor*>* colors,
                                 TModelUnit scale_x, TModelUnit scale_y) const
{
    GLint mode;
    glGetIntegerv(GL_RENDER_MODE, &mode);
    switch (mode) {
    case GL_FEEDBACK:
        {
            // first, retrieve our OpenGL font
            const BitmapFontRec * font_ptr = s_GetFont(m_Font, m_FontRotate);
            CFeedbackTextArrayPrinter printer(text, x, y, dx, dy,
                                              scale_x, scale_y, font_ptr);
            DoTextOut(printer);
        }
        break;
    default:
        {
            CTextArrayPrinter    printer(text, colors,
                                         x, y, dx, dy,
                                         scale_x, scale_y,
                                         const_cast<CGlBitmapFont*>(this));
            DoTextOut(printer);
        }
        break;
    }
}


// This version of TextOut() performs some label alignment in the specified box
void CGlBitmapFont::TextOut(TModelUnit x1, TModelUnit y1, TModelUnit x2, TModelUnit y2,
                            const char* text, TAlign align, ETruncate trunc,
                            TModelUnit scale_x, TModelUnit scale_y) const
{
    if(x2 < x1) {
        swap(x1, x2);
    }
    if(y2 < y1) {
        swap(y1, y2);
    }
    TModelUnit x = x1;
    TModelUnit y = y1;
    TModelUnit w = x2 - x1;
    TModelUnit h = y2 - y1;

    /// truncate as appropriate
    string str;
    switch (trunc) {
    case eTruncate_None:
        str = text;
        break;

    case eTruncate_Empty:
    case eTruncate_Ellipsis:
        {{
            TModelUnit text_width = w;
            switch (GetFontRotate()) {
            case eFontRotate_0:
            case eFontRotate_180_Base:
            case eFontRotate_180_Cap:
                text_width = w;
                break;

            case eFontRotate_90_Base:
            case eFontRotate_90_Cap:
            case eFontRotate_270_Base:
            case eFontRotate_270_Cap:
                text_width = h;
                break;
            }

            x_Truncate(text, text_width, trunc, &str);
        }}
        break;
    }

    ///
    /// determine where our text starts
    ///
    TModelUnit ras_x = 0;
    TModelUnit ras_y = 0;

    EFontRotate rot = GetFontRotate();
    switch (rot) {
    case eFontRotate_0:
        // eAlign_Right or eAlign_Center are applied to horz position only if
        // eAlign_Left is not specifed
        if ((align & eAlign_HorizMask) == eAlign_Left)  {
            ras_x = x;
        } else if ((align & eAlign_HorizMask) == eAlign_Right)  {
            // right justify
            TModelUnit text_wid = TextWidth(str.c_str()) * scale_x;
            ras_x = x + (w - text_wid);
        } else  { // center
            TModelUnit text_wid = TextWidth(str.c_str()) * scale_x;
            ras_x = x + (w - text_wid) * 0.5;
        }

        if ((align & eAlign_VertMask) == eAlign_Bottom)  {
            ras_y = y;
        } else if ((align & eAlign_VertMask) == eAlign_Top) {
            // skip down enough to keep our text inside
            TModelUnit text_ht = TextHeight() * scale_y;
            ras_y = y + (h - text_ht);
        } else {
            // if nothing given or eAlign_Center is specified - center
            // vertically
            TModelUnit text_ht = TextHeight() * scale_y;
            ras_y = y + (h - text_ht) * 0.5;
        }
        break;

    case eFontRotate_90_Base:
    case eFontRotate_90_Cap:
        // eAlign_Right or eAlign_Center are applied to horz position only if
        // eAlign_Left is not specifed
        if ((align & eAlign_HorizMask) == eAlign_Left)  {
            ras_y = y;
        } else if ((align & eAlign_HorizMask) == eAlign_Right)  {
            // right justify
            TModelUnit text_wid = TextWidth(str.c_str()) * scale_y;
            ras_y = y + (h - text_wid);
        } else  { // center
            TModelUnit text_wid = TextWidth(str.c_str()) * scale_y;
            ras_y = y + (h - text_wid) * 0.5;
        }

        // descender is not taken into account
        if ((align & eAlign_VertMask) == eAlign_Bottom)  {
            // skip down enough to keep our text inside
            TModelUnit text_ht = TextHeight() * scale_x;
            if (rot == eFontRotate_90_Base)
                ras_x = x + w;
            else
                ras_x = x + w - text_ht;
        } else if ((align & eAlign_VertMask) == eAlign_Top) {
            if (rot == eFontRotate_90_Base) {
                TModelUnit text_ht = TextHeight() * scale_x;
                ras_x = x - text_ht;
            } else
                ras_x = x;
        } else {
            // if nothing given or eAlign_Center is specified - center
            // vertically
            TModelUnit text_ht = TextHeight() * scale_x;
            if (rot == eFontRotate_90_Base)
                ras_x = x + (w + text_ht) * 0.5;
            else
                ras_x = x + (w - text_ht) * 0.5;
        }
        break;

    case eFontRotate_180_Base:
    case eFontRotate_180_Cap:
        // eAlign_Right or eAlign_Center are applied to horz position only if
        // eAlign_Left is not specifed
        if ((align & eAlign_HorizMask) == eAlign_Left)  {
            ras_x = x + w;
        } else if ((align & eAlign_HorizMask) == eAlign_Right)  {
            // right justify
            TModelUnit text_wid = TextWidth(str.c_str()) * scale_x;
            ras_x = x + text_wid;
        } else  { // center
            TModelUnit text_wid = TextWidth(str.c_str()) * scale_x;
            ras_x = x + w - (w - text_wid) * 0.5;
        }

        if ((align & eAlign_VertMask) == eAlign_Bottom)  {
            if (rot == eFontRotate_180_Base) {
                TModelUnit text_ht = TextHeight() * scale_y;
                ras_y = y - text_ht;
            } else
                ras_y = y;
        } else if ((align & eAlign_VertMask) == eAlign_Top) {
            // skip down enough to keep our text inside
            TModelUnit text_ht = TextHeight() * scale_y;
            if (rot == eFontRotate_180_Base)
                ras_y = y + h;
            else
                ras_y = y + (h - text_ht);
        } else {
            // if nothing given or eAlign_Center is specified - center
            // vertically
            TModelUnit text_ht = TextHeight() * scale_y;
            if (rot == eFontRotate_180_Base)
                ras_y = y + (h + text_ht) * 0.5;
            else
                ras_y = y + (h - text_ht) * 0.5;
        }
        break;

    case eFontRotate_270_Cap:
    case eFontRotate_270_Base:
        // eAlign_Right or eAlign_Center are applied to horz position only if
        // eAlign_Left is not specifed
        if ((align & eAlign_HorizMask) == eAlign_Left)  {
            ras_y = y + h;
        } else if ((align & eAlign_HorizMask) == eAlign_Right)  {
            // right justify
            TModelUnit text_wid = TextWidth(str.c_str()) * scale_y;
            ras_y = y + text_wid;
        } else  { // center
            TModelUnit text_wid = TextWidth(str.c_str()) * scale_y;
            ras_y = y + (h + text_wid) * 0.5f;
        }

        if ((align & eAlign_VertMask) == eAlign_Bottom)  {
            if (rot == eFontRotate_270_Base)
                ras_x = x;
            else {
                TModelUnit text_ht = TextHeight() * scale_y;
                ras_x = x + text_ht;
            }
        } else if ((align & eAlign_VertMask) == eAlign_Top) {
            // skip down enough to keep our text inside
            if (rot == eFontRotate_270_Base) {
                TModelUnit text_ht = TextHeight() * scale_x;
                ras_x = x + (w - text_ht);
            } else
                ras_x = x + w;
        } else {
            // if nothing given or eAlign_Center is specified - center
            // vertically
            TModelUnit text_ht = TextHeight() * scale_x;
            if (rot == eFontRotate_270_Base) {
                ras_x = x + (w - text_ht) * 0.5f;
            } else {
                ras_x = x + (w + text_ht) * 0.5f;
            }
        }
        break;
    }

    TextOut(ras_x, ras_y, str.c_str());
}

void CGlBitmapFont::TextOut(TModelUnit x, TModelUnit y, const char* text) const
{
    CTextUtils::SetRasterPosSafe(x, y);

    TextOut(text);
}

//
// compute the width of a string of characters
// this will count out at most n characters, stopping on a null
// note that the comparison function used for 'n' is 0 or non-zero -
// so passing '-1' for 'n' means 'compute until a null character is found'
// This allows us to avoid calling strlen()
//
TModelUnit CGlBitmapFont::TextWidth(const char* text, int n) const
{
    return GetMetric(eMetric_TextWidth, text, n);
}


/// compute the length of a null-terminated string
TModelUnit CGlBitmapFont::TextWidth(const char* text) const
{
    return GetMetric(eMetric_TextWidth, text);
}


/// compute the height of a string
TModelUnit CGlBitmapFont::TextHeight(void) const
{
    return GetMetric(eMetric_CharHeight);
}


TModelUnit   CGlBitmapFont::GetAdvance(char c) const
{
    const BitmapFontRec * const& font_ptr = s_FindFontPtr(m_Font);
    return m_Condensed? s_GetCharBitmapWidth(c, font_ptr) + 1 : s_GetCharAdvance(c, font_ptr);
}


// generic text metric retrieval
TModelUnit CGlBitmapFont::GetMetric(EMetric metric,
                               const char* text, int text_len) const
{
    // first, retrieve our OpenGL font
    const BitmapFontRec* font_ptr = s_FindFontPtr(m_Font);

    switch (metric) {
    case eMetric_CharHeight:
        // return the height of a capital letter
        {{
            // we first scan the capital letters, this gives us the height
            // of the text without descents but with ascents
            static const char* sc_caps = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            GLsizei max_ht = 0;

            for (const char* p = sc_caps;  p  &&  *p;  ++p) {
                const BitmapCharRec *ch = font_ptr->ch[*p - font_ptr->first];
                if (ch) {
                    GLsizei ht = GLsizei(ch->height - ch->yorig);
                    max_ht     = max(max_ht, ht);
                }
            }
            return max_ht;
        }}

    case eMetric_FullCharHeight:
        // return the maximal height of all available characters
        {{
            // we first scan the capital letters
            // this gives us the height of the text without descents but with
            // ascents
            GLsizei max_ht = 0;
            for (int i = font_ptr->first;  i < font_ptr->num_chars;  ++i) {
                const BitmapCharRec *ch = font_ptr->ch[i];
                if (ch) {
                    GLsizei ht = GLsizei(ch->height - ch->yorig);
                    max_ht     = max(max_ht, ht);
                }
            }
            return max_ht;
        }}

    case eMetric_AvgCharWidth:
        // return the maximal height of all available characters
        {{
            // we first scan the capital letters
            // this gives us the height of the text without descents but with
            // ascents
            GLfloat wid   = 0;
            GLsizei count = 0;
            for (int i = font_ptr->first;  i < font_ptr->num_chars;  ++i) {
                const BitmapCharRec *ch = font_ptr->ch[i];
                if (ch) {
                    wid += ch->advance;
                    ++count;
                }
            }
            return wid / float(count);
        }}

    case eMetric_MaxCharWidth:
        // return the maximal height of all available characters
        {{
            // we first scan the capital letters this gives us the height
            // of the text without descents but with ascents
            GLfloat wid   = 0;
            GLsizei count = 0;
            for (int i = font_ptr->first;  i < font_ptr->num_chars;  ++i) {
                const BitmapCharRec *ch = font_ptr->ch[i];
                if (ch) {
                    wid = max(ch->advance, wid);
                    ++count;
                }
            }
            return wid;
        }}

    case eMetric_TextWidth:
    case eMetric_FullTextWidth:
        // return the width of the text with or without the final advance
        if (text) {
            // iterate over our text
            TModelUnit sum = 0.0;
            const char* p = text;
            for ( ;  text_len  &&  p  &&  *p;  ++p, --text_len) {
                sum += m_Condensed? s_GetCharBitmapWidth(*p, font_ptr) + 1.0 : s_GetCharAdvance(*p, font_ptr);
            }

            // adjust for the trailing character
            if (p  &&  metric == eMetric_TextWidth) {
                --p;
                if(m_Condensed)
                    sum -= 1.0;
                else
                    sum += s_GetCharWidth(*p, font_ptr) - s_GetCharAdvance(*p, font_ptr);
            }
            return sum;
        }
        break;

    case eMetric_Descender:
        // return character descender for a given string
        // use to define the string base line
        if(text) {
            TModelUnit desc = 0.0;
            const char *p = text;
            for( ; text_len && p && *p; ++p, --text_len) {
                desc = max(desc, s_GetCharDescender(*p, font_ptr));
            }
            return desc;
        }
        break;

    default:
        break;
    }

    return 0.0;
}


string CGlBitmapFont::Truncate(const char* text, TModelUnit w, ETruncate trunc) const
{
    string res;
    x_Truncate(text, w, trunc, &res);
    return res;
}


string CGlBitmapFont::Truncate(const string& str, TModelUnit w, ETruncate trunc) const
{
    string res;
    x_Truncate(str.c_str(), w, trunc, &res);
    return res;
}


// Truncate a string for display.  This version will return the maximum
// number of characters that can fit into the given width
int CGlBitmapFont::x_Truncate(const char* text, TModelUnit w, ETruncate trunc,
                              string* str) const
{
    // first, retrieve our OpenGL font
    const BitmapFontRec* font_ptr = s_GetFont(m_Font, m_FontRotate);
    static const char* sc_ellipsis = "...";
    const char* ellip = NULL;

    const char* end_pos = text;
    switch (trunc) {
    case eTruncate_None:
        end_pos = text + strlen(text);
        break;

    case eTruncate_Ellipsis:    {
            // first, do a blunt cut
            int pos = x_Truncate(text, w, eTruncate_Empty);
            end_pos = text + pos;
            if ( !*end_pos ) {
                // we can fit the whole thing
                break;
            }

            // adjust for the ellipsis
            TModelUnit e_wid = TextWidth(sc_ellipsis);
            while (e_wid > 0) {
                ellip = sc_ellipsis;
                e_wid -= s_GetCharWidth(*end_pos, font_ptr);
                if (end_pos > text) {
                    --end_pos;
                } else {
                    break;
                }
            }
        }
        break;

    case eTruncate_Empty:
        for ( ;  end_pos  &&  *end_pos  && w >= 0;  ++end_pos) {
            TModelUnit wid = s_GetCharAdvance(*end_pos, font_ptr);
            if (w - wid < 0) {
                wid = s_GetCharWidth(*end_pos, font_ptr);
            }

            w -= wid;
        }

        if (w < 0) {
            --end_pos;
        }
        break;
    }

    if (end_pos < text) {
        end_pos = text;
    }

    if (str) {
        str->assign(text, end_pos);
        if (ellip) {
            *str += ellip;
        }
    }
    return (int)(end_pos - text);
}


static const  char* kPostfixes[] = { "", " K", " M", " G" };


string CTextUtils::FormatSeparatedNumber(int Number, bool b_postfix)
{
    bool bNegative = Number < 0;
    Number = abs(Number);

    string s_number, s_postfix;
    char cSep = ',';

    int i_postfix = 0;
    if(Number != 0  &&  b_postfix)    {
        int Rest = Number % 1000;
        for( i_postfix = 0; Rest == 0; i_postfix++)  {
            Number = Number / 1000;
            Rest = Number % 1000;
        }
        _ASSERT(i_postfix < 4);
        s_postfix = kPostfixes[i_postfix];
    }

    char sTemplGroup[] = " %.3d";
    sTemplGroup[0] = cSep;
    char sTemplLast[] = "%d";

    char S[20];
    do    {
        int Rest = Number % 1000;
        Number = Number / 1000;
        sprintf(S, Number ? sTemplGroup : sTemplLast, Rest);
        s_number = string(S) + s_number;
    } while(Number);
    s_number += s_postfix;

    if(bNegative) {
        s_number = "-" + s_number;
    }
    return s_number;
}


int CTextUtils::GetCharsCount(int Number)
{
    int MaxDigits = (int) ceil( log10((double) Number)); // decimal digist in number
    int nSep = (MaxDigits - 1) / 3; // number of separators between groups
    return MaxDigits + nSep;
}


TModelUnit  CTextUtils::GetMaxWidth(int max_num, const CGlBitmapFont& font)
{
    double max_dig_w = 0;

    for( char c = '0'; c <= '9'; c++) {
        double char_w = font.GetAdvance(c);
        max_dig_w = max(max_dig_w, char_w);
    }
    double comma_w = font.GetAdvance(',');

    double mod = abs(max_num);
    int digits_count = (int) ceil(log10(mod));
    int commas_count = (digits_count - 1) / 3;

    // length of the longest possible label in pixels (add 2 for separation)
    double w = digits_count * max_dig_w + commas_count * comma_w;
    if(max_num < 0) {
        w += font.GetAdvance('-');
    }
    return w;
}

void CTextUtils::SetRasterPosSafe(TModelUnit x, TModelUnit y)
{
    // Set the current raster position
    glRasterPos2d(x, y);

    // check if the position was valid (if (x,y) projects ont a pixel inside
    // the viewport boundary):
    GLboolean valid;
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &valid);


    // If the raster position is outside of the viewport, nothing will
    // display on screen, even if the text extends from outside the
    // viewport to inside. So instead we do a 'trick' where we
    // set the raster position to a point inside the viewport,
    // then call glBitmap(...) to move the raster position to the 
    // correct position outside of the viewport.

    // Note that this does slow things down if you draw lots of off-screen
    // text, so avoid drawing text that is fully outside the viewing area.
    // (probably because drawing to invalid raster positions has no cost)
    if (!valid) {
        GLdouble modelview_matrix[16];
        GLdouble projection_matrix[16];
        GLint viewport[4];

        glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
        glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
        glGetIntegerv(GL_VIEWPORT, viewport);

        // Get the window coordinates of the current raster position.  Since
        // our raster position is invaid, this will be outside of the viewport
        // bounds.
        GLdouble wx, wy, wz;
        gluProject(x, y, 0.0, 
                   modelview_matrix, projection_matrix, viewport, 
                   &wx, &wy, &wz);

        // Now get the world coordinates of the window position at the center
        // of the viewport. (This will give us a point in world coordinates that
        // we know projects to a valid raster position).
        GLdouble unprojected_center_x = 0;
        GLdouble unprojected_center_y = 0;
        GLdouble unprojected_center_z = 0;        
        GLdouble centerx = viewport[0] + (viewport[2]-viewport[0])/2.0;
        GLdouble centery = viewport[1] + (viewport[3]-viewport[1])/2.0;
        gluUnProject( centerx, centery, 0.0,
                      modelview_matrix, projection_matrix, viewport,
                      &unprojected_center_x, &unprojected_center_y, &unprojected_center_z);

        // Set the current raster position to the center of the viewport
        glRasterPos2d(unprojected_center_x, unprojected_center_y);

        // Get the delta (in raster coordinates) between text position which is
        // outside of the window area and the center of the viewport, then
        // move that distance to give us the correct raster position.
        GLfloat deltax = (GLfloat)(wx-centerx);
        GLfloat deltay = (GLfloat)(wy-centery);
        glBitmap(0, 0, 
                 0.0f, 0.0f, 
                 deltax, deltay, NULL);
    }
}


END_NCBI_SCOPE
