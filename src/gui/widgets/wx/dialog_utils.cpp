/*  $Id: dialog_utils.cpp 17671 2008-08-30 18:30:30Z dicuccio $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>


#include <gui/widgets/wx/dialog_utils.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/opengl/glbitmapfont.hpp>


#include <wx/ctrlsub.h>


BEGIN_NCBI_SCOPE


void  Init_wxControl(wxControlWithItems& control, const vector<string>& values)
{
    for( size_t i = 0;  i < values.size();  i++ )  {
        wxString wx_s(ToWxString(values[i]));
        control.Append(wx_s);
    }
}


wxString GlFontSizeTo_wxString(const CGlBitmapFont& font)
{
    CGlBitmapFont::EFontSize size = font.GetFontSize();
    string str = CGlBitmapFont::SizeToString(size);
    return ToWxString(str);
}


wxString GlFontFaceTo_wxString(const CGlBitmapFont& font)
{
    CGlBitmapFont::EFontFace face = font.GetFontFace();
    string str = CGlBitmapFont::FaceToString(face);
    return ToWxString(str);
}


void  wxStringsToGlFont(CGlBitmapFont& font, const wxString& wx_face, const wxString& wx_size)
{
    string s_face = ToStdString(wx_face);
    CGlBitmapFont::EFontFace face = CGlBitmapFont::FaceFromString(s_face);
    font.SetFontFace(face);

    string s_size = ToStdString(wx_size);
    CGlBitmapFont::EFontSize size = CGlBitmapFont::SizeFromString(s_size);
    font.SetFontSize(size);
}


END_NCBI_SCOPE
