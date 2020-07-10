/*  $Id: 5col_line_reader.hpp 596193 2019-11-04 14:08:10Z ludwigf $
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
* Author:  Frank Ludwig, NCBI
*
* File Description:
*   Test application for the CFormatGuess component
*
* ===========================================================================
*/

#ifndef FIVECOL_LINE_READER__HPP
#define FIVECOL_LINE_READER__HPP

#include <corelib/ncbifile.hpp>
#include <util/line_reader.hpp>

#include "feat_line_reader.hpp"

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE

//  ============================================================================
class C5ColLineReader: 
    public CFeatLineReader
//  ============================================================================
{
public:
    C5ColLineReader(
        CImportMessageHandler&);

    virtual ~C5ColLineReader() {};

    virtual bool
    GetNextRecord(
        CStreamLineReader&,
        CFeatImportData&) override;

private:
    enum ELineType {
        eLineTypeNone,
        eLineTypeSeqId,
        eLineTypeIntervalAndType,
        eLineTypeBareInterval,
        eLineTypeAttribute,
        eLineTypeOffset
    };

    void
    xSplitLine(
        const std::string&,
        std::vector<std::string>&);

    ELineType
    xLineTypeOf(
        const std::vector<std::string>&);

    void
    xInitializeRecord(
        const std::vector<std::string>&,
        CFeatImportData&) override;

    std::string mCurrentSeqId;
    std::vector<std::string> mCollectedLines;
    ELineType mLastTypeSeen;
    int mCurrentOffset;
};

END_objects_SCOPE
END_NCBI_SCOPE

#endif
