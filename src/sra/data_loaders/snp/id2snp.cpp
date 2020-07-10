/*  $Id: id2snp.cpp 529989 2017-03-09 16:52:38Z vasilche $
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
 * Authors:  Eugene Vasilchenko
 *
 * File Description:
 *   Processor of ID2 requests for SNP data
 *
 */

#include <ncbi_pch.hpp>
#include <sra/data_loaders/snp/id2snp.hpp>
#include <sra/data_loaders/snp/impl/id2snp_impl.hpp>
#include <corelib/plugin_manager_store.hpp>
#include <corelib/plugin_manager_impl.hpp>
#include <objects/id2/ID2_Request_Packet.hpp>
#include <objects/id2/ID2_Reply.hpp>
#include <objects/id2/id2processor_interface.hpp>
#include <sra/data_loaders/snp/id2snp_entry.hpp>
#include <sra/data_loaders/snp/id2snp_params.h>

BEGIN_NCBI_NAMESPACE;
BEGIN_NAMESPACE(objects);


/////////////////////////////////////////////////////////////////////////////
// CID2SNPProcessor
/////////////////////////////////////////////////////////////////////////////


CID2SNPProcessor::CID2SNPProcessor(void)
    : m_Impl(new CID2SNPProcessor_Impl)
{
}


CID2SNPProcessor::CID2SNPProcessor(const CConfig::TParamTree* params,
                                   const string& driver_name)
    : m_Impl(new CID2SNPProcessor_Impl(params, driver_name))
{
}


CID2SNPProcessor::~CID2SNPProcessor(void)
{
}


/////////////////////////////////////////////////////////////////////////////
// new interface


CRef<CID2ProcessorContext>
CID2SNPProcessor::CreateContext(void)
{
    return CRef<CID2ProcessorContext>(m_Impl->CreateContext());
}


CRef<CID2ProcessorPacketContext>
CID2SNPProcessor::ProcessPacket(CID2ProcessorContext* context,
                                CID2_Request_Packet& packet,
                                TReplies& replies)
{
    return CRef<CID2ProcessorPacketContext>(
        m_Impl->ProcessPacket(dynamic_cast<CID2SNPProcessorContext*>(context),
                              packet,
                              replies));
}


void CID2SNPProcessor::ProcessReply(CID2ProcessorContext* context,
                                    CID2ProcessorPacketContext* packet_context,
                                    CID2_Reply& reply,
                                    TReplies& replies)
{
    m_Impl->ProcessReply(dynamic_cast<CID2SNPProcessorContext*>(context),
                         dynamic_cast<CID2SNPProcessorPacketContext*>(packet_context),
                         reply,
                         replies);
}


// end of new interface
/////////////////////////////////////////////////////////////////////////////


END_NAMESPACE(objects);

void ID2Processors_Register_SNP(void)
{
    RegisterEntryPoint<objects::CID2Processor>(NCBI_EntryPoint_id2proc_snp);
}


/// Class factory for ID1 reader
///
/// @internal
///
class CID2SNPProcessorCF : 
    public CSimpleClassFactoryImpl<objects::CID2Processor,
                                   objects::CID2SNPProcessor>
{
public:
    typedef CSimpleClassFactoryImpl<objects::CID2Processor,
                                    objects::CID2SNPProcessor> TParent;
public:
    CID2SNPProcessorCF()
        : TParent(NCBI_ID2PROC_SNP_DRIVER_NAME, 0)
        {
        }

    objects::CID2Processor* 
    CreateInstance(const string& driver  = kEmptyStr,
                   CVersionInfo version =
                   NCBI_INTERFACE_VERSION(objects::CID2Processor),
                   const TPluginManagerParamTree* params = 0) const
    {
        objects::CID2Processor* drv = 0;
        if ( !driver.empty()  &&  driver != m_DriverName ) {
            return 0;
        }
        if (version.Match(NCBI_INTERFACE_VERSION(objects::CID2Processor)) 
                            != CVersionInfo::eNonCompatible) {
            drv = new objects::CID2SNPProcessor(params, driver);
        }
        return drv;
    }
};


void NCBI_EntryPoint_id2proc_snp(
     CPluginManager<objects::CID2Processor>::TDriverInfoList&   info_list,
     CPluginManager<objects::CID2Processor>::EEntryPointRequest method)
{
    CHostEntryPointImpl<CID2SNPProcessorCF>::NCBI_EntryPointImpl(info_list, method);
}


END_NCBI_NAMESPACE;
