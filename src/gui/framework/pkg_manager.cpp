/*  $Id: pkg_manager.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <gui/framework/pkg_manager.hpp>

#include <gui/framework/GuiPkgDependency.hpp>
#include <gui/framework/GuiDependentPkg.hpp>
#include <gui/framework/gui_package.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <corelib/ncbidll.hpp>

#include <serial/objistr.hpp>
#include <serial/serial.hpp>

#include <util/xregexp/regexp.hpp>

#include <wx/dynlib.h>
#include <wx/filename.h>
#include <wx/dir.h>

BEGIN_NCBI_SCOPE


CPkgManager::CPkgManager()
{
}


static bool sParseVersion (const string& str, size_t& verMajor, size_t& verMinor, size_t& verPatch)
{
    CRegexp regex("(\\d\\d)\\.(\\d\\d)\\.(\\d+)");
    if (!regex.IsMatch(str))
        return false;

    verMajor = NStr::StringToInt(regex.GetSub(str,1));
    verMinor = NStr::StringToInt(regex.GetSub(str,2));
    verPatch = NStr::StringToInt(regex.GetSub(str,3));
    return true;
}


static bool sValidateVersion (size_t& reqMajor, size_t& reqMinor, size_t& reqPatch,
                              size_t& verMajor, size_t& verMinor, size_t& verPatch)
{
    return reqMajor == verMajor/* && reqMinor == verMinor*/;
}


void CPkgManager::AddDefaultPkg(const string& pkgName)
{
    m_DefaultPkgs.push_back(pkgName);
}

void CPkgManager::SetAppPkg(const string appName,
                            size_t appVerMajor,
                            size_t appVerMinor,
                            size_t appVerPatch)
{
    string appPkgName = appName;
    NStr::ToUpper(appPkgName);
    const_iterator it = find(appPkgName);
    if (it != end()) {
        LOG_POST(Error << "CPkgManager::SetAppPkg(): \'" << appName
                         << "\' - duplicate package found. Replacing by App package.");
    }

    CPackageInfo pkg_info;
    pkg_info.m_State = CPackageInfo::eLoaded;
    pkg_info.m_VerMajor = appVerMajor;
    pkg_info.m_VerMinor = appVerMinor;
    pkg_info.m_VerPatch = appVerPatch;
    insert(value_type(appPkgName, pkg_info));
}

