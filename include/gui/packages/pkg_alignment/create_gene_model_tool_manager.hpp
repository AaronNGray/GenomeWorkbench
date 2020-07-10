#ifndef PKG_ALIGNMENT___CREATE_GENE_MODEL_TOOL_MANAGER__HPP
#define PKG_ALIGNMENT___CREATE_GENE_MODEL_TOOL_MANAGER__HPP

/*  $Id: create_gene_model_tool_manager.hpp 42821 2019-04-18 19:32:56Z joukovv $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/core/algo_tool_manager_base.hpp>

#include <gui/packages/pkg_alignment/create_gene_model_params_panel.hpp>

#include <objects/general/Object_id.hpp>

#include <gui/core/select_project_options.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CCmdComposite;
///////////////////////////////////////////////////////////////////////////////
/// CCreateGeneModelToolManager
class  CCreateGeneModelToolManager :
    public CAlgoToolManagerBase
{
public:
    CCreateGeneModelToolManager();

    /// @name overriding IUIToolManager functions implemented in CAlgoToolManagerBase
    /// @{
    virtual void    InitUI();
    virtual void    CleanUI();
    virtual IAppTask*   GetTask();
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

protected:
    /// @name overriding CAlgoToolManagerBase virtual functions
    /// @{
    virtual CAlgoToolManagerParamsPanel* x_GetParamsPanel();
    virtual IRegSettings* x_GetParamsAsRegSetting();
    virtual void x_CreateParamsPanelIfNeeded();
    virtual bool x_ValidateParams();
    virtual CDataLoadingAppJob* x_CreateLoadingJob();
    
    /// @}

    void x_SelectCompatibleInputObjects();
    void x_RemoveDuplicates();

protected:
    TConstScopedObjects     m_Alignments;

    SCreateGeneModelParams  m_Params;
    CCreateGeneModelParamsPanel*    m_ParamsPanel;
};

// mrna only
// cds only
// mrna + cds
class SGeneCdsmRnaFeats
{
public:
    SGeneCdsmRnaFeats() : gene(), cds(), mRNA() {}
    ~SGeneCdsmRnaFeats(){}

    CConstRef<objects::CSeq_id> GetID(objects::CScope* scope) const;
    // placing feature id Xrefs between:
    // mRNA <=> CDS, mRNA => gene, CDS => gene ( create a link to the parent gene feature) (CR0011)
    // additionaly: add a dbxref for the GeneID to RNA and CDS features (CR0006)
    void CrossLinkTriple();
    static void s_CreateXRefLink(objects::CSeq_feat& from_feat, objects::CSeq_feat& to_feat);
    static void s_CreateDBXref(const objects::CSeq_feat& from_feat, objects::CSeq_feat& to_feat, const string& dbname);

    CRef<objects::CSeq_feat> gene;
    CRef<objects::CSeq_feat> cds;
    CRef<objects::CSeq_feat> mRNA;
};


///////////////////////////////////////////////////////////////////////////////
/// CCreateGeneModelTask
class CCreateGeneModelTask : public CAppTask
{
public:
    typedef CRef<objects::CLoaderDescriptor>   TLoaderRef;
    typedef CRef<objects::CProjectItem>        TItemRef;
    /// a loader might be tied with a specific project item.
    /// if so, we need to know such that they will go together and
    /// get added to the same project.

    typedef list<SGeneCdsmRnaFeats> TGeneCDSmRNAList;
    typedef map<objects::CSeq_id_Handle, TGeneCDSmRNAList> TGeneModelMap;

    CCreateGeneModelTask(const string& descr, 
        CProjectService* service, 
        CSelectProjectOptions& options, 
        const SCreateGeneModelParams& params) 
        : CAppTask(descr), m_Service(service), m_Options(options), m_Params(params) {}
    virtual ~CCreateGeneModelTask();

    static objects::CObject_id::TId s_GetGeneID(const objects::CSeq_feat& feat);
    static objects::CObject_id::TId s_FindHighestFeatId(const objects::CSeq_entry_Handle seh);
    // return true if there is at least one local feat id
    static bool s_FeaturesHaveIDs(const objects::CSeq_annot& annot);
    // return true if there is at least one feature Xref to a local id
    static bool s_FeaturesHaveXrefs(const objects::CSeq_annot& annot);
    // local identifiers are assigned to each new feature (CR0010)
    static void s_AssignFeatureIds(objects::CSeq_annot& annot, objects::CObject_id::TId& max_id);

    static const string sGeneDbName;

protected:
    virtual ETaskState    x_Run();

private:
    CRef<CCmdComposite> x_GetCommand();

    void x_StoreGeneratedFeats(const objects::CSeq_annot& annot, TGeneCDSmRNAList& gene_cds_rna) const;

    void x_Group_By_GeneID(objects::CSeq_annot& annot, const objects::CSeq_id_Handle& idh);

    void x_GroupGenes(const objects::CSeq_id_Handle& idh, CRef<objects::CSeq_feat> gene);

    void x_AssignIDsAndCrossLinkFeats(objects::CSeq_annot& annot, const objects::CSeq_id_Handle& idh, objects::CObject_id::TId max_id);

    CRef<CCmdComposite> x_AssignIDsAndUpdateGenes(objects::CSeq_annot& annot, const objects::CSeq_id_Handle& idh, objects::CSeq_entry_Handle seh);
    
    void x_GetUpdatedGeneCommand(objects::CSeq_entry_Handle seh, const objects::CSeq_id_Handle& idh, CCmdComposite* cmd);
    
    bool x_UpdateGeneOnSequence(TGeneCDSmRNAList& gene_cds_rna, CRef<objects::CSeq_feat> gene);

    CIRef<CProjectService> m_Service;
    CSelectProjectOptions  m_Options;
    SCreateGeneModelParams  m_Params;
    TGeneModelMap m_GeneModelMap;

    CSelectProjectOptions::TItems m_Items;
    CRef<objects::CScope>  m_Scope;
};

/* @} */

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___CREATE_GENE_MODEL_TOOL_MANAGER__HPP
