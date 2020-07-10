/*  $Id: init.cpp 34564 2016-01-25 19:02:35Z katargir $
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
* Authors: Roman Katargin
*
* File Description:
*
*/

#include <ncbi_pch.hpp>
#include "init.hpp"

#include <gui/framework/app_mode_extension.hpp>
#include <gui/core/app_open_file_ext.hpp>
#include <gui/core/app_named_pipe_client.hpp>

#include <gui/utils/extension_impl.hpp>

BEGIN_NCBI_SCOPE

bool init_gui_app_gbench()
{
    return true;
}

namespace
{

static bool init_gui_app_gbench_static()
{
    CExtensionPointDeclaration (EXT_POINT__GBENCH_NAMED_PIPE_CLIENTS,
                                "Gbench named pipe clients extension");
    CExtensionPointDeclaration (EXT_POINT__GBENCH_OPEN_FILE,
                                "Gbench open file extension");
    CExtensionPointDeclaration (EXT_POINT__APP_MODE_EXTENSION,
                                "Gbench app mode extension");
    return true;
}

const bool gui_app_gbench_static_registered = init_gui_app_gbench_static();

} // namespace


END_NCBI_SCOPE
