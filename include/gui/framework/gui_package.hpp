#ifndef GUI_FRAMEWORK___GUI_PACKAGE__HPP
#define GUI_FRAMEWORK___GUI_PACKAGE__HPP

/*  $Id: gui_package.hpp 18397 2008-11-28 19:45:17Z dicuccio $
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

BEGIN_NCBI_SCOPE

#ifdef NCBI_OS_MSWIN
#  define NCBI_PACKAGEENTRYPOINT_EXPORT __declspec(dllexport)
#else
#  define NCBI_PACKAGEENTRYPOINT_EXPORT
#endif


////////////////////////////////////////////////////////////////////////////////
/// TODO - Document
class IGuiPackage
{
public:
    virtual ~IGuiPackage() {}

    // this is a typedef for the main DLL entry point for a package
    typedef IGuiPackage* (*FPackageEntryPoint)();

    virtual string GetName() const = 0;
    virtual void GetVersion(size_t& verMajor,
                            size_t& verMinor,
                            size_t& verPatch) const = 0;
    virtual bool Init() = 0;
    virtual void Shut() = 0;
};

END_NCBI_SCOPE

/* @} */

#endif // GUI_FRAMEWORK___GUI_PACKAGE__HPP
