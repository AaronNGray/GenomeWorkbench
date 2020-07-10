#ifndef GUI_UTILS___GUI_OBJECT_INFO__HPP
#define GUI_UTILS___GUI_OBJECT_INFO__HPP

/*  $Id: gui_object_info.hpp 41372 2018-07-18 20:45:38Z rudnev $
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

class ITooltipFormatter;

class IGuiObjectInfo
{
public:
    virtual ~IGuiObjectInfo() {}

    virtual string GetType() const = 0;
    virtual string GetSubtype() const = 0;
    virtual string GetLabel() const = 0;
    /// Get tooltip.
    /// The tooltip may contain HTML tags.
    /// @param tooltip tooltip content 
    /// @param t_title tooltip title (can be empty)
    /// @param at_p user-interested sequence pos. For some cases,
    /// we may want to return postion-specific information, such
    /// as flanking sequence around that position, along with other
    /// object information. It is optional. (TSeqPos)-1 means no
    /// position is provided.
    /// @param isGeneratedBySvc if pointer is not NULL, the method will set this to true if the full tooltip
    /// was supplied by a service (e.g. dbsnp_tooltip_service) and should not be further modified
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const = 0;
    /// Get a set of links.
    /// @param no_ncbi_base if set to true, then the NCBI base path
    ///  should be removed from the returned URL links.
    virtual void GetLinks(ITooltipFormatter& links, bool no_ncbi_base) const = 0;
    virtual string GetIcon() const = 0;
    /// To share default view between different types.
    virtual string GetViewCategory() const = 0;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___GUI_OBJECT_INFO__HPP
