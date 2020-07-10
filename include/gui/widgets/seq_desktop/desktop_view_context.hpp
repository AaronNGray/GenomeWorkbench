#ifndef GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_VIEW_CONTEXT__HPP
#define GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_VIEW_CONTEXT__HPP

/*  $Id: desktop_view_context.hpp 41060 2018-05-16 18:11:00Z katargir $
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
 * Authors:  Andrea Asztalos, based on a file by Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <gui/widgets/wx/ibioseq_editor.hpp>
#include <wx/event.h>

class wxMenu;

BEGIN_NCBI_SCOPE

class CDesktopCanvas;
class IDesktopDataItem;

class CDesktopViewContext : public wxEvtHandler, public IBioseqEditorCB
{
    DECLARE_EVENT_TABLE();

public:
    enum ESavedObjectType {
        eSavedObjectType_not_set,
        eSavedObjectType_Bioseq,
        eSavedObjectType_Bioseqset,
        eSavedObjectType_ContactInfo,
        eSavedObjectType_CitSub,
        eSavedObjectType_Seqdesc,
        eSavedObjectType_SeqAnnot,
        eSavedObjectType_SeqFeat,
        eSavedObjectType_SeqAlign,
        eSavedObjectType_SeqGraph
    };

    CDesktopViewContext(IBioseqEditor* bioseqEditor, ICommandProccessor& cmdProccessor, 
        const CSerialObject* saved_object = nullptr, ESavedObjectType saved_type = eSavedObjectType_not_set);
    ~CDesktopViewContext();

    wxMenu* CreateMenu() const;
    void SetCanvas(CDesktopCanvas* canvas);

    /// @name IBioseqEditorCB interface implementation
    /// @{
    virtual void GetItemSelection(TFlatItemList&) {}
    virtual void GetCurrentSelection(TEditObjectList&);
    virtual objects::CBioseq_Handle GetCurrentBioseq();
    virtual wxString GetWorkDir() const { return wxEmptyString;  }
    /// @}

    // event handlers
    void OnCutSelection(wxCommandEvent& event);
    void OnCopySelection(wxCommandEvent& event);
    void OnPasteSelection(wxCommandEvent& event);

    void OnUpdateCutSelection(wxUpdateUIEvent& event);
    void OnUpdateCopySelection(wxUpdateUIEvent& event);
    void OnUpdatePasteSelection(wxUpdateUIEvent& event);

    const CSerialObject* GetSavedObject() const { return m_SavedObject.GetPointerOrNull(); }
    ESavedObjectType GetSavedObjectType() const { return m_SavedType; }

    void EditSelection();

private:
    CConstRef<IDesktopDataItem> x_GetSelectedDataItem() const;
    void x_UpdateCutCopySelection(wxUpdateUIEvent& event);

    objects::CBioseq_Handle m_CurBioseq;
    CIRef<IBioseqEditor> m_BioseqEditor;
    ICommandProccessor& m_CmdProccessor;

    CDesktopCanvas* m_Canvas;
    /// Saved object resulting from a Cut/Copy operation
    CConstRef<CSerialObject> m_SavedObject;
    /// Type of the saved object resulting from a Cut/Copy operation
    ESavedObjectType m_SavedType;
};


END_NCBI_SCOPE

#endif  
    // GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_VIEW_CONTEXT__HPP


