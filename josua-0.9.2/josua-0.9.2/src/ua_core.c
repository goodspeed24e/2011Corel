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


#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>

#ifndef __VXWORKS_OS__
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif
#else
/* should be ok */
#include "sockLib.h" 
#include "inetLib.h" 
#endif

#include "josua.h"
#include <osip/osip.h>
#include <osip/smsg.h>
#include <osip/dialog.h>
#include <osip/port.h>

extern int control_fd;

void *
osip_timers_thread(ua_core_t *ua_core)
{
  transaction_mt_t *transaction_mt;
  int i;
  while (1)
    {
      susleep(500000);
      if (ua_core->timers_delete==1)
	return NULL;
      osip_timers_ict_execute(ua_core->config);
      osip_timers_ist_execute(ua_core->config);
      osip_timers_nict_execute(ua_core->config);
      osip_timers_nist_execute(ua_core->config);

      transaction_mt = fifo_tryget(ua_core->threads_to_join);
      while (transaction_mt!=NULL)
	{
	  /* arg is a pointer to a allocated thread... */
	  i = sthread_join(transaction_mt->thread);
	  if (i!=0) printf("ERROR: could not join thread!! (transaction: %i)\n",i);
	  else
	    {
	      transaction_free(transaction_mt->transaction);
	      sfree(transaction_mt->transaction);
	      sfree(transaction_mt->thread);
	      sfree(transaction_mt);
	    }
	  transaction_mt = fifo_tryget(ua_core->threads_to_join);
	}
      if (ua_core->timers_delete==1)
	return NULL;
    }
  return 0;
}

int
ua_core_timers_init(ua_core_t *ua_core)
{
#ifdef OSIP_MT
  ua_core->timers = sthread_create(20000,NULL,(void *(*)(void *))osip_timers_thread,(void *)ua_core);
#endif
  return 0;
}

