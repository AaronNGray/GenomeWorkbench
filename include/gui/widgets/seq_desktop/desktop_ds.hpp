#ifndef GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_DS__HPP
#define GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_DS__HPP

/*  $Id: desktop_ds.hpp 44449 2019-12-19 17:06:00Z filippov $
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

#include <corelib/ncbistl.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/widgets/seq_desktop/desktop_item.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CBioseq_Handle;
    class CBioseq_set_Handle;
    class CScope;
    class CSeq_submit;
END_SCOPE(objects)


// class responsible 
class CDesktopDataBuilder
{
public:
    CDesktopDataBuilder(const CSerialObject& so, objects::CScope& scope);

    CIRef<IDesktopItem> GetRootDesktopItem() const { return m_RootItem; }
private:
    /// Creates IDesktopItems depending on the type of input
    void x_CreateDesktopItems();

    void x_CreateSeqSubmit();
    void x_CreateSeq(const objects::CBioseq_Handle& bsh, CIRef<IDesktopItem> parent);
    void x_CreateSet(const objects::CBioseq_set_Handle& bssh, CIRef<IDesktopItem> parent);
    void x_CreateAnnot(const objects::CSeq_annot_Handle& ah, CIRef<IDesktopItem> parent);

    CIRef<IDesktopItem> m_RootItem;
    objects::CSeq_entry_Handle m_Seh;
    CConstRef<objects::CSeq_submit> m_SeqSubmit;
    CRef<objects::CScope> m_Scope;
    objects::CSeq_annot_Handle m_AnnotHandle;
};

END_NCBI_SCOPE

#endif  
    // GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_DS__HPP

