/*  $Id: seq_text_display_choice_list.cpp 25551 2012-04-06 15:55:27Z katargir $
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
 *    Implements a wxChoice populated with options for feature display in the
 *    Sequence Text Viewer
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_text/seq_text_display_choice_list.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CSeqTextDisplayChoiceList::CSeqTextDisplayChoiceList(wxWindow* parent) :
    wxChoice (parent, wxID_PREFERENCES)
{
    Append (_("None"));
    Append (_("None"));
    Append (_("All"));
    Append (_("Selected"));
    Append (_("MouseOver"));

    SetSelection (0);
}

CSeqTextPaneConfig::EFeatureDisplayType CSeqTextDisplayChoiceList::GetDisplayType() const
{
    int sel = GetSelection();
    if (sel == 0) {
        return CSeqTextPaneConfig::eNone;
    } else if (sel == 1) {
        return CSeqTextPaneConfig::eAll;
    } else if (sel == 2) {
        return CSeqTextPaneConfig::eSelected;
    } else if (sel == 3) {
        return CSeqTextPaneConfig::eMouseOver;
    } else {
        return CSeqTextPaneConfig::eNone;
    }
}


void CSeqTextDisplayChoiceList::SetDisplayType(CSeqTextPaneConfig::EFeatureDisplayType disp)
{
    if (disp == CSeqTextPaneConfig::eNone) {
        SetSelection (0);
    } else if (disp == CSeqTextPaneConfig::eAll) {
        SetSelection (1);
    } else if (disp == CSeqTextPaneConfig::eSelected) {
        SetSelection (2);
    } else if (disp == CSeqTextPaneConfig::eMouseOver) {
        SetSelection (3);
    } else {
        SetSelection (0);
    }
}


END_NCBI_SCOPE
