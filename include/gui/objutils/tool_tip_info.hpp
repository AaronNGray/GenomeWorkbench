#ifndef GUI_WIDGETS_OBJUTILS___TOOL_TIP_INFO_HPP
#define GUI_WIDGETS_OBJUTILS___TOOL_TIP_INFO_HPP
 /* ===========================================================================
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
 * Authors:  Bob Falk
 *
 * File Description:
 * CToolTipInfo is a simple container class used to hold information that can
 * can be used to configure the display of a tooltip including text, title text
 * and what buttons to display.
 */

#include <corelib/ncbistl.hpp>

#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE

/** Contents of a single tool tip including display text and an ID
    that will indicate which underlying visual element the tip is
    attached to.
*/
class NCBI_GUIOBJUTILS_EXPORT CTooltipInfo
{
public:    

    /// Any combination of available buttons can be displayed by or-ing
    /// the desired options together, or choos eAllOptions for all buttons.
    enum EToolTipOptions   {
        ePin  = 0x01,
        eSearch  = 0x02,
        eMagnify  = 0x04,
        eInfo = 0x08,
        eAllOptions   = ePin | eSearch | eMagnify | eInfo
    };

    /// Ctor allows override of button options 
    CTooltipInfo(int opt=eAllOptions) : m_ID(""), m_Options(opt) {}

    /// Set/get display text
    void SetTipText(const std::string& tip_text) { m_Text = tip_text; }
    std::string GetTipText() const { return m_Text; }

    /// Set/get display text
    void SetTitleText(const std::string& title_text) { m_Title = title_text; }
    std::string GetTitleText() const { return m_Title; }

    /// Set/get id that identifies the underlying element
    void SetTipID(std::string id) { m_ID = id; }
    std::string  GetTipID() const { return m_ID; }

    /// Set/get options that determine which buttons are displayed
    void SetToolTipOptions(int opt) { m_Options = opt; }
    int GetToolTipOptions() const { return m_Options; }

protected:
    std::string m_Text;
    std::string m_ID;   
    std::string m_Title;

    int m_Options;
};

END_NCBI_SCOPE

#endif //GUI_WIDGETS_OBJUTILS___TOOL_TIP_INFO_HPP
