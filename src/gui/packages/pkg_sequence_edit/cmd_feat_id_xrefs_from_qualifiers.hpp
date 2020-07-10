#ifndef GUI_PACKAGES_PKG_SEQUENCE_EDIT___CMD_FEAT_ID_XREFS_FROM_PROTEIN_ID_QUALIFIERS__HPP
#define GUI_PACKAGES_PKG_SEQUENCE_EDIT___CMD_FEAT_ID_XREFS_FROM_PROTEIN_ID_QUALIFIERS__HPP

/*  $Id: cmd_feat_id_xrefs_from_qualifiers.hpp 41095 2018-05-22 18:06:27Z katargir $
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
 * Authors:  Andrea Asztalos
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/objutils/cmd_dynamic.hpp>
#include <objmgr/seq_entry_handle.hpp>

BEGIN_NCBI_SCOPE

class CCmdFeatIdXrefsFromQualifiers :  public CCmdDynamic
{
public:
    static CIRef<IEditCommand> Create(const objects::CSeq_entry_Handle& seh, const vector<string>& quals);

    /// @name IEditCommand interface implementation
    /// @{
    virtual string GetLabel();
    /// @}

protected:
    virtual CIRef<IEditCommand> x_CreateActionCmd();

private:
    CCmdFeatIdXrefsFromQualifiers(const objects::CSeq_entry_Handle& seh, const vector<string>& quals) : m_seh(seh), m_Quals(quals) {}

    objects::CSeq_entry_Handle m_seh;
    vector<string>             m_Quals;
};

END_NCBI_SCOPE

#endif  // GUI_PACKAGES_PKG_SEQUENCE_EDIT___CMD_FEAT_ID_XREFS_FROM_PROTEIN_ID_QUALIFIERS__HPP
