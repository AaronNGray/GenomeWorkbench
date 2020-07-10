/*  $Id: seqdesc_title_change.cpp 25347 2012-03-01 18:22:54Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/seqdesc_title_change.hpp>

#include <objects/seq/Seq_descr.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CChangeSeqdescTitleCommand::Execute()
{
    CBioseq_EditHandle eh = m_BH.GetEditHandle();
    CConstRef<CBioseq> bsc = eh.GetBioseqCore();
    CBioseq& bioseq = const_cast<CBioseq&>(*bsc);

    if (!bioseq.IsSetDescr()) {
        m_SeqDescrCreated = true;
        CRef<CSeq_descr> seq_descr(new CSeq_descr());
        bioseq.SetDescr(*seq_descr);
    }

    CSeq_descr::Tdata& seq_descr = bioseq.SetDescr();
    CSeq_descr::Tdata::iterator desc_it = seq_descr.begin();
    for (; desc_it != seq_descr.end(); ++desc_it) {
        if ((**desc_it).IsTitle())
            break;
    }

    CRef<CSeqdesc> seqdesc;
    if (desc_it == seq_descr.end()) {
        m_TitleCreated = true;
        seqdesc.Reset(new CSeqdesc());
        seqdesc->SetTitle(NcbiEmptyString);
        seq_descr.push_back(seqdesc);
        desc_it = seq_descr.end();
        --desc_it;
    }
    else
        seqdesc = *desc_it;

    string old_title = seqdesc->GetTitle();
    seqdesc->SetTitle(m_Title);
    m_Title = old_title;
}

void CChangeSeqdescTitleCommand::Unexecute()
{
    CBioseq_EditHandle eh = m_BH.GetEditHandle();
    CConstRef<CBioseq> bsc = eh.GetBioseqCore();
    CBioseq& bioseq = const_cast<CBioseq&>(*bsc);

    if (!bioseq.IsSetDescr()) {
        return; // Something is wrong
    }

    CSeq_descr::Tdata& seq_descr = bioseq.SetDescr();
    CSeq_descr::Tdata::iterator desc_it = seq_descr.begin();
    for (; desc_it != seq_descr.end(); ++desc_it) {
        if ((**desc_it).IsTitle())
            break;
    }

    if (desc_it == seq_descr.end()) {
        return; // Something is wrong
    }

    string new_title = (**desc_it).GetTitle();

    if (m_TitleCreated) {
        m_TitleCreated = false;
        seq_descr.erase(desc_it);
    }
    else {
        (**desc_it).SetTitle(m_Title);
    }

    m_Title = new_title;

    if (m_SeqDescrCreated) {
        m_SeqDescrCreated = false;
        bioseq.ResetDescr();
    }
}

END_NCBI_SCOPE
