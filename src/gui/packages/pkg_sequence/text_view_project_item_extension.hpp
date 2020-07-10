#ifndef PKG_SEQUENCE___TEXT_VIEW_PROJECT_ITEM_EXTENSION__HPP
#define PKG_SEQUENCE___TEXT_VIEW_PROJECT_ITEM_EXTENSION__HPP

/*  $Id: text_view_project_item_extension.hpp 34517 2016-01-19 21:24:00Z katargir $
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

#include <gui/core/project_item_extension.hpp>
#include <gui/utils/extension.hpp>

BEGIN_NCBI_SCOPE

class  CTextViewProjectItemExtension :
    public CObject,
    public IProjectItemExtension,
    public IExtension
{
public:
    CTextViewProjectItemExtension() {}

    /// @name IProjectItemExtension interface implementation
    /// @{
    virtual void ProjectItemCreated(objects::CProjectItem& /*item*/, IServiceLocator* /*service_locator*/) {}
    virtual void ProjectItemAttached(objects::CProjectItem& item, CGBDocument& doc, IServiceLocator* service_locator);
    virtual void ProjectItemDetached(objects::CProjectItem& /*item*/, CGBDocument& /*doc*/, IServiceLocator* /*service_locator*/) {}
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}
};

END_NCBI_SCOPE

#endif  // PKG_SEQUENCE___TEXT_VIEW_PROJECT_ITEM_EXTENSION__HPP
