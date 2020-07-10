#ifndef GUI_UTILS___GUI_OBJECT_INFO_COMPONENT__HPP
#define GUI_UTILS___GUI_OBJECT_INFO_COMPONENT__HPP

/*  $Id: gui_object_info_component.hpp 42079 2018-12-13 20:37:10Z evgeniev $
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
 * Authors:  Vladislav Evgeniev
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_object_info.hpp>

/** @addtogroup GUI_UTILS
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CGuiObjectInfoComponent : public CObject, public IGuiObjectInfo
{
public:
    static CGuiObjectInfoComponent* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual string GetType() const { return "Component"; }
    virtual string GetSubtype() const { return ""; }
    virtual string GetLabel() const;
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
    virtual void GetLinks(ITooltipFormatter& /*links*/, bool /*no_ncbi_base*/) const;
    virtual string GetIcon() const;
    virtual string GetViewCategory() const { return "Sequence"; }

private:
    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;
    CConstRef<objects::CSeq_id> m_Id;
};


END_NCBI_SCOPE


/* @} */

#endif  // GUI_UTILS___GUI_OBJECT_INFO_COMPONENT__HPP
