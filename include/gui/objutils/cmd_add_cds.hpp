#ifndef GUI_OBJUTILS___CMD_ADD_CDS__HPP
#define GUI_OBJUTILS___CMD_ADD_CDS__HPP

/*  $Id: cmd_add_cds.hpp 42385 2019-02-13 16:34:44Z asztalos $
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
 * Authors:  Colleen Bollin, based on a file by Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objmgr/seq_feat_handle.hpp>

#include <gui/utils/command_processor.hpp>

#include <objmgr/seq_entry_handle.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CCmdAddCDS : 
    public CObject, public IEditCommand
{
public:
    CCmdAddCDS(objects::CSeq_entry_Handle& seh, const objects::CSeq_feat& cds, CRef<objects::CSeq_feat> prot, bool create_general_only)
        : m_seh(seh), m_Prot(prot), m_create_general_only(create_general_only)
    {
        m_CDS.Reset(new objects::CSeq_feat);
        m_CDS->Assign(cds);
    }

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    objects::CSeq_entry_Handle m_seh;
    CRef<objects::CSeq_feat>  m_CDS;
    CRef<objects::CSeq_feat>  m_Prot;
    objects::CSeq_feat_EditHandle  m_feh_CDS;
    objects::CSeq_annot_EditHandle m_FTableCreated;
    objects::CBioseq_EditHandle    m_ProtHandle;
    bool m_create_general_only;
};


class NCBI_GUIOBJUTILS_EXPORT CCmdCreateCDS : public CObject, public IEditCommand
{
public:
    CCmdCreateCDS(objects::CSeq_entry_Handle& seh, const objects::CSeq_feat& cds, CRef<objects::CSeq_feat> prot, const vector<CRef<objects::CSeq_id> > &new_prot_id,
        const vector<CRef<objects::CSeq_feat> > &other_prot_feats = vector<CRef<objects::CSeq_feat> >())
        : m_seh(seh), m_Prot(prot), m_prot_id(new_prot_id), m_other_prot_feats(other_prot_feats)
    {
        m_CDS.Reset(new objects::CSeq_feat);
        m_CDS->Assign(cds);
    }

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

    static string s_GetProductSequence(const objects::CSeq_feat& cds, objects::CScope& scope);
protected:

    void x_AddMolinfoDescriptorToProtein();
    void x_AddProteinFeatures(const TSeqPos& seq_length);

    objects::CSeq_entry_Handle m_seh;
    CRef<objects::CSeq_feat>  m_CDS;
    CRef<objects::CSeq_feat>  m_Prot;
    objects::CSeq_feat_EditHandle  m_feh_CDS;
    objects::CSeq_annot_EditHandle m_FTableCreated;
    objects::CBioseq_EditHandle    m_ProtHandle;
    vector<CRef<objects::CSeq_id> > m_prot_id;
    vector<CRef<objects::CSeq_feat> > m_other_prot_feats;
};


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_ADD_CDS__HPP
