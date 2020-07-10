#ifndef GUI_UTILS___PRIMARY_EDIT__HPP
#define GUI_UTILS___PRIMARY_EDIT__HPP

/*  $Id: primary_edit.hpp 36776 2016-10-31 16:06:13Z filippov $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <objmgr/bioseq_handle.hpp>
#include <objects/general/Object_id.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

// This object is intended to be created on the fly
// Not related to any other bio object
// But it can be used by Seqdesc editor for initialization and
// after modification applied to CBioseq_Handle.

class CPrimaryEdit : public CSeqdesc
{
public:
    CPrimaryEdit(CBioseq_Handle& bh) : m_BH(bh)
    {
        for ( CSeqdesc_CI desc(m_BH, CSeqdesc::e_User); desc; ++desc ) 
        {
            const CUser_object& o = desc->GetUser(); 
            if ( o.IsSetType()  &&  o.GetType().IsStr() && o.GetType().GetStr() == "TpaAssembly") 
            {
                SetUser(const_cast<CUser_object&>(desc->GetUser()));
                break;
            }             
        }  
    }


    CBioseq_Handle GetBioseq_Handle() const { return m_BH; }
        
protected:
    CBioseq_Handle m_BH;
};

END_SCOPE(objects)
END_NCBI_SCOPE

#endif  // GUI_UTILS___PRIMARY_EDIT__HPP
