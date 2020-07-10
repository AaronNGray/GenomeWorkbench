#ifndef GUI_CORE___UI_EXPORT_TOOL__HPP
#define GUI_CORE___UI_EXPORT_TOOL__HPP

/*  $Id: ui_export_tool.hpp 38593 2017-06-01 17:31:50Z evgeniev $
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

#include <gui/gui_export.h>

#include <gui/objutils/objects.hpp>

#include <gui/core/ui_tool_manager.hpp>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// IUIExporter is a IUIToolManager that represents an export tool.
///
/// TODO - consider placing this interface in a separate file
class  NCBI_GUICORE_EXPORT  IUIExportTool : public IUIToolManager
{
public:
    /// sets objects to be exported.
    virtual void SetLocations(const TRelatedObjectsMap& /*input_map*/) = 0;
    virtual void SetObjects(const TConstScopedObjects& input) = 0;
    virtual void SetFileName(const string& /*filename*/) {}
};

///////////////////////////////////////////////////////////////////////////////
/// IViewFactory
///
class NCBI_GUICORE_EXPORT  IExporterFactory
{
public:
    virtual ~IExporterFactory() {}

    /// create and instance of the IUIExporter
    virtual IUIExportTool*  CreateInstance() const = 0;

    /// Check if exporter can handle the objects
    virtual bool TestInputObjects(TConstScopedObjects& objects) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// This Extension Point allows to add exporter factories.
#define EXT_POINT__UI_EXPORTER_FACTORY         "export_service::exporter_factory"

END_NCBI_SCOPE


#endif  // GUI_CORE___UI_EXPORT_TOOL__HPP
