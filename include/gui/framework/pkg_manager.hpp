#ifndef GUI_FRAMEWORK___PKG_MANAGER__HPP
#define GUI_FRAMEWORK___PKG_MANAGER__HPP

/*  $Id: pkg_manager.hpp 35300 2016-04-20 23:59:29Z whlavina $
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

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>


#include <gui/gui_export.h>

#include <gui/framework/gui_package.hpp>
#include <gui/framework/GuiPkgManifest.hpp>

#include <set>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CGuiPkgManifest;
END_SCOPE(objects)

class CPackageInfo
{
public:
    enum EPackageState
    {
        eUndefined = -1,
        eNotValid,
        eValidated,
        eLoaded
    };

    CPackageInfo() :
        m_State(eUndefined), m_VerMajor(0), m_VerMinor(0), m_VerPatch(0) {}
    wxString        m_Dir;
    EPackageState   m_State;
    size_t          m_VerMajor;
    size_t          m_VerMinor;
    size_t          m_VerPatch;
    string          m_ErrMsg;
    CConstRef<objects::CGuiPkgManifest> m_Manifest;
    AutoPtr<IGuiPackage> m_Package;
};

///////////////////////////////////////////////////////////////////////////////
/// CPkgManager - application Package Manager.

class NCBI_GUIFRAMEWORK_EXPORT CPkgManager : public map<string, CPackageInfo>
{
public:
    typedef set<string> TPackagesSet;

    CPkgManager();

    void SetRegistryPath(const string& path) { m_RegPath = path; }

    void SetAppPkg(const string appName,
                   size_t appVerMajor,
                   size_t appVerMinor,
                   size_t appVerPatch);

    void AddDefaultPkg(const string& pkg_name);

    void ScanPackages(const wxString& path);
    void ValidatePackages();
    void LoadPackages();

    void SetPackagesToLoad(const vector<string>& packages);

    const CPackageInfo* GetPkgInfo(int index, string& pkg_name) const;

    void GetBasePkgs(const string& pkg_name, vector<string>& basePkgs) const;
    void GetDependentPkgs(const string& pkg_name,
                          vector<string>& dependents) const;
    bool DependsOn(const CPackageInfo& pkg, const string& pkg_name) const;

private:
    void x_ValidatePackage(CPackageInfo& pkg);
    void x_LoadPackage(CPackageInfo& pkg);

    void x_GetBasePkgs(const CPackageInfo& pkg, set<string>& basePkgs) const;

    bool x_InitPackage( IGuiPackage* pkg );

    string m_RegPath;
    vector<string> m_DefaultPkgs;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___PKG_MANAGER__HPP