int
ua_core_global_init(osip_t *cf)
{
  /* initialise the mutex that protects the access on */
  /* the list of transactions in osip_t structure.    */
  if (-1==osip_global_init())
    return -1; /* mutex is not initialised properly */

  osip_setcb_send_message(cf, &cb_udp_snd_message);

  osip_setcb_ict_kill_transaction(cf,&cb_ict_kill_transaction);
  osip_setcb_ist_kill_transaction(cf,&cb_ist_kill_transaction);
  osip_setcb_nict_kill_transaction(cf,&cb_nict_kill_transaction);
  osip_setcb_nist_kill_transaction(cf,&cb_nist_kill_transaction);

  osip_setcb_ict_2xx_received2(cf,&cb_rcvresp_retransmission);
  osip_setcb_ict_3456xx_received2(cf,&cb_rcvresp_retransmission);
  osip_setcb_ict_invite_sent2(cf,&cb_sndreq_retransmission);
  osip_setcb_ist_2xx_sent2(cf,&cb_sndresp_retransmission);
  osip_setcb_ist_3456xx_sent2(cf,&cb_sndresp_retransmission);
  osip_setcb_ist_invite_received2(cf,&cb_rcvreq_retransmission);
  osip_setcb_nict_2xx_received2(cf,&cb_rcvresp_retransmission);
  osip_setcb_nict_3456xx_received2(cf,&cb_rcvresp_retransmission);
  osip_setcb_nict_request_sent2(cf,&cb_sndreq_retransmission);
  osip_setcb_nist_2xx_sent2(cf,&cb_sndresp_retransmission);
  osip_setcb_nist_3456xx_sent2(cf,&cb_sndresp_retransmission);
  osip_setcb_nist_request_received2(cf,&cb_rcvreq_retransmission);
  
  /*  osip_setcb_killtransaction(cf,&cb_killtransaction);
      osip_setcb_endoftransaction(cf,&cb_endoftransaction); */
  
  osip_setcb_ict_transport_error(cf,&cb_transport_error);
  osip_setcb_ist_transport_error(cf,&cb_transport_error);
  osip_setcb_nict_transport_error(cf,&cb_transport_error);
  osip_setcb_nist_transport_error(cf,&cb_transport_error);
  
  osip_setcb_ict_invite_sent  (cf,&cb_sndinvite);
  osip_setcb_ict_ack_sent     (cf,&cb_sndack);
  osip_setcb_nict_register_sent(cf,&cb_sndregister);
  osip_setcb_nict_bye_sent     (cf,&cb_sndbye);
  osip_setcb_nict_cancel_sent  (cf,&cb_sndcancel);
  osip_setcb_nict_info_sent    (cf,&cb_sndinfo);
  osip_setcb_nict_options_sent (cf,&cb_sndoptions);
  osip_setcb_nict_subscribe_sent (cf,&cb_sndoptions);
  osip_setcb_nict_notify_sent (cf,&cb_sndoptions);
  /*  osip_setcb_nict_sndprack   (cf,&cb_sndprack); */
  osip_setcb_nict_unknown_sent(cf,&cb_sndunkrequest);

#ifndef EXTENDED_CALLBACKS

  osip_setcb_ict_1xx_received(cf,&cb_rcv1xx);
  osip_setcb_ict_2xx_received(cf,&cb_rcv2xx);
  osip_setcb_ict_3xx_received(cf,&cb_rcv3xx);
  osip_setcb_ict_4xx_received(cf,&cb_rcv4xx);
  osip_setcb_ict_5xx_received(cf,&cb_rcv5xx);
  osip_setcb_ict_6xx_received(cf,&cb_rcv6xx);
  
  osip_setcb_ist_1xx_sent(cf,&cb_snd1xx);
  osip_setcb_ist_2xx_sent(cf,&cb_snd2xx);
  osip_setcb_ist_3xx_sent(cf,&cb_snd3xx);
  osip_setcb_ist_4xx_sent(cf,&cb_snd4xx);
  osip_setcb_ist_5xx_sent(cf,&cb_snd5xx);
  osip_setcb_ist_6xx_sent(cf,&cb_snd6xx);

  osip_setcb_nict_1xx_received(cf,&cb_rcv1xx);
  osip_setcb_nict_2xx_received(cf,&cb_rcv2xx);
  osip_setcb_nict_3xx_received(cf,&cb_rcv3xx);
  osip_setcb_nict_4xx_received(cf,&cb_rcv4xx);
  osip_setcb_nict_5xx_received(cf,&cb_rcv5xx);
  osip_setcb_nict_6xx_received(cf,&cb_rcv6xx);
  
  osip_setcb_nist_1xx_sent(cf,&cb_snd1xx);
  osip_setcb_nist_2xx_sent(cf,&cb_snd2xx);
  osip_setcb_nist_3xx_sent(cf,&cb_snd3xx);
  osip_setcb_nist_4xx_sent(cf,&cb_snd4xx);
  osip_setcb_nist_5xx_sent(cf,&cb_snd5xx);
  osip_setcb_nist_6xx_sent(cf,&cb_snd6xx);

#else

  osip_setcb_rcvinvite1xx(cf,&cb_rcv1xx);
  osip_setcb_rcvinvite2xx(cf,&cb_rcv2xx);
  osip_setcb_rcvinvite3xx(cf,&cb_rcv3xx);
  osip_setcb_rcvinvite4xx(cf,&cb_rcv4xx);
  osip_setcb_rcvinvite5xx(cf,&cb_rcv5xx);
  osip_setcb_rcvinvite6xx(cf,&cb_rcv6xx);
  
  osip_setcb_sndinvite1xx(cf,&cb_snd1xx);
  osip_setcb_sndinvite2xx(cf,&cb_snd2xx);
  osip_setcb_sndinvite3xx(cf,&cb_snd3xx);
  osip_setcb_sndinvite4xx(cf,&cb_snd4xx);
  osip_setcb_sndinvite5xx(cf,&cb_snd5xx);
  osip_setcb_sndinvite6xx(cf,&cb_snd6xx);

  osip_setcb_rcvack1xx(cf,&cb_rcv1xx);
  osip_setcb_rcvack2xx(cf,&cb_rcv2xx);
  osip_setcb_rcvack3xx(cf,&cb_rcv3xx);
  osip_setcb_rcvack4xx(cf,&cb_rcv4xx);
  osip_setcb_rcvack5xx(cf,&cb_rcv5xx);
  osip_setcb_rcvack6xx(cf,&cb_rcv6xx);
  
  osip_setcb_sndack1xx(cf,&cb_snd1xx);
  osip_setcb_sndack2xx(cf,&cb_snd2xx);
  osip_setcb_sndack3xx(cf,&cb_snd3xx);
  osip_setcb_sndack4xx(cf,&cb_snd4xx);
  osip_setcb_sndack5xx(cf,&cb_snd5xx);
  osip_setcb_sndack6xx(cf,&cb_snd6xx);

  osip_setcb_rcvbye1xx(cf,&cb_rcv1xx);
  osip_setcb_rcvbye2xx(cf,&cb_rcv2xx);
  osip_setcb_rcvbye3xx(cf,&cb_rcv3xx);
  osip_setcb_rcvbye4xx(cf,&cb_rcv4xx);
  osip_setcb_rcvbye5xx(cf,&cb_rcv5xx);
  osip_setcb_rcvbye6xx(cf,&cb_rcv6xx);
  
  osip_setcb_sndbye1xx(cf,&cb_snd1xx);
  osip_setcb_sndbye2xx(cf,&cb_snd2xx);
  osip_setcb_sndbye3xx(cf,&cb_snd3xx);
  osip_setcb_sndbye4xx(cf,&cb_snd4xx);
  osip_setcb_sndbye5xx(cf,&cb_snd5xx);
  osip_setcb_sndbye6xx(cf,&cb_snd6xx);

  osip_setcb_rcvcancel1xx(cf,&cb_rcv1xx);
  osip_setcb_rcvcancel2xx(cf,&cb_rcv2xx);
  osip_setcb_rcvcancel3xx(cf,&cb_rcv3xx);
  osip_setcb_rcvcancel4xx(cf,&cb_rcv4xx);
  osip_setcb_rcvcancel5xx(cf,&cb_rcv5xx);
  osip_setcb_rcvcancel6xx(cf,&cb_rcv6xx);
  
  osip_setcb_sndcancel1xx(cf,&cb_snd1xx);
  osip_setcb_sndcancel2xx(cf,&cb_snd2xx);
  osip_setcb_sndcancel3xx(cf,&cb_snd3xx);
  osip_setcb_sndcancel4xx(cf,&cb_snd4xx);
  osip_setcb_sndcancel5xx(cf,&cb_snd5xx);
  osip_setcb_sndcancel6xx(cf,&cb_snd6xx);

  osip_setcb_rcvinfo1xx(cf,&cb_rcv1xx);
  osip_setcb_rcvinfo2xx(cf,&cb_rcv2xx);
  osip_setcb_rcvinfo3xx(cf,&cb_rcv3xx);
  osip_setcb_rcvinfo4xx(cf,&cb_rcv4xx);
  osip_setcb_rcvinfo5xx(cf,&cb_rcv5xx);
  osip_setcb_rcvinfo6xx(cf,&cb_rcv6xx);
  
  osip_setcb_sndinfo1xx(cf,&cb_snd1xx);
  osip_setcb_sndinfo2xx(cf,&cb_snd2xx);
  osip_setcb_sndinfo3xx(cf,&cb_snd3xx);
  osip_setcb_sndinfo4xx(cf,&cb_snd4xx);
  osip_setcb_sndinfo5xx(cf,&cb_snd5xx);
  osip_setcb_sndinfo6xx(cf,&cb_snd6xx);

  osip_setcb_rcvoptions1xx(cf,&cb_rcv1xx);
  osip_setcb_rcvoptions2xx(cf,&cb_rcv2xx);
  osip_setcb_rcvoptions3xx(cf,&cb_rcv3xx);
  osip_setcb_rcvoptions4xx(cf,&cb_rcv4xx);
  osip_setcb_rcvoptions5xx(cf,&cb_rcv5xx);
  osip_setcb_rcvoptions6xx(cf,&cb_rcv6xx);

  osip_setcb_sndoptions1xx(cf,&cb_snd1xx);
  osip_setcb_sndoptions2xx(cf,&cb_snd2xx);
  osip_setcb_sndoptions3xx(cf,&cb_snd3xx);
  osip_setcb_sndoptions4xx(cf,&cb_snd4xx);
  osip_setcb_sndoptions5xx(cf,&cb_snd5xx);
  osip_setcb_sndoptions6xx(cf,&cb_snd6xx);

  osip_setcb_rcvregister1xx(cf,&cb_rcv1xx);
  osip_setcb_rcvregister2xx(cf,&cb_rcv2xx);
  osip_setcb_rcvregister3xx(cf,&cb_rcv3xx);
  osip_setcb_rcvregister4xx(cf,&cb_rcv4xx);
  osip_setcb_rcvregister5xx(cf,&cb_rcv5xx);
  osip_setcb_rcvregister6xx(cf,&cb_rcv6xx);
  
  osip_setcb_sndregister1xx(cf,&cb_snd1xx);
  osip_setcb_sndregister2xx(cf,&cb_snd2xx);
  osip_setcb_sndregister3xx(cf,&cb_snd3xx);
  osip_setcb_sndregister4xx(cf,&cb_snd4xx);
  osip_setcb_sndregister5xx(cf,&cb_snd5xx);
  osip_setcb_sndregister6xx(cf,&cb_snd6xx);

  osip_setcb_rcvprack1xx(cf,&cb_rcv1xx);
  osip_setcb_rcvprack2xx(cf,&cb_rcv2xx);
  osip_setcb_rcvprack3xx(cf,&cb_rcv3xx);
  osip_setcb_rcvprack4xx(cf,&cb_rcv4xx);
  osip_setcb_rcvprack5xx(cf,&cb_rcv5xx);
  osip_setcb_rcvprack6xx(cf,&cb_rcv6xx);
  
  osip_setcb_sndprack1xx(cf,&cb_snd1xx);
  osip_setcb_sndprack2xx(cf,&cb_snd2xx);
  osip_setcb_sndprack3xx(cf,&cb_snd3xx);
  osip_setcb_sndprack4xx(cf,&cb_snd4xx);
  osip_setcb_sndprack5xx(cf,&cb_snd5xx);
  osip_setcb_sndprack6xx(cf,&cb_snd6xx);
  
  osip_setcb_rcvunkrequest1xx(cf,&cb_rcv1xx);
  osip_setcb_rcvunkrequest2xx(cf,&cb_rcv2xx);
  osip_setcb_rcvunkrequest3xx(cf,&cb_rcv3xx);
  osip_setcb_rcvunkrequest4xx(cf,&cb_rcv4xx);
  osip_setcb_rcvunkrequest5xx(cf,&cb_rcv5xx);
  osip_setcb_rcvunkrequest6xx(cf,&cb_rcv6xx);
  
  osip_setcb_sndunkrequest1xx(cf,&cb_snd1xx);
  osip_setcb_sndunkrequest2xx(cf,&cb_snd2xx);
  osip_setcb_sndunkrequest3xx(cf,&cb_snd3xx);
  osip_setcb_sndunkrequest4xx(cf,&cb_snd4xx);
  osip_setcb_sndunkrequest5xx(cf,&cb_snd5xx);
  osip_setcb_sndunkrequest6xx(cf,&cb_snd6xx);

#endif

  osip_setcb_ist_invite_received  (cf,&cb_rcvinvite);
  osip_setcb_ist_ack_received     (cf,&cb_rcvack);
  osip_setcb_ist_ack_received2    (cf,&cb_rcvack2);
  osip_setcb_nist_register_received(cf,&cb_rcvregister);
  osip_setcb_nist_bye_received     (cf,&cb_rcvbye);
  osip_setcb_nist_cancel_received  (cf,&cb_rcvcancel);
  osip_setcb_nist_info_received    (cf,&cb_rcvinfo);
  osip_setcb_nist_options_received (cf,&cb_rcvoptions);
  osip_setcb_nist_subscribe_received (cf,&cb_rcvoptions);
  osip_setcb_nist_notify_received (cf,&cb_rcvoptions);
  osip_setcb_nist_unknown_received(cf,&cb_rcvunkrequest);
  
#if defined(HAVE_PTHREAD_PTH_H)
  pth_init();
#endif
  return 0;
}

