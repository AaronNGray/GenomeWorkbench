#ifndef GUI_OBJUTILS___VIEW_EVENT__HPP
#define GUI_OBJUTILS___VIEW_EVENT__HPP

/*  $Id: obj_event.hpp 38850 2017-06-26 17:44:57Z katargir $
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
 * Authors:  Vladimir Tereshkov, Andrey Yazhuk, Mike Dicuccio
 *
 * File Description:
 *    Event object for view communications
 */

#include <util/range_coll.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <gui/objutils/taxid_sel_set.hpp>


#include <objmgr/impl/handle_range_map.hpp>

#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///
class NCBI_GUIOBJUTILS_EXPORT CIdLoc : public CObject
{
public: // TODO

    CConstRef<objects::CSeq_id>  m_Id;
    CRange<TSeqPos> m_Range;
};

///////////////////////////////////////////////////////////////////////////////
/// CSelectionEvent
/// CSelectionEvent is used for broadcasting selection between views.
/// It containes separate "packages" for different types of selection.
///  If package
/// This class is not thread-safe.
class NCBI_GUIOBJUTILS_EXPORT CSelectionEvent
{
public:
    typedef CRangeCollection<TSeqPos>   TRangeColl;
    typedef TRangeColl::TRange          TRange;
    typedef vector< CConstRef<objects::CSeq_feat> >     TFeats;
    typedef vector< CConstRef<objects::CSeq_align> >    TAligns;
    typedef vector< CConstRef<objects::CSeq_loc> >      TSeqLocs;
    typedef vector< CConstRef<objects::CSeq_id> >       TIds;
    typedef vector< CConstRef<CIdLoc> >    TIdLocs;
    typedef CTaxIdSelSet                   TTaxIds;
    typedef TTaxIds::TTaxId                TTaxId;
    typedef vector< CConstRef<CObject> >   TOther;

    typedef map<string, TConstObjects>     TIndexed;

    CSelectionEvent(objects::CScope& scope);

    enum    EObjMatchPolicy {
        eAllIds,
        eAtLeastOne
    };
    enum    EIdMatchPolicy {
        eExact,
        eAccOnly
    };

    objects::CScope& GetScope()  {   return  *m_Scope;    }

    /// Range Selection - represents a collection of selected segments
    bool    HasRangeSelection() const;
    const objects::CHandleRangeMap&  GetRangeSelection() const;
    void    AddRangeSelection(const objects::CSeq_id& id, const TRangeColl& segs);
    void    AddRangeSelection(const objects::CSeq_loc& loc);
    bool    GetRangeSelection(const objects::CSeq_id& id, objects::CScope& scope,
                         TRangeColl& segs);
    void    GetRangeSelectionAsLocs(TConstObjects& objs) const;

    /// Object Selection - represents select Objects
    /// (such as features, alignments, etc)
    bool    HasObjectSelection();
    bool    AddObjectSelection(const CObject& obj);
    void    AddObjectSelection(const TConstObjects& objs); // temporary
    void    AddObjectSelection(const objects::CSeq_id& id);
    void    AddObjectSelection(const objects::CSeq_feat& feat);
    void    AddObjectSelection(const objects::CSeq_align& align);
    void    AddObjectSelection(const objects::CSeq_loc& loc);
    void    AddObjectSelection(const objects::CSeq_id& id, const CRange<TSeqPos>& range);
    void    AddTaxIDSelection(TTaxId tid);

    const TFeats&   GetFeats()  const   {   return m_Feats;    }
    const TAligns&  GetAligns() const   {   return m_Aligns;    }
    const TSeqLocs& GetSeqLocs() const   {   return m_SeqLocs;    }
    const TIds&     GetIds() const   {   return m_Ids;    }
    const TIdLocs&  GetIdLocs() const   {   return m_IdLocs;    }
    const TTaxIds&  GetTaxIDs() const { return m_TaxIds; }

    void            GetAllObjects(TConstObjects& objs) const;
    void            GetOther(TConstObjects& objs) const;

    /// @name Private selection broadcasting functions
    /// Private is used for View-to-View communication, when Views need to send extended 
    /// broadcastin information which only view of the same type can understand.
    ///
    /// Selection inspector is not supposed to interpret this.
    ///
    /// Functionality is added to implement robust tree comparison GB-3152
    ///
    /// @{

