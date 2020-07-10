#ifndef OBJTOOLS_READERS___ALN_FORMATS__HPP
#define OBJTOOLS_READERS___ALN_FORMATS__HPP

/*  $Id: aln_formats.hpp 593747 2019-09-24 14:21:49Z ludwigf $
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
 * Authors:  
 *
 * File Description:  C++ wrappers for alignment file reading
 *
 */

BEGIN_NCBI_SCOPE

enum EAlignFormat {
    UNKNOWN,
    NEXUS,
    PHYLIP,
    CLUSTAL,
    FASTAGAP,
    SEQUIN,
    MULTALIN
};

NCBI_XOBJREAD_EXPORT string EAlignFormatToString(EAlignFormat alnFmt);
NCBI_XOBJREAD_EXPORT EAlignFormat StringToEAlignFormat(const string&);

END_NCBI_SCOPE

#endif // OBJTOOLS_READERS___ALN_READER__HPP
