#ifndef GUI_WIDGETS___LOADERS___TEXT_ALIGN_LOAD_PARAMS__HPP
#define GUI_WIDGETS___LOADERS___TEXT_ALIGN_LOAD_PARAMS__HPP

/*  $Id: text_align_load_params.hpp 38443 2017-05-10 15:49:30Z katargir $
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
#include <gui/gui_export.h>

#include <wx/string.h>

#include <gui/objutils/reg_settings.hpp>

BEGIN_NCBI_SCOPE


/*!
 * CTextAlignParams class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CTextAlignParams: public IRegSettings
{    
    friend class CTextAlignParamsPanel;
public:
    CTextAlignParams();

    CTextAlignParams(const CTextAlignParams& data);

    ~CTextAlignParams();

    void operator=(const CTextAlignParams& data);

    bool operator==(const CTextAlignParams& data) const;

    void Copy(const CTextAlignParams& data);

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

////@begin CTextAlignParams member function declarations
    wxString GetBegin() const { return m_Begin ; }
    void SetBegin(wxString value) { m_Begin = value ; }

    wxString GetEnd() const { return m_End ; }
    void SetEnd(wxString value) { m_End = value ; }

    wxString GetMatch() const { return m_Match ; }
    void SetMatch(wxString value) { m_Match = value ; }

    wxString GetMiddle() const { return m_Middle ; }
    void SetMiddle(wxString value) { m_Middle = value ; }

    int GetSeqType() const { return m_SeqType ; }
    void SetSeqType(int value) { m_SeqType = value ; }

    wxString GetUnknown() const { return m_Unknown ; }
    void SetUnknown(wxString value) { m_Unknown = value ; }

////@end CTextAlignParams member function declarations

////@begin CTextAlignParams member variables
private:
    wxString m_Begin;
    wxString m_End;
    wxString m_Match;
    wxString m_Middle;
    int m_SeqType;
    wxString m_Unknown;
////@end CTextAlignParams member variables

private:
    string m_RegPath;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___TEXT_ALIGN_LOAD_PARAMS__HPP
