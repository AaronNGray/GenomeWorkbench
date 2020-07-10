#ifndef GUI_WIDGETS_SEQ___TEXT_FASTA_DS__HPP
#define GUI_WIDGETS_SEQ___TEXT_FASTA_DS__HPP

/*  $Id: text_fasta_ds.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objects/seqloc/Seq_loc.hpp>

#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE


class NCBI_GUIWIDGETS_SEQ_EXPORT CFastaTextDS : public CObject
{
public:

    CFastaTextDS(const objects::CBioseq_Handle& handle,
                 const objects::CSeq_loc& loc);

    const objects::CSeq_loc&       GetLocation(void) const;
    const objects::CBioseq_Handle& GetBioseqHandle(void) const;

private:

    objects::CBioseq_Handle      m_Handle;
    CConstRef<objects::CSeq_loc> m_Loc;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___TEXT_FASTA_DS__HPP
