#ifndef GUI_UTILS___ICLIPBOARD_HANDLER__HPP
#define GUI_UTILS___ICLIPBOARD_HANDLER__HPP

/*  $Id: iclipboard_handler.hpp 18080 2008-10-14 22:33:27Z yazhuk $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */


#include <corelib/ncbiexpt.hpp>
#include <gui/gui.hpp>
#include <set>
#include <algorithm>

BEGIN_NCBI_SCOPE


class NCBI_GUIUTILS_EXPORT IClipboardHandler
{
public:
    IClipboardHandler(){};
    virtual ~IClipboardHandler(){};

protected:
    /// clipboard operations
    virtual int x_OnCopy();
    virtual int x_OnPaste();
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___ICLIPBOARD_HANDLER__HPP