    /// Add private selection broadcasting info
    ///
    /// @param area_name is a string used to identify private information
    /// @param obj - CObject derived class with selection infomation (can be downcasted by the receiver)
    ///
    void    AddIndexed(const string& area_name, const CObject& obj);

    /// Get Objects from the private broadcasting area 
    /// (caller has to make sure output objs is empty)
    ///
    void    GetIndexed(const string& area_name, TConstObjects& objs) const;

    /// Get access to private index map (all areas)
    /// (mostly to avoid copy-ctr overhead associated with GetIndexed() )
    ///
    const TIndexed& GetIndexMap() const { return m_Indexed; } 
    /// @}

    /// @name Matching functons
    /// @{
    static bool Match(const CObject& obj1, objects::CScope& scope1,
                      const CObject& obj2, objects::CScope& scope2);

    static bool MatchFeatWithFeat(const objects::CSeq_feat& feat1,
                                  objects::CScope& scope1,
                                  const objects::CSeq_feat& feat2,
                                  objects::CScope& scope2);

    static bool MatchFeatWithLoc(const objects::CSeq_feat& feat1,
                                 objects::CScope& scope1,
                                 const objects::CSeq_loc& loc2,
                                 objects::CScope& scope2);

    static bool MatchFeatWithId(const objects::CSeq_feat& feat1,
                                objects::CScope& scope1,
                                const objects::CSeq_id& id2,
                                objects::CScope& scope2);

    static bool MatchAlignWithAlign(const objects::CSeq_align& align1,
                                    objects::CScope& scope1,
                                    const objects::CSeq_align& align2,
                                    objects::CScope& scope2);

    static bool MatchLocWithLoc(const objects::CSeq_loc& loc1,
                                objects::CScope& scope1,
                                const objects::CSeq_loc& loc2,
                                objects::CScope& scope2);

    static bool MatchLocWithId(const objects::CSeq_loc& loc1,
                               objects::CScope& scope1,
                               const objects::CSeq_id& id2,
                               objects::CScope& scope2);

    static bool MatchIdWithId(const objects::CSeq_id& id1,
                              objects::CScope& scope1,
                              const objects::CSeq_id& id2,
                              objects::CScope& scope2);
    /// @}
protected:
    /// Matches two ids using current policy; does not consider synonyms
    static bool x_SimpleMatch(const objects::CSeq_id_Handle& h_sel_id,
                              const objects::CSeq_id_Handle& h_id);
private:
    void x_Init();

public:
    static  EObjMatchPolicy     sm_ObjMatchPolicy;
    static  EIdMatchPolicy      sm_IdMatchPolicy;
    static  bool    sm_MatchAlnLocs;
    static  bool    sm_MatchByProduct;
    static  bool    sm_AutoBroadcast;
    static  bool    sm_InterDocBroadcast; /// broadcast sel. between documents
    
    static  vector<string> sm_TreeBroadcastProperties; /// Node properties used to compare trees when broadcasting
    /// If true, when views of the same tree are opened, selection will be broadcasted based on node ids.
    /// If false, selection will be defined by broadcast properties
    static  bool    sm_TreeBroadcastOneToOne;

protected:
    enum    EObjType {
        eSeq_feat,
        eSeq_align,
        eSeq_loc,
        eSeq_id
    };
    typedef map<string, EObjType> TRawNameToType;
    static  TRawNameToType  sm_RawNameToType;

    CRef<objects::CScope> m_Scope; // scope corresponding to the source of selection

    // Range Selection package
    bool    m_HasRangeSelection;
    objects::CHandleRangeMap m_RangeMap; // storage for Range Selection

    // Object Selection Package
    bool    m_HasObjectSelection;
    TFeats      m_Feats;
    TAligns     m_Aligns;
    TSeqLocs    m_SeqLocs;
    TIds        m_Ids;
    TIdLocs     m_IdLocs;
    TTaxIds     m_TaxIds;
	TOther      m_Other;
    TIndexed    m_Indexed;
};

END_NCBI_SCOPE

#endif  // GUI_UTILS___VIEW_EVENT__HPP
