/*  josua - Jack'Open SIP User Agent is a softphone for SIP.
    Copyright (C) 2002  Aymeric MOIZARD  - jack@atosc.org

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _JOSUA_H_
#define _JOSUA_H_

#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#include <osip/port.h>
#include <osip/thread.h>
#include <osip/osip.h>
#include <osip/dialog.h>

#ifdef OSIP_MT
#include <osip/thread.h>
#endif

void cb_ict_kill_transaction(transaction_t *tr);
void cb_ist_kill_transaction(transaction_t *tr);
void cb_nict_kill_transaction(transaction_t *tr);
void cb_nist_kill_transaction(transaction_t *tr);

void cb_rcvinvite  (transaction_t *tr,sip_t *sip);
void cb_rcvack     (transaction_t *tr,sip_t *sip);
void cb_rcvack2    (transaction_t *tr,sip_t *sip);

void cb_rcvregister(transaction_t *tr,sip_t *sip);
void cb_rcvbye     (transaction_t *tr,sip_t *sip);
void cb_rcvcancel  (transaction_t *tr,sip_t *sip);
void cb_rcvinfo    (transaction_t *tr,sip_t *sip);
void cb_rcvoptions (transaction_t *tr,sip_t *sip);
void cb_rcvnotify  (transaction_t *tr,sip_t *sip);
void cb_rcvsubscribe (transaction_t *tr,sip_t *sip);
void cb_rcvunkrequest(transaction_t *tr,sip_t *sip);

void cb_sndinvite  (transaction_t *tr,sip_t *sip);
void cb_sndack     (transaction_t *tr,sip_t *sip);

void cb_sndregister(transaction_t *tr,sip_t *sip);
void cb_sndbye     (transaction_t *tr,sip_t *sip);
void cb_sndcancel  (transaction_t *tr,sip_t *sip);
void cb_sndinfo    (transaction_t *tr,sip_t *sip);
void cb_sndoptions (transaction_t *tr,sip_t *sip);
void cb_sndnotify  (transaction_t *tr,sip_t *sip);
void cb_sndsubscribe(transaction_t *tr,sip_t *sip);
void cb_sndunkrequest(transaction_t *tr,sip_t *sip);

void cb_rcv1xx(transaction_t *tr,sip_t *sip);
void cb_rcv2xx(transaction_t *tr,sip_t *sip);
void cb_rcv3xx(transaction_t *tr,sip_t *sip);
void cb_rcv4xx(transaction_t *tr,sip_t *sip);
void cb_rcv5xx(transaction_t *tr,sip_t *sip);
void cb_rcv6xx(transaction_t *tr,sip_t *sip);

void cb_snd1xx(transaction_t *tr,sip_t *sip);
void cb_snd2xx(transaction_t *tr,sip_t *sip);
void cb_snd3xx(transaction_t *tr,sip_t *sip);
void cb_snd4xx(transaction_t *tr,sip_t *sip);
void cb_snd5xx(transaction_t *tr,sip_t *sip);
void cb_snd6xx(transaction_t *tr,sip_t *sip);

void cb_rcvresp_retransmission(transaction_t *tr, sip_t *sip);
void cb_sndreq_retransmission(transaction_t *tr, sip_t *sip);
void cb_sndresp_retransmission(transaction_t *tr, sip_t *sip);
void cb_rcvreq_retransmission(transaction_t *tr, sip_t *sip);

void cb_killtransaction(transaction_t *tr);
void cb_endoftransaction(transaction_t *tr);

void cb_transport_error(transaction_t *tr, int error);

#ifdef OSIP_MT
typedef struct _transaction_mt_t {

  transaction_t *transaction;
#ifdef __VXWORKS_OS__
  int thread;
#else
  sthread_t *thread;
#endif
} transaction_mt_t;

int  transaction_mt_init(transaction_mt_t **transaction_mt);
void transaction_mt_set_transaction(transaction_mt_t *transaction_mt,
				   transaction_t *transaction);
int transaction_mt_start_transaction(transaction_mt_t *transaction_mt);
void transaction_mt_stop_transaction(transaction_mt_t *transaction_mt);
#endif

typedef struct _udp_transport_layer_t {
  int in_port;
  int in_socket;
#ifdef OSIP_MT
  sthread_t *thread;
#endif

  int out_port;
  int out_socket;

  int control_fds[2];
} udp_transport_layer_t;

/*
typedef struct _tcp_sip_t {
  int in_socket;

  int out_port;
  int out_socket;
  transaction_t *transaction;
} tcp_sip_t;
*/