void
ua_core_set_outbound_proxy(ua_core_t *ua_core, url_t *url)
{
  ua_core->outbound_proxy = url;
}

void
ua_core_set_registrar(ua_core_t *ua_core, url_t *url)
{
  ua_core->proxy = url;
}

void
ua_core_set_proxy(ua_core_t *ua_core, url_t *url)
{
  ua_core->proxy = url;
}


url_t*
ua_core_get_outbound_proxy(ua_core_t *ua_core)
{
  return ua_core->outbound_proxy;
}

url_t*
ua_core_get_registrar(ua_core_t *ua_core)
{
  return ua_core->proxy;
}

url_t*
ua_core_get_proxy(ua_core_t *ua_core)
{
  return ua_core->proxy;
}

void
ua_core_lock_dialog_access(ua_core_t *ua_core)
{
#ifdef OSIP_MT
  smutex_lock(ua_core->lock_dialog_access);
#endif
}

void
ua_core_unlock_dialog_access(ua_core_t *ua_core)
{
#ifdef OSIP_MT
  smutex_unlock(ua_core->lock_dialog_access);
#endif
}

void
ua_core_add_dialog(ua_core_t *ua_core, dialog_t *dialog)
{
  ua_core_lock_dialog_access(ua_core);
  list_add(ua_core->dialogs, dialog, -1);
  ua_core_unlock_dialog_access(ua_core);
}

