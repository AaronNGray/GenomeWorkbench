#ifndef GUI_OBJUTILS___CMD_PROMOTE_CDS__HPP
#define GUI_OBJUTILS___CMD_PROMOTE_CDS__HPP

/*  $Id: cmd_promote_cds.hpp 45082 2020-05-26 20:40:33Z asztalos $
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
 * Authors:  Colleen Bollin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_feat_handle.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CCmdPromoteCDS : 
    public CObject, public IEditCommand
{
public:
    CCmdPromoteCDS(const objects::CSeq_feat_Handle& orig_feat)
        : m_OrigFeat(orig_feat) {}

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    objects::CSeq_feat_Handle     m_OrigFeat;
    objects::CSeq_feat_EditHandle m_NewFeh;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_PROMOTE_CDS__HPP