typedef struct _tcp_transport_layer_t {
  int in_port;
  int in_socket;
  int control_fds[2];
} tcp_transport_layer_t;

typedef struct _audio_ctxt_t {
  int payload;
  int remote_port;
  int remote_ip;
  int local_port;
  int local_ip;
} audio_ctxt_t;

int audio_ctxt_init(audio_ctxt_t **ctx);
void audio_ctxt_free(audio_ctxt_t *ctx);

typedef struct _video_ctxt_t {
  int payload;
  int remote_port;
  int remote_ip;
  int local_port;
  int local_ip;
} video_ctxt_t;

int video_ctxt_init(video_ctxt_t **ctx);
void video_ctxt_free(video_ctxt_t *ctx);

typedef struct _application_ctxt_t {
  char *type;
  char *subtype;
  int remote_port;
  int remote_ip;
  int local_port;
  int local_ip;
} application_ctxt_t;

int application_ctxt_init(application_ctxt_t **ctx);
void application_ctxt_free(application_ctxt_t *ctx);

typedef struct _ua_core_t {

  osip_t *config;
  fifo_t *threads_to_join;

  int timers_delete;
  sthread_t *timers;
  /* protect the dialog access */
#ifdef OSIP_MT
  smutex_t *lock_dialog_access;
#endif  
  list_t *dialogs;

  list_t *audios_ctxt;
  list_t *videos_ctxt;
  list_t *applications_ctxt;

  FILE *sip_media;

  udp_transport_layer_t *udp_tl;
  tcp_transport_layer_t *tcp_tl;

  url_t *outbound_proxy;
  url_t *registrar;
  url_t *proxy;

} ua_core_t;

/* this is used to get the main instance of ua_core */
ua_core_t *ua_core_get();

int udp_transport_layer_init(udp_transport_layer_t **utl, int in_port,
			     int out_port);
void udp_transport_layer_free(udp_transport_layer_t *utl);
int udp_transport_layer_set_in_udp_port(udp_transport_layer_t *udp_tl, int port);
int udp_transport_layer_set_out_udp_port(udp_transport_layer_t *udp_tl, int port);
int udp_transport_layer_close(udp_transport_layer_t *utl);
int udp_transport_layer_stop(udp_transport_layer_t *utl);
int udp_transport_layer_send(transaction_t *tr, sip_t *sip,
			     char *host, int port, int out_socket);
int udp_transport_layer_execute(udp_transport_layer_t *utl, osip_t *osip,
				int sec_max, int usec_max, int max_analysed);

int tcp_transport_layer_init(tcp_transport_layer_t **ttl, int in_port,
			     int out_port);
void tcp_transport_layer_free(tcp_transport_layer_t *ttl);
int tcp_transport_layer_close(tcp_transport_layer_t *ttl);
int tcp_transport_layer_stop(tcp_transport_layer_t *ttl);
int tcp_transport_layer_send(transaction_t *tr, sip_t *sip,
			     char *host, int port, int out_socket);
int tcp_transport_layer_execute(tcp_transport_layer_t *ttl, osip_t *osip,
				int sec_max, int usec_max, int max_analysed);
int tcp_transport_layer_set_in_tcp_port(tcp_transport_layer_t *tcp_tl, int port);
int tcp_transport_layer_set_out_tcp_port(tcp_transport_layer_t *tcp_tl, int port);

int cb_udp_snd_message(transaction_t *tr, sip_t *sip, char *host,
		       int port, int out_socket);

