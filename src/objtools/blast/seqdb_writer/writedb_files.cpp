/*  $Id: writedb_files.cpp 557074 2018-02-09 14:23:28Z fongah2 $
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
 * Author:  Kevin Bealer
 *
 */

/// @file writedb_files.cpp
/// Implementation for the CWriteDB_Files class.
/// class for WriteDB.
#include <ncbi_pch.hpp>
#include <objtools/blast/seqdb_writer/writedb_files.hpp>
#include <objtools/blast/seqdb_writer/writedb_convert.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/serial.hpp>
#include <iostream>
#include <sstream>

BEGIN_NCBI_SCOPE

/// Use standard C++ definitions.
USING_SCOPE(std);

// Blast Database Format Notes (version 4).
// (See below for version 5.)
//
// Integers are 4 bytes stored in big endian format, except for the
// volume length.  The volume length is 8 bytes, but is stored in a
// little endian byte order (reason unknown).

// The 'standard' packing for strings in Blast DBs is as follows:
//   0..4: length
//   4..4+length: string data
//
// The title string follows this rule, but the create date has an
// additional detail; if it does not end on an offset that is a
// multiple of 8 bytes, extra 'NUL' characters are added to bring it
// to a multiple of 8 bytes.  The NUL characters are added after the
// string bytes, and the stored length of the string is increased to
// include them.  After extracting the string, 0-7 NUL bytes will need
// to be stripped from the end of the string (if any are found).
//
// (If this were not done, the offsets in the file would be unaligned;
// on some architectures this could cause a performance penalty or
// other problems.  On little endian architectures such as Intel, this
// penalty is always paid.)

// INDEX FILE FORMAT, for "Blast DB Version 4"
//
// 0..4:         format version  (Blast DB version, current is "4").
// 4..8:         seqtype (1 for protein or 0 for nucleotide).
// 8..N1:        title (string).
// N1..N2:       create date (string).
// N2..N2+4:     number of OIDs (#OIDS).
// N2+4..N2+12:  number of letters in volume. (note: 8 bytes)
// N2+12..N2+16: maxlength (size of longest sequence in DB)
//
// N2+16..(end): Array data
//
//  Array data is 2 or 3 arrays of (#OIDS + 1) four byte integers.
//  For protein, 2 arrays are used; for nucleotide, 3 are used.
//
//  The first array is header offsets, the second array is sequence
//  offsets, and the third (optional) array is offsets of ambiguity
//  data.  Each array has a final element which is the length of the
//  file; this makes it possible to compute the last sequence's length
//  without adding a special case.
//
// As shown, the total size of index header =
//   4*4 bytes          // 4 int fields (4 bytes each)
//   + 8 bytes          // 8 byte field
//   + 2*4 + strings    // 4 bytes length for each plus string data.
//   = (32 + strings), rounded up to nearest multiple of 8
//
// "strings" here refers to the unterminated length of both strings.

// Blast Database Format Notes (version 5).
// (See above for version 4.)
//
// Integers are 4 bytes stored in big endian format, except for the
// volume length.  The volume length is 8 bytes, but is stored in a
// little endian byte order (reason unknown).

// The 'standard' packing for strings in Blast DBs is as follows:
//   0..4: length
//   4..4+length: string data
//
// The title string and LMDB string follow this rule, but the create
// date has an additional detail; if it does not end on an offset that
// is a multiple of 8 bytes, extra 'NUL' characters are added to bring
// it to a multiple of 8 bytes.  The NUL characters are added after the
// string bytes, and the stored length of the string is increased to
// include them.  After extracting the string, 0-7 NUL bytes will need
// to be stripped from the end of the string (if any are found).
//
// (If this were not done, the offsets in the file would be unaligned;
// on some architectures this could cause a performance penalty or
// other problems.  On little endian architectures such as Intel, this
// penalty is always paid.)

// --------------------------------------------

// INDEX FILE FORMAT, for "Blast DB Version 5"
//
// 0..4:         format version  (Blast DB version, current is "5").
// 4..8:         seqtype (1 for protein or 0 for nucleotide).
// 8..12:        this volume number (0 and up).
// 12..N1:       title (string).
// N1..N2:       name of LMDB database file (string)
// N2..N3:       create date (string).
// N3..N3+4:     number of OIDs (#OIDS).
// N3+4..N3+12:  number of letters in volume. (note: 8 bytes)
// N3+12..N3+16: maxlength (size of longest sequence in DB)
//
// N3+16..(end): Array data
//
//  Array data is 2 or 3 arrays of (#OIDS + 1) four byte integers.
//  For protein, 2 arrays are used; for nucleotide, 3 are used.
//
//  The first array is header offsets, the second array is sequence
//  offsets, and the third (optional) array is offsets of ambiguity
//  data.  Each array has a final element which is the length of the
//  file; this makes it possible to compute the last sequence's length
//  without adding a special case.
//
// As shown, the total size of index header =
//   5*4 bytes          // 5 int fields (4 bytes each)
//   + 8 bytes          // 8 byte field
//   + 3*4 + strings    // 4 bytes length for each plus string data.
//   = (40 + strings), rounded up to nearest multiple of 8
//
// "strings" here refers to the unterminated length of both strings.

