#ifndef GUI_UTILS__PDF_OBJECT_WRITER_HPP
#define GUI_UTILS__PDF_OBJECT_WRITER_HPP

/*  $Id: pdf_object_writer.hpp 27010 2012-12-07 16:37:16Z falkrb $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CPdfObjectWriter - write PDF objects to an output stream
 */

#include <corelib/ncbistd.hpp>
#include <gui/print/pdf_object.hpp>


BEGIN_NCBI_SCOPE



class CPdfObjectWriter
{
public:
    CPdfObjectWriter(CNcbiOstream* ostream = &NcbiCerr);
    virtual ~CPdfObjectWriter();

    void SetOutputStream(CNcbiOstream* ostream);
    void WriteObject(const CRef<CPdfObject>& obj);
    CT_POS_TYPE WriteXRef(unsigned int obj_num);

protected:
    void x_SaveObjectOffset(const CRef<CPdfObject>& obj);

    //
    // list of output stream offsets for the cross-reference table
    //
public:
    struct ObjOffset
    {
        ObjOffset(unsigned int obj_num,
                  unsigned int generation,
                  CT_OFF_TYPE offset)
            : m_ObjNum(obj_num), m_Generation(generation), m_Offset(offset)
        {
        }
        unsigned int m_ObjNum, m_Generation;
        CT_OFF_TYPE m_Offset;
    };
    struct ObjOffsetCompare
    {
        bool operator()(const ObjOffset& t1, const ObjOffset& t2)
        {
            return t1.m_ObjNum < t2.m_ObjNum;
        }
    };

private:
    CNcbiOstream* m_Strm;
    vector<ObjOffset> m_ObjOffsets;
};


inline CNcbiOstream& pdfeol(CNcbiOstream& strm)
{
    return strm << '\n';
}



END_NCBI_SCOPE


#endif // GUI_UTILS__PDF_OBJECT_WRITER_HPP
