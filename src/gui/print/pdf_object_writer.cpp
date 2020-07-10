/*  $Id: pdf_object_writer.cpp 29316 2013-11-29 16:14:38Z falkrb $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CPdfObjectWriter - write PDF objects to an output stream
 */


#include <ncbi_pch.hpp>
#include "pdf_object_writer.hpp"
#include <gui/print/pdf_object.hpp>
#include <algorithm>


BEGIN_NCBI_SCOPE



CPdfObjectWriter::CPdfObjectWriter(CNcbiOstream* ostream)
: m_Strm(ostream)
{
}


CPdfObjectWriter::~CPdfObjectWriter()
{
}


void CPdfObjectWriter::SetOutputStream(CNcbiOstream* ostream)
{
    m_Strm = ostream;
}


void CPdfObjectWriter::x_SaveObjectOffset(const CRef<CPdfObject>& obj)
{
    ObjOffset off(obj->GetObjNum(), obj->GetGeneration(),
                  m_Strm->tellp() - CT_POS_TYPE(0));
    m_ObjOffsets.push_back(off);
}


void CPdfObjectWriter::WriteObject(const CRef<CPdfObject>& obj)
{
    x_SaveObjectOffset(obj);

    *m_Strm << *obj;
}


CT_POS_TYPE CPdfObjectWriter::WriteXRef(unsigned int obj_num)
{
    const CT_POS_TYPE xref_start = m_Strm->tellp();

    // write the cross - reference
    *m_Strm << "xref" << pdfeol;
    *m_Strm << "0 " << obj_num << pdfeol;
    *m_Strm << setfill('0');
    *m_Strm << setw(10) << 0 << " 65535 f " << pdfeol;
    sort(m_ObjOffsets.begin(), m_ObjOffsets.end(), ObjOffsetCompare());
    ITERATE(vector<ObjOffset>, it, m_ObjOffsets) {
        const ObjOffset& off = *it;
        *m_Strm << setw(10) << off.m_Offset << ' ';
        *m_Strm << setw(5) << off.m_Generation;
        *m_Strm << " n " << pdfeol;
    }

    return xref_start;
}


END_NCBI_SCOPE
