#ifndef GUI_WIDGETS_EDIT___BIOSEQ_EDITOR__HPP
#define GUI_WIDGETS_EDIT___BIOSEQ_EDITOR__HPP

/*  $Id: bioseq_editor.hpp 44907 2020-04-14 13:03:07Z asztalos $
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

#include <objects/seqfeat/Seq_feat.hpp>
#include <gui/utils/extension.hpp>
#include <gui/objutils/cmd_composite.hpp>

#include <gui/widgets/wx/ibioseq_editor.hpp>
#include <gui/widgets/wx/ieditor_factory.hpp>
#include <gui/widgets/wx/ui_command.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeqdesc;
    class CSeq_feat;
END_SCOPE(objects)

class IEditObject;

enum EBioseqEditCommands   {
        eCmdEditBiosourceDesc = eBioseqEditBaseCommandsEnd,
        eCmdDeleteBiosourceDesc,
        eCmdCreateBiosourceDesc,
        eCmdEditBiosourceFeat,
        eCmdDeleteBiosourceFeat,
        eCmdCreateBiosourceFeat,
        eCmdCreateCodingRegion,
        eCmdCreateCodingRegion_ext,
        eCmdCreateGene,
        eCmdCreateProt,
        eCmdCreateLTR,
        eCmdCreateRepeatRegion,
        eCmdCreateStemLoop,
        eCmdCreateSTS,
        eCmdCreateRegion,
        eCmdCreateSecondaryStructure,
        eCmdCreateProprotein,
        eCmdCreateMatPeptide,
        eCmdCreateSigPeptide,
        eCmdCreateTransitPeptide,
        eCmdCreateOperon,
        eCmdCreateMobileElement,
        eCmdCreateCentromere,
        eCmdCreateTelomere,
        eCmdCreateRegulatory,
        eCmdCreateRegulatory_promoter,
        eCmdCreateRegulatory_enhancer,
        eCmdCreateRegulatory_ribosome_binding_site,
        eCmdCreateRegulatory_riboswitch,
        eCmdCreateRegulatory_terminator,
        eCmdCreatePreRNA,
        eCmdCreateMRNA,
        eCmdCreateExon,
        eCmdCreateIntron,
        eCmdCreate3UTR,
        eCmdCreate5UTR,
        eCmdCreatetRNA,
        eCmdCreaterRNA,
        eCmdCreateRNA,
        eCmdCreatemiscRNA,
        eCmdCreatepreRNA,
        eCmdCreatencRNA,
        eCmdCreatetmRNA,
        eCmdCreatePolyASite,
        eCmdCreatePrimerBind,
        eCmdCreateProteinBind,
        eCmdCreateRepOrigin,
        eCmdCreateBond,
        eCmdCreateSite,
        eCmdCreateOriT,
        eCmdCreateCRegion, 
        eCmdCreateDLoop, 
        eCmdCreateDSegment, 
        eCmdCreateiDNA,
        eCmdCreateJSegment, 
        eCmdCreateMiscBinding,
        eCmdCreateMiscDifference,  
        eCmdCreateMiscFeature, 
        eCmdCreateMiscRecomb,
        eCmdCreateMiscStructure, 
        eCmdCreateModifiedBase,  
        eCmdCreateNRegion, 
        eCmdCreatePrimTranscript,
        eCmdCreateSRegion, 
        eCmdCreateUnsure, 
        eCmdCreateVRegion, 
        eCmdCreateVSegment,   
        eCmdCreateVariation,
        eCmdCreateAssemblyGap,
        eCmdCreateTPAAssembly,
        eCmdCreateStructuredComment,
        eCmdCreateDBLink,
        eCmdCreateUnverified,
        eCmdCreateAuthorizedAccess,
        eCmdCreateRefGeneTracking,
        eCmdCreateRegionDescriptor,
        eCmdCreateCommentDescriptor,
        eCmdCreateTitle,
        eCmdCreateMolInfo,
        eCmdCreatePubDescriptor,
        eCmdCreatePubDescriptorLabeled,
        eCmdCreatePubFeatureLabeled,
        eCmdPropagateAllFeatures,
        eCmdPropagateSelectedFeatures,
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CBioseqEditor :
    public CObject,
    public IBioseqEditor,
    public IMenuContributor
{
    DECLARE_EVENT_TABLE()
public:

    CBioseqEditor(ICommandProccessor& cmdProccessor, IGuiWidgetHost* guiWidgetHost) : m_CmdProccessor(cmdProccessor), m_CB(nullptr), m_GuiWidgetHost(guiWidgetHost) {}

    virtual void SetCallBack(IBioseqEditorCB* cb) { m_CB = cb; }
    virtual wxMenu* CreateContextMenu(bool shorter = false) const;
    virtual wxMenu* CreateBioseqMenu() const;
    virtual void EditSelection();

    virtual int GetEditFlags(CEditObject& editObj) const;
    virtual int GetEditFlags(const objects::IFlatItem& item) const;

    bool IsDeletable(CEditObject& editObj) const;
    bool IsEditable(CEditObject& editObj) const;

    bool IsDeletable(const objects::IFlatItem& item) const;
    bool IsEditable(const objects::IFlatItem& item) const;

    /// @name IMenuContributor interface implementation
    /// @{
    virtual const SwxMenuItemRec* GetMenuDef() const;
    /// @}

    void OnEditBiosourceDesc(wxCommandEvent& event);
    void OnUpdateEditBiosourceDesc(wxUpdateUIEvent& event);
    void OnCreateBiosourceDesc(wxCommandEvent& event);
    void OnUpdateCreateBiosourceDesc(wxUpdateUIEvent& event);
    void OnDeleteBiosourceDesc(wxCommandEvent& event);
    void OnUpdateDeleteBiosourceDesc(wxUpdateUIEvent& event);

    void OnEditBiosourceFeat(wxCommandEvent& event);
    void OnUpdateEditBiosourceFeat(wxUpdateUIEvent& event);
    void OnCreateBiosourceFeat(wxCommandEvent& event);
    void OnUpdateCreateBiosourceFeat(wxUpdateUIEvent& event);
    void OnDeleteBiosourceFeat(wxCommandEvent& event);
    void OnUpdateDeleteBiosourceFeat(wxUpdateUIEvent& event);

    void OnPropagateAllFeatures(wxCommandEvent& event);
    void OnUpdatePropagateAllFeatures(wxUpdateUIEvent& event);
    void OnPropagateSelectedFeatures(wxCommandEvent& event);
    void OnUpdatePropagateSelectedFeatures(wxUpdateUIEvent& event);

    // feature menu items
    void OnCreateFeature(CRef<objects::CSeq_feat> feat, wxCommandEvent& event);
    void OnUpdateNucleotideFeat(wxUpdateUIEvent& event);
    void OnUpdateProteinFeat(wxUpdateUIEvent& event);
    void OnUpdateAnyFeat(wxUpdateUIEvent& event);
    void OnCreateCodingRegion(wxCommandEvent& event);
    void OnCreateGene(wxCommandEvent& event);
    void OnCreateProt(wxCommandEvent& event);
    void OnCreateImport(wxCommandEvent& evt);
    void OnCreateRegion(wxCommandEvent& evt);
    void OnCreateSecondaryStructure(wxCommandEvent& evt);
    void OnCreateRNA(wxCommandEvent& evt);
    void OnCreateSite(wxCommandEvent& event);
    void OnCreateBond(wxCommandEvent& event);
    void OnCreatePubFeat(wxCommandEvent& event);
    void OnCreateDescriptor(wxCommandEvent& evt);

    void OnEditSelection(wxCommandEvent& event);
    void OnUpdateEditSelection(wxUpdateUIEvent& event);

    void OnDeleteSelection(wxCommandEvent& event);
    void OnUpdateDeleteSelection(wxUpdateUIEvent& event);

    virtual bool ProcessEvent(wxEvent& event);

    static objects::CSeqFeatData::ESubtype GetFeatTypeFromCmdID (int cmd_id);
    static CRef<objects::CSeq_feat>  MakeDefaultFeature(objects::CSeqFeatData::ESubtype subtype);

private:
    void x_DeleteObject(objects::CSeq_entry_Handle& seh, const objects::CSeqdesc& seqDesq);
    void x_DeleteObject(objects::CSeq_entry_Handle& seh, const objects::CSeq_feat& seqFeat);
    bool x_DeleteFromSingleSeq(objects::CSeq_entry_Handle& seh, objects::CSeq_entry_Handle& sehdesc, const objects::CSeqdesc& seqDesq, bool &any, bool& cancel);
    CIRef<IEditCommand> x_GetDeleteCommand(objects::CSeq_entry_Handle& seh, const objects::CSeqdesc& seqDesc, bool from_single);
    CRef<CCmdComposite> x_GetDeleteCommand(objects::CSeq_entry_Handle& seh, const objects::CSeq_feat& seqFeat);
    CRef<CCmdComposite> x_GetDeleteCommand(objects::CSeq_entry_Handle& seh, const objects::CSeq_annot& annot);
    CIRef<IEditCommand> x_GetDeleteCommand(objects::CSeq_entry_Handle& seh, const objects::CSeq_align& align);
    void x_EditObject(CIRef<IEditObject> edit, const string& title = kEmptyStr);

    void x_ReportError(const string& cmd, const std::exception& e);

    bool x_HandleFeatCreationHere(wxCommandEvent& event);
    bool x_HaveBiosourceDesc();
    bool x_HaveBiosourceFeat();
    int  x_GetFromPosition(void);
    bool x_HasAlignment(void);
    bool x_IsNa();

    ICommandProccessor& m_CmdProccessor;
    IBioseqEditorCB* m_CB;
    IGuiWidgetHost* m_GuiWidgetHost;
};

NCBI_GUIWIDGETS_EDIT_EXPORT CIRef<IEditObject> CreateEditorForObject(CConstRef<CObject> obj, objects::CSeq_entry_Handle seh, bool create);

class NCBI_GUIWIDGETS_EDIT_EXPORT CBioseqEditorFactory :
    public CObject,
    public IExtension,
    public IEditorFactory
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IEditorFactory interface implementation
    /// @{
    virtual void RegisterIconAliases(wxFileArtProvider& provider) {}
    virtual void RegisterCommands(CUICommandRegistry&, wxFileArtProvider&);

    virtual CObject* CreateEditor(const string& interface_name, ICommandProccessor& cmdProccessor, IGuiWidgetHost* guiWidgetHost);
    /// @}
    static void CollectBioseqEditorCommands(vector<CUICommand*> &commands);

};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___BIOSEQ_EDITOR__HPP
