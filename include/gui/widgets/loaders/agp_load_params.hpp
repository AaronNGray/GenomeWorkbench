#ifndef GUI_WIDGETS___LOADERS___AGP_LOAD_PARAMS__HPP
#define GUI_WIDGETS___LOADERS___AGP_LOAD_PARAMS__HPP

/*  $Id: agp_load_params.hpp 33803 2015-09-17 16:18:33Z katargir $
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
 * Authors:  Roman Katargin
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/objutils/reg_settings.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

/*!
 * CAgpLoadParams class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CAgpLoadParams: public IRegSettings
{
    friend class CAgpLoadPage;
public:
    /// Default constructor for CAgpLoadParams
    CAgpLoadParams();

    /// Copy constructor for CAgpLoadParams
    CAgpLoadParams(const CAgpLoadParams& data);

    /// Destructor for CAgpLoadParams
    ~CAgpLoadParams();

    /// Assignment operator for CAgpLoadParams
    void operator=(const CAgpLoadParams& data);

    /// Equality operator for CAgpLoadParams
    bool operator==(const CAgpLoadParams& data) const;

    /// Copy function for CAgpLoadParams
    void Copy(const CAgpLoadParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

////@begin CAgpLoadParams member function declarations
    wxString GetFastaFile() const { return m_FastaFile ; }
    void SetFastaFile(wxString value) { m_FastaFile = value ; }

    int GetParseIDs() const { return m_ParseIDs ; }
    void SetParseIDs(int value) { m_ParseIDs = value ; }

    bool GetSetGapInfo() const { return m_SetGapInfo ; }
    void SetSetGapInfo(bool value) { m_SetGapInfo = value ; }

////@end CAgpLoadParams member function declarations

////@begin CAgpLoadParams member variables
private:
    wxString m_FastaFile;
    int m_ParseIDs;
    bool m_SetGapInfo;
////@end CAgpLoadParams member variables

private:
    string m_RegPath;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___AGP_LOAD_PARAMS__HPP
