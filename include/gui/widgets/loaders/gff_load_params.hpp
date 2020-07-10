#ifndef GUI_WIDGETS___LOADERS___GFF_LOAD_PARAMS__HPP
#define GUI_WIDGETS___LOADERS___GFF_LOAD_PARAMS__HPP

/*  $Id: gff_load_params.hpp 38105 2017-03-30 16:02:27Z katargir $
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
#include <util/format_guess.hpp>

#include <gui/widgets/loaders/map_assembly_params.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

/*!
 * CGffLoadParams class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CGffLoadParams: public IRegSettings
{
    friend class CGffParamsPanel;
public:
    /// Default constructor for CGffLoadParams
    CGffLoadParams();

    /// Copy constructor for CGffLoadParams
    CGffLoadParams(const CGffLoadParams& data);

    /// Destructor for CGffLoadParams
    ~CGffLoadParams();

    /// Assignment operator for CGffLoadParams
    void operator=(const CGffLoadParams& data);

    /// Equality operator for CGffLoadParams
    bool operator==(const CGffLoadParams& data) const;

    /// Copy function for CGffLoadParams
    void Copy(const CGffLoadParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

////@begin CGffLoadParams member function declarations
    int GetFileFormat() const { return m_FileFormat ; }
    void SetFileFormat(int value) { m_FileFormat = value ; }

    CMapAssemblyParams GetMapAssembly() const { return m_MapAssembly ; }
    void SetMapAssembly(CMapAssemblyParams value) { m_MapAssembly = value ; }

    wxString GetNameFeatSet() const { return m_NameFeatSet ; }
    void SetNameFeatSet(wxString value) { m_NameFeatSet = value ; }

    int GetParseSeqIds() const { return m_ParseSeqIds ; }
    void SetParseSeqIds(int value) { m_ParseSeqIds = value ; }

////@end CGffLoadParams member function declarations

////@begin CGffLoadParams member variables
private:
    int m_FileFormat;
    CMapAssemblyParams m_MapAssembly;
    wxString m_NameFeatSet;
    int m_ParseSeqIds;
////@end CGffLoadParams member variables

private:
    string m_RegPath;
};

/* @} */

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___GFF_LOAD_PARAMS__HPP
