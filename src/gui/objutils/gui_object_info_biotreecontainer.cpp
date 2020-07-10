/*  $Id: gui_object_info_biotreecontainer.cpp 41372 2018-07-18 20:45:38Z rudnev $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_object_info.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/tooltip.hpp>

#include <objects/biotree/BioTreeContainer.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CGuiObjectInfoBioTreeContainer : public CObject, public IGuiObjectInfo
{
public:
    static CGuiObjectInfoBioTreeContainer* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual string GetType() const { return "Bio Tree"; }
    virtual string GetSubtype() const { return ""; }
    virtual string GetLabel() const;
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
    virtual void GetLinks(ITooltipFormatter& /*links*/, bool /*no_ncbi_base*/) const {};
    virtual string GetIcon() const { return "tree::bio_tree_item"; }
    virtual string GetViewCategory() const { return "Tree"; }

private:
    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;
};

void initCGuiObjectInfoBioTreeContainer()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiObjectInfo).name(),
            CBioTreeContainer::GetTypeInfo(),
            new CObjectInterfaceFactory<CGuiObjectInfoBioTreeContainer>());
}

CGuiObjectInfoBioTreeContainer* CGuiObjectInfoBioTreeContainer::CreateObject(
    SConstScopedObject& object, ICreateParams* /*params*/)
{
    CGuiObjectInfoBioTreeContainer* gui_info = new CGuiObjectInfoBioTreeContainer();
    gui_info->m_Object = object.object;
    gui_info->m_Scope  = object.scope;
    return gui_info;
}

string CGuiObjectInfoBioTreeContainer::GetLabel() const
{
    string label;
    CLabel::GetLabel(*m_Object, &label, CLabel::eDescriptionBrief, m_Scope);
    return label;
}

void CGuiObjectInfoBioTreeContainer::GetToolTip(ITooltipFormatter& tooltip, string& /*t_title*/, TSeqPos /*at_p*/, bool* isGeneratedBySvc) const
{
    string t_text;
    CLabel::GetLabel(*m_Object, &t_text, CLabel::eDescriptionBrief, m_Scope);
    tooltip.AddRow(t_text);
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
}

END_NCBI_SCOPE