int ua_core_global_init(osip_t *config);
int ua_core_init(ua_core_t **ua_core, int port, int udp_or_tcp, osip_t *config);
void ua_core_free(ua_core_t *ua_core);
int ua_core_timers_init(ua_core_t *ua_core);
int ua_core_udp_tl_start(ua_core_t *ua_core);
int ua_core_udp_tl_close(ua_core_t *ua_core);
int ua_core_udp_tl_stop(ua_core_t *ua_core);
int ua_core_udp_tl_execute(ua_core_t *ua_core,
			   int sec_max, int usec_max, int max_analysed);
int ua_core_tcp_tl_start(ua_core_t *ua_core);
int ua_core_tcp_tl_close(ua_core_t *ua_core);
int ua_core_tcp_tl_stop(ua_core_t *ua_core);
int ua_core_tcp_tl_execute(ua_core_t *ua_core,
			   int sec_max, int usec_max, int max_analysed);
void ua_core_lock_dialog_access(ua_core_t *ua_core);
void ua_core_unlock_dialog_access(ua_core_t *ua_core);
void ua_core_add_dialog(ua_core_t *ua_core, dialog_t *dialog);
int ua_core_remove_dialog(ua_core_t *ua_core, dialog_t *dialog);
dialog_t *ua_core_search_dialog_as_uac(ua_core_t *ua_core, sip_t *answer);
dialog_t *ua_core_search_dialog_as_uas(ua_core_t *ua_core, sip_t *answer);
void ua_core_set_outbound_proxy(ua_core_t *ua_core, url_t *url);
void ua_core_set_registrar(ua_core_t *ua_core, url_t *url);
void ua_core_set_proxy(ua_core_t *ua_core, url_t *url);
url_t *ua_core_get_outbound_proxy(ua_core_t *ua_core);
url_t *ua_core_get_registrar(ua_core_t *ua_core);
url_t *ua_core_get_proxy(ua_core_t *ua_core);

#ifdef OSIP_MT
/* thread managing events for one transaction */
void *transaction_thread(transaction_t *transaction);
#endif


/* Those method names SHOULD be used to initiate event requesting */
/* new transactions. This could be extended on the same model     */
/* shown in "ualogic" directory */

int    sip_invite  (char *to, char *subject);
int    sip_bye     (dialog_t *dialog);
int    sip_info    (dialog_t *dialog);
int    sip_options (dialog_t *dialog, char *callee);
void   sip_register();

/******************************************* */
/* callback procedures you MUST implement    */
/* one exemple is provided in "ua",          */
/* "ualogic" and "uamess" directories        */
/* Those methods could be used to implement  */
/* any transactions types.                   */
/******************************************* */

void uaapp_failure(transaction_t *tr, char *error);

/* these are the method's names you SHOULD use to create YOUR answers. */
/* You could extend it. */

void   uaapp_snd_invite(transaction_t *tr, sip_t *invite);
void   uaapp_snd_ack(transaction_t *tr, sip_t *ack);
void   uaapp_snd_bye(transaction_t *tr, sip_t *request);
void   uaapp_snd_register(transaction_t *tr, sip_t *request);
void   uaapp_snd_options(transaction_t *tr, sip_t *request);
void   uaapp_snd_info(transaction_t *tr, sip_t *request);
void   uaapp_snd_cancel(transaction_t *tr, sip_t *request);
void   uaapp_snd_subscribe(transaction_t *tr, sip_t *request);
void   uaapp_snd_notify(transaction_t *tr, sip_t *request);
void   uaapp_snd_unknown(transaction_t *tr, sip_t *request);

void uaapp_snd1xx_invite(transaction_t *tr, sip_t *response);
void uaapp_snd2xx_invite(transaction_t *tr, sip_t *response);
void uaapp_snd3xx_invite(transaction_t *tr, sip_t *response);
void uaapp_snd4xx_invite(transaction_t *tr, sip_t *response);
void uaapp_snd5xx_invite(transaction_t *tr, sip_t *response);
void uaapp_snd6xx_invite(transaction_t *tr, sip_t *response);

void uaapp_snd1xx_register(transaction_t *tr, sip_t *response);
void uaapp_snd2xx_register(transaction_t *tr, sip_t *response);
void uaapp_snd3xx_register(transaction_t *tr, sip_t *response);
void uaapp_snd4xx_register(transaction_t *tr, sip_t *response);
void uaapp_snd5xx_register(transaction_t *tr, sip_t *response);
void uaapp_snd6xx_register(transaction_t *tr, sip_t *response);

