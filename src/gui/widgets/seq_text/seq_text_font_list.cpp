/*  $Id: seq_text_font_list.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
 *    Implements an Fl_Choice populated with options for fonts in the
 *    Sequence Text Viewer
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_text/seq_text_font_list.hpp>

#include <stdio.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


const int kNumFontSizes = 8;

CSeqTextFontList::CSeqTextFontList(int x, int y,int w, int h, const char *label) :
    CChoice (x, y, w, h, label)
{
    int i;
    char str[4];

    // initialize font choices
    available_font_sizes[0] = 6;
    available_font_sizes[1] = 8;
    available_font_sizes[2] = 10;
    available_font_sizes[3] = 12;
    available_font_sizes[4] = 14;
    available_font_sizes[5] = 18;
    available_font_sizes[6] = 20;
    available_font_sizes[7] = 24;

    for (i = 0; i < kNumFontSizes; i++) {
        sprintf (str, "%d", available_font_sizes[i]);
        add (str);
    }
    value (3);
}


int CSeqTextFontList::GetFontSize()
{
    int sel = value();
    if (sel >= 0 && sel < kNumFontSizes) {
        return available_font_sizes[sel];
    }
    else
    {
        value (3);
        return 12;
    }
}


int CSeqTextFontList::SetFontSize(int font_size)
{
    int i;
    for (i = 0; i < kNumFontSizes; i++) {
        if (available_font_sizes[i] == font_size) {
            value (i);
            return font_size;
        }
    }
    value (3);
    return 12;
}


END_NCBI_SCOPE
