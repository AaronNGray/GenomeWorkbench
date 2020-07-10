#ifndef GUI_WIDGETS_EDIT___EDIT_OBJECT_SEQ_DESC__HPP
#define GUI_WIDGETS_EDIT___EDIT_OBJECT_SEQ_DESC__HPP

/*  $Id: edit_object_seq_desc.hpp 43255 2019-06-06 14:38:10Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */

#include <corelib/ncbistd.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <gui/gui_export.h>
#include <gui/widgets/edit/edit_object.hpp>
#include <gui/widgets/edit/desc_placement_panel.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <wx/checkbox.h>

class wxTreebook;
class wxPanel;


BEGIN_NCBI_SCOPE

class CMolInfoPanel;

class NCBI_GUIWIDGETS_EDIT_EXPORT CEditObjectSeq_desc : public CObject, public IEditObject, public IDescEditorNotify
{
public:
    CEditObjectSeq_desc(const CObject& object,
                        const objects::CSeq_entry_Handle& seh,
                        objects::CScope& scope, bool createMode)
        : m_Object(&object), m_Scope(&scope), m_SEH(seh), m_CreateMode(createMode), m_Window(NULL), m_ReplaceAll(NULL) {}

    virtual wxWindow* CreateWindow(wxWindow* parent);
    virtual IEditCommand* GetEditCommand();
    virtual void LocationChange(objects::CSeq_entry_Handle seh, wxTopLevelWindow* gui_widget);
    void MolInfoLocationChange(objects::CSeq_entry_Handle seh, IDescEditorPanel* edit, CMolInfoPanel* molPanel);

    enum EDescriptorType {
        eUnknown = 0,
        eTPA,
        eStructuredComment,
        eRefGene,
        eDBLink,
        eUnverified,
        eAuthorizedAccess,
        eComment,
        eTitle,
        eName,
        eRegion,
        ePub,
        eMolinfo,
        eSource
    };

    static pair<EDescriptorType, string> GetDescriptorType(const objects::CSeqdesc& desc);
    static objects::CSeqdesc::E_Choice GetDescChoiceForType(EDescriptorType desc_type);
    CRef<CSerialObject> GetEditObject(void) {return m_EditedDescriptor;}

private:
    CConstRef<CObject> m_Object;
    CConstRef<CObject> m_Bioseq;
    mutable CRef<objects::CScope> m_Scope;

    objects::CSeq_entry_Handle m_SEH;
    bool m_CreateMode;

    CRef<CSerialObject> m_EditedDescriptor;
    CRef<CSerialObject> m_EditedBioseq;
    CIRef<IEditCommand> m_EditAction;

    wxPanel* m_Window;
    CDescPlacementPanel* m_Placement;
    wxCheckBox* m_ReplaceAll;

    void x_PopulateWindow(objects::CSeqdesc& desc);
    bool ReplaceAll(void) {return m_ReplaceAll && m_ReplaceAll->GetValue();}
};

class CHolderPanel: public wxPanel, public IDescEditorPanel
{
public:
    CHolderPanel(wxWindow* parent) : wxPanel(parent), m_Editor(NULL) {};
    ~CHolderPanel() {};
    void SetEditor(IDescEditorPanel* editor) { m_Editor = editor; };

    // IDescEditorPanel implementation
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc) 
        { if (m_Editor) { m_Editor->ChangeSeqdesc(desc); } }
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc)
        { if (m_Editor) { m_Editor->UpdateSeqdesc(desc); } }

protected:
    IDescEditorPanel* m_Editor;
};

class CSehHolderPanel: public wxPanel
{
public:
    CSehHolderPanel(wxWindow* parent,  objects::CSeq_entry_Handle seh) : wxPanel(parent), m_seh(seh) {};
    ~CSehHolderPanel() {};
    objects::CSeq_entry_Handle GetSEH() {return m_seh;}
private:
    objects::CSeq_entry_Handle m_seh;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___EDIT_OBJECT_SEQ_DESC__HPP