void uaapp_snd1xx_bye(transaction_t *tr, sip_t *response);
void uaapp_snd2xx_bye(transaction_t *tr, sip_t *response);
void uaapp_snd3xx_bye(transaction_t *tr, sip_t *response);
void uaapp_snd4xx_bye(transaction_t *tr, sip_t *response);
void uaapp_snd5xx_bye(transaction_t *tr, sip_t *response);
void uaapp_snd6xx_bye(transaction_t *tr, sip_t *response);

void uaapp_snd1xx_options(transaction_t *tr, sip_t *response);
void uaapp_snd2xx_options(transaction_t *tr, sip_t *response);
void uaapp_snd3xx_options(transaction_t *tr, sip_t *response);
void uaapp_snd4xx_options(transaction_t *tr, sip_t *response);
void uaapp_snd5xx_options(transaction_t *tr, sip_t *response);
void uaapp_snd6xx_options(transaction_t *tr, sip_t *response);

void uaapp_snd1xx_info(transaction_t *tr, sip_t *response);
void uaapp_snd2xx_info(transaction_t *tr, sip_t *response);
void uaapp_snd3xx_info(transaction_t *tr, sip_t *response);
void uaapp_snd4xx_info(transaction_t *tr, sip_t *response);
void uaapp_snd5xx_info(transaction_t *tr, sip_t *response);
void uaapp_snd6xx_info(transaction_t *tr, sip_t *response);

void uaapp_snd1xx_cancel(transaction_t *tr, sip_t *response);
void uaapp_snd2xx_cancel(transaction_t *tr, sip_t *response);
void uaapp_snd3xx_cancel(transaction_t *tr, sip_t *response);
void uaapp_snd4xx_cancel(transaction_t *tr, sip_t *response);
void uaapp_snd5xx_cancel(transaction_t *tr, sip_t *response);
void uaapp_snd6xx_cancel(transaction_t *tr, sip_t *response);

void uaapp_snd1xx_notify(transaction_t *tr, sip_t *response);
void uaapp_snd2xx_notify(transaction_t *tr, sip_t *response);
void uaapp_snd3xx_notify(transaction_t *tr, sip_t *response);
void uaapp_snd4xx_notify(transaction_t *tr, sip_t *response);
void uaapp_snd5xx_notify(transaction_t *tr, sip_t *response);
void uaapp_snd6xx_notify(transaction_t *tr, sip_t *response);

void uaapp_snd1xx_subscribe(transaction_t *tr, sip_t *response);
void uaapp_snd2xx_subscribe(transaction_t *tr, sip_t *response);
void uaapp_snd3xx_subscribe(transaction_t *tr, sip_t *response);
void uaapp_snd4xx_subscribe(transaction_t *tr, sip_t *response);
void uaapp_snd5xx_subscribe(transaction_t *tr, sip_t *response);
void uaapp_snd6xx_subscribe(transaction_t *tr, sip_t *response);

void uaapp_snd1xx_unknown(transaction_t *tr, sip_t *response);
void uaapp_snd2xx_unknown(transaction_t *tr, sip_t *response);
void uaapp_snd3xx_unknown(transaction_t *tr, sip_t *response);
void uaapp_snd4xx_unknown(transaction_t *tr, sip_t *response);
void uaapp_snd5xx_unknown(transaction_t *tr, sip_t *response);
void uaapp_snd6xx_unknown(transaction_t *tr, sip_t *response);

void   uaapp_rcv_invite(transaction_t *tr, sip_t *invite);
void   uaapp_rcv_ack(transaction_t *tr, sip_t *ack);
void   uaapp_rcv_bye(transaction_t *tr, sip_t *request);
void   uaapp_rcv_register(transaction_t *tr, sip_t *request);
void   uaapp_rcv_options(transaction_t *tr, sip_t *request);
void   uaapp_rcv_info(transaction_t *tr, sip_t *request);
void   uaapp_rcv_cancel(transaction_t *tr, sip_t *request);
void   uaapp_rcv_subscribe(transaction_t *tr, sip_t *request);
void   uaapp_rcv_notify(transaction_t *tr, sip_t *request);
void   uaapp_rcv_unknown(transaction_t *tr, sip_t *request);

