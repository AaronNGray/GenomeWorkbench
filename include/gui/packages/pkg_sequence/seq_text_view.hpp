#ifndef GUI_WIDGETS_SEQ___SEQ_TEXT_VIEW__HPP
#define GUI_WIDGETS_SEQ___SEQ_TEXT_VIEW__HPP

/*  $Id: seq_text_view.hpp 27556 2013-03-05 16:04:52Z katargir $
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

#include <corelib/ncbiobj.hpp>

#include <gui/core/project_view_impl.hpp>
#include <gui/core/open_view_manager_impl.hpp>



BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CSeqTextView
class CSeqTextView : public CProjectView
{
    friend class CSeqTextViewFactory;
public:
    CSeqTextView();

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow*      GetWindow();
    virtual wxEvtHandler*  GetCommandHandler();
    /// @}

    /// @name IView implementation
    /// @(
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    /// @}

    /// @name IProjectView implementation
    /// @{
    virtual bool    InitView(TConstScopedObjects& objects, const objects::CUser_object* params);
    /// @}

    /// @name ISelectionClient Implementation
    /// @{
    virtual void GetSelection(CSelectionEvent& evt) const;
    virtual void GetSelection(TConstScopedObjects& objs) const;
    virtual void GetMainObject(TConstScopedObjects& objs) const;
    /// @}

    /// @addtogroup CProjectViewBase overridables
    /// @{
    virtual const CObject* x_GetOrigObject() const;
    /// @}

protected:
    virtual void x_OnSetSelection(CSelectionEvent& evt);

    virtual bool x_MergeObjects( TConstScopedObjects& objects );

private:
    wxWindow* m_Window;

    CConstRef<CObject> m_OrigObj;
};


///////////////////////////////////////////////////////////////////////////////
/// CSeqTextViewFactory
class CSeqTextViewFactory :
    public CObject,
    public IExtension,
    public IProjectViewFactory
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IProjectViewFactory interface implementation
    /// @{
    virtual IView* CreateInstance() const;
    virtual IView* CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    virtual void RegisterIconAliases(wxFileArtProvider& provider);

    virtual const CProjectViewTypeDescriptor& GetProjectViewTypeDescriptor() const;
    virtual int TestInputObjects( TConstScopedObjects& objects );
    /// @}
};


/* @} */

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___SEQ_TEXT_VIEW__HPP
