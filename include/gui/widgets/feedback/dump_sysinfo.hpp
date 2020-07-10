#ifndef GUI_WIDGETS_FEEDBACK___DUMP_SYSINFO__HPP
#define GUI_WIDGETS_FEEDBACK___DUMP_SYSINFO__HPP

/*  $Id: dump_sysinfo.hpp 38225 2017-04-12 14:24:56Z evgeniev $
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
  */


#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>

BEGIN_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////
//  CDumpSystemInfo::
//  Generates a text formatted report of the current system configuration

class NCBI_GUIWIDGETS_FEEDBACK_EXPORT CDumpSysInfo
{
public:
    /// Dump a text formatted report of the current system configuration.
    /// This includes parameters for the host CPU, memory subsystem,
    /// graphics card, and OpenGL subsystem.
    static void DumpSystemInfo(CNcbiOstream& ostr);
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_FEEDBACK___DUMP_SYSINFO__HPP
