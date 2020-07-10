/* $Id: citem_util.hpp 29831 2014-02-24 16:24:25Z chenj $
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
 * Authors:  Jie Chen 
 */

#ifndef _ITEM_HPP
#define _ITEM_HPP

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/objects.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/feat_ci.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

template <class T> 
void add_to_cmd(CObject* obj, CObject* new_obj, CRef <CCmdComposite> composite)
{
      CRef < CChangeUnindexedObjectCommand<T> > cmd ( new  CChangeUnindexedObjectCommand<T> );
      cmd->Add(obj, new_obj);
      composite->AddCommand(*cmd);
};

class CItem : public CObject
{
  public:
    virtual ~CItem() { };
 
    bool apply(const CSeq_entry_Handle& tse, ICommandProccessor* cmdProcessor, const string& title);

    virtual void apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite) = 0;

    TConstScopedObjects* m_SelObjs;
};

class CRmvCddFeatDbxref : public CItem
{
  public:
    virtual ~CRmvCddFeatDbxref() { };
    virtual void apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite);
};

class CMRnaForCDS : public CItem
{
  public:
    virtual ~CMRnaForCDS() { };
    virtual void apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite);

  private:
    void x_ChkAndMakeMRna(SConstScopedObject& obj, CRef <CCmdComposite> composite);
};

class CPrefixAuthority : public CItem
{
  public:
    virtual ~CPrefixAuthority () {};
    virtual void apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite);
};

class CSplitDblinkQuals : public CItem
{
  public:
    virtual ~CSplitDblinkQuals () {};
    virtual void apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite);

  private:
    bool x_IsDblinkObj(const CUser_object& uobj);
    void x_ConvertStrToStrs(const string& dt_str, CRef <CUser_field> new_fld);
    void x_ConvertStrsToStrs(CRef <CUser_field> fld, CRef <CCmdComposite> composite);
    void x_SplitQuals(const CUser_object& user_obj, CRef <CCmdComposite> composite);
};

class CSrcFocusSet : public CItem
{
  public:
    virtual ~CSrcFocusSet() { };
    virtual void apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite);
};

class CSrcFocusClear : public CItem
{
  public:
    virtual ~CSrcFocusClear() { };
    virtual void apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite);
};

class CSetTransgSrcDesc : public CItem
{
  public:
    virtual ~CSetTransgSrcDesc() { };
    virtual void apply_to(const CSeq_entry_Handle& tse, CRef <CCmdComposite> composite);
  
  private:
    bool x_HasTransgenicSubtype(const CBioSource& biosrc);
};

END_NCBI_SCOPE

#endif