/* when can we safely remove the dialog?? */
/* TODO */
int
ua_core_remove_dialog(ua_core_t *ua_core, dialog_t *dialog)
{
  int pos=0;
  dialog_t *dlg;
  ua_core_lock_dialog_access(ua_core);
  while (!list_eol(ua_core->dialogs, pos))
    {
      dlg = (dialog_t*) list_get(ua_core->dialogs, pos);
      if (dlg==dialog)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,"INFO: Remove the dialog!\n"));
	  list_remove(ua_core->dialogs, pos);
	  dialog_free(dlg);
	  sfree(dlg);
	  ua_core_unlock_dialog_access(ua_core);
	  return 0;
	}
      dlg = (dialog_t*) list_get(ua_core->dialogs, pos);
      if (dlg==dialog)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,"INFO: Remove the dialog!\n"));
	  list_remove(ua_core->dialogs, pos);
	  dialog_free(dlg);
	  sfree(dlg);
	  ua_core_unlock_dialog_access(ua_core);
	  return 0;
	}
      pos++;
    }
  ua_core_unlock_dialog_access(ua_core);
  return -1;
}

dialog_t *
ua_core_search_dialog_as_uac(ua_core_t *ua_core, sip_t *answer)
{
  int pos=0;
  dialog_t *dlg;
  ua_core_lock_dialog_access(ua_core);
  while (!list_eol(ua_core->dialogs, pos))
    {
      dlg = (dialog_t*) list_get(ua_core->dialogs, pos);
      if (dialog_match_as_uac(dlg, answer)==0)
	{
	  ua_core_unlock_dialog_access(ua_core);
	  return dlg;
	}
      pos++;
    }
  ua_core_unlock_dialog_access(ua_core);
  return NULL;
}

