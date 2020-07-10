#if !defined(AFX_PHYLO_TREE_READER_HPP__98E3CB4B_9A98_4239_9733_5C60FAB0E84F__INCLUDED_)
#define AFX_PHYLO_TREE_READER_HPP__98E3CB4B_9A98_4239_9733_5C60FAB0E84F__INCLUDED_

/*  $Id: phylo_tree_reader.hpp 29104 2013-10-22 18:01:10Z falkrb $
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

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbi_tree.hpp>
#include <gui/gui_export.h>
#include <objects/biotree/BioTreeContainer.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class IPhyloTreeReader {
public:
    virtual const CBioTreeContainer& GetTree(void) = 0;
    virtual ~IPhyloTreeReader(){}
};

class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloPhylipReader
    : public IPhyloTreeReader {
protected:
    string m_Phylip;
public:
    CPhyloPhylipReader();
    CPhyloPhylipReader(string str);
    virtual ~CPhyloPhylipReader();
    virtual const CBioTreeContainer& GetTree(void);
};


END_NCBI_SCOPE

#endif // !defined(AFX_PHYLO_TREE_READER_HPP__98E3CB4B_9A98_4239_9733_5C60FAB0E84F__INCLUDED_)
