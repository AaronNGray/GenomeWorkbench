#ifndef __glutbitmap_h__
#define __glutbitmap_h__

/* Copyright (c) Mark J. Kilgard, 1994, 1998. */

/* This program is freely distributable without licensing fees
   and is provided without guarantee or warrantee expressed or
   implied. This program is -not- in the public domain. */

#include <ncbiconf.h>
#ifdef NCBI_OS_MSWIN
#  include <windows.h>
#endif

#include <gui/opengl.h>


typedef struct {
  GLsizei width;
  GLsizei height;
  GLfloat xorig;
  GLfloat yorig;
  GLfloat advance;
  const GLubyte *bitmap;
} BitmapCharRec, *BitmapCharPtr;

typedef struct {
  const char *name;
  int num_chars;
  int first;
  const BitmapCharRec* const * ch;
} BitmapFontRec, *BitmapFontPtr;

#ifdef __cplusplus
extern "C" {
#endif

extern const BitmapFontRec ncbi_clean_6;
extern const BitmapFontRec ncbi_clean_8;
extern const BitmapFontRec ncbi_clean_10;
extern const BitmapFontRec ncbi_clean_12;
extern const BitmapFontRec ncbi_clean_13;
extern const BitmapFontRec ncbi_clean_14;
extern const BitmapFontRec ncbi_clean_15;
extern const BitmapFontRec ncbi_clean_16;

extern const BitmapFontRec ncbi_courier_6;
extern const BitmapFontRec ncbi_courier_8;
extern const BitmapFontRec ncbi_courier_10;
extern const BitmapFontRec ncbi_courier_12;
extern const BitmapFontRec ncbi_courier_14;
extern const BitmapFontRec ncbi_courier_18;
extern const BitmapFontRec ncbi_courier_20;
extern const BitmapFontRec ncbi_courier_24;

extern const BitmapFontRec ncbi_fixed_6;
extern const BitmapFontRec ncbi_fixed_8;
extern const BitmapFontRec ncbi_fixed_10;
extern const BitmapFontRec ncbi_fixed_12;
extern const BitmapFontRec ncbi_fixed_14;
extern const BitmapFontRec ncbi_fixed_18;
extern const BitmapFontRec ncbi_fixed_20;

extern const BitmapFontRec ncbi_helvetica_6;
extern const BitmapFontRec ncbi_helvetica_8;
extern const BitmapFontRec ncbi_helvetica_10;
extern const BitmapFontRec ncbi_helvetica_12;
extern const BitmapFontRec ncbi_helvetica_14;
extern const BitmapFontRec ncbi_helvetica_18;
extern const BitmapFontRec ncbi_helvetica_20;
extern const BitmapFontRec ncbi_helvetica_24;

extern const BitmapFontRec ncbi_lucida_6;
extern const BitmapFontRec ncbi_lucida_8;
extern const BitmapFontRec ncbi_lucida_10;
extern const BitmapFontRec ncbi_lucida_12;
extern const BitmapFontRec ncbi_lucida_14;
extern const BitmapFontRec ncbi_lucida_18;
extern const BitmapFontRec ncbi_lucida_20;
extern const BitmapFontRec ncbi_lucida_24;

extern const BitmapFontRec ncbi_times_6;
extern const BitmapFontRec ncbi_times_8;
extern const BitmapFontRec ncbi_times_10;
extern const BitmapFontRec ncbi_times_12;
extern const BitmapFontRec ncbi_times_14;
extern const BitmapFontRec ncbi_times_18;
extern const BitmapFontRec ncbi_times_20;
extern const BitmapFontRec ncbi_times_24;
    
#ifdef __cplusplus
}
#endif

#endif /* __glutbitmap_h__ */
