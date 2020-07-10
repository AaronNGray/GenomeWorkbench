/*  $Id: phylo_tree_reader.cpp 29104 2013-10-22 18:01:10Z falkrb $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_reader.hpp>
#include <algo/phy_tree/phy_node.hpp>
#include <algo/phy_tree/bio_tree.hpp>
#include <algo/phy_tree/bio_tree_conv.hpp>
#include <algo/phy_tree/dist_methods.hpp>


#include <objects/biotree/BioTreeContainer.hpp>
#include <objects/biotree/FeatureDescr.hpp>
#include <objects/biotree/FeatureDictSet.hpp>
#include <objects/biotree/NodeFeatureSet.hpp>
#include <objects/biotree/NodeFeature.hpp>
#include <objects/biotree/NodeSet.hpp>
#include <objects/biotree/Node.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CPhyloPhylipReader::CPhyloPhylipReader()
{
    m_Phylip =
    "(((AARNA01:0.42141,PARMTCY2A:0.42859):0.01269,(((A11516:0.35378,A00224:0.40813):0.03876,G00415:0.44313):0.00819, \
    (((A05242:0.37704,G00417:0.42851):0.02126,G00425:0.46138):0.00757,G01327:0.47940):0.00264):0.00301):0.00114, \
    ((((A09942:0.45435,E13931:0.44688):0.01066,((((A11520:0.34307,G00995:0.41369):0.02900,A05245:0.44051):0.00650,AB018544:0.43895):0.00326, \
    ((A01530:0.36232,G00409:0.40512):0.03619,G00413:0.44599):0.00492):0.00252):0.00456,G00427:0.45501):0.00319, \
    (((A11518:0.33086,AF063588:0.42524):0.02579,A00660:0.44207):0.01143, \
    ((((A11522:0.30733,HSNCOL4:0.35933):0.04586,G00419:0.43999):0.00819,G00423:0.44750):0.00630, \
    ((A12565:0.39548,E12148:0.42644):0.01696,G00847:0.45553):0.00696):0.00154):0.00323):0.00097,G00411:0.46818);";
}


CPhyloPhylipReader::CPhyloPhylipReader(string str)
{
    m_Phylip = str;
}

CPhyloPhylipReader::~CPhyloPhylipReader()
{
}

const CBioTreeContainer & CPhyloPhylipReader::GetTree(void)
{
    CNcbiIstrstream stream(m_Phylip.c_str());

    auto_ptr<TPhyTreeNode> tree(ReadNewickTree(stream));
    CConstRef<CBioTreeContainer> btc = MakeBioTreeContainer(tree.get());

    return btc.GetObject();
}


END_NCBI_SCOPE
