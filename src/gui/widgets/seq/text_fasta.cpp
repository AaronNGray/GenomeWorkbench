/*  $Id: text_fasta.cpp 16877 2008-05-15 01:27:53Z dicuccio $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/seq/text_fasta.hpp>
#include <gui/widgets/seq/text_fasta_ds.hpp>

#include <gui/utils/clipboard.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <connect/ncbi_conn_stream.hpp>

#include <objmgr/util/sequence.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CFastaTextDisplay::CFastaTextDisplay(int x, int y, int w, int h,
                                     const char* label)
    : CTextEditor(x, y, w, h, label, true)
{
}


CFastaTextDisplay::~CFastaTextDisplay()
{
}


void CFastaTextDisplay::SetDataSource(CFastaTextDS& ds)
{
    m_DataSource.Reset(&ds);
    Update();
}


void CFastaTextDisplay::Update()
{
    if ( !buffer() ) {
        return;
    }

    if ( !m_DataSource ) {
        return;
    }

    string str;
    CBioseq_Handle handle = m_DataSource->GetBioseqHandle();
    if (handle) {
        CConn_MemoryStream ostr;
        {{
            CFastaOstream fasta_str(ostr);
            fasta_str.SetFlag(CFastaOstream::fAssembleParts);
            fasta_str.SetFlag(CFastaOstream::fInstantiateGaps);
            fasta_str.Write(handle, &m_DataSource->GetLocation());
        }}
        ostr.flush();
        str.resize(ostr.tellp() - CT_POS_TYPE(0));
        ostr.read(const_cast<char*>(str.data()), str.size());
    }

    buffer()->text(str.c_str());
}


END_NCBI_SCOPE
