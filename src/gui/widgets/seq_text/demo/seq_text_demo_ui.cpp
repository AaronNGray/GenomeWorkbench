/*  $Id: seq_text_demo_ui.cpp 15281 2007-11-01 16:22:10Z bollin $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include "seq_text_demo_ui.hpp"

#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <serial/objistr.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <serial/serial.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#include "seq_text_demo_ui_.cpp"


CSeqTextDemoUI::CSeqTextDemoUI()
{
    m_Window.reset(x_CreateWindow());

    m_MenuBar->SetCmdTarget(m_MatrixWidget);
}


CSeqTextDemoUI::~CSeqTextDemoUI()
{
    // these must be deleted in proper order
    m_DataSource.Reset();
    m_Window.reset();
    m_Scope.Reset();
    m_ObjMgr.Reset();
}


void CSeqTextDemoUI::Show(int argc, char** argv)
{
    m_Accession->value("gi|78214425");
    m_Accession->redraw();

    m_InputFile->value("E:\\Projects\\c++\\compilers\\msvc_prj\\gbench\\blastres.asn");
    m_InputFile->redraw();

    m_Window->show(argc, argv);
    while (m_Window->shown()) {
        Fl::wait();
    }
}

void CSeqTextDemoUI::x_OnLoadAccession()
{
    CSeq_id id;
    try {
        id.Set(m_Accession->value());
    }
    catch (CSeqIdException&) {
        string msg("Accession '");
        msg += m_Accession->value();
        msg += "' not recognized as a valid accession";
        NcbiMessageBox(msg, eDialog_Ok, eIcon_Exclamation,
                       "Unhandled Accession");
        return;
    }

    if ( !m_ObjMgr ) {
        m_ObjMgr = CObjectManager::GetInstance();
        CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);

        m_Scope.Reset(new CScope(*m_ObjMgr));
        m_Scope->AddDefaults();
    }

    // retrieve our sequence
    CBioseq_Handle handle = m_Scope->GetBioseqHandle(id);
    if ( !handle ) {
        string msg("Can't find sequence for accession '");
        msg += m_Accession->value();
        msg += "'";
        NcbiMessageBox(msg, eDialog_Ok, eIcon_Exclamation,
                       "Sequence Not Found");
        return;
    }

    m_DataSource.Reset(new CSeqTextDataSource(handle, *m_Scope));

    m_MatrixWidget->SetDataSource (*m_DataSource);
}

void CSeqTextDemoUI::x_OnLoadFile()
{
    string filename(m_InputFile->value());
    auto_ptr<CObjectIStream>    is(CObjectIStream::Open(eSerial_AsnText, filename));

    CRef<CSeq_entry> sep(new CSeq_entry());
    *is >> *sep;

    if ( !m_ObjMgr ) {
        m_ObjMgr = CObjectManager::GetInstance();
        CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);

        m_Scope.Reset(new CScope(*m_ObjMgr));
        m_Scope->AddDefaults();
    }

    m_DataSource.Reset(new CSeqTextDataSource(*sep, *m_Scope));

}

END_NCBI_SCOPE
