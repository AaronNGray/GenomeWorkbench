#ifndef GUI_FRAMEWORK___DATA_MINING_CONTEXT_HPP
#define GUI_FRAMEWORK___DATA_MINING_CONTEXT_HPP

/*  $Id: data_mining_context.hpp 18011 2008-10-06 21:44:26Z yazhuk $
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


#include <gui/gui_export.h>

#include <corelib/ncbistd.hpp>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// IDataMiningContext
/// IDataMiningContext represents an abstract context for a Search. The context
/// defines the scope of the data that will be searched. A IDataMiningContext or
/// a UI component such as View can serve as search contexts.
/// Specific tools will require specialized contexts that should be represented as
/// interfaces derived from IDataMiningContext.
class NCBI_GUICORE_EXPORT IDataMiningContext
{
public:
    virtual ~IDataMiningContext()   {}

    /// returns Name of the context to be used in UI
    virtual string  GetDMContextName() = 0;
};


END_NCBI_SCOPE


/*
 * ===========================================================================
 * ===========================================================================
 */

#endif  // GUI_FRAMEWORK___DATA_MINING_CONTEXT_HPP
