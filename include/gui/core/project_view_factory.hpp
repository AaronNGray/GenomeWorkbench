#ifndef GUI_CORE___PROJECT_VIEW_FACTORY__HPP
#define GUI_CORE___PROJECT_VIEW_FACTORY__HPP

/*  $Id: project_view_factory.hpp 26241 2012-08-09 19:12:33Z katargir $
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
* Authors: Yury Voronov
*
* File Description:
*
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/framework/view.hpp>
#include <gui/core/open_view_manager.hpp>

class wxPanel;
class wxWindow;


BEGIN_NCBI_SCOPE

enum EPVObjectsAccepted {
    eNoObjectsAccepted = 0,
    eOneObjectAccepted,
    eSimilarObjectsAccepted,
    eAnyObjectsAccepted
};

///////////////////////////////////////////////////////////////////////////////
/// CProjectViewTypeDescriptor - holds description of a project view type.
class NCBI_GUICORE_EXPORT CProjectViewTypeDescriptor
    : public CViewTypeDescriptor
{

public:
    CProjectViewTypeDescriptor(
        const string& label,
        const string& icon_alias,
        const string& hint,
        const string& description,
        const string& help_id,
        const string& category,
        bool singleton,
        const string& primary_type,
        EPVObjectsAccepted multi_accepted,
        bool prefetch_needed = true
    ) 
    : CViewTypeDescriptor(
        label, icon_alias, hint, description, help_id, category, singleton
    )
    , m_PrimaryType( primary_type )
    , m_MultiAccepted( multi_accepted )
    , mf_PrefetchNeeded( prefetch_needed )
    {
    }

    const string GetPrimaryInputType() const
    {
        return m_PrimaryType;
    }

    EPVObjectsAccepted GetObjectsAccepted() const
    {
        return m_MultiAccepted;
    }

    bool IsPrefetchNeeded() const
    {
        return mf_PrefetchNeeded;
    }

protected:
    string m_PrimaryType;
    EPVObjectsAccepted m_MultiAccepted;
    bool mf_PrefetchNeeded;
};

typedef map<string, TConstScopedObjects> TObjectsMap;

///////////////////////////////////////////////////////////////////////////////
/// IProjectViewFactory
///
class NCBI_GUICORE_EXPORT IProjectViewFactory
    : public IViewFactory
{
public:
    enum ETestResult {
        fCanShowNone        = 0x00,
        fCanCombine         = 0x01,
        fCanShowSeparated   = 0x02,
        fCanShowSome        = 0x04,
        fCanShowAll         = 0x08
    };

public:
    /// returns a Descriptor for the View Type supported by the Factory
    virtual const CProjectViewTypeDescriptor& GetProjectViewTypeDescriptor() const = 0;

    virtual const CViewTypeDescriptor& GetViewTypeDescriptor() const;
    virtual IOpenViewManager* GetOpenViewManager();

    /// tests input objects (probably using object conversion, or not)
    /// and returns a combination of ETestResult flags
    /// better name is IsCompatibleWith()
    virtual int TestInputObjects( TConstScopedObjects& objects ) = 0;
    virtual bool IsCompatibleWith( const CObject& object, objects::CScope& scope );
 
    virtual vector<int> CombineInputObjects( const TConstScopedObjects& objects );
    virtual void CombineObjects( const TConstScopedObjects& objects, TObjectsMap& objects_map );

    virtual bool ShowInOpenViewDlg() const { return true; }
};

typedef IProjectViewFactory CProjectViewFactory;

END_NCBI_SCOPE


#endif  // GUI_CORE___PROJECT_VIEW_FACTORY__HPP

