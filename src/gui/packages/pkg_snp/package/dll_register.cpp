/*  $Id: dll_register.cpp 42488 2019-03-08 17:00:00Z rudnev $
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
 *    Defines "SNP" package for GBENCH.
 */

#include <ncbi_pch.hpp>

#include <gui/framework/gui_package.hpp>
#include <gui/utils/extension_impl.hpp>

#include <gui/widgets/wx/ui_command.hpp>

#include <gui/packages/pkg_snp/track/snp_track.hpp>
#include <gui/packages/pkg_snp/track/snp_ds.hpp>
#include <gui/packages/pkg_snp/hapmap/hapmap_track.hpp>
#include <gui/packages/pkg_snp/hapmap/hapmap_ds.hpp>
#include <gui/packages/pkg_snp/ld/ld_track.hpp>
#include <gui/packages/pkg_snp/ld/ld_ds.hpp>
#include <gui/packages/pkg_snp/bins/bins_track.hpp>
#include <gui/packages/pkg_snp/bins/bins_ds.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// Declare the Package Object

class CSnpPackage : public IGuiPackage
{
public:
    virtual string GetName() const
    {
        return "SNP";
    }
    virtual void GetVersion(size_t& verMajor, size_t& verMinor, size_t& verPatch) const;

    virtual bool Init();
    virtual void Shut() {}

protected:
    void    x_RegisterCommands();
};


void CSnpPackage::GetVersion(size_t& verMajor, size_t& verMinor, size_t& verPatch) const
{
    verMajor = 2;
    verMinor = 0;
    verPatch = 0;
}


bool CSnpPackage::Init()
{
    CExtensionDeclaration("seqgraphic_data_source_type",//EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
                          new CSGSnpDSType());
    CExtensionDeclaration("seqgraphic_data_source_type",//EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
                          new CSGHapmapDSType());
    CExtensionDeclaration("seqgraphic_data_source_type",//EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
                          new CLDBlockDSType());
    CExtensionDeclaration("seqgraphic_data_source_type",//EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
                          new CBinsDSType());

    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
                          new CSnpTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
                          new CHapmapTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
                          new CLDBlockTrackFactory());
    CExtensionDeclaration(EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY,
                          new CBinsTrackFactory());

    x_RegisterCommands();
    return true;
}


// register commands defined in the package
void CSnpPackage::x_RegisterCommands()
{
}


///////////////////////////////////////////////////////////////////////////////
/// Declare Package Entry Point
extern "C" {
    NCBI_PACKAGEENTRYPOINT_EXPORT IGuiPackage* NCBIGBenchGetPackage()
    {
        return new CSnpPackage();
    }
}

END_NCBI_SCOPE