void uaapp_rcv1xx_invite(transaction_t *tr, sip_t *response);
void uaapp_rcv2xx_invite(transaction_t *tr, sip_t *response);
void uaapp_rcv3xx_invite(transaction_t *tr, sip_t *response);
void uaapp_rcv4xx_invite(transaction_t *tr, sip_t *response);
void uaapp_rcv5xx_invite(transaction_t *tr, sip_t *response);
void uaapp_rcv6xx_invite(transaction_t *tr, sip_t *response);

void uaapp_rcv1xx_register(transaction_t *tr, sip_t *response);
void uaapp_rcv2xx_register(transaction_t *tr, sip_t *response);
void uaapp_rcv3xx_register(transaction_t *tr, sip_t *response);
void uaapp_rcv4xx_register(transaction_t *tr, sip_t *response);
void uaapp_rcv5xx_register(transaction_t *tr, sip_t *response);
void uaapp_rcv6xx_register(transaction_t *tr, sip_t *response);

void uaapp_rcv1xx_bye(transaction_t *tr, sip_t *response);
void uaapp_rcv2xx_bye(transaction_t *tr, sip_t *response);
void uaapp_rcv3xx_bye(transaction_t *tr, sip_t *response);
void uaapp_rcv4xx_bye(transaction_t *tr, sip_t *response);
void uaapp_rcv5xx_bye(transaction_t *tr, sip_t *response);
void uaapp_rcv6xx_bye(transaction_t *tr, sip_t *response);

void uaapp_rcv1xx_options(transaction_t *tr, sip_t *response);
void uaapp_rcv2xx_options(transaction_t *tr, sip_t *response);
void uaapp_rcv3xx_options(transaction_t *tr, sip_t *response);
void uaapp_rcv4xx_options(transaction_t *tr, sip_t *response);
void uaapp_rcv5xx_options(transaction_t *tr, sip_t *response);
void uaapp_rcv6xx_options(transaction_t *tr, sip_t *response);

void uaapp_rcv1xx_info(transaction_t *tr, sip_t *response);
void uaapp_rcv2xx_info(transaction_t *tr, sip_t *response);
void uaapp_rcv3xx_info(transaction_t *tr, sip_t *response);
void uaapp_rcv4xx_info(transaction_t *tr, sip_t *response);
void uaapp_rcv5xx_info(transaction_t *tr, sip_t *response);
void uaapp_rcv6xx_info(transaction_t *tr, sip_t *response);

void uaapp_rcv1xx_cancel(transaction_t *tr, sip_t *response);
void uaapp_rcv2xx_cancel(transaction_t *tr, sip_t *response);
void uaapp_rcv3xx_cancel(transaction_t *tr, sip_t *response);
void uaapp_rcv4xx_cancel(transaction_t *tr, sip_t *response);
void uaapp_rcv5xx_cancel(transaction_t *tr, sip_t *response);
void uaapp_rcv6xx_cancel(transaction_t *tr, sip_t *response);

void uaapp_rcv1xx_notify(transaction_t *tr, sip_t *response);
void uaapp_rcv2xx_notify(transaction_t *tr, sip_t *response);
void uaapp_rcv3xx_notify(transaction_t *tr, sip_t *response);
void uaapp_rcv4xx_notify(transaction_t *tr, sip_t *response);
void uaapp_rcv5xx_notify(transaction_t *tr, sip_t *response);
void uaapp_rcv6xx_notify(transaction_t *tr, sip_t *response);

void uaapp_rcv1xx_subscribe(transaction_t *tr, sip_t *response);
void uaapp_rcv2xx_subscribe(transaction_t *tr, sip_t *response);
void uaapp_rcv3xx_subscribe(transaction_t *tr, sip_t *response);
void uaapp_rcv4xx_subscribe(transaction_t *tr, sip_t *response);
void uaapp_rcv5xx_subscribe(transaction_t *tr, sip_t *response);
void uaapp_rcv6xx_subscribe(transaction_t *tr, sip_t *response);

