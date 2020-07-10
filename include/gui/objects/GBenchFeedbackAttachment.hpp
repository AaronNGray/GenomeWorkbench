/* $Id: GBenchFeedbackAttachment.hpp 20230 2009-10-19 17:37:48Z voronov $
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
 */

/// @file GBenchFeedbackAttachment.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'gui_objects.asn'.
///
/// New methods or data members can be added to it if needed.
/// See also: GBenchFeedbackAttachment_.hpp


#ifndef GUI_OBJECTS_GBENCHFEEDBACKATTACHMENT_HPP
#define GUI_OBJECTS_GBENCHFEEDBACKATTACHMENT_HPP


// generated includes
#include <gui/objects/GBenchFeedbackAttachment_.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

/////////////////////////////////////////////////////////////////////////////
class NCBI_GUIOBJECTS_EXPORT CGBenchFeedbackAttachment : public CGBenchFeedbackAttachment_Base
{
    typedef CGBenchFeedbackAttachment_Base Tparent;
public:
    // constructor
    CGBenchFeedbackAttachment(void);
    // destructor
    ~CGBenchFeedbackAttachment(void);

    /// data is stored as a zlib-compressed entity
    /// to reduce the traffic

    /// decompressor
    void DecodeData( string& desc ) const;

    /// compressors
    void EncodeData( const string& str );
    void EncodeData( CNcbiIstream& istr );

private:
    // Prohibit copy constructor and assignment operator
    CGBenchFeedbackAttachment(const CGBenchFeedbackAttachment& value);
    CGBenchFeedbackAttachment& operator=(const CGBenchFeedbackAttachment& value);

};

/////////////////// CGBenchFeedbackAttachment inline methods

// constructor
inline
CGBenchFeedbackAttachment::CGBenchFeedbackAttachment(void)
{
}


/////////////////// end of CGBenchFeedbackAttachment inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE


#endif // GUI_OBJECTS_GBENCHFEEDBACKATTACHMENT_HPP
/* Original file checksum: lines: 86, chars: 2692, CRC32: de377654 */
