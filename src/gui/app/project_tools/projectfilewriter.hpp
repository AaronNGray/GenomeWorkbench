#ifndef PROJECT_FILE_WRITER__HPP
#define PROJECT_FILE_WRITER__HPP
/*  $Id: projectfilewriter.hpp 33920 2015-10-01 15:36:47Z perkovan $
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
 * Author:  Nathan Bouk
 *
 * File Description:
 *   Tries to save any given CSerialObject into a file
 *
 */


// Objects includes
#include <objects/seq/Bioseq.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objmgr/scope.hpp>

#include <serial/serialbase.hpp>

#include <util/format_guess.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

class CProjectFileWriter
{
public:
    typedef CFormatGuess::EFormat EFormat;

    CProjectFileWriter(CRef<CScope> tScope);

    void AddToScope(CConstRef<CSerialObject> Object);

    bool WriteFile(const string& Filename, CConstRef<CSerialObject> Object, 
                            EFormat Format); 

private:

    CRef<CScope> Scope;

    bool x_WriteFile(CNcbiOstream& Out, CConstRef<CSerialObject> Object, EFormat Format);
    bool x_WriteObject(CNcbiOstream& Out, CConstRef<CSerialObject> Object, ESerialDataFormat Format);
    bool x_WriteGtf(CNcbiOstream& Out, CConstRef<CSerialObject> Object);
    
    TGi x_FindParent(TGi Gi);
    TGi x_GetAnnotId(CConstRef<CSeq_annot> Annot);
    TGi x_GetGi(CConstRef<CSeq_id> Id);
};

END_NCBI_SCOPE

#endif
