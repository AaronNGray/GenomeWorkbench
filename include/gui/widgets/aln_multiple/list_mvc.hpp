#ifndef GUI_WIDGETS_ALN_MULTIPLE___LIST_MVC__HPP
#define GUI_WIDGETS_ALN_MULTIPLE___LIST_MVC__HPP

/*  $Id: list_mvc.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbistd.hpp>

BEGIN_NCBI_SCOPE

template <class Item> class ISelListView;

////////////////////////////////////////////////////////////////////////////////
///  interface ISelListModel
template <class Item> class ISelListModel
{
public:
    typedef Item    TItem;
    typedef int     TIndex;
    typedef vector<TIndex>   TIndexVector;

    virtual ~ISelListModel() {}

    virtual TIndex     SLM_GetItemsCount() const = 0;
    virtual TItem   SLM_GetItem(TIndex index) const = 0;

    virtual TIndex  SLM_GetFocusedItemIndex() const = 0;
    virtual bool    SLM_IsItemSelected(TIndex index) const = 0;
    virtual TIndex  SLM_GetSelectedCount() const = 0;
    virtual void    SLM_GetSelectedIndices(TIndexVector& vIndices) const = 0;

    virtual void    SLM_FocusItem(TIndex index) = 0;
    virtual void    SLM_SelectSingleItem(TIndex index) = 0;
    virtual void    SLM_InvertSingleItem(TIndex index) = 0;
    virtual void    SLM_SelectItems(const TIndexVector& vIndices,
                                    bool b_reset_others = false) = 0;
    virtual void    SLM_SelectAll(bool bSelect = true) = 0;
    virtual void    SLM_SelectTo(TIndex index) = 0;

    virtual void    SLM_AddSLView(ISelListView<Item>* pView) = 0;
    virtual void    SLM_RemoveSLView(ISelListView<Item>* pView) = 0;
};

////////////////////////////////////////////////////////////////////////////////
///  interface ISelListView
template <class Item>   class ISelListView
{
public:
    typedef ISelListModel<Item>                  TSelListModel;
    typedef typename TSelListModel::TIndexVector TIndexVector;

    virtual ~ISelListView() {}
    virtual void  SLV_SetModel(TSelListModel* pModel) = 0;

    virtual void  SLV_UpdateRange(int iFirstItem, int iLastItem) = 0;
    virtual void  SLV_UpdateItems(const TIndexVector& vIndices) = 0;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_MULTIPLE___LIST_MVC__HPP