CWriteDB_File::CWriteDB_File(const string & basename,
                             const string & extension,
                             int            index,
                             Uint8          max_file_size,
                             bool           always_create)
    : m_Created    (false),
      m_BaseName   (basename),
      m_Extension  (extension),
      m_Index      (index),
      m_Offset     (0),
      m_MaxFileSize(max_file_size)
{
    // Define number of usable bits in m_Offset,
    // deducting one for the sign bit.    
    // Define maximum allowed max_file_size.
#ifdef _DEBUG        
    static const int MAX_OFFSET_BITS = (sizeof m_Offset * 8);    
    static const Uint8 MAX_FILE_SIZE = ((Uint8) 1 << MAX_OFFSET_BITS);      
#endif

    if (m_MaxFileSize == 0) {
        m_MaxFileSize = x_DefaultByteLimit();
    } else {
#ifdef _DEBUG
        _ASSERT(max_file_size <= MAX_FILE_SIZE);
#endif
    }

    m_Nul.resize(1);
    m_Nul[0] = (char) 0;

    m_UseIndex = (index >= 0);
    x_MakeFileName();

    if (always_create) {
        Create();
    }
}

void CWriteDB_File::Create()
{
    _ASSERT(! m_Created);
    m_Created = true;
    m_RealFile.open(m_Fname.c_str(), ios::out | ios::binary);
}

unsigned int CWriteDB_File::Write(const CTempString & data)
{
    // Define maximum allowed max_file_size.
#ifdef _DEBUG    
    // Define number of usable bits in m_Offset,
    // deducting one for the sign bit.    
    static const int MAX_OFFSET_BITS = (sizeof m_Offset * 8);    
    static const Uint8 MAX_OFFSET = ((Uint8) 1 << MAX_OFFSET_BITS);        
#endif

    _ASSERT(m_Created);
#ifdef _DEBUG
    _ASSERT(((Uint8) m_Offset + data.length()) <= MAX_OFFSET);
#endif
    m_RealFile.write(data.data(), data.length());

    m_Offset += data.length();
    return m_Offset;
}

string CWriteDB_File::MakeShortName(const string & base, int index)
{
    ostringstream fns;

    fns << base;
    fns << ".";
    fns << (index / 10);
    fns << (index % 10);

    return fns.str();
}

void CWriteDB_File::x_MakeFileName()
{
    if (m_UseIndex) {
        m_Fname = MakeShortName(m_BaseName, m_Index);
    } else {
        m_Fname = m_BaseName;
    }

    m_Fname += ".";
    m_Fname += m_Extension;    
}

void CWriteDB_File::Close()
{
    x_Flush();
    if (m_Created) {
        m_RealFile.close();
    }
}

void CWriteDB_File::RenameSingle()
{
    _ASSERT(m_UseIndex == true);

    string nm1 = m_Fname;
    m_UseIndex = false;
    x_MakeFileName();

    CDirEntry fn1(nm1);
    fn1.Rename(m_Fname, CDirEntry::fRF_Overwrite);
}

CWriteDB_IndexFile::CWriteDB_IndexFile(const string & dbname,
                                       bool           protein,
                                       const string & title,
                                       const string & date,
                                       int            index,
                                       Uint8          max_file_size,
                                       EBlastDbVersion    dbver)
    : CWriteDB_File(dbname,
                    protein ? "pin" : "nin",
                    index,
                    max_file_size,
                    true),
      m_Protein   (protein),
      m_Title     (title),
      m_Date      (date),
      m_OIDs      (0),
      m_DataSize  (0),
      m_Letters   (0),
      m_MaxLength (0),
      m_Version   (dbver)
{
    // Compute index overhead, rounding up.

    m_Overhead = x_Overhead(title, date);
    if (dbver == eBDB_Version5) {
        m_Overhead = x_Overhead(title, x_MakeLmdbName(), date);
    } else {
        m_Overhead = x_Overhead(title, date);
    }
    m_Overhead = s_RoundUp(m_Overhead, 8);
    m_DataSize = m_Overhead;

    // The '1' added to the sequence offset array refers to the fact
    // that sequence files contain an initial NUL byte.  This seems to
    // be for the benefit of the protein database scanning code, but
    // it is also done for nucleotide databases.

    m_Hdr.push_back(0);
    m_Seq.push_back(1);
}

