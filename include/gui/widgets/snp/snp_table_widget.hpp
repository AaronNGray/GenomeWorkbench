#ifndef GUI_WIDGETS_SNP___SNP_TABLE_WIDGET__HPP
#define GUI_WIDGETS_SNP___SNP_TABLE_WIDGET__HPP

/*  $Id: snp_table_widget.hpp 18441 2008-12-04 20:37:02Z dicuccio $
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
 * Authors:  Melvin Quintos
 *
 * File Description:
 *
 */

#include <gui/utils/command.hpp>
#include <gui/widgets/controls/table_control.hpp>
#include <gui/widgets/controls/scroll_panel.hpp>

BEGIN_NCBI_SCOPE

class   CSnpTableData;
struct  SSnpFilter;

BEGIN_objects_SCOPE
class   CSeq_loc;
END_objects_SCOPE

class /*NCBI_GUIWIDGETS_SNP_EXPORT*/ CSnpTableWidget
    : public CCommandTarget
    , public CGroup
{
public:
    CSnpTableWidget(int x, int y, int w, int h, const char * label = 0);
    virtual ~CSnpTableWidget();

    void Init(CSnpTableData * ds, bool show_studyval = false);

    void LoadAccession(std::string acc);
    void SetFilter(const SSnpFilter &filter);
    void ExportTo(const char * filename);

private:
    CRef<CSnpTableData> m_SnpTable;

    CTableControl*          m_pTableControl;
    CScrollPanel*           m_pPanel;

};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SNP___SNP_TABLE_WIDGET__HPP

