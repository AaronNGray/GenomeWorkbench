#ifndef GUI_WIDGETS_WX___SYS_PATH__HPP
#define GUI_WIDGETS_WX___SYS_PATH__HPP

/*  $Id: sys_path.hpp 33237 2015-06-19 20:28:12Z katargir $
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
 * Authors:  Robert G. Smith, Michael DiCuccio
 *
 * File Description:
 *    CSysPath -- System dependent functions needed by the GUI framework
 * File Description:
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/string.h>

class wxStandardPathsBase;

BEGIN_NCBI_SCOPE

// This class replaces CSystemPath
// It uses wxStrings and wxWidgets API and is UNICODE capable
//

class NCBI_GUIWIDGETS_WX_EXPORT CSysPath
{
public:
    /// Bind an alias to an existing path
    /// The application expects certain aliases to be present; if they are
    /// not provided, reasonable defaults will be used.  Note that this
    /// function is not thread-safe by design; application aliases shall
    /// be created in a single thread at application start-up.
    /// Possible aliases include:
    ///  '<std>'  = location of the application installation
    ///  '<home>' = application- and user-specific data directory
    ///  '<res>'  = location of the standard resource directory
    ///  '<pkgdir>' = location of package installation
    static void BindAlias(const wxString& alias, const wxString& path);

    /// Utility function to hide the platform specifics of locating our
    /// standard directories and files.  This function is responsible for
    /// resolving '<std>', '<home>', and '<res>' correctly.
    static wxString  ResolvePath(const wxString& path, const wxString& rel_name);

    /// Call the two paramter ResolvePath after splitting the path.
    static wxString ResolvePath(const wxString& path);

    /// Utility function to hide the platform specifics of locating our
    /// standard directories.  This function is responsible for resolving
    /// '<std>', '<home>', and '<res>' correctly, and further ensures that
    /// the relevant final name already exists as a file or directory.
    /// The 'path' parameter here contains a delimited set of directories
    /// to search in order.
    static wxString  ResolvePathExisting(const wxString& path,
                                         const wxString& delim = wxT(","));

    /// the applicaton installation directory.  Usually the parent of the
    /// App Path.
    /// corresponds to the <std> alias above.
    static wxString GetStdPath(void);

    /// the user-specific application-specific home directory
    /// corresponds to the <home> alias above.
    static wxString GetHomePath(void);

    /// the shared resource path
    /// corresponds to the <res> alias above.
    static wxString GetResourcePath(void);

    /// Add an element to the environment's PATH variable.  This will make
    /// sure that the element is inserted in the approrpiate place, and will
    /// erase any occurrences of the path in the existing value
    enum EPathMode {
        ePath_Prepend,
        ePath_Append
    };
    static void AddToPath(const wxString& path, EPathMode mode = ePath_Prepend);

    static void SetInst(const string& inst);
    static string GetInst();

protected:
    static wxStandardPathsBase& GetStandardPaths();

private:
    typedef map<wxString, wxString> TAliasMap;
    static TAliasMap sm_AliasMap;

    static string sm_Inst;
};

END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___SYS_PATH__HPP
