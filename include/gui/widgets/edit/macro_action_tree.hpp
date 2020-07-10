#ifndef GUI_WIDGETS_EDIT___MACRO_ACTION_TREE__HPP
#define GUI_WIDGETS_EDIT___MACRO_ACTION_TREE__HPP
/*  $Id: macro_action_tree.hpp 40586 2018-03-15 18:14:24Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrea Asztalos
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/macro_action_type.hpp>

BEGIN_NCBI_SCOPE

class CMActionNode;

class CMActionNode : public CObject
{
public:
    CMActionNode(const string& label, const CTempString& target, EMActionType type = EMActionType::eNotSet);
    ~CMActionNode() {}

    CRef<CMActionNode> AddChild(const string& label, const CTempString& target, const EMActionType type = EMActionType::eNotSet);
    const vector<CRef<CMActionNode>>& GetChildren() const { return m_Children; }

    const string& GetLabel() const { return m_Label; }
    const CTempString& GetTarget() const { return m_Target; }
    EMActionType GetActionType() const { return m_ActionType; }

    CMActionNode(const CMActionNode&) = delete;
    CMActionNode& operator=(const CMActionNode&) = delete;
private:
    vector<CRef<CMActionNode>> m_Children;
    string m_Label;
    CTempString m_Target;
    EMActionType m_ActionType{ EMActionType::eNotSet };
};

class CMActionTree : public CObject
{
public:
    void SetRoot(CMActionNode& node) { m_Root.Reset(&node); }
    CConstRef<CMActionNode> GetTopNode() const { return CConstRef<CMActionNode>(m_Root); }
private:
    CRef<CMActionNode> m_Root;
};

class CMActionTreeBuilder
{
public:
    static CConstRef<CMActionTree> BuildActionTree();
};
    

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_ACTION_TREE__HPP
