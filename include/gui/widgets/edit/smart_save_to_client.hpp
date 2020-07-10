#ifndef PKG_SMART___SMART_SAVE_TO_CLIENT__HPP
#define PKG_SMART___SMART_SAVE_TO_CLIENT__HPP

/*  $Id: smart_save_to_client.hpp 37726 2017-02-10 19:22:39Z krasichkovye $
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
 * Authors:  Yoon Choi
 *
 * File Description:
 *
 */


#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>
#include <objects/gbproj/ProjectItem.hpp>
#include <objmgr/scope.hpp>
#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE


class NCBI_GUIWIDGETS_EDIT_EXPORT CSmartClient 
{
public:
	enum EResultType {
        eUpdate,
        eCancel,
    };

public:
	static void SendResult(objects::CScope* scope, 
						   const objects::CProjectItem& projitem, 
						   EResultType result_type);
};


END_NCBI_SCOPE


#endif  // PKG_SMART___SMART_SAVE_TO_CLIENT__HPP