void CPkgManager::ScanPackages( const wxString& path )
{
    wxDir dir( path );
    if( !dir.IsOpened() ){
        LOG_POST( Warning 
            << "CPkgManager::ScanPackages(): directory \""
            << path.ToUTF8() << "\" does not exist"
        );
        return;
    }

    LOG_POST( Info 
        << "CPkgManager: scanning packages at \"" << path.ToUTF8() << "\""
    );

    size_t dirs_scanned = 0, pkgs_loaded = 0, pkgs_failed = 0;

    wxString dir_entry;
    for( 
        bool more = dir.GetFirst( &dir_entry, wxEmptyString, wxDIR_DIRS );
        more;
        more = dir.GetNext( &dir_entry )
    ){
        dirs_scanned++;
        pkgs_failed++;

        wxFileName manf_file( path, wxT("package.manifest") );
        manf_file.AppendDir( dir_entry );
        wxString manifest_path = manf_file.GetFullPath();

        if( !manf_file.IsFileReadable() ){
            LOG_POST( Warning 
                << "CPkgManager::ScanPackages(): manifest file \""
                << manifest_path.ToUTF8() << "\" is not accessible"
            );
            continue;
        }

        string errMsg;

        CRef<objects::CGuiPkgManifest> manifest( new objects::CGuiPkgManifest() );
        try {
            CNcbiIfstream ifstr( manifest_path.fn_str() );
            auto_ptr<CObjectIStream> is( CObjectIStream::Open(eSerial_Xml, ifstr) );
            *is >> *manifest;
        
        } catch( CException& e ){
            LOG_POST( Error 
                << "CPkgManager::ScanPackages(): failed to load \""
                << manifest_path.ToUTF8() << "\""
            );
            LOG_POST( Error << "    " << e.GetMsg() );

            continue;
        }

        string pkg_name = NStr::TruncateSpaces( manifest->GetName() );
        if( pkg_name.empty() ){
            LOG_POST( Error 
                << "CPkgManager::ScanPackages(): failed to load \""
                << manifest_path.ToUTF8() << "\""
            );
            LOG_POST( Error << "    Error: Name field of manifest is empty" );

            continue;
        }

        size_t verMajor, verMinor, verPatch;
        string version = NStr::TruncateSpaces( manifest->GetVersion() );
        if( !sParseVersion( version, verMajor, verMinor, verPatch ) ){
            LOG_POST( Error 
                << "CPkgManager::ScanPackages(): \'" << pkg_name
                << "\' - invalid version \'" << version << "\'"
            );
            continue;
        }

        string dll = manifest->GetMainDll();
        if( dll.empty() ){
            LOG_POST( Error 
                << "CPkgManager::ScanPackages(): failed to load \"" 
                << manifest_path.ToUTF8() << "\""
            );
            LOG_POST( Error << "    Error: MainDll field of manifest is empty" );

            continue;
        }

        // wxDynamicLibrary::CanonicalizeName() does not work properly,
        // so we make our own changes
        dll = NCBI_PLUGIN_PREFIX + dll + NCBI_PLUGIN_SUFFIX;

        wxFileName dll_file( manf_file.GetPath(), wxString( dll.c_str(), wxConvFile ) );
        if( !dll_file.FileExists() ){
            LOG_POST( Error 
                << "CPkgManager::ScanPackages(): \'" << pkg_name 
                << "\' - main dll \"" << dll_file.GetFullPath().ToUTF8() 
                << "\" does not exist"
            );
            continue;
        }

        string pkg_key = pkg_name;
        NStr::ToUpper( pkg_key );

        if( find(pkg_key) != end() ){
            LOG_POST( Error 
                << "CPkgManager::ScanPackages(): \'" << pkg_name
                << "\' - duplicate package found."
            );
            LOG_POST( Error 
                << "    \'" << pkg_name << "\' ignored in \"" 
                << manf_file.GetPath().ToUTF8() << "\""
            );
            continue;
        }

        CPackageInfo pkg_info;
        pkg_info.m_Dir = manf_file.GetPath();
        pkg_info.m_VerMajor = verMajor;
        pkg_info.m_VerMinor = verMinor;
        pkg_info.m_VerPatch = verPatch;
        pkg_info.m_Manifest = manifest;

        insert( value_type(pkg_key, pkg_info) );

        pkgs_failed--;
        pkgs_loaded++;
    }

    LOG_POST( Info 
        << "CPkgManager: "
        << dirs_scanned << " directories scanned, "
        << pkgs_loaded << " package(s) read, "
        << pkgs_failed << " package(s) skipped."
    );
}


void CPkgManager::ValidatePackages()
{
    NON_CONST_ITERATE(CPkgManager, it, *this) {
        x_ValidatePackage(it->second);
    }
}


void CPkgManager::x_ValidatePackage(CPackageInfo& pkg)
{
    if (pkg.m_State != CPackageInfo::eUndefined) {
        return;
    }

    const objects::CGuiPkgManifest& manifest = *pkg.m_Manifest;
    ITERATE(
        objects::CGuiPkgManifest::TGuiPkgDependency, it,
        manifest.GetGuiPkgDependency()
    ){
        const objects::CGuiDependentPkg& dep = (*it)->GetGuiDependentPkg();
        string pkgKey = dep.GetName();
        NStr::ToUpper(pkgKey);

        iterator it2 = find(pkgKey);
        if (it2 == end()) {
            pkg.m_State = CPackageInfo::eNotValid;
            string errMsg = "dependent package \'" + dep.GetName() + "\' not found.";
            pkg.m_ErrMsg = errMsg;
            LOG_POST(Info << "CPkgManager::ValidatePackage \'"
                << manifest.GetName() << "\' : " << errMsg);
            return;
        }

        x_ValidatePackage(it2->second);

        if( 
            it2->second.m_State != CPackageInfo::eValidated 
            && it2->second.m_State != CPackageInfo::eLoaded
        ){
            pkg.m_State = CPackageInfo::eNotValid;
            string errMsg = "dependent package \'" + dep.GetName() + "\' not valid.";
            pkg.m_ErrMsg = errMsg;
            LOG_POST(
                Info << "CPkgManager::ValidatePackage \'"
                << manifest.GetName() << "\' : " << errMsg
            );
            return;
        }

        size_t reqMajor, reqMinor, reqPatch;
        string version = NStr::TruncateSpaces(dep.GetVersion());
        if (!sParseVersion (version, reqMajor, reqMinor, reqPatch)) {
            pkg.m_State = CPackageInfo::eNotValid;
            string errMsg = "failed to parse dependent package\'s \'"
                            + dep.GetName() + "\' version: "
                            + version + ".";
            pkg.m_ErrMsg = errMsg;
            LOG_POST(Info << "CPkgManager::ValidatePackage \'"
                << manifest.GetName() << "\' : " << errMsg);
            return;
        }

        if (! sValidateVersion(reqMajor, reqMinor, reqPatch,
                              it2->second.m_VerMajor,
                              it2->second.m_VerMinor,
                              it2->second.m_VerPatch)) {
            pkg.m_State = CPackageInfo::eNotValid;
            string errMsg = "invalid dependent package version: \'"
                            + dep.GetName() + "\' - " + version + ".";
            pkg.m_ErrMsg = errMsg;
            LOG_POST(Info << "CPkgManager::ValidatePackage \'"
                << manifest.GetName() << "\' : " << errMsg);
            return;
        }
    }

    pkg.m_State = CPackageInfo::eValidated;
    pkg.m_ErrMsg = "";
}


