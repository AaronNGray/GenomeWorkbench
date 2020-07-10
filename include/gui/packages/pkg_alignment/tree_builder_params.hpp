#ifndef PKG_ALIGNMENT___TREE_BUILDER_PARAMS__HPP
#define PKG_ALIGNMENT___TREE_BUILDER_PARAMS__HPP

/*  $Id: tree_builder_params.hpp 37337 2016-12-23 21:28:48Z katargir $
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
 * Authors:  Roman Katargin, Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

/*!
 * CTreeBuilderParams class declaration
 */

class CTreeBuilderParams: public IRegSettings
{
    friend class CTreeBuilderPanel;
    friend class CTreeBuilderTool;
    
public:
    CTreeBuilderParams();

    CTreeBuilderParams(const CTreeBuilderParams& data);

    ~CTreeBuilderParams();

    void operator=(const CTreeBuilderParams& data);

    bool operator==(const CTreeBuilderParams& data) const;

    void Copy(const CTreeBuilderParams& data);

    void Init();

/// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const SConstScopedObject& GetObject() const { return m_Object; }
    SConstScopedObject&  SetObject() { return m_Object; }

    wxString GetDistanceMethod() const { return m_DistanceMethod ; }
    void SetDistanceMethod(wxString value) { m_DistanceMethod = value ; }

    wxString GetConstructMethod() const { return m_ConstructMethod ; }
    void SetConstructMethod(wxString value) { m_ConstructMethod = value ; }

    wxString GetLeafLabels() const { return m_LeafLabels ; }
    void SetLeafLabels(wxString value) { m_LeafLabels = value ; }
    

////@begin CTreeBuilderParams member variables
private:
    wxString m_DistanceMethod;
    wxString m_ConstructMethod;
    wxString m_LeafLabels;    
    
////@end CTreeBuilderParams member variables
    string m_RegPath;
    SConstScopedObject m_Object;
};

/* @} */

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___TREE_BUILDER_PARAMS__HPP
