#ifndef GUI_UTILS___GUI_OBJECT_INFO_SEQ_ALIGN__HPP
#define GUI_UTILS___GUI_OBJECT_INFO_SEQ_ALIGN__HPP

/*  $Id: gui_object_info_seq_align.hpp 43343 2019-06-17 20:41:12Z evgeniev $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <corelib/ncbistr.hpp>
#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_object_info.hpp>
#include <objects/seqalign/Seq_align.hpp>


/** @addtogroup GUI_UTILS
 *
 * @{
 */

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CUser_field;
END_SCOPE(objects)

class NCBI_GUIOBJUTILS_EXPORT CGuiObjectInfoSeq_align
    : public CObject
    , public IGuiObjectInfo
{
public:
    static CGuiObjectInfoSeq_align* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual string GetType() const { return "Alignment"; }
    virtual string GetSubtype() const { return ""; }
    virtual string GetLabel() const;
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title,
        TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
    virtual void GetLinks(ITooltipFormatter& /*links*/, bool /*no_ncbi_base*/) const;
    virtual string GetIcon() const { return "symbol::alignment"; }
    virtual string GetViewCategory() const { return "Alignment"; }
    void SetAnchorRow(int anchor)  { m_AnchorRow = anchor; }
    void SetFlippedStrands(bool flipped = true) { m_FlippedStrands = flipped; }
    virtual string GetSRAReadStatus() const;
    virtual double GetCoverage() const;

    static bool IsPolyA(const objects::CBioseq_Handle& bsh, TSeqPos start, TSeqPos stop);

protected:
    void x_AddUserObject(ITooltipFormatter& tooltip, const objects::CUser_field &user_object) const;
private:
    CConstRef<objects::CSeq_align> m_Align;
    mutable CRef<objects::CScope> m_Scope;
    /// anchor sequence row number, default is -1 (unknown)
    int  m_AnchorRow;
    bool m_FlippedStrands = false;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___GUI_OBJECT_INFO_SEQ_ALIGN__HPP
