#ifndef GUI_WIDGETS_FEEDBACK___FEEDBACK__HPP
#define GUI_WIDGETS_FEEDBACK___FEEDBACK__HPP

/*  $Id: feedback.hpp 38225 2017-04-12 14:24:56Z evgeniev $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE


///
/// single callable function that will show the feedback wizard, collect
/// information, and file a report.  This function will block until all
/// actions have completed.
///
/// The 'opt out' parameter adds some logic to the dialog to present users with
/// the choice of not seeing the dialog in the future.  This is useful if the
/// dialog automatically pops up after a crash.  Other uses include people
/// directly accessing the dialog; the default assumes that the user really
/// wants to see the dialog.
///

void NCBI_GUIWIDGETS_FEEDBACK_EXPORT ShowFeedbackDialog(bool opt_out = false, const char* problem_description = nullptr);


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_FEEDBACK___FEEDBACK__HPP
