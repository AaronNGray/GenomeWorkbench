#ifndef GUI_WIDGETS_EDIT___UTILITIES__HPP
#define GUI_WIDGETS_EDIT___UTILITIES__HPP

/*  $Id: utilities.hpp 42855 2019-04-21 11:24:32Z bollin $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/seq/Seqdesc.hpp>

BEGIN_NCBI_SCOPE
NCBI_GUIWIDGETS_EDIT_EXPORT string GetAdjustedRnaName(const string& orig_rna_name);
NCBI_GUIWIDGETS_EDIT_EXPORT CRef<objects::CPubdesc> GetPubdescFromEntrezById(int id);
NCBI_GUIWIDGETS_EDIT_EXPORT void GetPubdescLabels 
(const objects::CPubdesc& pd, 
 vector<int>& pmids, vector<int>& muids, vector<int>& serials,
 vector<string>& published_labels, vector<string>& unpublished_labels);
NCBI_GUIWIDGETS_EDIT_EXPORT string GetPubdescLabel(const objects::CPubdesc& pd);
NCBI_GUIWIDGETS_EDIT_EXPORT string GetDocSumLabel(const objects::CPubdesc& pd);
NCBI_GUIWIDGETS_EDIT_EXPORT string GetStringFromTitle(const objects::CTitle::C_E& title);
NCBI_GUIWIDGETS_EDIT_EXPORT void UpdateTitle(objects::CTitle::C_E& title, const string& val);


// interface for descriptor panels
class IDescEditorPanel 
{
public:
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc) = 0;
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc) = 0;
    virtual void UpdateTaxname() {};
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___UTILITIES__HPP
