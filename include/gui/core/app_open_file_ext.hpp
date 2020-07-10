#ifndef GUI_CORE___APP_OPEN_FILE_EXT__HPP
#define GUI_CORE___APP_OPEN_FILE_EXT__HPP

/*  $Id: app_open_file_ext.hpp 33341 2015-07-08 20:11:28Z katargir $
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

class IWorkbench;

class  IAppOpenFileExt
{
public:
    virtual ~IAppOpenFileExt() {}

    virtual bool Open(const string& fileName, IWorkbench* workbench) = 0;
};

#define EXT_POINT__GBENCH_OPEN_FILE "gbench_open_file_extension"

END_NCBI_SCOPE

#endif  // GUI_CORE___APP_OPEN_FILE_EXT__HPP
