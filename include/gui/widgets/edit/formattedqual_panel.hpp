/*  $Id: formattedqual_panel.hpp 41013 2018-05-09 21:07:35Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */
#ifndef _CFORMATTEDQUALPANEL_H_
#define _CFORMATTEDQUALPANEL_H_

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Gb_qual.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>
#include <wx/hyperlink.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE


class CQualListItemPanel : public wxPanel
{
public:
    virtual void OnChildChange(wxWindow* wnd) {};
    virtual void NotifyParentOfChange();
    static void RemoveRepresentedQuals (objects::CSeq_feat& feat, const string& qual_name);
    virtual void GetDimensions (int *width, int *height) { if (width != NULL) *width = -1; if (height != NULL) *height = -1; };

protected:
    virtual void x_AdjustForControl (CQualListItemPanel* panel, wxHyperlinkCtrl* itemHyperlinkCtrl);

    int m_TotalHeight;
    int m_TotalWidth;
    int m_ScrollRate;
    int m_MinHeight;
    int m_MinWidth;
    int m_NumRows;
    bool m_AllowUpdates;
};


inline
void CQualListItemPanel::x_AdjustForControl (CQualListItemPanel* panel, wxHyperlinkCtrl* itemHyperlinkCtrl)
{
    int height, width;
    panel->GetDimensions(&width, &height);
    if (width < 0 || height < 0) {
	panel->GetSize(&width, &height);
    }
    m_TotalHeight += height;
    if (height > 0 && (m_ScrollRate == 0 || height < m_ScrollRate)) {
	m_ScrollRate = height;
    }
    if (height > m_MinHeight) {
	m_MinHeight = height;
    }
    int d_width, d_height;
    int scroll_width = 20;
    itemHyperlinkCtrl->GetSize(&d_width, &d_height);
    int new_width = width + d_width + scroll_width;
    if (new_width > m_TotalWidth) {
	m_TotalWidth = new_width;
    }
    m_NumRows++;
}




inline
void CQualListItemPanel::NotifyParentOfChange()
{
    wxWindow* parent = this->GetParent();

    CQualListItemPanel* listpanel = dynamic_cast<CQualListItemPanel*>(parent);

    while (parent && !listpanel) {
        parent = parent->GetParent();
        listpanel = dynamic_cast<CQualListItemPanel*>(parent);
    }

    if (!listpanel) {
        return;
    }
    listpanel->OnChildChange((wxWindow*)this);
}


inline
void CQualListItemPanel::RemoveRepresentedQuals (objects::CSeq_feat& feat, const string& qual_name)
{
    objects::CSeq_feat::TQual::iterator qual = feat.SetQual().begin();
    while (qual != feat.SetQual().end()) {
        if ((*qual)->IsSetQual() && NStr::EqualNocase((*qual)->GetQual(), qual_name)) {
            qual = feat.SetQual().erase(qual);
        } else {
            qual++;
        }
    }
}


/*!
 * CFormattedQualPanel class declaration
 */

class CFormattedQualPanel: public CQualListItemPanel
{    
public:

    virtual void SetValue(string val) {};
    virtual string GetValue() { return ""; };
    virtual void OnChildChange(wxWindow* wnd) {};
    virtual void GetDimensions (int *width, int *height) { if (width != NULL) *width = -1; if (height != NULL) *height = -1; };
};


END_NCBI_SCOPE

#endif
// _CFORMATTEDQUALPANEL_H_
