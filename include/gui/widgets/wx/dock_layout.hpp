#ifndef GUI_WX_DEMO___DOCK_LAYOUT__HPP
#define GUI_WX_DEMO___DOCK_LAYOUT__HPP

/*  $Id: dock_layout.hpp 34941 2016-03-03 15:58:19Z katargir $
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
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/wm_client.hpp>

class wxWindow;


BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CUser_object;
END_SCOPE(objects)

///////////////////////////////////////////////////////////////////////////////
/// CDockLayoutTree - represents hierarchical layout in a Dock Container
/// as a tree where nodes correspond to windows. Terminal nodes correspond to
/// clients (IWMClient-s), other nodes - to containers such as Notebooks and
/// Splitters.
/// Dock Container contains two instances of CDockLayoutTree - one represents
/// only windows actually visible on screen (Visible Tree), the other contains
/// node for both shown and hidden windows. Visible nodes in both trees are
/// linked to each other.

class NCBI_GUIWIDGETS_WX_EXPORT CDockLayoutTree
    : public CObject
{
public:
    typedef IWMClient::CFingerprint TFingerprint;

    enum ENodeType {
        eClient,
        eHorzSplitter,
        eVertSplitter,
        eTab
    };

    ///////////////////////////////////////////////////////////////////////////
    /// CNode - a node in layout tree corresponding to a window
    class NCBI_GUIWIDGETS_WX_EXPORT CNode : public CObject    {
    public:
        friend class CDockLayoutTree;

        typedef vector< CRef<CNode> >   TNodeVector;
        typedef IWMClient::CFingerprint TFingerprint;

        CNode();
        CNode(const CNode& orig);
        CNode(ENodeType type, wxWindow* window = NULL, bool hidden = false);
        virtual ~CNode();

        /// @name Accessors
        /// @{
        ENodeType   GetType()   const   {   return m_Type;    }
        bool    IsClient()  const       {   return m_Type == eClient;   }
        bool    IsContainer() const     {   return m_Type != eClient;   }
        bool    IsTab() const           {   return m_Type == eTab;  }
        bool    IsSplitter() const      {   return m_Type == eHorzSplitter  ||  m_Type == eVertSplitter;  }

        bool    IsHidden() const    {   return m_Hidden;    }
        void    SetHidden(bool set) {   m_Hidden = set; }
        bool    IsVisible() const   {   return ! m_Hidden;  }

        CRef<CNode> GetParent()             {   return m_Parent;    }
        CConstRef<CNode> GetParent() const  {   return m_Parent;    }

        CNode* GetClone()               {   return m_Clone;    }
        const CNode* GetClone() const   {   return m_Clone;    }

        bool    HasChildren()   const   {   return ! m_Children.empty();    }
        TNodeVector&    GetChildren()               {   return m_Children;  }
        const TNodeVector&    GetChildren() const   {   return m_Children;  }

        wxWindow* GetWindow()   {   return m_Window;    }
        const wxWindow* GetWindow() const   {   return m_Window;    }
        void    SetWindow(wxWindow* window);

        void    SetFingerprint(const TFingerprint& p) {   m_Fingerprint = p;  }
        TFingerprint  GetFingerprint()    const   {   return m_Fingerprint;   }

        const vector<int>&  GetSplitterSizes() const    {   return m_SplitterSizes;   }
        vector<int>&  GetSplitterSizes()                {   return m_SplitterSizes;   }
        /// @}

        CRef<CNode> GetTheOnlyChild();
        CRef<CNode> GetVisibleParent();

        /// @name Modifiers
        /// @{
        int     GetIndex(CNode& node) const;
        int     GetBranchIndex(CNode& grandchild_node) const;

        bool    HasOnlyOneChild() const;

        void    AddChild(CNode& node);
        void    RemoveChild(CNode& node);
        void    RemoveChild(size_t index);
        void    RemoveAllChildren();
        size_t  ReplaceChild(CNode& old_node, CNode& new_node);

        void    EnforceChildOrder(const vector<wxWindow*>& windows);

        void    Link(CNode& clone);
        void    Unlink();
        /// @}

        void    ValidateSplitterSizes();

        template<class F> bool DepthFirstSearch(F& func);
        template<class F> void DepthFirstForEach(F& func);
        template<class F> void DepthFirstForEach(F& func) const;

    private:
        objects::CUser_object* ToUserObject(const CNode* central) const;
        static CRef<CNode>     FromUserObject(const objects::CUser_object& obj, CNode*& central);

        CNode* CloneNode(CNode* parent, const CNode* central, CNode*& newCentral) const;

        ENodeType   m_Type;
        bool    m_Hidden;
        CRef<CNode> m_Parent; // link to the parent node in the same tree
        CRef<CNode> m_Clone; // link to a brother node in a different tree
        TNodeVector m_Children;

        wxWindow*   m_Window; // pointer to window component corresponding to this Node

        TFingerprint m_Fingerprint; // string identifing a hidden window
        vector<int> m_SplitterSizes; // if node represent a splitter, this data memeber
        // is used to save split section sizes
    };
    // end of CNode ////////////////////////////////////////////////////////////

    typedef vector< CRef<CNode> >   TNodeVector;

    ////////////////////////////////////////////////////////////////////////////
    /// CDockLayoutTree interface
public:
    CDockLayoutTree()   {}
    CDockLayoutTree(CNode* root, CNode* central_pane);
    virtual ~CDockLayoutTree();

    CConstRef<CNode>   GetRoot() const  {    return CConstRef<CNode>(m_Root);    }
    CRef<CNode>   GetRoot()             {   return CRef<CNode>(m_Root); }
    CConstRef<CNode> GetCentralPane() const {   return m_CentralPane;   }
    CRef<CNode> GetCentralPane()            {   return m_CentralPane;   }

    void    DisconnectAllNodes();
    void    Remove(CNode& node);

    void    HideAll();

    CNode*  FindByFingerprint(const TFingerprint& s);

    template<class F>   bool DepthFirstSearch(F& func);
    template<class F>   void DepthFirstForEach(F& func);

    void    LogPost() const;
    void    LogPost(const CNode& node, unsigned int level) const;
    void    Validate(const CNode& node, bool vis_tree) const;

    
    objects::CUser_object* ToUserObject() const;
    static CRef<CDockLayoutTree> FromUserObject(const objects::CUser_object& obj);

    CDockLayoutTree* CloneTree() const;

    void    SaveSplitterLayout();

protected:
    void    x_HideNodes(CNode& node);
    void    x_DisconnectNodes(CNode& node);

    CNode*  x_FindByFingerprint(CNode& node, const TFingerprint& s);

public:
    CRef<CNode>     m_Root; // the top-most node (root parent)
    CRef<CNode>     m_CentralPane;  // Main Tab Control (main container only)
};

///////////////////////////////////////////////////////////////////////////////
/// Inline functions

template<class F> bool CDockLayoutTree::CNode::DepthFirstSearch(F& func)
{
    if(func(*this)) {
        return true;
    } else {
        for( size_t i = 0; i < m_Children.size();  i++ )   {
            CNode& child = *m_Children[i];
            if(child.DepthFirstSearch(func))
                return true;
        }
    }
    return false;
}


template<class F> void CDockLayoutTree::CNode::DepthFirstForEach(F& func)
{
    func(*this);
    for( size_t i = 0; i < m_Children.size();  i++ )   {
        CNode& child = *m_Children[i];
        child.DepthFirstForEach(func);
    }
}


template<class F> void CDockLayoutTree::CNode::DepthFirstForEach(F& func) const
{
    func(*this);
    for( size_t i = 0; i < m_Children.size();  i++ )   {
        const CNode& child = *m_Children[i];
        child.DepthFirstForEach(func);
    }
}


template<class F>   bool CDockLayoutTree::DepthFirstSearch(F& func)
{
    return m_Root ? m_Root->DepthFirstSearch(func) : false;
}


template<class F>   void CDockLayoutTree::DepthFirstForEach(F& func)
{
    if(m_Root)  {
        m_Root->DepthFirstForEach(func);
    }
}


END_NCBI_SCOPE


#endif  // GUI_WX_DEMO___DOCK_LAYOUT__HPP
