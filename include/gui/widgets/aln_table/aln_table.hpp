#ifndef GUI_WIDGETS_ALN_TABLE___ALN_TABLE__HPP
#define GUI_WIDGETS_ALN_TABLE___ALN_TABLE__HPP

/*  $Id: aln_table.hpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <gui/utils/command.hpp>
#include <gui/widgets/wx/table_listctrl.hpp>
#include <gui/widgets/aln_table/aln_table_ds.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/registry.hpp>

#include <objmgr/scope.hpp>

#include <objects/seqalign/Seq_align.hpp>


BEGIN_NCBI_SCOPE


class NCBI_GUIWIDGETS_ALNTABLE_EXPORT CAlnTableWidget
    : public CwxTableListCtrl
{
public:
    CAlnTableWidget() {}

    CAlnTableWidget(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxLC_REPORT | wxLC_VIRTUAL
    );

    virtual ~CAlnTableWidget();

    //void SaveSettings(CRegistryWriteView& view) const;
    //void LoadSettings(CRegistryReadView& view);

    void Add(objects::CScope& scope, const objects::CSeq_align& aln);
    void SetDataSource(CAlnTableDS& ds);
    CAlnTableDS& GetDataSource() { return *m_Model; }
    void Update();

    void GetSelection( TConstObjects& objs ) const;

    /// @name ISelectionModel::ISMListener Interface
    /// @{
    //void SelectionChanged(const ISelectionModel::CSMNotice& msg);
    /// @}

private:
    CRef<CAlnTableDS> m_Model;

    /// prohibited
    CAlnTableWidget(const CAlnTableWidget&);
    CAlnTableWidget& operator=(const CAlnTableWidget&);
};




END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_TABLE___ALN_TABLE__HPP
