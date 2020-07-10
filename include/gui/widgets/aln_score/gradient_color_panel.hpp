#ifndef GUI_WIDGETS_ALN_SCORE___GRADIENT_PANEL__HPP
#define GUI_WIDGETS_ALN_SCORE___GRADIENT_PANEL__HPP

/*  $Id: gradient_color_panel.hpp 30858 2014-07-31 14:05:43Z ucko $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/utils/rgba_color.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/panel.h>
#include <wx/clrpicker.h>
#include <wx/stattext.h>

BEGIN_NCBI_SCOPE

class  NCBI_GUIWIDGETS_ALNSCORE_EXPORT CGradientColorPanel : public wxPanel
{
    DECLARE_EVENT_TABLE()
public:
    struct  stParams {
        stParams() : m_Reversable(false) {}
        CRgbaColor  m_FirstColor;
        CRgbaColor  m_MidColor;
        CRgbaColor  m_LastColor;
        bool    m_ThreeColors; // "true" if gradinent based on 3 colors rather then on 2
        int     m_Steps; // number of colors in gradient
        bool    m_Reversable;
        string  m_MinLabel;
        string  m_MidLabel;
        string  m_MaxLabel;
    };

    CGradientColorPanel(const stParams& params, wxWindow* parent, wxWindowID id = wxID_ANY);
    void   GetParams(stParams& params) const;

protected:
    void CreateControls();

    void OnSliderUpdated( wxCommandEvent& event );
    void OnSwapClick( wxCommandEvent& event );
    void OnPaint( wxPaintEvent& event );
    void OnColourPickChanged( wxColourPickerEvent& event );

    stParams m_Params;
    wxStaticText*   m_ItemStepsTxt;
    wxWindow*       m_GradWindow;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_SCORE___GRADIENT_PANEL__HPP
