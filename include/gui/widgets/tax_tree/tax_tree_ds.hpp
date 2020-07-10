#ifndef GUI_WIDGETS_TAX_TREE___TAX_TREE_DS__HPP
#define GUI_WIDGETS_TAX_TREE___TAX_TREE_DS__HPP

/*  $Id: tax_tree_ds.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <gui/gui_export.h>
#include <corelib/ncbiobj.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/taxon1/taxon1.hpp>
#include <objmgr/scope.hpp>

BEGIN_NCBI_SCOPE


class CTaxTreeDataSource : public CObject
{
public:

    // enum to control the level of detail on our data retrieval and
    // presentation
    enum EDisplayMode {
        eDisplay_All,
        eDisplay_Best,
        eDisplay_Blast,

        eDisplay_Default = eDisplay_All
    };

    // tax-ids are always integers, with 0 = invalid
    typedef int TTaxId;

    // a UID four our data source is really a seq-id
    typedef CConstRef<objects::CSeq_id> TUid;
    typedef vector<TUid> TUidVec;

    // map of a tax-id to a set of UIDs that corrspond to this tax-id
    typedef map<TTaxId, TUidVec> TTaxMap;

    // retrieve the map of tax-ids
    virtual void GetTaxMap(TTaxMap& taxmap) = 0;

    // retrieve the UIDs from our data source
    virtual void GetUids(TUidVec& uids) = 0;
    virtual const TUidVec& GetUids() const = 0;

    // retrieve a title for a given UID
    virtual void GetTitle(const objects::CSeq_id& uid,
                          string* title) const = 0;

    // retrieve a title for a given taxonomy node
    virtual void GetTitle(const objects::ITaxon1Node& node,
                          string* title) const = 0;

    // retrieve an iterator for our taxonomy tree
    virtual objects::ITreeIterator&
        GetIterator(EDisplayMode mode = eDisplay_Default) = 0;

    // retrieve scope
    virtual const CRef<objects::CScope> & GetScope(void) = 0;
};


////////////////////////////////////////////////////////////////////////////
//
// class CTaxTreeDS_ObjMgr provides a standard data source for the tax tree
// widget whose data is derived from a set of seq-ids.
//
// This class will use a provided object manager scope to convert the seq-id
// to its corresponding gi and build a CTaxon1 structure from these.
//
class NCBI_GUIWIDGETS_TAXTREE_EXPORT CTaxTreeDS_ObjMgr : public CTaxTreeDataSource
{
public:
    CTaxTreeDS_ObjMgr(objects::CScope& scope, const TUidVec& ids);

    // retrieve the map of tax-ids
    void GetTaxMap(TTaxMap& taxmap);

    void GetUids(TUidVec& gis);
    const TUidVec& GetUids() const;

    // get a title for a given UID
    void GetTitle(const objects::CSeq_id& uids, string* title) const;

    // get a title for a given tax tree node
    void GetTitle(const objects::ITaxon1Node& node, string* title) const;

    // retrieve an iterator for our taxonomy tree
    objects::ITreeIterator& GetIterator(EDisplayMode mode = eDisplay_Default);

    // retrieve scope
    const CRef<objects::CScope> & GetScope(void);

private:
    // internal initialization hook
    void x_Init(void);

private:
    // our original IDs
    TUidVec m_Ids;

    // a non-const scope we can use to retrieve information about or sequences
    mutable CRef<objects::CScope> m_Scope;

    // a CTaxon1 class to manage our taxonomy ID conversion
    objects::CTaxon1 m_TaxCache;

    // display and tree iteration mode, controlling our depth of traversal and
    // the breadth of nodes included
    EDisplayMode m_Mode;

    // our iterator class
    mutable CRef<objects::ITreeIterator> m_Iter;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TAX_TREE___TAX_TREE_DS__HPP
