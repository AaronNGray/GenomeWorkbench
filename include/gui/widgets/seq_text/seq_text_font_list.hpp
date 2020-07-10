#error don't include me
#ifndef GUI_WIDGETS_SEQ_TEXT___SEQTEXT_FONT_LIST___HPP
#define GUI_WIDGETS_SEQ_TEXT___SEQTEXT_FONT_LIST___HPP

/*  $Id: seq_text_font_list.hpp 15308 2007-11-06 17:02:35Z dicuccio $
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
 *    Choice box that lists fonts for display in the Sequence Text Viewer
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbimisc.hpp>

#include <objmgr/scope.hpp>
#include <gui/widgets/fl/choice.hpp>
#include <gui/widgets/seq_text/seq_text_ds.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_SEQTEXT_EXPORT CSeqTextFontList
    : public CChoice
{
public:
    CSeqTextFontList(int,int,int,int,const char * = 0);
    int GetFontSize();

    int SetFontSize(int font_size);

private:
    int available_font_sizes[8];
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_TEXT___SEQTEXT_FONT_LIST___HPP