dialog_t *
ua_core_search_dialog_as_uas(ua_core_t *ua_core, sip_t *request)
{
  int pos=0;
  dialog_t *dlg;
  ua_core_lock_dialog_access(ua_core);
  while (!list_eol(ua_core->dialogs, pos))
    {
      dlg = (dialog_t*) list_get(ua_core->dialogs, pos);
      if (dialog_match_as_uas(dlg, request)==0)
	{
	  ua_core_unlock_dialog_access(ua_core);
	  return dlg;
	}
      pos++;
    }
  ua_core_unlock_dialog_access(ua_core);
  return NULL;
}

/* udp_or_tcp==0 UDP
   udp_or_tcp==1 TCP   
   udp_or_tcp==? NOT_SET
*/
int
ua_core_init(ua_core_t **ua_core, int port, int udp_or_tcp, osip_t *config)
{
  (*ua_core) = (ua_core_t *)smalloc(sizeof(ua_core_t));
  if ((*ua_core)==NULL) return -1;

  (*ua_core)->timers_delete = 0; /* 0 not set, 1 set */
  (*ua_core)->timers = NULL;
  (*ua_core)->config = config;

  (*ua_core)->threads_to_join = (fifo_t *)smalloc(sizeof(fifo_t));
  if ((*ua_core)->threads_to_join==NULL) goto uai_error_0;
  fifo_init((*ua_core)->threads_to_join);

  (*ua_core)->dialogs = (list_t *)smalloc(sizeof(list_t));
  if ((*ua_core)->dialogs==NULL) goto uai_error_1;
  list_init((*ua_core)->dialogs);

#ifdef OSIP_MT
  (*ua_core)->lock_dialog_access = smutex_init();
#endif

  (*ua_core)->audios_ctxt = (list_t *)smalloc(sizeof(list_t));
  if ((*ua_core)->audios_ctxt==NULL) goto uai_error_2;
  list_init((*ua_core)->audios_ctxt);

  (*ua_core)->videos_ctxt = (list_t *)smalloc(sizeof(list_t));
  if ((*ua_core)->videos_ctxt==NULL) goto uai_error_3;
  list_init((*ua_core)->videos_ctxt);

  (*ua_core)->applications_ctxt = (list_t *)smalloc(sizeof(list_t));
  if ((*ua_core)->applications_ctxt==NULL) goto uai_error_4;
  list_init((*ua_core)->applications_ctxt);

  /*
  (*ua_core)->sip_media = fopen("/dev/osipmd","w");
  (*ua_core)->sip_media = fopen("/dev/osipmd2","w");
  */
  (*ua_core)->sip_media = NULL;

  if (udp_or_tcp==0) /* O for UDP */
    {
      udp_transport_layer_init(&((*ua_core)->udp_tl), port, port+2);
      (*ua_core)->tcp_tl = NULL;
    }
  else if (udp_or_tcp==1) /* 1 for TCP */
    {
      tcp_transport_layer_init(&((*ua_core)->tcp_tl), port, port+2);
      (*ua_core)->udp_tl = NULL;
    }
  else
    {
      (*ua_core)->udp_tl = NULL;
      (*ua_core)->tcp_tl = NULL;
    }
  
  (*ua_core)->outbound_proxy = NULL;
  (*ua_core)->registrar = NULL;
  (*ua_core)->proxy = NULL;

  return 0;

 uai_error_4:
  sfree((*ua_core)->videos_ctxt);
 uai_error_3:
  sfree((*ua_core)->audios_ctxt);
 uai_error_2:
  sfree((*ua_core)->dialogs);
 uai_error_1:
  fifo_free((*ua_core)->threads_to_join);
  sfree((*ua_core)->threads_to_join);
 uai_error_0:
  sfree(*ua_core);
  return -1;
}

