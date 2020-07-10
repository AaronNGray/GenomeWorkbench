#ifndef PKG_ALIGNMENT___PHY_EXPORT_PARAMS__HPP
#define PKG_ALIGNMENT___PHY_EXPORT_PARAMS__HPP

/*  $Id: phy_export_params.hpp 36053 2016-08-02 14:06:39Z evgeniev $
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
 * Authors:  Vladimir Tereshkov
 */

#include <corelib/ncbistd.hpp>

#include <gui/objutils/reg_settings.hpp>
#include <algo/phy_tree/dist_methods.hpp>


#include <wx/string.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

/*!
 * CPhyExportParams class declaration
 */

class CPhyExportParams: public IRegSettings
{
    friend class CPhyLoadPage;
public:
    /// Default constructor for CPhyExportParams
    CPhyExportParams();

    /// Copy constructor for CPhyExportParams
    CPhyExportParams(const CPhyExportParams& data);

    /// Destructor for CPhyExportParams
    ~CPhyExportParams();

    /// Assignment operator for CPhyExportParams
    void operator=(const CPhyExportParams& data);

    /// Equality operator for CPhyExportParams
    bool operator==(const CPhyExportParams& data) const;

    /// Copy function for CPhyExportParams
    void Copy(const CPhyExportParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

////@begin CPhyExportParams member function declarations    
    const wxString& GetFileName() const { return m_FileName ; } 
    wxString * SetFileName()  { return &m_FileName ; }
    void SetFileName(const wxString &value) { m_FileName = value ; }

    const wxString& GetFileFormat() const { return m_FileFormat; }
    void SetFileFormat(const wxString &value) { m_FileFormat = value; }

    const wxString& GetLabelFormat() const { return m_LabelFormat; }
    void SetLabelFormat(const wxString &value) { m_LabelFormat = value; }

    CBioTreeContainer * GetBioTree()  { return m_pBioTree.GetPointerOrNull() ; }
    void SetBioTree(CBioTreeContainer * value) { m_pBioTree = value ; }

////@end CPhyExportParams member function declarations

////@begin CPhyExportParams member variables
private:
    wxString m_FileName;
    wxString m_FileFormat;
    wxString m_LabelFormat;
    CRef<CBioTreeContainer> m_pBioTree;

////@end CPhyExportParams member variables

private:
    string m_RegPath;
};

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___PHY_EXPORT_PARAMS__HPP
