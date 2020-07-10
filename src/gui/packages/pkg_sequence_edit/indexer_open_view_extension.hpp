#ifndef PKG_SEQUENCE_EDIT___INDEXER_OPEN_VIEW_EXTENSION__HPP
#define PKG_SEQUENCE_EDIT___INDEXER_OPEN_VIEW_EXTENSION__HPP

/*  $Id: indexer_open_view_extension.hpp 34986 2016-03-07 20:00:00Z katargir $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/core/open_view_extension.hpp>
#include <gui/utils/extension.hpp>

BEGIN_NCBI_SCOPE

class  CIndexerOpenViewExtension :
    public CObject,
    public IOpenProjectViewExtension,
    public IExtension
{
public:
    /// @name IOpenProjectViewExtension interface implementation
    /// @{
    virtual void OnViewOpen(IProjectView& view, IServiceLocator* serviceLocator);
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}
};

END_NCBI_SCOPE

#endif  // PKG_SEQUENCE_EDIT___INDEXER_OPEN_VIEW_EXTENSION__HPP

