/*  $Id: ui_tool.cpp 33229 2015-06-19 16:27:50Z katargir $
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
 * Authors:  Andrey Yazhuk
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistl.hpp>
#include <corelib/ncbireg.hpp>
#include <corelib/ncbifile.hpp>

#include <gui/widgets/wx/ui_tool.hpp>

#include <math.h>
#include <stdio.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// ITemplateUITool

ITemplateUITool::~ITemplateUITool()
{
}


bool    ITemplateUITool::Load(CNcbiRegistry& reg)
{
    return LoadInfo(reg);
}


bool    ITemplateUITool::Save(CNcbiRegistry& reg)
{
    return SaveInfo(reg);
}



END_NCBI_SCOPE
