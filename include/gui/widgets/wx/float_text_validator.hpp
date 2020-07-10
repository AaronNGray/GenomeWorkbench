#ifndef GUI_WIDGETS_WX___FLOAT_TEXT_VALIDATOR_H
#define GUI_WIDGETS_WX___FLOAT_TEXT_VALIDATOR_H

/*  $Id: float_text_validator.hpp 25478 2012-03-27 14:54:26Z kuznets $
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

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <wx/valtext.h>
#include <wx/textctrl.h>


BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CFloatTextValidator
    : public wxTextValidator
{
public:
    CFloatTextValidator(
        double* val_ptr,
        double minVal = -numeric_limits<double>::infinity(),
        double maxVal = numeric_limits<double>::infinity(),
        const char* out_format = "%.3g"
    );
    CFloatTextValidator( double* val_ptr, const char* out_format );
    CFloatTextValidator(const CFloatTextValidator& val);
    virtual ~CFloatTextValidator(){}

    virtual wxObject *Clone() const { return new CFloatTextValidator(*this); }
    bool Copy(const CFloatTextValidator& val);

    void SetFormat( const char* out_format ){ m_OutputFormat = out_format ? out_format : "%.3g"; }

    virtual bool Validate(wxWindow *parent);

    virtual bool TransferToWindow();

    virtual bool TransferFromWindow();

protected:
    bool CheckValidator() const
    {
        wxCHECK_MSG( m_validatorWindow, false,
                     wxT("No window associated with validator") );
        wxCHECK_MSG( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)), false,
                     wxT("wxTextValidator is only for wxTextCtrl's") );

        return true;
    }
private:
    void Init();
    CFloatTextValidator& operator=(const CFloatTextValidator&);

protected:
    string  m_OutputFormat;
    double* m_ValPtr;
    double  m_MinVal;
    double  m_MaxVal;
};


END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___FLOAT_TEXT_VALIDATOR_H

