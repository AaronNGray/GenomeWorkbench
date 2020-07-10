#ifndef IDCASSSCOPE__HPP
#define IDCASSSCOPE__HPP

/*  $Id: IdCassScope.hpp 568694 2018-08-09 16:07:59Z saprykin $
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
 * Authors: Dmitri Dmitrienko
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>

#define IDBLOB_NS_IDBLOB idblob

#define BEGIN_IDBLOB_SCOPE BEGIN_SCOPE(IDBLOB_NS_IDBLOB)
#define END_IDBLOB_SCOPE   END_SCOPE(IDBLOB_NS_IDBLOB)
#define USING_IDBLOB_SCOPE USING_SCOPE(IDBLOB_NS_IDBLOB)


#endif
