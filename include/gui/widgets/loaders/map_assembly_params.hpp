#ifndef GUI_WIDGETS___LOADERS___MAP_ASSEMBLY_PARAMS__HPP
#define GUI_WIDGETS___LOADERS___MAP_ASSEMBLY_PARAMS__HPP

/*  $Id: map_assembly_params.hpp 44756 2020-03-05 18:44:36Z shkeda $
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

#include <gui/objutils/registry.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CMapAssemblyParams class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CMapAssemblyParams
{
    friend class CAssemblySelPanel;
public:
    CMapAssemblyParams();

    CMapAssemblyParams(const CMapAssemblyParams& data);

    ~CMapAssemblyParams();

    void SaveAsn(objects::CUser_field& user_field) const;
    void LoadAsn(objects::CUser_field& user_field);

    void SaveSettings(CRegistryWriteView& view) const;
    void LoadSettings(CRegistryReadView& view);

    void operator=(const CMapAssemblyParams& data);

    bool operator==(const CMapAssemblyParams& data) const;

    void Copy(const CMapAssemblyParams& data);

    void Init();

////@begin CMapAssemblyParams member function declarations
    string GetAssemblyAcc() const { return m_AssemblyAcc ; }
    void SetAssemblyAcc(string value) { m_AssemblyAcc = value ; }

    string GetAssemblyDesc() const { return m_AssemblyDesc ; }
    void SetAssemblyDesc(string value) { m_AssemblyDesc = value ; }

    string GetAssemblyName() const { return m_AssemblyName ; }
    void SetAssemblyName(string value) { m_AssemblyName = value ; }

    string GetSearchTerm() const { return m_SearchTerm ; }
    void SetSearchTerm(string value) { m_SearchTerm = value ; }

    bool GetUseMapping() const { return m_UseMapping ; }
    void SetUseMapping(bool value) { m_UseMapping = value ; }

////@end CMapAssemblyParams member function declarations

////@begin CMapAssemblyParams member variables
private:
    string m_AssemblyAcc;
    string m_AssemblyDesc;
    string m_AssemblyName;
    string m_SearchTerm;
    bool m_UseMapping;
////@end CMapAssemblyParams member variables
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___MAP_ASSEMBLY_PARAMS__HPP
