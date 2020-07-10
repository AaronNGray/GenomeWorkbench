/*  $Id: import_message_handler.cpp 585671 2019-05-02 15:04:11Z ludwigf $
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
 * Author: Frank Ludwig
 *
 * File Description:  Iterate through file names matching a given glob pattern
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <objtools/import/import_message_handler.hpp>

USING_NCBI_SCOPE;
//USING_SCOPE(objects);

//  ============================================================================
CImportMessageHandler::CImportMessageHandler():
//  ============================================================================
    mWorstErrorLevel(CImportError::NONE)
{
};

//  ============================================================================
CImportMessageHandler::~CImportMessageHandler()
//  ============================================================================
{
};

//  ============================================================================
void
CImportMessageHandler::ReportError(
    const CImportError& error)
//  ============================================================================
{
    if (error.Severity() < mWorstErrorLevel) {
        mWorstErrorLevel = error.Severity();
    }
    switch(error.Severity()) {
    default:
        mErrors.push_back(error);
        return;
    case CImportError::PROGRESS:
        cerr << error.Message() << "\n";
        return;
    case CImportError::FATAL:
        mErrors.push_back(error);
        throw error;
    }
}

//  ============================================================================
void
CImportMessageHandler::ReportProgress(
    const CImportProgress& progress)
//  ============================================================================
{
    progress.Serialize(cerr);
}

//  ============================================================================
void
CImportMessageHandler::Dump(
    CNcbiOstream& out)
//  ============================================================================
{
    for (auto error: mErrors) {
        error.Serialize(out);
    } 
    out.flush();
}
