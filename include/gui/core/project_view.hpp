#ifndef GUI_CORE___PROJECT_VIEW__HPP
#define GUI_CORE___PROJECT_VIEW__HPP

/*  $Id: project_view.hpp 44126 2019-11-01 16:58:03Z filippov $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 * Authors:  Mike DiCuccio, Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/framework/view.hpp>

#include <gui/core/selection_service.hpp>

#include <gui/widgets/wx/wm_client.hpp>

#include <gui/objutils/objects.hpp>

#include <gui/objects/GBProjectHandle.hpp>

#include <memory>


BEGIN_NCBI_SCOPE

class IServiceLocator;
class ISelectionService;
class CProjectViewEvent;
class IGBDocument;

BEGIN_SCOPE(objects)
    class CPluginMessage;
    class CDataHandle;
    class CUser_object;
    class CGBProjectHandle;
END_SCOPE(objects)


///////////////////////////////////////////////////////////////////////////////
/// class IProjectView defines the abstract interface for views observing projects.

class IProjectView 
    : public IView
    , public IWMClient
    , public ISelectionClient
{
public:
    typedef objects::CGBProjectHandle::TId TProjectId;
    typedef size_t TId;

    /// Retrieve a label for this view.  The label has several different
    enum ELabelType {
        eType,              //< usually a description of the class
        eContent,           //< The content (what the view is looking at)
        eTypeAndContent,
        eId,                //< an identifier for the view, usually given as a base-26 letter
        eProject,
        eDefault = eTypeAndContent
    };

public:
    /// initialize view with data, inside this function the view
    /// must call CProjectService::AttachView to connect to a project
    virtual bool InitView(TConstScopedObjects& objects, const objects::CUser_object* params) = 0;

    /// disconnects view from the project and associated data
    virtual void DestroyView() = 0;

    /// called when view creation is completed and view size is known
    virtual void InitialLayout() = 0;
    virtual void Destroy() = 0;
    /// Set async destruction mode
    virtual void SetAsyncDestroy(bool reset_hist_async) = 0;

    /// retrieve a unique ID for this view
    virtual TId GetId() const = 0;

    virtual TProjectId GetProjectId() const = 0;

    /// returns name of the plug-in created this view (view class name)
    virtual string  GetLabel( ELabelType type ) const = 0;
    virtual void SetOrigObject(SConstScopedObject& obj) {}

    virtual IServiceLocator* GetServiceLocator() const = 0;
    virtual CRef<objects::CScope> GetScope() const = 0;

    /// @name Event Handling
    /// @{
    virtual void OnSelectionChanged( const TConstScopedObjects& sels ) = 0;
    virtual void OnProjectChanged( CProjectViewEvent& evt ) = 0;
    virtual void OnViewReleased( IView& view ) = 0;
    virtual void OnPrint() = 0;
    /// @}

    virtual ~IProjectView() {}
};

class IViewSettingsAtProject
{
public:
    virtual ~IViewSettingsAtProject() {}
    virtual void SaveSettingsAtProject(objects::CGBProjectHandle& project) const = 0;
};

class IProjectViewLabel
{
public:
    virtual ~IProjectViewLabel() {}
    virtual bool GetLabel(string& label, const IProjectView& view, IProjectView::ELabelType type) = 0;
};

END_NCBI_SCOPE

#endif  /// GUI_CORE___PROJECT_VIEW__HPP
