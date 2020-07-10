#ifndef GUI_UTILS___TOOLTIP__HPP
#define GUI_UTILS___TOOLTIP__HPP

/*  $Id: tooltip.hpp 44048 2019-10-15 18:28:23Z evgeniev $
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
 * Authors:  Dmitry Rudnev
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>

/** @addtogroup GUI_UTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE


/// primitive interface to arrange tabular data in the tooltips
///
/// does absolutely no checking that columns and rows are added and finished in a regular manner,
/// so must be used with care, otherwise skewed tables may result
/// generally, the table is supposed to have two or three columns:
/// [bullet | ] tag | value
/// rows can be grouped and divided with a horizontal line
class NCBI_GUIOBJUTILS_EXPORT ITooltipFormatter
{
public:
    /// supported types of formatters
	enum ETooltipFormatters {
        eTooltipFormatter_CSSTable,         ///< generated table is CSS based, generated NCBI URLs are paths (recommended for SViewer)
        eTooltipFormatter_Html,             ///< generated table is HTML attributes (no CSS) based, generate NCBI URLs are absolute (recommended for GBench)
        eTooltipFormatter_Text,             ///< plain text formatted
    };

    /// factory for requested tooltip formatter creation
	static CIRef<ITooltipFormatter> CreateTooltipFormatter(ETooltipFormatters);

    /// creates another instance of the same tooltip formatter
    virtual CIRef<ITooltipFormatter> CreateInstance() = 0;

    /// start a new table row
    virtual void StartRow() = 0;

    /// add a cell with an image
	///
	/// @note: if you add a bullet column for at least one of the table rows, a dummy bullet column
	///        needs to be added in all rows, otherwise the table will be skewed
	///
	/// @param sBulletSrc
	///   - URL of the image, is supposed to be a path to an image that is hosted on NCBI servers
	///	  - will add an empty cell if sBulletSrc is empty
	///   - for a plain text formatter, will add an asterisk (*) when sBulletSrc is not empty
    virtual void AddBulletCol(const string& sBulletSrc = "") = 0;

	/// adds a tag column to the row
	///
	/// @param sContents
	///   contents for the added tag cell
	///   - if the string is empty, an empty cell will be added
    /// @param sBulletSrc
    ///   - URL of the bullet image, is supposed to be a path to an image that is hosted on NCBI servers
    ///   - for a plain text formatter, will add an asterisk (*) when sBulletSrc is not empty
    virtual void AddTagCol(const string& sContents = "", const string& sBulletSrc = "") = 0;

	/// adds a value column to the row
	///
	/// @param sContents
	///   contents for the added value cell
	///   - if the string is empty, an empty cell will be added
    /// @param width
    ///   width of the column
	/// @param isNoWrap
	///   if set to true, will prevent the value cell contents from wrapping
    virtual void AddValueCol(const string& sContents = "", unsigned width = 200, bool isNoWrap = false) = 0;

	/// finish the row (i.e. no other contents will be added to it)
	virtual void FinishRow() = 0;

	/// add a horizontal divider between sections of the table
    virtual void AddDividerRow(unsigned colspan = 2) = 0;

    /// add a row with a cell, spanning across all columns
    ///
    /// @param sContents
    ///   contents for the added cell
    ///   - if the string is empty, an empty cell will be added
    /// @param colspan
    ///   defines the number of columns a cell should span
    virtual void AddRow(const string& sContents = "", unsigned colspan = 2) = 0;

    /// add a row with two columns
    ///
    /// @param sTag
    ///   contents for the added tag cell
    /// @param sValue
    ///   contents for the added value cell
    /// @param valueColWidth
    ///   width of the value column
	/// @param noSpaceInsert
	///   do no try to insert any spaces into sValue (some tooltip formatters may to do this to improve wrapping)
    virtual void AddRow(const string& sTag, const string& sValue, unsigned valueColWidth = 200, bool noSpaceInsert = false) = 0;

    /// add a row with two columns
    ///
    /// @param sTag
    ///   contents for the added tag cell
    /// @param sValue
    ///   contents for the added value cell
    /// @param valueColWidth
    ///   width of the value column
    virtual void AddLinkRow(const string& sTag, const string& sValue, unsigned valueColWidth = 200) = 0;

    /// add a row with two columns, the second containing a link
    ///
    /// @param sTag
    ///   contents for the added tag cell
    /// @param sText
    ///   name of the link
    /// @param sUrl
    ///   specifies the destination address
    /// @param valueColWidth
    ///   width of the value column
    virtual void AddLinkRow(const string& sTag, const string& sText, const string& sUrl, unsigned valueColWidth = 200) = 0;

    /// add a row with the links title
    ///
    /// @param sTitle
    ///   title for the links section of the tooltip
    virtual void AddLinksTitle(const string& sTitle) = 0;

    /// add a section row
    ///
    /// @param sContents
    ///   contents for the added cell
    virtual void AddSectionRow(const string& sContents) = 0;

    /// returns the formatted tooltip text and resets the formatter for reuse
    virtual string Render() = 0;

    /// Indicates if the tooltip is empty
    virtual bool IsEmpty() const = 0;

    // helper functions:
	/// construct the HTML code for a link from the displayed text label and supplied URL
	///
	/// @param sLabel
	///   text of the link label that is to be presented to the user
	/// @param sUrl
	///   URL for the link, that will be used directly as supplied
    virtual string CreateLink(const string& sLabel, const string& sUrl) const = 0;

	/// construct the HTML code for a link from the displayed text label and supplied NCBI URL path
	///
	/// @param sLabel
	///   text of the link label that is to be presented to the user
	/// @param sUrl
	///   - URL path for the link
	///   - depending on the formatter type, the generated link will contain a path or absolute URL
	///		to an NCBI resource
    virtual string CreateNcbiLink(const string& sText, const string& sUrl) const = 0;

    virtual string CreateGenomicLink(const string& sText, const string& sUrl) const { return CreateNcbiLink(sText, sUrl); }


    /// add a row containing all links to specified Pubmed IDs
	///
	/// this call will create a new row with tag "Pubmed" and value list of pubmed URLs and
	/// will do everything necessary for adding a row, including calls to StartRow()/FinishRow()

	/// @param pmids
	///   comma-delimited list of numeric pubmed ids
	///   - if pmids is empty, no row will be generated at all
	/// @param isGoToPresent
	///   flag that at call time should contain true if a "Go to" row has already been created within the current row group and will be set to true after this call if pmids is not empty
	/// @param isBulletColPresent
	///   true if other rows in the group have a bullet column
    virtual void AddPubmedLinksRow(const string& pmids, bool& isGoToPresent, bool isBulletColPresent) = 0;

    /// conditionally add a Go To row
	///
	/// @param isGoToPresent
	///   flag that at call time should contain false if a "Go to" row has not yet been created within the current row group and will be set to true after this call if pmids is not empty
	///   it will add the row then and the flag to true
	/// @param isBulletColPresent
	///   true if other rows in the group have a bullet column
    virtual void MaybeAddGoToRow(bool& isGoToPresent, bool isBulletColPresent) = 0;

    /// appends another formatter to this one
    ///
    /// @param tooltip
    ///   tooltip to append to the current one
    virtual void Append(const ITooltipFormatter& tooltip) = 0;

    virtual ~ITooltipFormatter() {}
};

/// tooltips using html tables with css classes from projects/sviewer/css/style.css and generated NCBI URLs are paths
/// (recommended for SViewer)
class NCBI_GUIOBJUTILS_EXPORT CCSSTableTooltipFormatter : public CObject, public ITooltipFormatter
{
public:
    static CIRef<ITooltipFormatter> CreateTooltipFormatter();
    virtual CIRef<ITooltipFormatter> CreateInstance();
    virtual void StartRow();
    virtual void AddBulletCol(const string& sBulletSrc = "");
    virtual void AddTagCol(const string& sContents = "", const string& sBulletSrc = "");
    virtual void AddValueCol(const string& sContents = "", unsigned width = 200, bool isNoWrap = false);
    virtual void FinishRow();
    virtual void AddDividerRow(unsigned colspan = 2);
    virtual void AddRow(const string& sContents = "", unsigned colspan = 2);
    virtual void AddRow(const string& sTag, const string& sValue, unsigned valueColWidth = 200, bool = false);
    virtual void AddLinkRow(const string& sTag, const string& sValue, unsigned valueColWidth = 200);
    virtual void AddLinkRow(const string& sTag, const string& sText, const string& sUrl, unsigned valueColWidth = 200);
    virtual void AddLinksTitle(const string& sTitle);
    virtual void AddSectionRow(const string& sContents);

    virtual string Render();

    virtual bool IsEmpty() const;

    virtual string CreateLink(const string& sText, const string& sUrl) const;
    virtual string CreateNcbiLink(const string& sText, const string& sUrl) const;
    virtual void AddPubmedLinksRow(const string& pmids, bool& isGoToPresent, bool isBulletColPresent);
    virtual void MaybeAddGoToRow(bool& isGoToPresent, bool isBulletColPresent);

    virtual void Append(const ITooltipFormatter& tooltip);
private:
    string m_sTooltipText;
    string m_sLinksText;

    CCSSTableTooltipFormatter() {}
    CCSSTableTooltipFormatter(const CCSSTableTooltipFormatter&);
    CCSSTableTooltipFormatter& operator=(const CCSSTableTooltipFormatter&);
};




/// tooltips using html tables with html attibute formatting and generate NCBI URLs are absolute
/// (recommended for GBench)
class NCBI_GUIOBJUTILS_EXPORT CHtmlTooltipFormatter : public CObject, public ITooltipFormatter
{
public:
    static CIRef<ITooltipFormatter> CreateTooltipFormatter();
    virtual CIRef<ITooltipFormatter> CreateInstance();
    virtual void StartRow();
    virtual void AddBulletCol(const string& sBulletSrc = "");
    virtual void AddTagCol(const string& sContents = "", const string& sBulletSrc = "");
    virtual void AddValueCol(const string& sContents = "", unsigned width = 200, bool isNoWrap = false);
    virtual void FinishRow();
    virtual void AddDividerRow(unsigned colspan = 2);
    virtual void AddRow(const string& sContents = "", unsigned colspan = 2);
    virtual void AddRow(const string& sTag, const string& sValue, unsigned valueColWidth = 200, bool noSpaceInsert = false);
    virtual void AddLinkRow(const string& sTag, const string& sValue, unsigned valueColWidth = 200);
    virtual void AddLinkRow(const string& sTag, const string& sText, const string& sUrl, unsigned valueColWidth = 200);
    virtual void AddLinksTitle(const string& sTitle);
    virtual void AddSectionRow(const string& sContents);

    // returns the formatted tooltip and resets the formatter for reuse
    virtual string Render();

    virtual bool IsEmpty() const;

    virtual string CreateLink(const string& sText, const string& sUrl) const;
    virtual string CreateNcbiLink(const string& sText, const string& sUrl) const;
    virtual string CreateGenomicLink(const string& sText, const string& sUrl) const;
    virtual void AddPubmedLinksRow(const string& pmids, bool& isGoToPresent, bool isBulletColPresent);
    virtual void MaybeAddGoToRow(bool& isGoToPresent, bool isBulletColPresent);

    virtual void Append(const ITooltipFormatter& tooltip);
private:
    string m_sTooltipText;
    string m_sLinksText;

    void x_FindAllOccurences(const string &input, char search, vector<size_t> &occurences) const;
    unsigned x_InsertSpaces(string &input, size_t start, size_t end, size_t fragment_length = 65) const;
    void x_InsertSpaces(string &input, size_t fragment_length = 65) const;
private:
    CHtmlTooltipFormatter() {}
    CHtmlTooltipFormatter(const CHtmlTooltipFormatter&);
    CHtmlTooltipFormatter& operator=(const CHtmlTooltipFormatter&);
};


/// tooltips using plain text formatting
class NCBI_GUIOBJUTILS_EXPORT CTextTooltipFormatter : public CObject, public ITooltipFormatter
{
public:
    static CIRef<ITooltipFormatter> CreateTooltipFormatter();
    virtual CIRef<ITooltipFormatter> CreateInstance();
    virtual void StartRow();
    virtual void AddBulletCol(const string& sBulletSrc = "");
    virtual void AddTagCol(const string& sContents = "", const string& sBulletSrc = "");
    virtual void AddValueCol(const string& sContents = "", unsigned width = 200, bool isNoWrap = false);
    virtual void FinishRow();
    virtual void AddDividerRow(unsigned colspan = 2);
    virtual void AddRow(const string& sContents = "", unsigned colspan = 2);
    virtual void AddRow(const string& sTag, const string& sValue, unsigned valueColWidth = 200, bool = false);
    virtual void AddLinkRow(const string& sTag, const string& sValue, unsigned valueColWidth = 200);
    virtual void AddLinkRow(const string& sTag, const string& sText, const string& sUrl, unsigned valueColWidth = 200);
    virtual void AddLinksTitle(const string& sTitle);
    virtual void AddSectionRow(const string& sContents);


    // returns the formatted tooltip and resets the formatter for reuse
    virtual string Render();

    virtual bool IsEmpty() const;

    virtual string CreateLink(const string& sText, const string& sUrl) const;
    virtual string CreateNcbiLink(const string& sText, const string& sUrl) const;
    virtual void AddPubmedLinksRow(const string& pmids, bool& isGoToPresent, bool isBulletColPresent);
    virtual void MaybeAddGoToRow(bool& isGoToPresent, bool isBulletColPresent);

    virtual void Append(const ITooltipFormatter& tooltip);
private:
    string m_sTooltipText;
    string m_sLinksText;

    CTextTooltipFormatter() {}
    CTextTooltipFormatter(const CTextTooltipFormatter&);
    CTextTooltipFormatter& operator=(const CTextTooltipFormatter&);
};

END_NCBI_SCOPE


/* @} */

#endif  // GUI_UTILS___TOOLTIP__HPP