int CWriteDB_IndexFile::x_Overhead(const string & T,
                                   const string & lmdbName,
                                   const string & D)
{
    return 5 * sizeof(int) + sizeof(long)
            + 3 * sizeof(int) + T.size() + lmdbName.size() + D.size();
}

int CWriteDB_IndexFile::x_Overhead(const string & T,
                                   const string & D)
{
    return 4 * sizeof(int) + sizeof(long)
            + 2 * sizeof(int) + T.size() + D.size();
}

void CWriteDB_IndexFile::x_Flush()
{
    _ASSERT(m_Created);

    bool use_lmdb = (m_Version == eBDB_Version5);

    int format_version = (int) m_Version;
    int seq_type = (m_Protein ? 1 : 0);

    // Pad the date string (see comments at top.)

    string pad_date = m_Date;
    int count = 0;
    const string lmdb_name = use_lmdb ? x_MakeLmdbName() : "";
    int overhead = use_lmdb
            ? x_Overhead(m_Title, lmdb_name, pad_date)
            : x_Overhead(m_Title, pad_date);
    while (overhead & 0x7) {
        pad_date.append(m_Nul);
        if (count != -1) {
            _ASSERT(count++ < 8);
        }
        overhead = use_lmdb
                ? x_Overhead(m_Title, lmdb_name, pad_date)
                : x_Overhead(m_Title, pad_date);
    }

    // Write header

    ostream & F = m_RealFile;

    s_WriteInt4  (F, format_version);
    s_WriteInt4  (F, seq_type);
    if (!lmdb_name.empty()) {
        s_WriteInt4  (F, m_Index);
        s_WriteString(F, m_Title);
        s_WriteString(F, lmdb_name);
    } else {
        s_WriteString(F, m_Title);
    }
    s_WriteString(F, pad_date);    
    s_WriteInt4  (F, m_OIDs);
    s_WriteInt8LE(F, m_Letters);
    s_WriteInt4  (F, m_MaxLength);

    for(unsigned i = 0; i < m_Hdr.size(); i++) {        
        s_WriteInt4(F, m_Hdr[i]);
    }

    for(unsigned i = 0; i < m_Seq.size(); i++) {
        s_WriteInt4(F, m_Seq[i]);
    }

    // Should loop m_OID times, or not at all.
    for(unsigned i = 0; i < m_Amb.size(); i++) {
        s_WriteInt4(F, m_Amb[i]);
    }

    // This extra index is added here because formatdb adds it.  SeqDB
    // depends on its existence, but I don't think anyone reads (or
    // needs) the data.  The last offset in the ambiguity column
    // represents the position of the set of ambiguities corresponding
    // to the last offset in the sequence column.  But the last
    // sequence offset is not really a sequence start, it is the
    // 'extra' offset used by sequence length computations.

    if (m_Amb.size()) {
        s_WriteInt4(F, m_Seq.back());
    }

    vector<unsigned int> tmp1, tmp2, tmp3;
    m_Hdr.swap(tmp1);
    m_Seq.swap(tmp2);
    m_Amb.swap(tmp3);
}

/// Form name of lmdb database file.
const string CWriteDB_IndexFile::x_MakeLmdbName()
{
    string suffix = (m_Protein ? ".pdb" : ".ndb");
    size_t last_slash = m_BaseName.find_last_of('/');
    if (last_slash == m_BaseName.npos) {
        return m_BaseName + suffix;
    } else {
        return m_BaseName.substr(last_slash + 1) + suffix;
    }
}

CWriteDB_HeaderFile::CWriteDB_HeaderFile(const string & dbname,
                                         bool           protein,
                                         int            index,
                                         Uint8          max_file_size)
    : CWriteDB_File(dbname,
                    protein ? "phr" : "nhr",
                    index,
                    max_file_size,
                    true),
      m_DataSize(0)
{
}

CWriteDB_SequenceFile::CWriteDB_SequenceFile(const string & dbname,
                                             bool           protein,
                                             int            index,
                                             Uint8          max_file_size,
                                             Uint8          max_letters)
    : CWriteDB_File(dbname,
                    protein ? "psq" : "nsq",
                    index,
                    max_file_size,
                    true),
      m_Letters  (0),
#ifdef _DEBUG
      m_BaseLimit(max_letters),
      m_Protein  (protein)
#else
      m_BaseLimit(max_letters)
#endif
{
    // Only protein sequences need the inter-sequence NUL bytes.
    // The first null written here is for nucleotide sequences.
    // It doesn't seem necessary, but formatdb provides it, so I
    // will too.

    WriteWithNull(string());
}

END_NCBI_SCOPE

