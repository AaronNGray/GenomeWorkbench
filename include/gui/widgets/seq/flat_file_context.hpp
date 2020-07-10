#ifndef GUI_WIDGETS_SEQ___FLAT_FILE_CONTEXT__HPP
#define GUI_WIDGETS_SEQ___FLAT_FILE_CONTEXT__HPP

/*  $Id: flat_file_context.hpp 41060 2018-05-16 18:11:00Z katargir $
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

#include <gui/widgets/wx/ibioseq_editor.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

class wxFileArtProvider;

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects) // namespace ncbi::objects::
class IFormatter;
class CBioseq_Handle;
class CFlatItem;
class CSeqdesc;
class CSeq_feat_Handle;
END_SCOPE(objects)

class ITextItem;
class CUICommandRegistry;
class CFlatFileTextItem;

class NCBI_GUIWIDGETS_SEQ_EXPORT CFlatFileViewContext
    : public CTextPanelContext, public IBioseqEditorCB
{
    DECLARE_EVENT_TABLE()
public:
    CFlatFileViewContext(objects::IFormatter& formatter,
                         objects::CScope& scope,
                         const CSerialObject* so,
                         IBioseqEditor* bioseqEditor);
    ~CFlatFileViewContext();

    static void RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);

    virtual int GetLeftMargin() const;

    virtual void RootInitialized();

    objects::IFormatter& GetFormatter() { return *m_Formatter; }

    virtual wxMenu* CreateMenu() const;

    virtual void SetPanel(CTextItemPanel* panel);
    virtual void GetVisibleObjects (TConstScopedObjects& objects);

    void OnEditBioseq(objects::CBioseq_Handle& bh, wxPoint pos);

    /// @name IBioseqEditorCB interface implementation
    /// @{
    virtual void GetItemSelection(TFlatItemList& itemList);
    virtual void GetCurrentSelection(TEditObjectList& objects);
    virtual objects::CBioseq_Handle GetCurrentBioseq();
    virtual wxString GetWorkDir() const;
    /// @}

    virtual void EditItem(ITextItem* item, bool controlDown, bool shiftDown);

    virtual bool IsEditingEnabled() const { return m_BioseqEditor; }

    virtual bool FindSequence(const wxString& text);
    virtual void InitDrawStream(CDrawTextOStream& stream, int startLine);

    bool ShowEditIcons() const;

    CEditObject GetEditObject(CFlatFileTextItem& ffItem);

protected:
    typedef pair<const objects::CSeqdesc*, objects::CBioseqContext*> TEditDesc;
    typedef pair<objects::CSeq_feat_Handle, objects::CBioseqContext*> TEditFeat;

    CRef<objects::IFormatter> m_Formatter;

    void OnExpandAllFeatures(wxCommandEvent& event);
    void OnCollapseAllFeatures(wxCommandEvent& event);
    void OnExpandSeqBlocks(wxCommandEvent& event);
    void OnCollapseSeqBlocks(wxCommandEvent& event);

    void OnCollapseSequences(wxCommandEvent& event);
    void OnExpandAll(wxCommandEvent& event);

    objects::CBioseq_Handle m_CurBioseq;
    CIRef<IBioseqEditor> m_BioseqEditor;

    CTextBlock m_MarkedSeq;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FLAT_FILE_CONTEXT__HPP
