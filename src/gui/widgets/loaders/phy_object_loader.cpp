/*  $Id: phy_object_loader.cpp 38873 2017-06-28 15:10:39Z falkrb $
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
* Authors:  Roman Katargin
*/


#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>
#include <corelib/ncbistr.hpp>

#include <gui/objutils/label.hpp>

#include <algo/phy_tree/phy_node.hpp>
#include <algo/phy_tree/dist_methods.hpp>

#include <objmgr/object_manager.hpp>

#include <gui/widgets/loaders/phy_object_loader.hpp>

#include <util/icanceled.hpp>
#include <util/xregexp/regexp.hpp>
#include <gui/widgets/wx/compressed_file.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CPhyObjectLoader::CPhyObjectLoader(const vector<wxString>& filenames)
 : m_FileNames(filenames)
{
}

IObjectLoader::TObjects& CPhyObjectLoader::GetObjects()
{
    return m_Objects;
}

string CPhyObjectLoader::GetDescription() const
{
    return "Loading Newick Tree Files";
}

bool CPhyObjectLoader::PreExecute()
{
    return true;
}

bool CPhyObjectLoader::Execute(ICanceled& canceled)
{
    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    CRef<CScope> scope(new CScope(*obj_mgr));
    scope->AddDefaults();

    ITERATE(vector<wxString>, it, m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        const wxString& fn = *it;

        try {
            CCompressedFile file(fn);

            // load the tree(s).  If the file is a nexus file, have to first find
            // the tree(s) within the file, then pass them to the newick reader. 
            // Otherwise call the newick reader on the whole file
            CNcbiIstream& is = file.GetIstream();
            if (is.peek() == '#') {

                // Must be a nexus file
                // find tree section (starts with "begin trees;")
                string line;
                while (!is.eof() && is.good()) {
                    NcbiGetlineEOL(is, line);

                    // find tree block
                    if (NPOS != NStr::FindNoCase(line, "BEGIN TREES;")) {

                        // Find line(s) that start with "tree" and pass the first 
                        // '(' to the last ';' to ReadNewickTree
                        while (!is.eof() && is.good()) {
                            NcbiGetlineEOL(is, line);
                            if (NPOS != NStr::FindNoCase(line, "tree")) {                               
                                size_t start_pos = line.find_first_of('(');
                                size_t stop_pos = line.find_last_of(';');

                                if (start_pos != NPOS) {
                                    // Read tree into a string.  Tree may be on multiple lines
                                    string tree_str = line.substr(start_pos);
                                    if (stop_pos == NPOS) {
                                        while (!is.eof() && is.good()) {
                                            NcbiGetlineEOL(is, line);
                                            tree_str += line;
                                            if (NPOS != line.find_last_of(';')) {                               
                                                break;
                                            }
                                        }
                                    }

                                    // Load the tree.  But first remove any nexus metacomments:
                                    // https://code.google.com/p/beast-mcmc/wiki/NexusMetacommentFormat
                                    CRegexpUtil reg(tree_str);
                                    reg.Replace("\\)\\[([^]]+)\\]:", "):");
                                    string res = reg.GetResult();

                                    CNcbiIstrstream tree_istr(res.c_str());
                                    auto_ptr<TPhyTreeNode>  tree(ReadNewickTree(tree_istr));
                                    CRef<CBioTreeContainer> btc = MakeDistanceSensitiveBioTreeContainer(tree.get());

                                    string label;
                                    CLabel::GetLabel(*btc, &label, CLabel::eDefault, scope);
                                    m_Objects.push_back(SObject(*btc, label));
                                }
                            }
                        }
                       
                        // Done 
                        break;
                    }
                }
            }
            else {
                // Standard Newick format - file contains 1 tree
                auto_ptr<TPhyTreeNode>  tree(ReadNewickTree(file.GetIstream()));
                CRef<CBioTreeContainer> btc = MakeDistanceSensitiveBioTreeContainer(tree.get());

                // not implemented now
                /*
                if (m_Params.GetSort()) {
                CRef<CFeatureDescr> fd(new CFeatureDescr());
                fd->SetId(btc->SetFdict().Get().size());
                fd->SetName("do_sort");
                btc->SetFdict().Set().push_back(fd);
                }
                */

                string label;
                CLabel::GetLabel(*btc, &label, CLabel::eDefault, scope);
                m_Objects.push_back(SObject(*btc, label));
            }
        }
        catch (const CException& e) {
            x_UpdateHTMLResults(fn, 0, e.GetMsg());
        }
        catch (const exception& e) {
            x_UpdateHTMLResults(fn, 0, e.what());
        }
    }

    return true;
}

bool CPhyObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("Newick tree import errors"));
    return true;
}

END_NCBI_SCOPE
