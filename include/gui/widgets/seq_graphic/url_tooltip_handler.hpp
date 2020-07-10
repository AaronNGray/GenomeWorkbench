#ifndef GUI_WIDGETS_SEQ_GRAPHIC___URL_TOOLTIP_HANDLER__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___URL_TOOLTIP_HANDLER__HPP

/* $Id: url_tooltip_handler.hpp 43343 2019-06-17 20:41:12Z evgeniev $
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
* Author:  Vladislav Evgeniev
*
* File Description: Defines a class to handle URL clicks in GBench
*/

#include <corelib/ncbistl.hpp>

#include <gui/gui_export.h>
#include <gui/utils/event_handler.hpp>
#include <objmgr/seq_vector.hpp>

BEGIN_NCBI_SCOPE

class CUnalignedRegionDlg;

BEGIN_SCOPE(objects)
class CScope;
END_SCOPE(objects)

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CURLTooltipHandler {
public:
    CURLTooltipHandler(objects::CScope &scope, CEventHandler &handler) : m_Scope(scope), m_Handler(handler) {}
    bool ProcessURL(const string &href);
protected:
    void    x_ParseUnalignedRegionHref(const string &href);
    void    x_ParseGenomicLinkHref(const string &href);
    void    x_ReverseSequence(string& seq, bool is_protein, bool reverse, bool flipped_strands);
    void    x_DisplayUnalignedRegion(const std::string &seq_id, TSignedSeqPos from, TSignedSeqPos to, bool polyA, bool reverse, bool flipped_strands);
    void    x_DisplayForwardStrandRegion(const objects::CSeqVector &vec, TSignedSeqPos from, TSignedSeqPos to, CUnalignedRegionDlg &dlgUnalignedRegion, bool is_protein, bool reverse, bool flipped_strands);
    void    x_DisplayReverseStrandRegion(const objects::CSeqVector &vec, TSignedSeqPos from, TSignedSeqPos to, CUnalignedRegionDlg &dlgUnalignedRegion, bool is_protein, bool reverse, bool flipped_strands);
    string  x_ReformatUrlMarkerParam(const std::string &url_params);
private:
    objects::CScope &m_Scope;
    CEventHandler   &m_Handler;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___URL_TOOLTIP_HANDLER__HPP
