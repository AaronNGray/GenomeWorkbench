#ifndef GUI_OBJUTILS___OBJECT_INDEX__HPP
#define GUI_OBJUTILS___OBJECT_INDEX__HPP

/*  $Id: object_index.hpp 38779 2017-06-16 16:27:59Z katargir $
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
 *  Government have not placed CAnyType restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for CAnyType particular
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <objmgr/scope.hpp>

#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE

typedef vector<objects::CSeq_id_Handle>  TIdHandleVector;

///////////////////////////////////////////////////////////////////////////////
///
class  NCBI_GUIOBJUTILS_EXPORT ISelObjectHandle : public CObject
{
public:
    virtual ~ISelObjectHandle() {};
};

///////////////////////////////////////////////////////////////////////////////
///
class  NCBI_GUIOBJUTILS_EXPORT CSeq_id_Descr
{
public:
    CSeq_id_Descr();
    CSeq_id_Descr(ISelObjectHandle* obj_handle, const objects::CSeq_id& id,
                  objects::CScope& scope, bool product = false);

    ~CSeq_id_Descr();

    // assumes ownership of the handle
    void Assign(ISelObjectHandle* obj_handle, const objects::CSeq_id& id,
                objects::CScope& scope, bool product = false);

    const CRef<ISelObjectHandle>   GetObjectHandle()   const {   return m_ObjHandle; }
    const objects::CSeq_id*      GetId()  const   {   return m_Id.GetPointer();  }
    const TIdHandleVector&  GetIdHandles()  const   {   return m_IdHandles; }
    const objects::CSeq_id_Handle&   GetAccNoVer()   const   {   return m_AccNoVer;  }
    bool  HasOriginalAccNoVer()   const   {   return m_OriginalAccNoVer;   }
    bool  IsProduct()   const   {   return m_Product;   }

protected:
    CRef<ISelObjectHandle>   m_ObjHandle; // a proxy for the object being selected
    CConstRef<objects::CSeq_id>  m_Id;
    TIdHandleVector     m_IdHandles; // always sorted (to speed-up comparison)

    objects::CSeq_id_Handle      m_AccNoVer;
    bool                m_OriginalAccNoVer;
    bool                m_Product; // this record represent a product ID
};

///////////////////////////////////////////////////////////////////////////////
/// CSeq_feat_Descr
class  NCBI_GUIOBJUTILS_EXPORT CSeq_feat_Descr
{
public:
    CSeq_feat_Descr();
    ~CSeq_feat_Descr();

    // assumes ownership of the handle
    void Assign(ISelObjectHandle* obj_handle, const objects::CSeq_feat& feat,
                CSeq_id_Descr*, objects::CScope& scope);

    const CRef<ISelObjectHandle>   GetObjectHandle()   const {   return m_ObjHandle; }
    const objects::CSeq_feat&      GetFeat()  const   {   return m_Feat.GetObject();  }
    const CSeq_id_Descr*    GetProductDescr()   const   {   return m_ProductDescr;  }

protected:
    CRef<ISelObjectHandle>  m_ObjHandle; // a proxy for the object being selected
    CConstRef<objects::CSeq_feat>    m_Feat;
    CRef<objects::CScope>    m_Scope;
    CSeq_id_Descr*  m_ProductDescr;

private:
    CSeq_feat_Descr(const CSeq_feat_Descr&);
    CSeq_feat_Descr& operator=(const CSeq_feat_Descr&);
};

class NCBI_GUIOBJUTILS_EXPORT CObjectIndex
{
public:
    typedef set<const ISelObjectHandle*>   TResults;

    CObjectIndex();
    ~CObjectIndex();

    void    Clear(objects::CScope* scope);

    void    Add(ISelObjectHandle* obj_handle, CObject& obj);
    void    Add(ISelObjectHandle* obj_handle, const objects::CSeq_id& id, bool product = false);
    void    Add(ISelObjectHandle* obj_handle, const objects::CSeq_feat& feature);
    void    Add(ISelObjectHandle* obj_handle, const objects::CSeq_align& align);

    bool    Remove(const CObject& obj);
    bool    Remove(const objects::CSeq_id& id);
    bool    Remove(const objects::CSeq_feat& feature);
    bool    Remove(const objects::CSeq_align& align);

    bool    Empty() const;

    // HasMathes() function is generally faster than GetMathes() as it does not
    // need to perform exhaustive search
    bool    HasMatches(const CObject& object, objects::CScope& scope) const;
    void    GetMatches(const CObject& object, objects::CScope& scope, TResults& results) const;

    // takes a CSeq_id_Descr not a objects::CSeq_id for performance reasons; CSeq_id_Descr
    // can be created for objects::CSeq_id
    bool    HasMatches(const CSeq_id_Descr& descr) const;
    void    GetMatches(const CSeq_id_Descr& descr, TResults& results) const;

    bool    HasMatches(const objects::CSeq_feat& feature, objects::CScope& scope) const;
    void    GetMatches(const objects::CSeq_feat& feature, objects::CScope& scope, TResults& results) const;

    bool    HasMatches(const objects::CSeq_align& align) const;
    void    GetMatches(const objects::CSeq_align& align, TResults& results) const;

    //TODO
    void    GetObjects(TConstObjects& objects) const;
    void    GetOriginalIds(vector<const objects::CSeq_id*>& ids) const;
    void    GetFeatures(vector<const objects::CSeq_feat*>& features) const;
    void    GetAligns(vector<const objects::CSeq_align*>& aligns) const;

protected:
    typedef unsigned int TFeatCode;

    struct  SRec    {
        bool m_Original; // handle was NOT artificially created for verisonless matching
        bool m_Product;
        CSeq_id_Descr*  m_Descr;

        SRec(CSeq_id_Descr* descr, bool original = true, bool product = false);

        inline  const CSeq_id_Descr*      GetIdDescr() const;
    };

    CSeq_id_Descr* x_Add(ISelObjectHandle* obj_handle, const objects::CSeq_id& id, bool product);
    void    x_Remove(const CSeq_id_Descr* descr);
    void    x_RemoveFromIdMap(const CSeq_id_Descr* descr);


    static  inline TFeatCode    x_GetFeatureCode(const objects::CSeq_feat& feature);

    void    x_GetMatches(const CSeq_id_Descr& descr, const objects::CSeq_id_Handle& h,
                         bool at_least_one, bool no_ver, TResults& results) const;
    bool    x_HasMatches(const CSeq_id_Descr& descr, const objects::CSeq_id_Handle& h,
                         bool at_least_one, bool no_ver) const;

protected:
    typedef vector<CSeq_id_Descr*>    TIdDescrs;
    typedef multimap<objects::CSeq_id_Handle, SRec>    TIdMap;

    // TODO use hashtables here ?
    typedef map<const objects::CSeq_feat*, CSeq_feat_Descr*>    TFeatDescrs; // for lookup by pointer and storage
    typedef multimap<TFeatCode, const CSeq_feat_Descr*>  TCodeToFeatMap;

    typedef map< CConstRef<objects::CSeq_align>, ISelObjectHandle*>  TAlignMap;

    CRef<objects::CScope>    m_Scope;
    TIdDescrs   m_IdDescrs; // storage for CSeq_id_Descr-s
    TIdMap      m_IdMap;

    TFeatDescrs     m_FeatDescrs;
    TCodeToFeatMap  m_FeatMap;

    TAlignMap   m_AlignMap;
};

END_NCBI_SCOPE

#endif  // GUI_UTILS___OBJECT_INDEX__HPP
