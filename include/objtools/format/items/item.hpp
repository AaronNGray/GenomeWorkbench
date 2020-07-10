#ifndef OBJTOOLS_FORMAT_ITEMS___ITEM__HPP
#define OBJTOOLS_FORMAT_ITEMS___ITEM__HPP

/*  $Id: item.hpp 601813 2020-02-13 18:41:46Z kans $
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
* Author:  Aaron Ucko, NCBI
*          Mati Shomrat
*
* File Description:
*   new (early 2003) flat-file generator -- base class for items
*   (which roughly correspond to blocks/paragraphs in the C version)
*
*/
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <serial/serialbase.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)


class IFlatTextOStream;
class IFormatter;


class NCBI_FORMAT_EXPORT IFlatItem : public CObject
{
public:

    enum EItem {
        eItem_None = 0,
        eItem_StartSection,
        eItem_StartItem,
        eItem_HtmlAnchor,
        eItem_Head,
        eItem_Locus,
        eItem_Date,
        eItem_Defline,
        eItem_Accession,
        eItem_Version,
        eItem_Project,
        eItem_GenomeProject,
        eItem_DbSource,
        eItem_Keywords,
        eItem_Segment,
        eItem_Source,
        eItem_Reference,
        eItem_Cache,
        eItem_Comment,
        eItem_Primary,
        eItem_FeatHeader,
        eItem_SourceFeat,
        eItem_Feature,
        eItem_Gap,
        eItem_BaseCount,
        eItem_Origin,
        eItem_Sequence,
        eItem_Contig,
        eItem_Wgs,
        eItem_Tsa,
        eItem_Alignment,
        eItem_EndItem,
        eItem_EndSection,
        eItem_Slash
    };

    virtual void Format(IFormatter& formatter,
                        IFlatTextOStream& text_os) const = 0;

    // For editing, return the underlying serial object (if exists).
    virtual const CSerialObject* GetObject(void) const = 0;

    // returns true, if the formatter should skip this object
    virtual bool Skip(void) const = 0;

    // returns paragraph type
    virtual EItem GetItemType(void) const = 0;

    virtual ~IFlatItem(void) {}
};


END_SCOPE(objects)
END_NCBI_SCOPE

#endif  /* OBJTOOLS_FORMAT_ITEMS___ITEM__HPP */