void uaapp_rcv1xx_unknown(transaction_t *tr, sip_t *response);
void uaapp_rcv2xx_unknown(transaction_t *tr, sip_t *response);
void uaapp_rcv3xx_unknown(transaction_t *tr, sip_t *response);
void uaapp_rcv4xx_unknown(transaction_t *tr, sip_t *response);
void uaapp_rcv5xx_unknown(transaction_t *tr, sip_t *response);
void uaapp_rcv6xx_unknown(transaction_t *tr, sip_t *response);


char *call_id_new_random();
unsigned int via_branch_new_random();
char *from_tag_new_random();
char *to_tag_new_random();


int generating_register(sip_t **reg, char *transport);
int generating_initial_invite(sip_t **invite,
			      char *to,
			      char *subject,
			      char *transport,
			      char *sdp);
int generating_options(sip_t **options, char *to, char *transport, char *sdp);
int generating_cancel(sip_t **dest, sip_t *request_cancelled);

int generating_invite_within_dialog(sip_t **invite, dialog_t *dialog,
				    char *subject, char *sdp);
int generating_ack_for_2xx(sip_t **ack, dialog_t *dialog);
int generating_bye(sip_t **bye, dialog_t *dialog);
int generating_options_within_dialog(sip_t **options, dialog_t *dialog, char *sdp);
#ifdef ENABLE_DEBUG
int generating_info(sip_t **info, dialog_t *dialog);
#endif

int osip_ul_sendmsg(transaction_t *transaction,sip_t *msg);

void generating_default_response(transaction_t *tr, int code);
void generating_answer_to_invite(transaction_t *tr, int code);
void generating_answer_to_options(transaction_t *tr, int code);
void generating_answer_to_bye(transaction_t *tr, int code);
void generating_1xx_answer_to_invite(dialog_t *dlg, transaction_t *tr, int code);
void generating_2xx_answer_to_invite(dialog_t *dlg, transaction_t *tr, int code);
void generating_3456xx_answer_to_invite(dialog_t *dlg,transaction_t *tr, int cd);
void generating_1xx_answer_to_options(dialog_t *dlg, transaction_t *tr, int code);
void generating_2xx_answer_to_options(dialog_t *dlg, transaction_t *tr, int code);
void generating_3456xx_answer_to_options(dialog_t *dlg,transaction_t *tr, int cd);
int  generating_response_default(sip_t **dest, dialog_t *dlg, int cd, sip_t *req);
int  complete_answer_that_establish_a_dialog(sip_t *response, sip_t *request);

/*
typedef enum _session_state_t {
  RUNNING,
  ONHOLD,
  KILLED
} session_state_t;

typedef struct _sdp_session_t {
  dialog_t *dialog;

  sdp_t *sdp_local;
  sdp_t *sdp_remote;

  session_state_t state;
} sdp_session_t;

int sdp_session_init(sdp_session_t **sdp_session, dialog_t *dialog);
int sdp_session_free(sdp_session_t *sdp_session);
int sdp_session_set_local_sdp(sdp_session_t *sdp_session, sdp_t *local_sdp);
int sdp_session_set_remote_sdp(sdp_session_t *sdp_session, sdp_t *remote_sdp);
sdp_t *sdp_session_get_local_sdp(sdp_session_t *sdp_session, sdp_t *local_sdp);
sdp_t *sdp_session_get_remote_sdp(sdp_session_t *sdp_session, sdp_t *remote_sdp);

int sdp_session_build_offer_as_uac(sdp_session_t *sdp_session);
int sdp_session_build_answer_as_uas(sdp_session_t *sdp_session);

int sdp_session_match_dialog_as_uac(sdp_session_t *sdp_session, sip_t *answer);
int sdp_session_match_dialog_as_uas(sdp_session_t *sdp_session, sip_t *request);

typedef struct _sip_conference_t {
  ua_core_t *ua_core;
  list_t *sdp_sessions;
} sip_conference_t;
*/
#endif
