#ifndef GUI_FRAMEWORK___VIEW__HPP
#define GUI_FRAMEWORK___VIEW__HPP

/*  $Id: view.hpp 24057 2011-07-15 15:37:02Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/wm_client.hpp>

#include <gui/utils/ui_object.hpp>

class wxFileArtProvider;
class wxWindow;


BEGIN_NCBI_SCOPE

class IWorkbench;
class CViewTypeDescriptor;
class CUICommandRegistry;

///////////////////////////////////////////////////////////////////////////////
/// IView - represents a standard visual part of Workbench UI.
/// IView usually implements other UI-related interfaces such as IWMClient,
/// IMenuContributor, IToolBarContext, IHelpContext etc.
///
/// IView lifetime shall be managed using CIRef and CRef smart pointers.
/// Because of this classes that implement IView shall not derive from
/// wxWindow.  wxWindow has its own lifetime management policy that is not
/// compatible with smart pointers.
///
/// It is recommended that all classes inherited from IView implement
/// IRegSettings interface, this will allow the framework to save / restore
/// view's settings automatically.

class IView
{
public:
    virtual ~IView() {}

    /// return an object providing meta information about thei view type
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const = 0;

    /// connect / disconnect this view with / from Workbench
    virtual void    SetWorkbench(IWorkbench* workbench) = 0;

    /// create Window corresponding to the view
    virtual void    CreateViewWindow(wxWindow* parent) = 0;

    /// destroy Window corresponding to the view
    virtual void    DestroyViewWindow() = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// CViewTypeDescriptor - holds description of a view type.
/// CViewTypeDescriptor is used for declaring view types and presenting the
/// information about available types in the UI.

class NCBI_GUIFRAMEWORK_EXPORT CViewTypeDescriptor
    : public CUIObject
{
public:
    enum EAcceptionType {
        eNone,
        eSingle,
        eMultiple
    };

public:
    CViewTypeDescriptor(
        const string& label,
        const string& icon_alias,
        const string& hint,
        const string& description,
        const string& help_id,
        const string& category,
        bool singleton
    );

    virtual ~CViewTypeDescriptor();

    /// get view category (used for view grouping in UI)
    virtual string GetCategory() const;

    /// true if only one instance of this type can be created
    virtual bool IsSingleton() const;

    virtual void SetCategory( const string& category );
    virtual void SetSingleton( bool singleton );

protected:
    string m_Category;
    bool m_Singleton;
};


///////////////////////////////////////////////////////////////////////////////
/// IViewFactory - view factory for use with IViewManagerService.
/// IViewFactory correspond to one specific view type and used by View Manager
/// Service to produce instances of this type.

class NCBI_GUIFRAMEWORK_EXPORT  IViewFactory
{
public:
    typedef IWMClient::CFingerprint TFingerprint;

    /// called by the framework to give Factory a chance to register images
    /// used by view
    virtual void    RegisterIconAliases(wxFileArtProvider& provider) = 0;

    /// called by the framework to give Factory a chance to register commands
    /// used by view
    virtual void    RegisterCommands(CUICommandRegistry&, wxFileArtProvider&) {}

    /// returns a Descriptor for the View Type supported by the Factory
    virtual const CViewTypeDescriptor&   GetViewTypeDescriptor() const = 0;

    /// creates a view instance
    virtual IView*  CreateInstance() const = 0;

    /// if fingerprint is recognized - creates and returns a new instance
    virtual IView*
        CreateInstanceByFingerprint(const TFingerprint& fingerprint) const = 0;

    virtual ~IViewFactory() {};
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___VIEW__HPP

