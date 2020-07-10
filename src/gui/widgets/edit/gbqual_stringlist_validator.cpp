/*  $Id: gbqual_stringlist_validator.cpp 37586 2017-01-25 15:53:12Z filippov $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Gb_qual.hpp>

#include <serial/objectinfo.hpp>

#include <gui/widgets/edit/gbqual_stringlist_validator.hpp>
#include "string_list_ctrl.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CSerialStringListValidator
 */


CGBQualStringListValidator::CGBQualStringListValidator(CSerialObject& object, const string& qualName)
    : m_Object(object), m_QualName(qualName)
{
}


CGBQualStringListValidator::CGBQualStringListValidator(const CGBQualStringListValidator& val)
    : wxValidator(), m_Object(val.m_Object), m_QualName(val.m_QualName)
{
}


bool CGBQualStringListValidator::TransferToWindow()
{
    CStringListCtrl *control = (CStringListCtrl*)m_validatorWindow;
    control->Clear();
    size_t maxStringsCount =  control->GetMaxStringsCount();

	  const CSeq_feat& seqfeat = dynamic_cast<const CSeq_feat&>(m_Object);
	  if (seqfeat.IsSetQual()) {
		    ITERATE (CSeq_feat::TQual, qual, seqfeat.GetQual()) {
			      if ((*qual)->IsSetQual() && NStr::Equal((*qual)->GetQual(), m_QualName)) {
				        if ((*qual)->IsSetVal()) {
				            control->AddString((*qual)->GetVal());
				        } else {
					          control->AddString("");
				        }
                if (--maxStringsCount == 0)
                    break;
			      }
		    }
	  }

    if (maxStringsCount > 0)
        control->AddString("");

    control->FitInside();

    return true;
}

bool CGBQualStringListValidator::TransferFromWindow()
{
	CSeq_feat& seqfeat = dynamic_cast<CSeq_feat&>(m_Object);

	// first, remove existing gbquals
	if (seqfeat.IsSetQual()) {
		CSeq_feat::TQual::iterator qual = seqfeat.SetQual().begin();
		while (qual != seqfeat.SetQual().end()) {
            if ((*qual)->IsSetQual() && NStr::Equal((*qual)->GetQual(), m_QualName)) {
                qual = seqfeat.SetQual().erase(qual);
			} else {
				qual++;
			}
		}
	}
	//now add new ones
    CStringListCtrl  *control = (CStringListCtrl *)m_validatorWindow;
    CStringListCtrl::CConstIterator it(*control);

    if (it) {
        for (; it; ++it) {
            string value = it.GetValue();
            NStr::TruncateSpacesInPlace(value);
			if (!value.empty()) {
				CRef<CGb_qual> qual(new CGb_qual(m_QualName, value));
                seqfeat.SetQual().push_back(qual);
			}
        }
    }
    return true;
}

END_NCBI_SCOPE
