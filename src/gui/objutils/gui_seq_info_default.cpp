/*  $Id: gui_seq_info_default.cpp 26341 2012-08-29 20:54:24Z katargir $
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
#include <gui/objutils/gui_seq_info.hpp>

BEGIN_NCBI_SCOPE

class CGuiSeqInfoDefault : public CObject, public IGuiSeqInfo
{
public:
    static CGuiSeqInfoDefault* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual bool IsSequence() const {return false; }
    virtual bool IsDNA() const { return false; }
    virtual bool IsProtein() const {return false; }
    virtual bool IsGenomic() const {return false; }
    virtual bool IsRNA() const {return false; }
    virtual bool IscDNA() const {return false; }

private:
    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;
};

void initCGuiSeqInfoDefault()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiSeqInfo).name(),
            new CObjectInterfaceFactory<CGuiSeqInfoDefault>());
}

CGuiSeqInfoDefault* CGuiSeqInfoDefault::CreateObject(
    SConstScopedObject& object, ICreateParams* /*params*/)
{
    CGuiSeqInfoDefault* gui_info = new CGuiSeqInfoDefault();
    gui_info->m_Object = object.object;
    gui_info->m_Scope  = object.scope;
    return gui_info;
}

END_NCBI_SCOPE
