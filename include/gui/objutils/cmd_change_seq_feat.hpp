#ifndef GUI_OBJUTILS___CMD_CHANGE_SEQ_FEAT__HPP
#define GUI_OBJUTILS___CMD_CHANGE_SEQ_FEAT__HPP

/*  $Id: cmd_change_seq_feat.hpp 32428 2015-02-26 16:30:16Z bollin $
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

#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_feat_handle.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CCmdChangeSeq_feat : 
    public CObject, public IEditCommand
{
public:
    CCmdChangeSeq_feat(const objects::CSeq_feat_Handle& orig_feat, const objects::CSeq_feat& new_feat, bool promote = false)
        : m_Orig_feat(orig_feat), m_New_feat(&new_feat), m_PromoteRequested(promote), m_ShouldDemote(false) {}

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    objects::CSeq_feat_Handle     m_Orig_feat;
    CConstRef<objects::CSeq_feat> m_New_feat;
    bool m_PromoteRequested;
    bool m_ShouldDemote;

    void x_Switch();
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_CHANGE_SEQ_FEAT__HPP
