#ifndef GUI_WIDGETS___TAX_TREE__HPP
#define GUI_WIDGETS___TAX_TREE__HPP

/*  $Id: tax_tree.hpp 30873 2014-07-31 20:37:16Z ucko $
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
#include <corelib/ncbi_tree.hpp>

#include <gui/objutils/objects.hpp>

#include <gui/widgets/tax_tree/tax_tree_ds.hpp>

#include <gui/utils/view_event.hpp>
#include <gui/utils/command.hpp>
#include <gui/utils/event_handler.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/treectrl.h>

#include <memory>

BEGIN_NCBI_SCOPE

class IReporter;


/// class CTaxTreeBrowser implements a tree-based view of a taxonomic
/// hierarchy.  This class was designed to deal with generating common
/// taxonomic trees, but should be flexible enough to deal with any
/// sort of taxonomic tree.
///
/// This class wraps a data source that provides an abstraction for dealing
/// with taxonomic trees.  The data source must provide a taxonomy tree
/// iterator conforming to that defined in <objects/taxon1/taxon1.hpp>.
/// No assumption is made about how the data are derived internally.  In
/// addition, no assumption is made about the UIDs that this data source
/// contains.  The data source must provide a mapping between a UID of
/// user-specified significance and a taxonomy ID and tax tree node.
///

class NCBI_GUIWIDGETS_TAXTREE_EXPORT CTaxTreeBrowser : public wxTreeCtrl
{
public:
    /// our internal tree structure
    struct STaxInfo {
        STaxInfo()
            : tax_node(NULL) {}

        CRef<CTaxTreeDataSource>    data_source;
        const objects::ITaxon1Node* tax_node;
        CTaxTreeDataSource::TUidVec uids;
    };
    typedef CTreeNode<STaxInfo> TTaxTree;

        
    class wxTaxTreeData : public wxTreeItemData {   
        TTaxTree * m_Item;
    public:
        wxTaxTreeData(TTaxTree * item) : m_Item(item){}
        TTaxTree * GetData(){return m_Item;}
        virtual ~wxTaxTreeData(){}
    };


    /// default ctor (for FLTK)
    CTaxTreeBrowser(wxWindow *parent, wxWindowID id,
            const wxPoint& pos, const wxSize& size,
            long style, const wxValidator& validator,
            const wxString& name);

    CTaxTreeBrowser(wxWindow *parent, wxWindowID id,
            const wxPoint& pos, const wxSize& size,
            long style);


    /// dtor
    ~CTaxTreeBrowser();

    /// set the abstract reporter interface
    void SetReporter(IReporter* reporter);

    /// set the data source for this widget
    void SetDataSource(CTaxTreeDataSource& ds);

    /// set the current display mode.  This controls how many nodes in the
    /// tax tree we show, and how deep the branches should run.
    void SetDisplayMode(CTaxTreeDataSource::EDisplayMode mode);

    /// retrieve the selected UIDs
    void GetSelectedUids(CTaxTreeDataSource::TUidVec& uids) const;

    /// set selected ids
    void SetSelectedUids(const CTaxTreeDataSource::TUidVec& uids,  wxTreeItemId node);

    /// Selection API
    void    GetObjectSelection(TConstObjects& objs);
    void    SetObjectSelection(const CTaxTreeDataSource::TUidVec& uids);

    /// callback function
   // friend void OnTaxtreeNotify(Fl_Widget * widget, void * data);

    /// handler for selection changed event
   // void OnSelChangedEvent(CEvent * evt);

protected:
    /// This will clear our current tree and repopulate it from our data source
    void x_Refresh();

    /// populate a given node from a tax tree node
    void x_PopulateTree(wxTreeItemId  node, TTaxTree* tree);

    /// retrieve a name for a node
    void x_GetName(TTaxTree& node, string& str) const;

    /// count the number of UIDs held underneath a given tree
    size_t x_CountNodes(TTaxTree& node) const;

protected:
    friend class CTreeFiller;

    /// the data source we use
    CRef<CTaxTreeDataSource> m_DataSource;

    auto_ptr<TTaxTree> m_TaxTree;

    /// typedef for a map of tax-id to UID vector.  This is defined in
    /// the data source; we re-typedef it here for convenience
    typedef CTaxTreeDataSource::TTaxMap TTaxMap;

    /// display mode
    CTaxTreeDataSource::EDisplayMode m_DispMode;

    /// our reporter interface
    IReporter* m_Reporter;

    wxTreeItemId m_RootId;

private:
    /// forbidden
    CTaxTreeBrowser(const CTaxTreeBrowser&);
    CTaxTreeBrowser& operator=(const CTaxTreeBrowser&);
};



END_NCBI_SCOPE

#endif  /// GUI_WIDGETS___TAX_TREE__HPP
