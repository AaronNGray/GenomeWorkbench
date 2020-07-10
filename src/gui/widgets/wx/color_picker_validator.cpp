/*  $Id: color_picker_validator.cpp 25496 2012-03-27 20:41:38Z kuznets $
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

#include <ncbi_pch.hpp>

#include <gui/utils/rgba_color.hpp>
#include <gui/widgets/wx/color_picker_validator.hpp>

#include <wx/clrpicker.h>
#include <gui/widgets/wx/color_picker.hpp>

BEGIN_NCBI_SCOPE

CColorPickerValidator::CColorPickerValidator(wxColor* val_ptr)
    : m_pwxColor(val_ptr), m_pCRgbaColor(0)
{
}

CColorPickerValidator::CColorPickerValidator(CRgbaColor* val_ptr)
    : m_pwxColor(0), m_pCRgbaColor(val_ptr)
{
}

CColorPickerValidator::CColorPickerValidator(const CColorPickerValidator& val)
    : wxValidator()
{
    CopyValidator(val);
}

bool CColorPickerValidator::CopyValidator(const CColorPickerValidator& val)
{
    wxValidator::Copy(val);
    m_pwxColor = val.m_pwxColor;
    m_pCRgbaColor = val.m_pCRgbaColor;
    return true;
}

// Called to transfer data to the window
bool CColorPickerValidator::TransferToWindow()
{
    if( !CheckValidator() )
        return false;

    CColorPicker *control = dynamic_cast<CColorPicker *>(m_validatorWindow);

    if( m_pwxColor )    {
        control->SetColour(*m_pwxColor);
    } else if (m_pCRgbaColor) {
        wxColor color;
        color.Set(m_pCRgbaColor->GetRedUC(),
                  m_pCRgbaColor->GetGreenUC(),
                  m_pCRgbaColor->GetBlueUC(),
                  m_pCRgbaColor->GetAlphaUC());
        control->SetColour(color);
    }

    return true;
}

// Called to transfer data to the window
bool CColorPickerValidator::TransferFromWindow()
{
    if( !CheckValidator() )
        return false;

    CColorPicker *control = dynamic_cast<CColorPicker *>(m_validatorWindow);

    if (m_pwxColor)
        *m_pwxColor = control->GetColour();
    else if (m_pCRgbaColor) {
        wxColor color = control->GetColour();
        m_pCRgbaColor->Set(color.Red(), color.Green(), color.Blue(), color.Alpha());
    }

    return true;
}

bool CColorPickerValidator::CheckValidator() const
{
    wxCHECK_MSG( m_validatorWindow, false,
                 wxT("No window associated with validator") );
    wxCHECK_MSG( m_validatorWindow->IsKindOf(CLASSINFO(CColorPicker)), false,
                 wxT("CColorPickerValidator is only for CColorPicker's") );

    return true;
}

END_NCBI_SCOPE
