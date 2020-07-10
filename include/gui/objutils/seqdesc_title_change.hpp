#ifndef GUI_OBJUTILS___SEQDESC_TITLE_CHANGE__HPP
#define GUI_OBJUTILS___SEQDESC_TITLE_CHANGE__HPP

/*  $Id: seqdesc_title_change.hpp 31553 2014-10-22 16:28:19Z katargir $
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
#include <gui/gui_export.h>

#include <objmgr/bioseq_handle.hpp>

#include <gui/utils/command_processor.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CChangeSeqdescTitleCommand : public CObject, public IEditCommand
{
friend class CChangeSeqFeatCommandFactory;

public:
    CChangeSeqdescTitleCommand(objects::CBioseq_Handle& bh, const string& title)
        : m_BH(bh), m_Title(title), m_SeqDescrCreated(false), m_TitleCreated(false) {}

    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel() { return "Modify Sequence Title"; };

private:
    objects::CBioseq_Handle m_BH;
    string m_Title;
    bool   m_SeqDescrCreated;
    bool   m_TitleCreated;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___SEQDESC_TITLE_CHANGE__HPP
