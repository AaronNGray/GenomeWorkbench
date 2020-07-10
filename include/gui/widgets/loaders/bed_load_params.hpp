#ifndef GUI_WIDGETS___LOADERS___BED_LOAD_PARAMS__HPP
#define GUI_WIDGETS___LOADERS___BED_LOAD_PARAMS__HPP

/*  $Id: bed_load_params.hpp 28583 2013-08-02 18:59:27Z katargir $
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

#include <gui/widgets/loaders/map_assembly_params.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CBedLoadParams class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CBedLoadParams: public IRegSettings
{    
    friend class CBedParamsPanel;
public:
    /// Default constructor for CBedLoadParams
    CBedLoadParams();

    /// Copy constructor for CBedLoadParams
    CBedLoadParams(const CBedLoadParams& data);

    /// Destructor for CBedLoadParams
    ~CBedLoadParams();

    /// Assignment operator for CBedLoadParams
    void operator=(const CBedLoadParams& data);

    /// Equality operator for CBedLoadParams
    bool operator==(const CBedLoadParams& data) const;

    /// Copy function for CBedLoadParams
    void Copy(const CBedLoadParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    ////@begin CBedLoadParams member function declarations
    int GetNumErrors() const { return m_NumErrors ; }
    void SetNumErrors(int value) { m_NumErrors = value ; }

    CMapAssemblyParams GetMapAssembly() const { return m_MapAssembly ; }
    void SetMapAssembly(CMapAssemblyParams value) { m_MapAssembly = value ; }

////@end CBedLoadParams member function declarations

////@begin CBedLoadParams member variables
    int m_NumErrors;
private:
    CMapAssemblyParams m_MapAssembly;
////@end CBedLoadParams member variables

private:
    string m_RegPath;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___BED_LOAD_PARAMS__HPP
