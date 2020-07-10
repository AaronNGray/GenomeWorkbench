#ifndef GUI_WIDGETS_WX__IBIOSEQ_EDITOR__HPP
#define GUI_WIDGETS_WX__IBIOSEQ_EDITOR__HPP

/*  $Id: ibioseq_editor.hpp 44907 2020-04-14 13:03:07Z asztalos $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/scope.hpp>

#include <objtools/format/items/item.hpp>

#include <wx/clntdata.h>
#include <wx/event.h>
#include <wx/string.h>

class wxMenu;

BEGIN_NCBI_SCOPE

class ICommandProccessor;

class CTitleEdit : public CObject
{
public:
    void   SetTitle(const string& title) { m_Title = title; }
    string GetTitle() const { return m_Title; }

private:
    string m_Title;
};

class CEditObject
{
public:
    CEditObject() {}
    CEditObject(const CObject& obj, const objects::CSeq_entry_Handle& seh)
        : m_Obj(&obj), m_SEH(seh) {}
    CEditObject(const CObject& obj, objects::CScope& scope)
        : m_Obj(&obj), m_Scope(&scope) {}

    CConstRef<CObject> m_Obj;
    objects::CSeq_entry_Handle m_SEH;
    CRef<objects::CScope> m_Scope;
};

typedef vector<CEditObject> TEditObjectList;
typedef vector<CConstRef<objects::IFlatItem> > TFlatItemList;

class IBioseqEditorCB
{
public:
    virtual ~IBioseqEditorCB() {}

    virtual void GetItemSelection(TFlatItemList&) = 0;         // FlaFile view returns it's selection here
    virtual void GetCurrentSelection(TEditObjectList&) = 0;    // ASN.1 view returns it's selection here
    virtual objects::CBioseq_Handle GetCurrentBioseq() = 0;    // only FlaFile view implements it
    virtual wxString GetWorkDir() const = 0;                   // folder in which original edited file resides (if available)
    virtual bool MayCreateFeatures() const { return true; }    // override it in CSeqGraphicWidget
};

enum EBioseqEditBaseCommands {
    eCmdEditObjects = 29000,
    eCmdDeleteObjects,
    eBioseqEditBaseCommandsEnd
};

/////////////////////////////////////////////////////////////////////////////
/// IBioseqEditor
/////////////////////////////////////////////////////////////////////////////

class IBioseqEditor : public wxEvtHandler
{
public:
    enum {
        kObjectEditable = 1,
        kObjectDeletable = 2
    };

    virtual void SetCallBack(IBioseqEditorCB* cb) = 0;

    virtual wxMenu* CreateContextMenu(bool shorter = false) const = 0;
    virtual wxMenu* CreateBioseqMenu() const = 0;
    virtual void EditSelection() = 0;

    virtual int GetEditFlags(CEditObject& editObj) const = 0;             // called from ASN.1 view
    virtual int GetEditFlags(const objects::IFlatItem& item) const = 0;   // called from FlatFile view
};

END_NCBI_SCOPE

#endif //GUI_WIDGETS_WX__IBIOSEQ_EDITOR__HPP
