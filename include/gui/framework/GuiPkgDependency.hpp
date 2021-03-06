/* $Id: GuiPkgDependency.hpp 16663 2008-04-15 16:41:49Z katargir $
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
 */

/// @file GuiPkgDependency.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'gui_framework.dtd'.
///
/// New methods or data members can be added to it if needed.
/// See also: GuiPkgDependency_.hpp


#ifndef GUI_FRAMEWORK_GUIPKGDEPENDENCY_HPP
#define GUI_FRAMEWORK_GUIPKGDEPENDENCY_HPP


// generated includes
#include <gui/framework/GuiPkgDependency_.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

/////////////////////////////////////////////////////////////////////////////
class CGuiPkgDependency : public CGuiPkgDependency_Base
{
    typedef CGuiPkgDependency_Base Tparent;
public:
    // constructor
    CGuiPkgDependency(void);
    // destructor
    ~CGuiPkgDependency(void);

private:
    // Prohibit copy constructor and assignment operator
    CGuiPkgDependency(const CGuiPkgDependency& value);
    CGuiPkgDependency& operator=(const CGuiPkgDependency& value);

};

/////////////////// CGuiPkgDependency inline methods

// constructor
inline
CGuiPkgDependency::CGuiPkgDependency(void)
{
}


/////////////////// end of CGuiPkgDependency inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE


#endif // GUI_FRAMEWORK_GUIPKGDEPENDENCY_HPP
/* Original file checksum: lines: 86, chars: 2527, CRC32: 8dd4618f */
