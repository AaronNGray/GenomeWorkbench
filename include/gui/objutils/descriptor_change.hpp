#ifndef GUI_OBJUTILS___DESCRIPTOR_CHANGE__HPP
#define GUI_OBJUTILS___DESCRIPTOR_CHANGE__HPP

/*  $Id: descriptor_change.hpp 43804 2019-09-04 15:33:58Z asztalos $
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
 * Authors:  Roman Katargin, Colleen Bollin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/submit/Submit_block.hpp>
#include <gui/gui_export.h>

#include <gui/utils/command_processor.hpp>
#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE

template <class T>
class CChangeUnindexedObjectCommand : public CObject, public IEditCommand
{
public:
    CChangeUnindexedObjectCommand();
    void Add (CObject* obj, const CObject* new_obj);
    void AddBioseq (SConstScopedObject& obj, const CObject* new_obj);

    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel() { return "Edit unindexed object"; };

private:

    struct SUnindexedObject {
        CRef<T> actual_obj;
        CConstRef<T> orig_obj;
        CConstRef<T> new_obj;
    };

    typedef vector<SUnindexedObject> TUnindexedObjects;
    struct SBioseq {
        objects::CBioseq_EditHandle handle;
        CRef<objects::CBioseq> actual_bioseq;
        CRef<objects::CBioseq> orig_bioseq;
        CConstRef<objects::CBioseq> new_bioseq;
    };
    typedef vector<SBioseq> TBioseqs;

    TUnindexedObjects m_UnindexedObjects;
    TBioseqs m_Bioseqs;
};


template <class T>
CChangeUnindexedObjectCommand<T>::CChangeUnindexedObjectCommand()
{
}


template <class T>
void CChangeUnindexedObjectCommand<T>::Add (CObject* obj, const CObject* new_obj)
{
    T* actual_obj = dynamic_cast<T*>(obj);

    if( ! actual_obj) {
        LOG_POST(Error << "Error: attempt to edit wrong kind of object");
        return;
    }

    const T* tnew_obj = dynamic_cast<const T*>(new_obj);
    if( ! tnew_obj) {
        LOG_POST(Error << "Error: attempt to edit object with object of different type");
        return;
    }

    SUnindexedObject sobj;
    sobj.actual_obj.Reset(actual_obj);
    sobj.new_obj.Reset(tnew_obj);
    CRef<T> orig_obj(new T());
    orig_obj->Assign(*actual_obj);
    sobj.orig_obj.Reset(orig_obj);
    m_UnindexedObjects.push_back(sobj);
}


template <class T>
void CChangeUnindexedObjectCommand<T>::AddBioseq (SConstScopedObject& obj, const CObject* new_obj)
{
    const objects::CBioseq* orig_bioseq = dynamic_cast<const objects::CBioseq*> (obj.object.GetPointer());
    if ( ! orig_bioseq) {
        LOG_POST(Error << "Error: attempt to edit non-CBioseq object"
                 << " as CBioseq");
        return;
    }

    objects::CBioseq* actual_bioseq = (objects::CBioseq*) (orig_bioseq);

    const objects::CBioseq* new_bioseq = dynamic_cast<const objects::CBioseq*>(new_obj);
    if( ! new_bioseq) {
        LOG_POST(Error << "Error: attempt to edit non-CBioseq object"
                 << " as CBioseq");
        return;
    }

    SBioseq sbioseq;
    sbioseq.handle = obj.scope->GetBioseqEditHandle(*orig_bioseq);
    sbioseq.actual_bioseq = actual_bioseq;
    sbioseq.new_bioseq.Reset(new_bioseq);
    sbioseq.orig_bioseq = new objects::CBioseq();
    sbioseq.orig_bioseq->Assign(*actual_bioseq);
    m_Bioseqs.push_back(sbioseq);
    
}


template <class T>
void CChangeUnindexedObjectCommand<T>::Execute()
{
    NON_CONST_ITERATE(typename TUnindexedObjects, iter, m_UnindexedObjects) {
        SUnindexedObject& sobj = *iter;
        sobj.actual_obj->Assign(*sobj.new_obj);
    }
    NON_CONST_ITERATE(typename TBioseqs, iter, m_Bioseqs) {
        SBioseq& bioseq = *iter;
        CRef<objects::CSeq_inst> inst(new objects::CSeq_inst());
        inst->Assign(bioseq.new_bioseq->GetInst());
        bioseq.handle.SetInst(*inst);
    }
}

template <class T>
void CChangeUnindexedObjectCommand<T>::Unexecute()
{
    NON_CONST_ITERATE(typename TUnindexedObjects, iter, m_UnindexedObjects) {
        SUnindexedObject& sobj = *iter;
        sobj.actual_obj->Assign(*sobj.orig_obj);
    }
    NON_CONST_ITERATE(typename TBioseqs, iter, m_Bioseqs) {
        SBioseq& bioseq = *iter;
        CRef<objects::CSeq_inst> inst(new objects::CSeq_inst());
        inst->Assign(bioseq.orig_bioseq->GetInst());
        bioseq.handle.SetInst(*inst);
    }
}


typedef NCBI_GUIOBJUTILS_EXPORT CChangeUnindexedObjectCommand<objects::CSubmit_block> CChangeSubmitBlockCommand;


class NCBI_GUIOBJUTILS_EXPORT CCmdChangeSeqdesc : public CObject, public IEditCommand
{
public:
    CCmdChangeSeqdesc(const objects::CSeq_entry_Handle& seh, const objects::CSeqdesc& old_desc, const objects::CSeqdesc& new_desc)
        : m_Seh(seh), m_OldDesc(&old_desc), m_NewDesc(&new_desc)
    {
    }

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}
protected:
    objects::CSeq_entry_Handle m_Seh;
    CConstRef<objects::CSeqdesc> m_OldDesc;
    CConstRef<objects::CSeqdesc> m_NewDesc;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___FEATURE_CHANGE__HPP
