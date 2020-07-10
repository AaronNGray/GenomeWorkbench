#ifndef GUI_WIDGETS___LOADERS___LBLAST_LOAD_PARAMS__HPP
#define GUI_WIDGETS___LOADERS___LBLAST_LOAD_PARAMS__HPP

/*  $Id: lblast_load_params.hpp 33742 2015-09-11 18:55:29Z katargir $
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
 * CLBLASTLoadParams class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CLBLASTLoadParams: public IRegSettings
{    
    friend class CLBLASTParamsPanel;
public:
    /// Default constructor for CLBLASTLoadParams
    CLBLASTLoadParams();

    /// Copy constructor for CLBLASTLoadParams
    CLBLASTLoadParams(const CLBLASTLoadParams& data);

    /// Destructor for CLBLASTLoadParams
    ~CLBLASTLoadParams();

    /// Assignment operator for CLBLASTLoadParams
    void operator=(const CLBLASTLoadParams& data);

    /// Equality operator for CLBLASTLoadParams
    bool operator==(const CLBLASTLoadParams& data) const;

    /// Copy function for CLBLASTLoadParams
    void Copy(const CLBLASTLoadParams& data);

    /// Initialises member variables
    void Init();

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

////@begin CLBLASTLoadParams member function declarations
    wxString GetNucDB() const { return m_NucDB ; }
    void SetNucDB(wxString value) { m_NucDB = value ; }

    bool GetCreateSeqs() const { return m_CreateSeqs ; }
    void SetCreateSeqs(bool value) { m_CreateSeqs = value ; }

    wxString GetProtDB() const { return m_ProtDB ; }
    void SetProtDB(wxString value) { m_ProtDB = value ; }

    bool GetProtSeqs() const { return m_ProtSeqs ; }
    void SetProtSeqs(bool value) { m_ProtSeqs = value ; }

////@end CLBLASTLoadParams member function declarations

////@begin CLBLASTLoadParams member variables
    bool m_CreateSeqs;
private:
    wxString m_NucDB;
    wxString m_ProtDB;
    bool m_ProtSeqs;
////@end CLBLASTLoadParams member variables

    string m_RegPath;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___LBLAST_LOAD_PARAMS__HPP
