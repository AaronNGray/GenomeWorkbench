#ifndef GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_TYPED_ITEMS__HPP
#define GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_TYPED_ITEMS__HPP

/*  $Id: desktop_typed_items.hpp 37477 2017-01-13 18:20:22Z asztalos $
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

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Contact_info.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seq_annot_handle.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <objmgr/seq_align_handle.hpp>
#include <objmgr/seq_graph_handle.hpp>

#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/brush.h>

BEGIN_NCBI_SCOPE

class IDesktopDataItem : public CObject
{
public:
    typedef vector<string> TLines;

    virtual ~IDesktopDataItem() {}
    virtual TLines GetDescription(bool root = false) const = 0;
    virtual wxColour GetTextColor() const = 0;
    virtual wxColour GetFrameColor() const = 0;
    virtual const wxBrush& GetBackgroundBrush() const = 0;
    virtual const string& GetType() const { return kEmptyStr; }
    virtual bool ShouldBeReduced() const { return false; }

    virtual const CObject* GetObject(void) const = 0;
    virtual objects::CSeq_entry_Handle GetSeqentryHandle() const = 0;
    
};

class NCBI_GUIWIDGETS_DESKTOP_EXPORT CDesktopBioseqItem : public IDesktopDataItem
{
public:
    CDesktopBioseqItem(const objects::CBioseq_Handle& bsh);
    ~CDesktopBioseqItem() {}

    virtual TLines GetDescription(bool root = false) const;
    virtual wxColour GetTextColor() const { return m_Color; }
    virtual wxColour GetFrameColor() const { return m_Color; }
    virtual const wxBrush& GetBackgroundBrush() const { return *wxWHITE_BRUSH; }
    virtual const string& GetType() const;

    virtual const CObject* GetObject(void) const;
    virtual objects::CSeq_entry_Handle GetSeqentryHandle() const;

    objects::CBioseq_Handle GetBioseqHandle() const { return m_Bsh; }
private:
    objects::CBioseq_Handle m_Bsh;
    wxColour m_Color;
};

class NCBI_GUIWIDGETS_DESKTOP_EXPORT CDesktopBioseqsetItem : public IDesktopDataItem
{
public:
    CDesktopBioseqsetItem(const objects::CBioseq_set_Handle& bssh);
    ~CDesktopBioseqsetItem() {}

    virtual TLines GetDescription(bool root = false) const;
    virtual wxColour GetTextColor() const { return *wxBLUE; }
    virtual wxColour GetFrameColor() const { return m_BkgdBrush.GetColour(); }
    virtual const wxBrush& GetBackgroundBrush() const { return m_BkgdBrush; }
    virtual const string& GetType() const;

    virtual const CObject* GetObject(void) const;
    virtual objects::CSeq_entry_Handle GetSeqentryHandle() const;

    objects::CBioseq_set_Handle GetBioseqsetHandle() const { return m_Bssh; }
private:
    void x_SetBrush();

    objects::CBioseq_set_Handle m_Bssh;
    wxBrush m_BkgdBrush;
};

class NCBI_GUIWIDGETS_DESKTOP_EXPORT CDesktopSeqSubmitItem : public IDesktopDataItem
{
public:
    CDesktopSeqSubmitItem(const objects::CSeq_submit& submit, objects::CScope& scope)
        : m_Submit(ConstRef(&submit)), m_Scope(&scope) {}
    ~CDesktopSeqSubmitItem() {}

    virtual TLines GetDescription(bool root = false) const;
    virtual wxColour GetTextColor() const { return *wxBLUE; }
    virtual wxColour GetFrameColor() const { return *wxBLUE; }
    virtual const wxBrush& GetBackgroundBrush() const { return *wxWHITE_BRUSH; }
    virtual const string& GetType() const;

    virtual const CObject* GetObject(void) const;
    virtual objects::CSeq_entry_Handle GetSeqentryHandle() const;

    const objects::CSeq_submit& GetSeqSubmit() const { return m_Submit.GetObject(); }
private:
    CConstRef<objects::CSeq_submit> m_Submit;
    mutable CRef<objects::CScope> m_Scope;
};

class NCBI_GUIWIDGETS_DESKTOP_EXPORT CDesktopContactInfoItem : public IDesktopDataItem
{
public:
    CDesktopContactInfoItem(const objects::CSeq_submit& submit, const objects::CContact_info& contact, objects::CScope& scope)
        : m_Submit(ConstRef(&submit)), m_Contact(ConstRef(&contact)), m_Scope(&scope) {}
    ~CDesktopContactInfoItem() {}

    virtual TLines GetDescription(bool root = false) const;
    virtual wxColour GetTextColor() const { return *wxBLACK; }
    virtual wxColour GetFrameColor() const { return *wxWHITE; }
    virtual const wxBrush& GetBackgroundBrush() const { return wxNullBrush; }
    virtual const string& GetType() const;

    virtual const CObject* GetObject(void) const;
    virtual objects::CSeq_entry_Handle GetSeqentryHandle() const;

    const objects::CSubmit_block& GetSubmitBlock() const;
    const objects::CContact_info& GetContactInfo() const { return m_Contact.GetObject(); }

private:
    CConstRef<objects::CSeq_submit> m_Submit;
    CConstRef<objects::CContact_info> m_Contact;
    mutable CRef<objects::CScope> m_Scope;
};

class NCBI_GUIWIDGETS_DESKTOP_EXPORT CDesktopCitSubItem : public IDesktopDataItem
{
public:
    CDesktopCitSubItem(const objects::CSeq_submit& submit, const objects::CCit_sub& sub, objects::CScope& scope)
        : m_Submit(ConstRef(&submit)), m_CitSub(ConstRef(&sub)), m_Scope(&scope) {}
    ~CDesktopCitSubItem() {}

    virtual TLines GetDescription(bool root = false) const;
    virtual wxColour GetTextColor() const { return *wxBLACK; }
    virtual wxColour GetFrameColor() const { return *wxWHITE; }
    virtual const wxBrush& GetBackgroundBrush() const { return wxNullBrush; }
    virtual const string& GetType() const;

    virtual const CObject* GetObject(void) const;
    virtual objects::CSeq_entry_Handle GetSeqentryHandle() const;

    const objects::CSubmit_block& GetSubmitBlock() const;
    const objects::CCit_sub& GetCitSub() const { return m_CitSub.GetObject(); }

private:
    CConstRef<objects::CSeq_submit> m_Submit;
    CConstRef<objects::CCit_sub> m_CitSub;
    mutable CRef<objects::CScope> m_Scope;
};

class NCBI_GUIWIDGETS_DESKTOP_EXPORT CDesktopSeqdescItem : public IDesktopDataItem
{
public:
    CDesktopSeqdescItem(const objects::CSeqdesc& desc, const objects::CSeq_entry_Handle& seh)
        : m_Seqdesc(ConstRef(&desc)), m_Seh(seh) {}
    ~CDesktopSeqdescItem() {}

    virtual TLines GetDescription(bool root = false) const;
    virtual wxColour GetTextColor() const { return *wxBLACK; }
    virtual wxColour GetFrameColor() const { return *wxWHITE; }
    virtual const wxBrush& GetBackgroundBrush() const { return wxNullBrush; }
    virtual const string& GetType() const;
    virtual bool ShouldBeReduced() const { return true; }
    virtual const CObject* GetObject(void) const;
    virtual objects::CSeq_entry_Handle GetSeqentryHandle() const { return m_Seh; }

    const objects::CSeqdesc& GetSeqdesc() const { return m_Seqdesc.GetObject(); }

    static TLines s_GetDescription(const objects::CUser_object& user_obj);
    static TLines s_GetDescription(const objects::COrg_ref& org);
    static TLines s_GetDescription(const objects::CSeqdesc::TModif& modif);
private:
    CConstRef<objects::CSeqdesc> m_Seqdesc;
    objects::CSeq_entry_Handle m_Seh;
};

class NCBI_GUIWIDGETS_DESKTOP_EXPORT CDesktopAnnotItem : public IDesktopDataItem
{
public:
    CDesktopAnnotItem(const objects::CSeq_annot_Handle& annoth);
    ~CDesktopAnnotItem() {}

    virtual TLines GetDescription(bool root = false) const;
    virtual wxColour GetTextColor() const { return m_Colour; }
    virtual wxColour GetFrameColor() const { return m_Colour; }
    virtual const wxBrush& GetBackgroundBrush() const { return *wxWHITE_BRUSH; }
    virtual const string& GetType() const;
    virtual const CObject* GetObject(void) const;
    virtual objects::CSeq_entry_Handle GetSeqentryHandle() const;

    objects::CSeq_annot_Handle GetAnnotHandle() const { return m_Annoth; }
private:
    objects::CSeq_annot_Handle m_Annoth;
    wxColour m_Colour;
};

class NCBI_GUIWIDGETS_DESKTOP_EXPORT CDesktopFeatItem : public IDesktopDataItem
{
public:
    CDesktopFeatItem(const objects::CSeq_feat_Handle& fh)
        : m_Feath(fh) {}
    ~CDesktopFeatItem() {}

    virtual TLines GetDescription(bool root = false) const;
    virtual wxColour GetTextColor() const { return *wxBLUE; }
    virtual wxColour GetFrameColor() const { return *wxBLUE; }
    virtual const wxBrush& GetBackgroundBrush() const { return wxNullBrush; }
    virtual const string& GetType() const;
    virtual bool ShouldBeReduced() const { return true; }

    virtual const CObject* GetObject(void) const;
    virtual objects::CSeq_entry_Handle GetSeqentryHandle() const;

    objects::CSeq_feat_Handle GetFeatHandle() const { return m_Feath; }

    static const string& s_GetProteinName(const objects::CSeq_feat_Handle& fh);
private:
    mutable objects::CSeq_feat_Handle m_Feath;
};

class NCBI_GUIWIDGETS_DESKTOP_EXPORT CDesktopAlignItem : public IDesktopDataItem
{
public:
    CDesktopAlignItem(const objects::CSeq_align_Handle& ah);
    ~CDesktopAlignItem() {}

    virtual TLines GetDescription(bool root = false) const;
    virtual wxColour GetTextColor() const { return m_Colour; }
    virtual wxColour GetFrameColor() const { return m_Colour; }
    virtual const wxBrush& GetBackgroundBrush() const { return wxNullBrush; }
    virtual const string& GetType() const;
    virtual bool ShouldBeReduced() const { return true; }

    virtual const CObject* GetObject(void) const;
    virtual objects::CSeq_entry_Handle GetSeqentryHandle() const;

    objects::CSeq_align_Handle GetAlignHandle() const { return m_Alignh; }

    static bool s_ContainsFarPointers(const objects::CSeq_align_Handle& ah);
private:
    objects::CSeq_align_Handle m_Alignh;
    wxColour m_Colour;
};

class NCBI_GUIWIDGETS_DESKTOP_EXPORT CDesktopGraphItem : public IDesktopDataItem
{
public:
    CDesktopGraphItem(const objects::CSeq_graph_Handle& gh)
        : m_Graphh(gh) {}
    ~CDesktopGraphItem() {}

    virtual TLines GetDescription(bool root = false) const;
    virtual wxColour GetTextColor() const { return *wxBLUE; }
    virtual wxColour GetFrameColor() const { return *wxBLUE; }
    virtual const wxBrush& GetBackgroundBrush() const { return wxNullBrush; }
    virtual const string& GetType() const;
    virtual bool ShouldBeReduced() const { return true; }

    virtual const CObject* GetObject(void) const;
    virtual objects::CSeq_entry_Handle GetSeqentryHandle() const;

    objects::CSeq_graph_Handle GetGraphHandle() const { return m_Graphh; }
private:
    objects::CSeq_graph_Handle m_Graphh;
};


END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_TYPED_ITEMS__HPP
