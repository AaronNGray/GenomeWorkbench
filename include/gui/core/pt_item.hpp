#ifndef GUI_CORE___PT_ITEM__HPP
#define GUI_CORE___PT_ITEM__HPP

/*  $Id: pt_item.hpp 33078 2015-05-21 15:06:52Z katargir $
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
 *  CItem - represents an item in Application Explorer service.
 */

#include <corelib/ncbiobj.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/treebase.h>

class wxTreeCtrl;
class wxMenu;
class wxEvtHandler;

BEGIN_NCBI_SCOPE

namespace PT
{
    class  CItem : public wxTreeItemData
    {
    public:
        CItem();
        virtual ~CItem();

        static void LogInstanceCount();

        wxString GetLabel(wxTreeCtrl& treeCtrl) const;

        CItem*  GetParent(wxTreeCtrl& treeCtrl) const;

        wxTreeItemId GetTreeItemId() const { return m_TreeItemId; }
        void SetTreeItemId(wxTreeItemId treeItemId) { m_TreeItemId = treeItemId; }

        void    AppendChildItem(wxTreeCtrl& treeCtrl, CItem& ch_item);
        void    InsertChildItem(wxTreeCtrl& treeCtrl, CItem& ch_item, size_t pos);
    
        virtual int GetType() const  = 0;

        virtual void OnItemExpandedCollapsed(wxTreeCtrl&) {}

        virtual bool CanDoNewFolder() const { return false; }
        virtual void DoNewFolder(wxTreeCtrl&) {}

        virtual bool CanDoProperties() { return false; }
        virtual bool DoProperties(wxTreeCtrl&) { return false; }

        virtual void BeginLabelEdit(wxTreeCtrl&, wxTreeEvent& event) { event.Veto(); }
        virtual bool EndLabelEdit(wxTreeCtrl&, wxTreeEvent& event) { event.Veto(); return false; }

        virtual bool CanDoRemove(wxTreeCtrl&) const { return false; }
        virtual bool DoRemove(wxTreeCtrl&) { return false; }

        virtual bool CanCopyToClipboard(wxTreeCtrl&) const { return false; }
        virtual bool CanCutToClipboard(wxTreeCtrl&) const { return false; }
        virtual bool CanPaste(wxTreeCtrl&) const { return false; }
        virtual bool Paste(wxTreeCtrl&, vector<CItem*>&, bool /*move*/) { return false; }

    protected:
        wxTreeItemId m_TreeItemId;

        #ifdef _DEBUG
        static size_t m_InstanceCount;
        #endif
    };

    class CChildIterator
    {
    public:
        CChildIterator(wxTreeCtrl& treeCtrl, const CItem& item);

        CChildIterator& operator++ (void);
        CItem& operator*  (void) const;
        operator bool() const;

    private:
        wxTreeCtrl& m_TreeCtrl;
        wxTreeItemIdValue m_Cookie;
        wxTreeItemId m_Item;
        wxTreeItemId m_Child;
    };

    typedef vector<CItem*>  TItems;

    enum EItemType {
        eRoot              = 0x001,
        eWorkspace         = 0x002,
        eProject           = 0x004,
        eProjectFolder     = 0x008,
        eProjectItem       = 0x010,
        eDataSource        = 0x020,
        eView              = 0x040,
        eFolder            = 0x080,
        eDataLoader        = 0x100,
        eHiddenItems       = 0x200
    };

    template<class TData, EItemType Type> class TPTItem : public CItem
    {
    public:
        typedef TData TDataType;
        TPTItem(const TData& data)
            : CItem(), m_Data(data) {}

        const TData&    GetData() const { return m_Data; }
        TData& GetData() { return m_Data; }
        void SetData(const TData&  data) { m_Data = data; }
        virtual int GetType() const { return Type; }

    protected:
        TData   m_Data;
    };
} //namespace PT

///////////////////////////////////////////////////////////////////////////////
/// IExplorerItemCmdContributor - interface representing a component that
/// contributes commands applicable to Explorer Items.
class IExplorerItemCmdContributor
{
public:
    /// Contribution consists of a Menu object and event handler
    typedef pair<wxMenu*, wxEvtHandler*>    TContribution;

    /// for the given set of items returns a contribution
    virtual TContribution   GetMenu(wxTreeCtrl& treeCtrl, PT::TItems& items) = 0;

    virtual ~IExplorerItemCmdContributor()  {};
};

END_NCBI_SCOPE

#endif  /// GUI_CORE___PT_ITEM__HPP
