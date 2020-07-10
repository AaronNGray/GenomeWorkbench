#ifndef GUI_WIDGETS_SEQ___ASN_VIEW_CONTEXT__HPP
#define GUI_WIDGETS_SEQ___ASN_VIEW_CONTEXT__HPP

/*  $Id: asn_view_context.hpp 41060 2018-05-16 18:11:00Z katargir $
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
#include <gui/gui_export.h>

#include <objtools/format/item_formatter.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <gui/widgets/wx/ibioseq_editor.hpp>

#include <gui/widgets/text_widget/text_item_panel.hpp>

BEGIN_NCBI_SCOPE

class ITextItem;

class CAsnBioContext
{
public:
    CAsnBioContext(const objects::CSeq_entry_Handle& seh) : m_SEH(seh) {}
    objects::CSeq_entry_Handle& GetSeq_entry_Handle() { return m_SEH; }

private:
    objects::CSeq_entry_Handle m_SEH;
};

class CAsnBioContextContainer : public wxObject
{
public:
    CAsnBioContextContainer() {}
    virtual ~CAsnBioContextContainer()
    {
        ITERATE(vector<CAsnBioContext*>, it, m_Contexts) {
            delete *it;
        }
    }
    void AddBioContext(CAsnBioContext* bioContext) { m_Contexts.push_back(bioContext); }

private:
    vector<CAsnBioContext*> m_Contexts;
};

class NCBI_GUIWIDGETS_SEQ_EXPORT CAsnViewContext
    : public CTextPanelContext, public IBioseqEditorCB
{
public:
    CAsnViewContext(objects::CScope& scope, const CSerialObject* so, IBioseqEditor* bioseqEditor = 0, bool tooltpContext = false);
    ~CAsnViewContext();

    virtual void MouseMoveInContainer(ITextItem* container);
    virtual wxWindow* CreateTooltipWindow();

    virtual void RootInitialized();

    virtual wxMenu* CreateMenu() const;

    bool GetShowAsnTypes() const { return m_ShowAsnTypes; }
    void SetShowAsnTypes(bool show);

    bool GetShowPathToRoot() const { return m_ShowPathToRoot; }
    void SetShowPathToRoot(bool show);

    virtual void SetPanel(CTextItemPanel* panel);

    /// @name IBioseqEditorCB interface implementation
    /// @{
    virtual void GetItemSelection(TFlatItemList&) {}
    virtual void GetCurrentSelection(TEditObjectList& objects);
    virtual objects::CBioseq_Handle GetCurrentBioseq();
    virtual wxString GetWorkDir() const;
    /// @}

protected:
    CIRef<IBioseqEditor> m_BioseqEditor;

    bool m_TooltipContext;
    bool m_ShowAsnTypes;
    bool m_ShowPathToRoot;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___ASN_VIEW_CONTEXT__HPP
