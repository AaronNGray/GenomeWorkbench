#ifndef GUI_UTILS___GUI_SEQ_INFO__HPP
#define GUI_UTILS___GUI_SEQ_INFO__HPP

/*  $Id: gui_seq_info.hpp 26341 2012-08-29 20:54:24Z katargir $
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

#include <corelib/ncbistr.hpp>

/** @addtogroup GUI_UTILS
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class IGuiSeqInfo
{
public:
    virtual ~IGuiSeqInfo() {}

    virtual bool IsSequence() const = 0;
    virtual bool IsDNA() const = 0;
    virtual bool IsProtein() const = 0;
    virtual bool IsGenomic() const = 0;
    virtual bool IsRNA() const = 0;
    virtual bool IscDNA() const = 0;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___GUI_SEQ_INFO__HPP
