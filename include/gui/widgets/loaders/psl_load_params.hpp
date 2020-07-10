#ifndef GUI_WIDGETS___LOADERS___PSL_LOAD_PARAMS__HPP
#define GUI_WIDGETS___LOADERS___PSL_LOAD_PARAMS__HPP

/*  $Id: psl_load_params.hpp 44439 2019-12-18 19:51:52Z katargir $
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
 * CPslLoadParams class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CPslLoadParams: public IRegSettings
{    
    friend class CPslParamsPanel;
public:
    /// Default constructor for CPslLoadParams
    CPslLoadParams();

    /// Copy constructor for CPslLoadParams
    CPslLoadParams(const CPslLoadParams& data);

    /// Destructor for CPslLoadParams
    ~CPslLoadParams();

    /// Assignment operator for CPslLoadParams
    void operator=(const CPslLoadParams& data);

    /// Equality operator for CPslLoadParams
    bool operator==(const CPslLoadParams& data) const;

    /// Copy function for CPslLoadParams
    void Copy(const CPslLoadParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

////@begin CPslLoadParams member function declarations
    CMapAssemblyParams GetMapAssembly() const { return m_MapAssembly ; }
    void SetMapAssembly(CMapAssemblyParams value) { m_MapAssembly = value ; }

////@end CPslLoadParams member function declarations

////@begin CPslLoadParams member variables
private:
    CMapAssemblyParams m_MapAssembly;
////@end CPslLoadParams member variables

private:
    string m_RegPath;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___PSL_LOAD_PARAMS__HPP
