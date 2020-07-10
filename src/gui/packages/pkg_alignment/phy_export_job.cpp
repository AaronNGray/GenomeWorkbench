/*  $Id: phy_export_job.cpp 36053 2016-08-02 14:06:39Z evgeniev $
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
* Authors: Roman Katargin, Vladimir Tereshkov
*
*/

#include <ncbi_pch.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objmgr/util/sequence.hpp>
#include <serial/iterator.hpp>

#include <gui/objutils/label.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objtools/writers/agp_write.hpp>

#include <gui/packages/pkg_alignment/phy_export_job.hpp>

#include <algo/phy_tree/phy_node.hpp>
#include <algo/phy_tree/dist_methods.hpp>
#include <algo/phy_tree/bio_tree.hpp>
#include <algo/phy_tree/bio_tree_conv.hpp>
#include <algo/phy_tree/bio_tree_format_label.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CPhyExportJob::CPhyExportJob(const CPhyExportParams& params)
: CAppJob("Phylogenetic Tree Export"), m_Params(params)
{
}

IAppJob::EJobState CPhyExportJob::Run()
{
    CConstRef<CBioTreeContainer> tree(m_Params.GetBioTree());
    wxString fname = m_Params.GetFileName();
    unique_ptr<CBioTreeDynamicLabelFormatter> label_format;
    string err_msg = "";

    if (tree != NULL) {
        CNcbiOfstream ostr(fname.fn_str());
        CBioTreeDynamic dtree;
        BioTreeConvertContainer2Dynamic(dtree, *tree);

        string fmt = ToStdString(m_Params.GetFileFormat());
        if (!m_Params.GetLabelFormat().empty()) {
            label_format.reset(new CBioTreeDynamicLabelFormatter(dtree.GetFeatureDict(), m_Params.GetLabelFormat().ToStdString()));
        }
       
        try {
            if (fmt == "Nexus") {
                WriteNexusTree(ostr, dtree, "the_tree", label_format.get());
            }
            else {
                WriteNewickTree(ostr, dtree, label_format.get());
            }
        }
        catch (const CException& e) {
            err_msg = "Failed to save file:\n";
            err_msg += e.GetMsg();
        }
    }
    else {
        err_msg = "No tree available for export.";
    }

    if (err_msg.empty())  {
        LOG_POST(Info << "CPhyExportJob::Run()  Finished  " << m_Descr);
        return eCompleted;
    } else {
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }
}

END_NCBI_SCOPE