static const char* kPackagesToLoad = "PackagesToLoad";


void CPkgManager::LoadPackages()
{
    vector<string> pkgsToLoad;

    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        if (gui_reg.HasField(CGuiRegistryUtil::MakeKey(m_RegPath, kPackagesToLoad))) {
            CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
            view.GetStringVec(kPackagesToLoad, pkgsToLoad);
        }
    }

    ///
    /// make sure we always include the default packages as well
    ///
    {{
        typedef set<string, PNocase> TSet;
        TSet items;
        ITERATE (vector<string>, iter, pkgsToLoad) {
            items.insert(*iter);
        }

        ITERATE (vector<string>, iter, m_DefaultPkgs) {
            items.insert(*iter);
        }

        pkgsToLoad.clear();
        std::copy( items.begin(), items.end(), back_inserter(pkgsToLoad) );
    }}

    ITERATE(vector<string>, it, pkgsToLoad) {
        string str = *it;
        NStr::ToUpper(str);
        iterator it2 = find(str);
        if (it2 != end()) {
            x_LoadPackage(it2->second);
        }
    }
}

void CPkgManager::SetPackagesToLoad(const vector<string>& packages)
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        view.Set(kPackagesToLoad, packages);
    }
}


#ifdef _DEBUG
    //#define LOCAL_LOG(message) void(0)
    #define LOCAL_LOG(message) LOG_POST(message)
#else
    #define LOCAL_LOG(message) void(0)
#endif

