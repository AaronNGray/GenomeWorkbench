/*  $Id: insert.cpp 580779 2019-02-20 18:59:05Z saprykin $
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
 * Authors: Dmitrii Saprykin
 *
 * File Description:
 *
 * Cassandra insert named annotation task.
 *
 */

#include <ncbi_pch.hpp>

#include "insert.hpp"

#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <objtools/pubseq_gateway/impl/cassandra/cass_blob_op.hpp>
#include <objtools/pubseq_gateway/impl/cassandra/cass_driver.hpp>
#include <objtools/pubseq_gateway/impl/cassandra/IdCassScope.hpp>

BEGIN_IDBLOB_SCOPE
USING_NCBI_SCOPE;

CCassNAnnotTaskInsert::CCassNAnnotTaskInsert(
        unsigned int op_timeout_ms,
        shared_ptr<CCassConnection> conn,
        const string & keyspace,
        CBlobRecord * blob,
        CNAnnotRecord * annot,
        unsigned int max_retries,
        TDataErrorCallback data_error_cb
)
    : CCassBlobWaiter(
        op_timeout_ms, conn, keyspace, blob->GetKey(), true,
        max_retries, move(data_error_cb)
      )
    , m_Blob(blob)
    , m_Annot(annot)
{}

void CCassNAnnotTaskInsert::Wait1()
{
    bool b_need_repeat;
    do {
        b_need_repeat = false;
        switch (m_State) {
            case eError:
            case eDone:
                return;

            case eInit: {
                m_BlobInsertTask = unique_ptr<CCassBlobTaskInsertExtended>(
                     new CCassBlobTaskInsertExtended(
                         m_OpTimeoutMs, m_Conn, m_Keyspace,
                         m_Blob, true, m_MaxRetries,
                         [this]
                         (CRequestStatus::ECode status, int code, EDiagSev severity, const string & message)
                         {this->m_ErrorCb(status, code, severity, message);}
                     )
                );
                m_State = eWaitingBlobInserted;
                break;
            }

            case eWaitingBlobInserted: {
                if (m_BlobInsertTask->Wait()) {
                    if (m_BlobInsertTask->HasError()) {
                        m_State = eError;
                        m_LastError = m_BlobInsertTask->LastError();
                    } else {
                        m_State = eInsertNAnnotInfo;
                        b_need_repeat = true;
                    }
                    m_BlobInsertTask.reset();
                }
                break;
            }

            case eInsertNAnnotInfo: {
                m_QueryArr.resize(1);
                m_QueryArr[0] = { m_Conn->NewQuery(), 0};
                auto qry = m_QueryArr[0].query;
                string sql = "INSERT INTO " + GetKeySpace() + ".bioseq_na "
                      "(accession, version, seq_id_type, annot_name, sat_key, last_modified, start, stop, annot_info)"
                      "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)";
                qry->SetSQL(sql, 9);
                qry->BindStr(0, m_Annot->GetAccession());
                qry->BindInt16(1, m_Annot->GetVersion());
                qry->BindInt16(2, m_Annot->GetSeqIdType());
                qry->BindStr(3, m_Annot->GetAnnotName());
                qry->BindInt32(4, m_Annot->GetSatKey());
                qry->BindInt64(5, m_Annot->GetModified());
                qry->BindInt32(6, m_Annot->GetStart());
                qry->BindInt32(7, m_Annot->GetStop());
                qry->BindStr(8, m_Annot->GetAnnotInfo());

                UpdateLastActivity();
                qry->Execute(CASS_CONSISTENCY_LOCAL_QUORUM, m_Async);
                m_State = eWaitingNAnnotInfoInserted;
                break;
            }

            case eWaitingNAnnotInfoInserted: {
                if (!CheckReady(m_QueryArr[0])) {
                    break;
                }
                CloseAll();
                m_State = eDone;
                break;
            }

            default: {
                char msg[1024];
                snprintf(msg, sizeof(msg), "Failed to insert named annot (key=%s.%d) unexpected state (%d)",
                    m_Keyspace.c_str(), m_Key, static_cast<int>(m_State));
                Error(CRequestStatus::e502_BadGateway, CCassandraException::eQueryFailed, eDiag_Error, msg);
            }
        }
    } while (b_need_repeat);
}

END_IDBLOB_SCOPE
