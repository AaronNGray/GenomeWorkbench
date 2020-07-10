#ifndef GUI_OBJUTILS___CMD_FACTORY__HPP
#define GUI_OBJUTILS___CMD_FACTORY__HPP

/*  $Id: cmd_factory.hpp 31553 2014-10-22 16:28:19Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/utils/command_processor.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeq_entry_Handle;
    class CPub_equiv;
    class CCit_art;
    class CGene_ref;
END_SCOPE(objects)

class NCBI_GUIOBJUTILS_EXPORT CCmdFactory
{
public:
    static CIRef<IEditCommand>
        RemoveCitations(objects::CSeq_entry_Handle& seh, const objects::CPub_equiv& pubs);

    static CIRef<IEditCommand>
        DeleteGeneXRefs(objects::CSeq_entry_Handle& seh, const objects::CGene_ref& gene);
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_FACTORY__HPP
