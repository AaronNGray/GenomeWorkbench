#ifndef CONNECT___NCBI_LBSMD__H
#define CONNECT___NCBI_LBSMD__H

/* $Id: ncbi_lbsmd.h 590738 2019-08-03 02:57:10Z lavr $
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
 * Author:  Anton Lavrentiev
 *
 * File Description:
 *   Low-level API to resolve NCBI service name to server meta-address with
 *   the use of NCBI Load-Balancing Service Mapper (LBSMD).
 *
 */

#include "ncbi_servicep.h"
#include <connect/ncbi_heapmgr.h>


#ifdef __cplusplus
extern "C" {
#endif


const SSERV_VTable* SERV_LBSMD_Open(SERV_ITER    iter,
                                    SSERV_Info** info,
                                    HOST_INFO*   host_info,
                                    int/*bool*/  no_dispd);


/* Get configuration file name.  Returned '\0'-terminated string is to be
 * free()'d by the caller when no longer needed.  Return NULL if no config file
 * name is available.  If LBSMD_FastHeapAccess() was set to "eOff" and there is
 * a cached copy of LBSM heap kept in-core, it will be released by this call.
 */
NCBI_XCONNECT_EXPORT const char* LBSMD_GetConfig(void);


/* Get a (perhaps cached) copy of LBSM heap, which is guaranteed to be current
 * for the given time "time".  If "time" passed as 0, the heap (if present in
 * shmem) will be returned regardless of its freshness.  Return NULL if the
 * copy operation cannot be performed (due to various reasons, including the
 * original LBSM shmem to be stale).  Returned heap (if non-NULL) has a serial
 * number reflecting which shmem segment has been used to get the the snapshot,
 * negated.  The returned copy must be passed to (MT-locked by the caller)
 * HEAP_Destroy() when no longer needed.  The copy may be cached in-core, the
 * only way to release it is to call LBSMD_GetConfig() provided that
 * LBSM_FastHeapAccess() has been set to "eOff" (which is the default setting).
 */
NCBI_XCONNECT_EXPORT HEAP LBSMD_GetHeapCopy(TNCBI_Time time);


/* Get the value of a host parameter from the LBSMD host environment.  Return 0
 * if none found;  return a heap-allocated string otherwise (the caller is
 * responsible to call free() when no longer needing it).  The "host" argument
 * can be either an IP address of the host to inquire about, or SERV_LOCALHOST
 * (or 0) to get the information as defined for the current (local) host.
 */
NCBI_XCONNECT_EXPORT const char* LBSMD_GetHostParameter(unsigned int addr,
                                                        const char*  name);


/* MUST be used in the daemon code.  In client code, takes advantage of an
 * individual (usually ref-counted) heap copy per iterator -- useful in MT apps
 * using the service resolution facilities intensively (such as Apache HTTPD).
 */
NCBI_XCONNECT_EXPORT ESwitch LBSMD_FastHeapAccess(ESwitch onoff);


/* LBSMD-based helper functions for HINFO API <connect/ncbi_host_info.h> */

unsigned int LBSMD_GetLocalHostAddress(const void/*SLBSM_Version*/ *v);


int LBSM_HINFO_CpuCount(const HOST_INFO hinfo);


int LBSM_HINFO_CpuUnits(const HOST_INFO hinfo);


double LBSM_HINFO_CpuClock(const HOST_INFO hinfo);


int LBSM_HINFO_TaskCount(const HOST_INFO hinfo);


int/*bool*/ LBSM_HINFO_Memusage(const HOST_INFO hinfo,
                                double memusage[5]);


int/*bool*/ LBSM_HINFO_MachineParams(const HOST_INFO hinfo,
                                     SHINFO_Params* p);


int LBSM_HINFO_PortUsage(const HOST_INFO hinfo,
                         SHINFO_PortUsage ports[], size_t count);


int/*bool*/ LBSM_HINFO_LoadAverage(const HOST_INFO hinfo,
                                   double lavg[2]);


int/*bool*/ LBSM_HINFO_Status(const HOST_INFO hinfo,
                              double status[2]);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* CONNECT___NCBI_LBSMD__H */
