/*  $Id: dock_layout.cpp 34942 2016-03-03 16:27:35Z katargir $
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
#include <ncbi_pch.hpp>

#include <gui/widgets/wx/dock_layout.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/splitter.hpp>
#include <gui/widgets/wx/dock_manager.hpp>
#include <gui/widgets/wx/dock_panel.hpp>

#ifdef _DEBUG
    #include <gui/widgets/wx/dock_notebook.hpp>
#endif

#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CDockLayoutTree::CNode

CDockLayoutTree::CNode::CNode()
:   m_Type(eClient),
    m_Hidden(false),
    m_Window(NULL)
{
}


CDockLayoutTree::CNode::CNode(const CNode& orig)
:   m_Type(orig.m_Type),
    m_Hidden(orig.m_Hidden),
    m_Clone(NULL),
    m_Window(orig.m_Window),
    m_Fingerprint(orig.m_Fingerprint)
{
}


CDockLayoutTree::CNode::CNode(ENodeType type, wxWindow* window, bool hidden)
:   m_Type(type),
    m_Hidden(hidden),
    m_Clone(NULL),
    m_Window(window)
{
}


CDockLayoutTree::CNode::~CNode()
{
}


CRef<CDockLayoutTree::CNode> CDockLayoutTree::CNode::GetTheOnlyChild()
{
    if (m_Children.size() == 1) {
        return m_Children[0];
    } else {
        _ASSERT(false);
        return CRef<CNode>();
    }
}


CRef<CDockLayoutTree::CNode> CDockLayoutTree::CNode::GetVisibleParent()
{
    CNode* p = m_Parent.GetPointer();
    while(p) {
        if (p->IsVisible())
            break;
        p = p->m_Parent;
    }
    return CRef<CNode>(p);
}

void CDockLayoutTree::CNode::SetWindow(wxWindow* window)
{
#ifdef _DEBUG
    if (window) {
        //LOG_POST("CNode::SetWindow() " << window << "  " << typeid(*window).name());

        bool splitter = (dynamic_cast<CSplitter*>(window) != NULL);
        bool tab = (dynamic_cast<CDockNotebook*>(window) != NULL);

        switch(m_Type) {
        case eTab:
            _ASSERT(tab);
            break;
        case eHorzSplitter:
        case eVertSplitter:
            _ASSERT(splitter);
            break;
        default:
            _ASSERT(! tab  &&  ! splitter);
        }
    }
#endif

    m_Window = window;
}


void CDockLayoutTree::CNode::AddChild(CNode& node)
{
    m_Children.push_back(CRef<CNode>(&node));
    node.m_Parent.Reset(this);
}


void CDockLayoutTree::CNode::RemoveChild(CNode& node)
{
    CRef<CNode> ref(&node);
    TNodeVector::iterator it = std::find(m_Children.begin(), m_Children.end(), ref);

    _ASSERT(it != m_Children.end());

    m_Children.erase(it);
    node.m_Parent.Reset();
}


void CDockLayoutTree::CNode::RemoveChild(size_t index)
{
    _ASSERT(index < m_Children.size());

    TNodeVector::iterator it = m_Children.begin() + index;
    (*it)->m_Parent.Reset();
    m_Children.erase(it);
}


void CDockLayoutTree::CNode::RemoveAllChildren()
{
    NON_CONST_ITERATE(TNodeVector, it, m_Children) {
        CNode& node = **it;
        node.m_Parent.Reset();
    }
    m_Children.clear();
}


size_t CDockLayoutTree::CNode::ReplaceChild(CNode& old_node, CNode& new_node)
{
    CRef<CNode> ref(&old_node);
    TNodeVector::iterator it = std::find(m_Children.begin(), m_Children.end(), ref);

    _ASSERT(it != m_Children.end());

    it->Reset(&new_node);

    old_node.m_Parent.Reset();
    new_node.m_Parent.Reset(this);
    return it - m_Children.begin();
}


// rearrange m_Children according to the given order of windows
void CDockLayoutTree::CNode::EnforceChildOrder(const vector<wxWindow*>& windows)
{
    _ASSERT(windows.size() <= m_Children.size());
    // here we accumulate Nodes in the right order
    TNodeVector sel;

    for( size_t i = 0;  i < windows.size(); i++ ) {
        wxWindow* win = windows[i];
        // find a child for this window
        for( size_t j = 0;  j < m_Children.size(); j++ ) {
            CRef<CNode>& child = m_Children[j];
            if (child->GetWindow() == win) {
                sel.push_back(child);
                m_Children.erase(m_Children.begin() + j);
                break;
            }
        }
    }
    _ASSERT(sel.size() == windows.size());

    for( size_t i = 0;  i < sel.size(); i++ ) {
        CRef<CNode>& child = sel[i];
        m_Children.insert(m_Children.begin() + i, child);
    }
}


void CDockLayoutTree::CNode::Link(CNode& clone)
{
    m_Clone.Reset(&clone);
    clone.m_Clone.Reset(this);
}


void CDockLayoutTree::CNode::Unlink()
{
    if (m_Clone) {
        m_Clone->m_Clone.Reset();
        m_Clone.Reset();
    }
}


int CDockLayoutTree::CNode::GetIndex(CNode& node) const
{
    CRef<CNode> ref(&node);
    TNodeVector::const_iterator it = std::find(m_Children.begin(), m_Children.end(), ref);
    return (it == m_Children.end()) ? -1 : (it - m_Children.begin());
}


// find index of a child node that has the given node as one of its children
// / grandchildren (on any level). In other words find a child node that is
// the root of a branch containing the given node.
int CDockLayoutTree::CNode::GetBranchIndex(CNode& grandchild_node) const
{
    CNode* p = &grandchild_node;
    while(p) {
        if (p->m_Parent == this) {
            // this is our child
            int index = GetIndex(*p);
            _ASSERT(index >= 0);
            return index;
        }
        p = p->GetParent();
    }
    return -1; // not our grandchild
}

bool CDockLayoutTree::CNode::HasOnlyOneChild() const
{
    return m_Children.size() == 1;
}

static string kWindowLayoutHdr = "Window Layout: ";

void CDockLayoutTree::CNode::ValidateSplitterSizes()
{
    if (m_Type != eHorzSplitter && m_Type != eVertSplitter)
        return;

    if (m_SplitterSizes.size() == 0)
        return;

    if (m_SplitterSizes.size() != 2) {
        LOG_POST(Error << kWindowLayoutHdr << "Invalid dimension of splitter sizes: " << m_SplitterSizes.size());
        m_SplitterSizes.resize(2);
        m_SplitterSizes[0] = m_SplitterSizes[1] = 100;
        return;
    }

    if (m_SplitterSizes[0] <= 0 || m_SplitterSizes[1] <= 0) {
        LOG_POST(Error << kWindowLayoutHdr << "Invalid splitter sizes: ("
            << m_SplitterSizes[0] << ", " << m_SplitterSizes[1] << ")");

        m_SplitterSizes[0] = m_SplitterSizes[1] = 100;
    }
}

static const char*  kNodeTag = "Layout Tree Node";
static const char*  kTypeTag    = "Type";
static const char*  kHiddenTag  = "Hidden";
static const char*  kCentralTag  = "Central";
static const char*  kFingerprintTag  = "Fingerprint";
static const char*  kSplitterSizersTag  = "Splitter Sizes";
static const char*  kChildrenTag = "Children";

CUser_object* CDockLayoutTree::CNode::ToUserObject(const CNode* central) const
{
    CUser_object* object = new CUser_object();
    object->SetType().SetStr(kNodeTag);

    TFingerprint fingerprint = GetFingerprint();
    if (fingerprint.IsEmpty()  &&  IsClient()  &&  GetWindow()) {
        /// this is a Client node - extract fingerprint from the Client
        wxWindow* win = const_cast<wxWindow*>(GetWindow());
        CDockPanel* panel = dynamic_cast<CDockPanel*>(win);
        fingerprint = panel->GetClient()->GetFingerprint();
    }

    object->AddField(kTypeTag, GetType());
    object->AddField(kHiddenTag, IsHidden());

    string s_print = fingerprint.AsPersistentString();
    object->AddField(kFingerprintTag, s_print);

    object->AddField(kCentralTag, this == central);

    if (IsSplitter()) {
        object->AddField(kSplitterSizersTag, GetSplitterSizes());
    }

    // save child nodes recursively
    if (IsContainer()) {
        vector< CRef<CUser_object> > objects;

        const TNodeVector& children = GetChildren();
        for( size_t i = 0;  i < children.size();  i++ ) {
            const CNode& child = *children[i];
            CRef<CUser_object> child_object(child.ToUserObject(central));
            objects.push_back(child_object);
        }

        object->AddField(kChildrenTag, objects);
    }
    return object;
}

CRef<CDockLayoutTree::CNode> CDockLayoutTree::CNode::FromUserObject(const objects::CUser_object& obj, CNode*& central)
{
    _ASSERT(obj.GetType().GetStr() == kNodeTag);

    const CUser_field& f_type = obj.GetField(kTypeTag);
    CDockLayoutTree::ENodeType type = (CDockLayoutTree::ENodeType) f_type.GetData().GetInt();

    const CUser_field& f_hidden = obj.GetField(kHiddenTag);
    bool hidden = f_hidden.GetData().GetBool();

    // create a Node
    CRef<CNode> node(new CNode(type, NULL, hidden));

    const CUser_field& f_fingerprint = obj.GetField(kFingerprintTag);
    string s_print = f_fingerprint.GetData().GetStr();
    TFingerprint fingerprint;
    fingerprint.FromPersistentString(s_print);
    node->SetFingerprint(fingerprint);

    if (node->IsSplitter()) {
        const CUser_field& f_sizes = obj.GetField(kSplitterSizersTag);
        node->GetSplitterSizes() = f_sizes.GetData().GetInts();
        node->ValidateSplitterSizes();
    }

    // create child nodes
    if (node->IsContainer()) {
        const CUser_field& f_children = obj.GetField(kChildrenTag);
        vector< CRef<CUser_object> > objects = f_children.GetData().GetObjects();

        for( size_t i = 0;  i < objects.size();  i++ ) {
            CUser_object& child_object = *objects[i];

            CRef<CNode> child_node = FromUserObject(child_object, central);
            node->AddChild(*child_node);
        }
    }
    // see if this node represents the central pane
    const CUser_field& f_central = obj.GetField(kCentralTag);
    if (f_central.GetData().GetBool()) {
        _ASSERT( ! central); // not initialized yet
        central = node.GetPointer();
    }
    return node;
}

CDockLayoutTree::CNode* CDockLayoutTree::CNode::CloneNode(CNode* parent, const CNode* central, CNode*& newCentral) const
{
  CRef<CNode> clone(new CNode());
  if (this == central)
    newCentral = clone;

  clone->m_Type = m_Type;
  clone->m_Hidden = m_Hidden;
  clone->m_Parent.Reset(parent);
  clone->m_Window = 0;
  clone->m_Fingerprint = m_Fingerprint;
  clone->m_SplitterSizes = m_SplitterSizes;

  ITERATE(TNodeVector, it, m_Children) {
    clone->m_Children.push_back(CRef<CNode>((*it)->CloneNode(clone, central, newCentral)));
  }

  return clone.Release();
}



///////////////////////////////////////////////////////////////////////////////
/// CDockLayoutTree
CDockLayoutTree::CDockLayoutTree(CNode* root, CNode* central_pane)
:   m_Root(root), m_CentralPane(central_pane)
{
}


CDockLayoutTree::~CDockLayoutTree()
{
    DisconnectAllNodes();
}

CUser_object* CDockLayoutTree::ToUserObject() const
{
    if (GetRoot()) {
        const CNode* central = GetCentralPane().GetPointer();
        return GetRoot()->ToUserObject(central);
    }
    return 0;
}

CRef<CDockLayoutTree> CDockLayoutTree::FromUserObject(const objects::CUser_object& obj)
{
    CNode* central = NULL;
    CRef<CNode> node = CNode::FromUserObject(obj, central);
    CRef<CDockLayoutTree> tree(new CDockLayoutTree(node.GetPointer(), central));
    return tree;
}

CDockLayoutTree* CDockLayoutTree::CloneTree() const
{
  CRef<CDockLayoutTree> clone(new CDockLayoutTree());

  if (m_Root) {
      CNode* central = 0;
      clone->m_Root.Reset(m_Root->CloneNode(0, m_CentralPane, central));
      clone->m_CentralPane.Reset(central);
  }

  return clone.Release();
}


void CDockLayoutTree::DisconnectAllNodes()
{
    if (m_Root) {
        x_DisconnectNodes(*m_Root);
        m_Root.Reset();
    }
}


// disconnect all nodes from each other, this most likely will result in nodes
// being deleted (unless someone hold a reference to a node)
void CDockLayoutTree::x_DisconnectNodes(CNode& node)
{
    for( size_t i = 0;  i < node.m_Children.size(); i++) {
        CNode& child = *node.m_Children[i];
        x_DisconnectNodes(child);
    }
    node.RemoveAllChildren();
    node.Unlink();
}


void CDockLayoutTree::HideAll()
{
    if (m_Root) {
        x_HideNodes(*m_Root);
    }
}


CDockLayoutTree::CNode* CDockLayoutTree::FindByFingerprint(const TFingerprint& s)
{
    if (m_Root) {
        return x_FindByFingerprint(*m_Root, s);
    } else {
        return NULL;
    }
}


CDockLayoutTree::CNode*
    CDockLayoutTree::x_FindByFingerprint(CNode& node, const TFingerprint& s)
{
    if (node.GetFingerprint() == s) {
        return &node;
    } else if (node.IsContainer()) {
        for( size_t i = 0;  i < node.m_Children.size();  i++ ) {
            CNode& child = *node.m_Children[i];
            CNode* res = x_FindByFingerprint(child, s);
            if (res) {
                return res;
            }
        }
    }
    return NULL;
}

static const char* kIndent = "    ";

void CDockLayoutTree::LogPost() const
{
    if (m_Root) {
        LogPost(*m_Root, 1);
    } else {
        LOG_POST(Info << kIndent << "Root is NULL");
    }
}


void CDockLayoutTree::LogPost(const CNode& node, unsigned int level) const
{
    string offset;
    for (size_t i = 0; i < level; ++i)
        offset += kIndent;

    string nodeTag;
    switch (node.GetType()) {
    case eClient:
        nodeTag = "CLIENT";
        break;
    case eHorzSplitter:
        nodeTag = "H-SPLITTER";
        break;
    case eVertSplitter :
        nodeTag = "V-SPLITTER";
        break;
    case eTab :
        if (m_CentralPane == &node)
            nodeTag = "CENTRAL_TAB";
        else
            nodeTag = "TAB";
        break;
    default:
        nodeTag = "ERROR";
        break;
    }

    string hash = node.GetFingerprint().AsPersistentString();
    if (!hash.empty()) {
        hash = " \"" + hash;
        hash += "\" ";
    }
    else {
        hash = " ";
    }

    LOG_POST(Info << offset
        << nodeTag << hash << &node
        << ", parent: " << node.GetParent()
        << ", win: " << node.GetWindow()
        << (node.IsHidden() ? ", HIDDEN" : ", VISIBLE"));

    for (size_t i = 0; i < node.GetChildren().size(); i++)
        LogPost(*node.GetChildren()[i], level + 1);
}


void CDockLayoutTree::Validate(const CNode& node, bool vis_tree) const
{
    // must have a parent or be the root of the tree
    _ASSERT(node.GetParent()  ||  m_Root.GetPointer() == &node);

    // visible nodes must have windows
    _ASSERT(node.IsVisible()  ==  (node.GetWindow()  != NULL));

    if (node.IsVisible()) {
        // visible nodes must have clones
        _ASSERT(node.m_Clone  &&  node.m_Clone->m_Clone == &node);
    } else {
        _ASSERT( ! vis_tree  &&  node.m_Clone == NULL);
    }

    if (node.IsContainer()) {
        // must be root tab or non empty
        _ASSERT(m_Root.GetPointer() == &node  ||  node.m_Children.size());

        for( size_t i = 0;  i < node.m_Children.size();  i++) {
            _ASSERT(node.m_Children[i]->m_Parent == &node); // parent-child consistency
        }
    }
}


void CDockLayoutTree::x_HideNodes(CNode& node)
{
    node.SetHidden(true);

    if (node.IsContainer()) {
        for( size_t i = 0;  i < node.m_Children.size();  i++ ) {
            CNode& child_node = *node.m_Children[i];
            x_HideNodes(child_node);
        }
    }
}

// Save splitter position in the full_node
struct FSaveSplitterLayout
{
    void operator()(CDockLayoutTree::CNode& node)
 {
        wxWindow* win = node.GetWindow();

        if (win) {
            vector<int> sizes;
            switch (node.GetType()) {
            case CDockLayoutTree::eHorzSplitter: {
                CSplitter* splitter = dynamic_cast<CSplitter*>(win);
                splitter->GetHeights(sizes);
                _ASSERT(sizes.size() == 2 && sizes[0] >= 0 && sizes[1] >= 0);
                node.GetSplitterSizes() = sizes;
                break;
            }
            case CDockLayoutTree::eVertSplitter: {
                CSplitter* splitter = dynamic_cast<CSplitter*>(win);
                splitter->GetWidths(sizes);
                _ASSERT(sizes.size() == 2 && sizes[0] >= 0 && sizes[1] >= 0);
                node.GetSplitterSizes() = sizes;
                break;
            }
            default:
                break;
            }
        }
    }
};

void CDockLayoutTree::SaveSplitterLayout()
{
    if (m_Root) {
        FSaveSplitterLayout saver;
        m_Root->DepthFirstForEach(saver);
    }
}

END_NCBI_SCOPE
