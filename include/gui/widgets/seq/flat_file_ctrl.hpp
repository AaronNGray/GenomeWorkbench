#ifndef GUI_WIDGETS_SEQ___FLAT_FILE_CTRL__HPP
#define GUI_WIDGETS_SEQ___FLAT_FILE_CTRL__HPP

/*  $Id: flat_file_ctrl.hpp 38170 2017-04-05 18:39:48Z katargir $
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

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CBioseq_Handle;
END_SCOPE(objects)

class IFlatFileCtrl
{
public:
    virtual ~IFlatFileCtrl() {}

    //
    // Parameters: seq - sequence name which must be shown, must not be empty                         
    //             object - an ASN object which should be made visible 
    //                      in the FlatFile, might be NULL
    //
    // Returns: false if seq is empty or positioning failed
    //          true in other cases including the case when data is not loaded yet
    //
    virtual bool SetPosition(const string& seq, const CObject* object) = 0;
    virtual bool SetPosition(const objects::CBioseq_Handle& h, const CObject* object) = 0;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FLAT_FILE_CTRL__HPP
