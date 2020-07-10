#ifndef DBAPI_DRIVER_FTDS100_IMPL___RENAME_FTDS_CTLIB__H
#define DBAPI_DRIVER_FTDS100_IMPL___RENAME_FTDS_CTLIB__H

/*  $Id: rename_ftds_ctlib.h 554193 2017-12-28 19:00:15Z ucko $
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
 * Authors:  Sergey Sikorskiy, Aaron Ucko
 *
 * File Description:
 *   Macros to rename FreeTDS CTLIB symbols -- to avoid their clashing
 *   with the Sybase CTLIB ones
 *
 */


#if defined(NCBI_DBAPI_RENAME_CTLIB)

#  define blk_alloc         blk_alloc_ver100
#  define blk_bind          blk_bind_ver100
#  define blk_colval        blk_colval_ver100
#  define blk_default       blk_default_ver100
#  define blk_describe      blk_describe_ver100
#  define blk_done          blk_done_ver100
#  define blk_drop          blk_drop_ver100
#  define blk_getrow        blk_getrow_ver100
#  define blk_gettext       blk_gettext_ver100
#  define blk_init          blk_init_ver100
#  define blk_props         blk_props_ver100
#  define blk_rowalloc      blk_rowalloc_ver100
#  define blk_rowdrop       blk_rowdrop_ver100
#  define blk_rowxfer       blk_rowxfer_ver100
#  define blk_rowxfer_mult  blk_rowxfer_mult_ver100
#  define blk_sendrow       blk_sendrow_ver100
#  define blk_sendtext      blk_sendtext_ver100
#  define blk_sethints      blk_sethints_ver100
#  define blk_srvinit       blk_srvinit_ver100
#  define blk_textxfer      blk_textxfer_ver100

#  define cs_calc           cs_calc_ver100
#  define cs_cmp            cs_cmp_ver100
#  define cs_config         cs_config_ver100
#  define cs_conv_mult      cs_conv_mult_ver100
#  define cs_convert        cs_convert_ver100
#  define cs_ctx_alloc      cs_ctx_alloc_ver100
#  define cs_ctx_drop       cs_ctx_drop_ver100
#  define cs_ctx_global     cs_ctx_global_ver100
#  define cs_diag           cs_diag_ver100
#  define cs_dt_crack       cs_dt_crack_ver100
#  define cs_dt_crack_v2    cs_dt_crack_v2_ver100
#  define cs_dt_info        cs_dt_info_ver100
#  define cs_loc_alloc      cs_loc_alloc_ver100
#  define cs_loc_drop       cs_loc_drop_ver100
#  define cs_locale         cs_locale_ver100
#  define cs_manage_convert cs_manage_convert_ver100
#  define cs_objects        cs_objects_ver100
#  define cs_prretcode      cs_prretcode_ver100
#  define cs_set_convert    cs_set_convert_ver100
#  define cs_setnull        cs_setnull_ver100
#  define cs_strbuild       cs_strbuild_ver100
#  define cs_strcmp         cs_strcmp_ver100
#  define cs_time           cs_time_ver100
#  define cs_will_convert   cs_will_convert_ver100

#  define ct_bind           ct_bind_ver100
#  define ct_callback       ct_callback_ver100
#  define ct_cancel         ct_cancel_ver100
#  define ct_capability     ct_capability_ver100
#  define ct_close          ct_close_ver100
#  define ct_cmd_alloc      ct_cmd_alloc_ver100
#  define ct_cmd_drop       ct_cmd_drop_ver100
#  define ct_cmd_props      ct_cmd_props_ver100
#  define ct_command        ct_command_ver100
#  define ct_compute_info   ct_compute_info_ver100
#  define ct_con_alloc      ct_con_alloc_ver100
#  define ct_con_drop       ct_con_drop_ver100
#  define ct_con_props      ct_con_props_ver100
#  define ct_config         ct_config_ver100
#  define ct_connect        ct_connect_ver100
#  define ct_cursor         ct_cursor_ver100
#  define ct_data_info      ct_data_info_ver100
#  define ct_describe       ct_describe_ver100
#  define ct_diag           ct_diag_ver100
#  define ct_dynamic        ct_dynamic_ver100
#  define ct_exit           ct_exit_ver100
#  define ct_fetch          ct_fetch_ver100
#  define ct_get_data       ct_get_data_ver100
#  define ct_init           ct_init_ver100
#  define ct_options        ct_options_ver100
#  define ct_param          ct_param_ver100
#  define ct_poll           ct_poll_ver100
#  define ct_res_info       ct_res_info_ver100
#  define ct_results        ct_results_ver100
#  define ct_send           ct_send_ver100
#  define ct_send_data      ct_send_data_ver100
#  define ct_setparam       ct_setparam_ver100

#  define _cs_convert_ex            _cs_convert_ex_ver100
#  define _cs_locale_copy           _cs_locale_copy_ver100
#  define _cs_locale_copy_inplace   _cs_locale_copy_inplace_ver100
#  define _cs_locale_free           _cs_locale_free_ver100

#  define _csclient_msg             _csclient_msg_ver100
#  define _ctclient_msg             _ctclient_msg_ver100
#  define _ct_bind_data             _ct_bind_data_ver100
#  define _ct_diag_clearmsg         _ct_diag_clearmsg_ver100
#  define _ct_get_client_type       _ct_get_client_type_ver100
#  define _ct_get_server_type       _ct_get_server_type_ver100
#  define _ct_handle_client_message _ct_handle_client_message_ver100
#  define _ct_handle_server_message _ct_handle_server_message_ver100

/* Also cover type names that appear as template parameters */
#  define _cs_datafmt _cs_datafmt_ver100
#  define _cs_varchar _cs_varchar_ver100

#endif  /* NCBI_DBAPI_RENAME_CTLIB */


#endif  /* DBAPI_DRIVER_FTDS100_IMPL___RENAME_FTDS_CTLIB__H */
