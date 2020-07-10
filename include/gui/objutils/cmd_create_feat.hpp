#ifndef GUI_OBJUTILS___CMD_CREATE_FEAT__HPP
#define GUI_OBJUTILS___CMD_CREATE_FEAT__HPP

/*  $Id: cmd_create_feat.hpp 42389 2019-02-13 19:05:46Z asztalos $
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
#include <objects/seqfeat/Seq_feat.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <gui/gui_export.h>

#include <gui/objutils/cmd_dynamic.hpp>

#include <objmgr/seq_entry_handle.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CCmdCreateFeat : 
    public CObject, public IEditCommand
{
public:
    CCmdCreateFeat(objects::CSeq_entry_Handle& seh, const objects::CSeq_feat& feat)
        : m_seh(seh), m_Feat(&feat)
    {
    }

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}
    
protected:
    objects::CSeq_entry_Handle m_seh;
    CConstRef<objects::CSeq_feat>  m_Feat;
    objects::CSeq_feat_EditHandle  m_feh;
    objects::CSeq_annot_EditHandle m_FTableCreated;
};

class NCBI_GUIOBJUTILS_EXPORT CCmdCreateFeatBioseq : public CCmdDynamic
{
public:
    CCmdCreateFeatBioseq(objects::CBioseq_Handle& bsh, const objects::CSeq_feat& feat)
        : m_bsh(bsh), m_Feat(&feat)
    {
    }

    /// @name IEditCommand interface implementation
    /// @{
    virtual string GetLabel();
    /// @}
    
protected:
    virtual CIRef<IEditCommand> x_CreateActionCmd();

    objects::CBioseq_Handle m_bsh;
    CConstRef<objects::CSeq_feat>  m_Feat;
};


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_CREATE_FEAT__HPP