void
ua_core_free(ua_core_t *ua_core)
{
  int i;

  /* Must have a way to stop the timers thread! */
  /* this is a quick hack */

  if (ua_core->timers!=NULL)
    {
      ua_core->timers_delete = 1; /* delete flag */
      i = sthread_join(ua_core->timers);
      if (i!=0) printf("ERROR: can't stop timer thread\n");
      sfree(ua_core->timers);
    }

  while (NULL!=fifo_tryget(ua_core->threads_to_join))
    {} /* hopefully already free... */

  fifo_free(ua_core->threads_to_join);
  sfree(ua_core->threads_to_join);


  list_special_free(ua_core->dialogs, (void *(*)(void *))&dialog_free);
  sfree(ua_core->dialogs);

#ifdef OSIP_MT
  smutex_destroy(ua_core->lock_dialog_access);
#ifndef __VXWORKS_OS__
  sfree(ua_core->lock_dialog_access);
#endif
#endif 

  list_special_free(ua_core->audios_ctxt, (void *(*)(void *))&audio_ctxt_free);
  sfree(ua_core->audios_ctxt);
  list_special_free(ua_core->videos_ctxt, (void *(*)(void *))&video_ctxt_free);
  sfree(ua_core->videos_ctxt);
  list_special_free(ua_core->applications_ctxt,
		    (void *(*)(void *))&application_ctxt_free);
  sfree(ua_core->applications_ctxt);

  udp_transport_layer_free(ua_core->udp_tl);
  sfree(ua_core->udp_tl);
  tcp_transport_layer_free(ua_core->tcp_tl);
  sfree(ua_core->tcp_tl);

  url_free(ua_core->outbound_proxy);
  sfree(ua_core->outbound_proxy);
  url_free(ua_core->registrar);
  sfree(ua_core->registrar);
  url_free(ua_core->proxy);
  sfree(ua_core->proxy);

  if (ua_core->sip_media!=NULL)
    fclose(ua_core->sip_media);

  osip_free(ua_core->config);
  sfree(ua_core->config);
}