void CPkgManager::x_LoadPackage( CPackageInfo& pkg )
{
    if( pkg.m_State != CPackageInfo::eValidated ){
        return;
    }

    // Load dependent packages
    const objects::CGuiPkgManifest& manifest = *pkg.m_Manifest;
    ITERATE(objects::CGuiPkgManifest::TGuiPkgDependency, it,
            manifest.GetGuiPkgDependency()) {
        const objects::CGuiDependentPkg& dep = (*it)->GetGuiDependentPkg();
        string pkgKey = dep.GetName();
        NStr::ToUpper(pkgKey);
        iterator it2 = find(pkgKey);
        if (it2 == end())
            return;

        x_LoadPackage(it2->second);

        if (it2->second.m_State != CPackageInfo::eLoaded)
            return;
    }

    // Load package


    CSysPath::AddToPath( pkg.m_Dir );

    // wxDynamicLibrary::CanonicalizeName() does not work properly,
    // so we make our own changes
    string _dll_name = NCBI_PLUGIN_PREFIX + manifest.GetMainDll() + NCBI_PLUGIN_SUFFIX;
    wxString dll_name = wxString( _dll_name.c_str(), wxConvFile );

    LOG_POST( Info << "Loading package \'" << manifest.GetName() << "\'..." );
    wxDynamicLibrary dll;

    bool loaded = false;
#ifdef NCBI_OS_MSWIN
    // On Windows we try to load library from std path
    wxString std_bin_path = CSysPath::ResolvePath( wxT("<bin>") );
    LOCAL_LOG( Info << "Loading from \'" << std_bin_path << "\'" );
    wxString dll_path = wxFileName( std_bin_path, dll_name ).GetFullPath();
    if( !dll.Load( dll_path, wxDL_GLOBAL | wxDL_VERBATIM ) ){
        LOG_POST( Warning << "Package \'" << manifest.GetName() << "\' is set aside." );
    } else {
        loaded = true;
    }
#endif

    if( !loaded ){
        // Then we try to load library from package path
        LOCAL_LOG( Info << "Loading from \'" << pkg.m_Dir << "\'" );
        wxString dll_path = wxFileName( pkg.m_Dir, dll_name ).GetFullPath();
        if( !dll.Load( dll_path, wxDL_GLOBAL | wxDL_VERBATIM ) ){
            LOG_POST( Error << "Failed to load package dll " << dll_path );

            return;
        }
    }
    LOCAL_LOG( Info << "...loaded." );


    wxString guiEntryName = wxT("NCBIGBenchGetPackage");

    if( !dll.HasSymbol( guiEntryName ) ){
        LOG_POST( Error << "No entry point NCBIGBenchGetPackage() is found." );
        NCBI_THROW( CException, eUnknown, "Library is not a valid GBench package" );
    }

    // wxDYNLIB_FUNCTION
    IGuiPackage::FPackageEntryPoint entry_point = 
        (IGuiPackage::FPackageEntryPoint)dll.GetSymbol( guiEntryName )
    ;
    if( !entry_point ){
        LOG_POST( Error << "No entry point NCBIGBenchGetPackage() is retrieved." );
        NCBI_THROW( CException, eUnknown, "Library is not a valid GBench package" );
    }

    auto_ptr<IGuiPackage> gui_package( entry_point() );
    if( !gui_package.get() ){
        LOG_POST( Error << "No gui package is found." );
        NCBI_THROW( CException, eUnknown, "Failed to get gui package" );
    }

    LOCAL_LOG( Info << "Initializing package " << gui_package->GetName() << "..." );

    if( ! gui_package->Init() ){
        LOG_POST(Error << "Initialization of " << gui_package->GetName() << " failed." );
        NCBI_THROW( CException, eUnknown, "Failed to initialize gui package" );
    }

    pkg.m_State = CPackageInfo::eLoaded;
    pkg.m_Package = gui_package.release();
    LOG_POST( Info << "Package \'" << manifest.GetName() << "\' loaded successfully.");

    dll.Detach();
}

const CPackageInfo* CPkgManager::GetPkgInfo(int index, string& pkg_name) const
{
    const_iterator it;
    for (it = begin();
        index > 0 && it != end(); ++it, index--) {
    }

    if (it == end()) {
        return 0;
    }

    pkg_name = it->first;
    return &it->second;
}

void CPkgManager::GetBasePkgs(const string& pkg_name, vector<string>& basePkgs) const
{
    string pkgKey = pkg_name;
    NStr::ToUpper(pkgKey);
    const_iterator it = find(pkgKey);
    if (it == end()) {
        return;
    }

    set<string> basePkgsSet;
    x_GetBasePkgs(it->second, basePkgsSet);
    back_insert_iterator<vector<string> > dest(basePkgs);
    copy(basePkgsSet.begin(), basePkgsSet.end(), dest);
}

void CPkgManager::x_GetBasePkgs(const CPackageInfo& pkg, set<string>& basePkgs) const
{
    const objects::CGuiPkgManifest* manifest = pkg.m_Manifest.GetPointer();
    if (!manifest) {
        return;
    }

    ITERATE(objects::CGuiPkgManifest::TGuiPkgDependency, it,
            manifest->GetGuiPkgDependency()) {
        const objects::CGuiDependentPkg& dep = (*it)->GetGuiDependentPkg();
        string pkgKey = dep.GetName();
        NStr::ToUpper(pkgKey);
        if (basePkgs.find(pkgKey) != basePkgs.end()) {
            continue;
        }
        basePkgs.insert(pkgKey);
        const_iterator it2 = find(pkgKey);
        if (it2 != end())
            x_GetBasePkgs(it2->second, basePkgs);
    }
}

void CPkgManager::GetDependentPkgs(const string& pkg_name, vector<string>& dependents) const
{
    string pkgKey = pkg_name;
    NStr::ToUpper(pkgKey);

    ITERATE(CPkgManager, it, *this) {
        if (DependsOn(it->second, pkgKey)) {
            dependents.push_back(it->first);
        }
    }
}

bool CPkgManager::DependsOn(const CPackageInfo& pkg, const string& pkg_name) const
{
    set<string> basePkgs;
    x_GetBasePkgs(pkg, basePkgs);

    string pkgKey = pkg_name;
    NStr::ToUpper(pkgKey);

    return (basePkgs.find(pkgKey) != basePkgs.end());
}

END_NCBI_SCOPE

