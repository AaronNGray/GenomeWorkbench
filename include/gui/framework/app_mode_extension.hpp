#ifndef GUI_FRAMEWORK___APP_MODE_EXTENSION__HPP
#define GUI_FRAMEWORK___APP_MODE_EXTENSION__HPP

/*  $Id: app_mode_extension.hpp 34714 2016-02-03 18:35:25Z katargir $
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


BEGIN_NCBI_SCOPE

class CWorkbench;

class IAppModeExtension
{
public:
    virtual ~IAppModeExtension() {}

    virtual string GetModeName() const = 0;
    virtual void SetWorkbench(CWorkbench* wb) = 0;
};

#define EXT_POINT__APP_MODE_EXTENSION "app_mode_extension"

END_NCBI_SCOPE


#endif  // GUI_FRAMEWORK___APP_MODE_EXTENSION__HPP
