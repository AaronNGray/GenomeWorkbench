/*  $Id: table_data_base.cpp 26033 2012-06-29 16:56:08Z katargir $
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

#include <util/checksum.hpp>

#include <gui/objutils/table_data_base.hpp>

BEGIN_NCBI_SCOPE

string CTableDataBase::GetTableTypeId() const
{
    if (m_Id.empty()) {
        CChecksum cs(CChecksum::eMD5);
        size_t i;
        for (i = 0; i < GetColsCount(); ++i) {
            cs.AddLine(GetColumnLabel(i));
        }

        unsigned char digest[16];
        cs.GetMD5Digest(digest);
        for (i = 0; i < 16; ++i) {
            char buf[4];
            sprintf(buf, "%02X", (int)digest[i]);
            m_Id += buf;
        }
    }

    return m_Id;
}

END_NCBI_SCOPE