void *ua_core_udp_tl_thread(ua_core_t *ua_core)
{
  int i;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
	      "INFO: New UDP listener on port %i\n" ,ua_core->udp_tl->in_port));
  i = udp_transport_layer_execute(ua_core->udp_tl, ua_core->config, -1, -1, -1);
  if (0<i)
    return NULL; /* success */
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
	      "ERROR: Old UDP listener failed on port %i, (error=%i)\n"
	      ,ua_core->udp_tl->in_port, i));
  /* free core! */
  return NULL;
}
/*
  -2 on error
  -1 on timeout
  0 on killed
  1 on max_analysed reached
*/ 
int ua_core_udp_tl_execute(ua_core_t *ua_core, int sec_max,
			   int usec_max, int max_analysed)
{
  return udp_transport_layer_execute(ua_core->udp_tl, ua_core->config,
				     sec_max, usec_max, max_analysed);
}

int ua_core_udp_tl_start(ua_core_t *ua_core)
{
#ifdef OSIP_MT
  ua_core->udp_tl->thread = sthread_create(20000,NULL,
				       (void *(*)(void *))ua_core_udp_tl_thread
					   ,(void *)ua_core);
  return 0;
#else
  return -1; /* not in mutlithread mode... */
#endif
}

int ua_core_udp_tl_close(ua_core_t *ua_core)
{
  return udp_transport_layer_close(ua_core->udp_tl);
}

int ua_core_udp_tl_stop(ua_core_t *ua_core)
{
  return udp_transport_layer_stop(ua_core->udp_tl);
}

void *ua_core_tcp_tl_thread(ua_core_t *ua_core)
{
  int i;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,
	      "INFO: New TCP listener on port %i\n" ,ua_core->tcp_tl->in_port));
  i = tcp_transport_layer_execute(ua_core->tcp_tl, ua_core->config, -1, -1, -1);
  if (0<i)
    return NULL; /* success */
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
	      "ERROR: Old TCP listener failed on port %i, (error=%i)\n"
	      ,ua_core->tcp_tl->in_port, i));
  return NULL;
}
/*
  -2 on error
  -1 on timeout
  0 on killed
  1 on max_analysed reached
*/ 
int ua_core_tcp_tl_execute(ua_core_t *ua_core, int sec_max,
			   int usec_max, int max_analysed)
{
  return tcp_transport_layer_execute(ua_core->tcp_tl, ua_core->config,
				     sec_max, usec_max, max_analysed);
}

int ua_core_tcp_tl_start(ua_core_t *ua_core)
{
#ifdef OSIP_MT
  sthread_create(20000,NULL,(void *(*)(void *))ua_core_tcp_tl_thread
		 ,(void *)ua_core);
  return 0;
#else
  return -1;
#endif
}

int ua_core_tcp_tl_close(ua_core_t *ua_core)
{
  return udp_transport_layer_close(ua_core->udp_tl);
}

int ua_core_tcp_tl_stop(ua_core_t *ua_core)
{
  return udp_transport_layer_stop(ua_core->udp_tl);
}

