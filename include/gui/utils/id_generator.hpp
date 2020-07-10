#ifndef GUI_UTILS__ID_GENERATOR_HPP
#define GUI_UTILS__ID_GENERATOR_HPP

/*  $Id: id_generator.hpp 28864 2013-09-05 19:16:28Z katargir $
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
 * Authors:  Peter Meric, Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

/** @addtogroup GUI_UTILS
 *
 * @{
 */

#include <set>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CIdGenerator - generates consecutive integer identifiers
class NCBI_GUIUTILS_EXPORT CIdGenerator : public CObject
{
public:
    CIdGenerator(unsigned int first_id = 1);
    virtual ~CIdGenerator();

    unsigned int NextId(void);

private:
    unsigned int m_Id;
};

////////////////////////////////////////////////////////////////////////////////
/// CUniqueLabelGenerator
/// Modifies  the given label so that it becomes unique in the given set
/// of labels by appending a numeric postfix ( "name" -> "name (1)" )
class NCBI_GUIUTILS_EXPORT CUniqueLabelGenerator
{
public:
    CUniqueLabelGenerator();

    /// adds an existing label
    void    AddExistingLabel(const string& label);

    /// after considering all existing labels produces a modified unique
    /// version of the original label
    string  MakeUniqueLabel(const string& label) const;

protected:
    set<string>    m_ExistingLabels;
};

END_NCBI_SCOPE

/* @} */

#endif // GUI_UTILS__ID_GENERATOR_HPP
