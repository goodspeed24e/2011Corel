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

/*用户代理客户机呼叫*/
#include "josua.h"
#include <osip/osip.h>

void cb_ict_kill_transaction(transaction_t *tr)
{
  static int with_ack = 0;
  static int ict_killed = 0;
  ict_killed++;
  if (tr->last_response==NULL)
    {
      with_ack++;
      fprintf(stdout, "Didn't rcv any response for this request, %i!\n", with_ack);
    }
  if (ict_killed%100==0)
    { OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,"INFO: Number of killed ICT transaction = %i\n", ict_killed)); }
#ifdef OSIP_MT
  transaction_mt_stop_transaction((transaction_mt_t *)
				  tr->your_instance);
#endif
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,"INFO: cb_ict_kill_transaction!\n"));
}

void cb_nict_kill_transaction(transaction_t *tr)
{
  static int nict_killed = 0;
  nict_killed++;
  if (nict_killed%100==0)
    { OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,"INFO: Number of killed NICT transaction = %i\n", nict_killed)); }
#ifdef OSIP_MT
  transaction_mt_stop_transaction((transaction_mt_t *)
				  tr->your_instance);
#endif
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,"INFO: cb_nict_kill_transaction!\n"));
}

void cb_transport_error(transaction_t *tr, int error)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: cb_network_error! \n"));
}

void cb_sndreq_retransmission(transaction_t *tr, sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndreq_retransmission! \n"));
}

void cb_rcvresp_retransmission(transaction_t *tr, sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvresp_retransmission! \n"));
}

void cb_sndinvite(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndinvite!\n"));
  uaapp_snd_invite(tr, sip);
}

void cb_sndack(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndack!\n"));
  uaapp_snd_ack(tr, sip);
}

void cb_sndbye(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndbye!\n"));
  uaapp_snd_bye(tr, sip);
}

void cb_sndcancel(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndcancel!\n"));
  uaapp_snd_cancel(tr, sip);
}

void cb_sndinfo(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndinfo!\n"));
  uaapp_snd_info(tr, sip);
}

void cb_sndoptions(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndoptions!\n"));
  uaapp_snd_options(tr, sip);
}

void cb_sndregister(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndregister!\n"));
  uaapp_snd_register(tr, sip);
}

void cb_sndprack(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndprack!\n"));
  /*  uaapp_snd_prack(tr, sip); */
}

void cb_sndunkrequest(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndunkrequest!\n"));
  uaapp_snd_unknown(tr, sip);
}

void cb_rcv1xx(transaction_t *tr,sip_t *sip)/*接收到1XX（暂时响应）时的判断*/
{
  dialog_t *dialog;
  ua_core_t *ua_core;
  int i;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcv1xx!\n"));

  /* create an early dialog for INVITE response建立一个用于邀请响应的先期对话 */
  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    {
      if (!MSG_TEST_CODE(sip, 100))
	{
	  ua_core = ua_core_get();
	  dialog = ua_core_search_dialog_as_uac(ua_core, sip);
	  
	  if (dialog==NULL)
	    {
	      i = dialog_init_as_uac(&dialog, sip);
	      if (i!=0) {
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
			    "ERROR: can't build dialog\n"));
	      }
	      else
		/* add this new dialog so it's known by the UA_core..加入新对话UA_core */
		ua_core_add_dialog(ua_core, dialog);
	    }
	}
      uaapp_rcv1xx_invite(tr, sip);
    }
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_rcv1xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    uaapp_rcv1xx_bye(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_rcv1xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_rcv1xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_rcv1xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_rcv1xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_rcv1xx_subscribe(tr, sip);
  else
    uaapp_rcv1xx_unknown(tr, sip);
}

void rcv2xx_for_invite(ua_core_t *ua_core, dialog_t *dialog,/*接收到2XX（成功响应）时的判断*/
		       transaction_t *tr, sip_t *sip)
{
  sip_t *ack;
  int i;
  if (dialog==NULL)
    {
      i = dialog_init_as_uac(&dialog, sip);
      if (i!=0)
	/* it does not worth to go further... */
	/* in this special case, if no other dialog
	   has been established, we'll probably refuse
	   other 2xx from other UAS? may be not.... */
	/* anyway, I got no ways to avoid the transaction
	   to be killed... May be I could just
	   change the state of the transaction...
	*/
	return;
      /* add this new dialog so it's known by the UA_core.. */
      ua_core_add_dialog(ua_core, dialog);
    }
  else
    {
      /* if a dialog already exist at this step, we MUST
	 move it to the CONFIRMED state. */
      dialog_update_route_set_as_uac(dialog, sip);
    }

  if (0==generating_ack_for_2xx(&ack, dialog))
    {
      route_t *route;
      /* ACK is sent by the UAC_core. It is not handled */
      /* by the fsm transction layer */
      
      /* TODO: here?
	 "follow the prodedure in [4] to send ACK"
      */
      /* reset the destination */
      msg_getroute(ack, 0, &route);
      if (route!=NULL)
	{
	  int port = 5060;
	  if (route->url->port!=NULL)
	    port = satoi(route->url->port);
	  ict_set_destination(tr->ict_context,
			      sgetcopy(route->url->host), port);
	}
      else
	{
	  int port = 5060;
	  if (ack->strtline->rquri->port!=NULL)
	    port = satoi(ack->strtline->rquri->port);
	  ict_set_destination(tr->ict_context,
			      sgetcopy(ack->strtline->rquri->host), port);
	}

      i = cb_udp_snd_message(tr, ack, tr->ict_context->destination,
			     tr->ict_context->port, tr->out_socket);
      if (i!=0) {
	OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: can't send ACK request\n"));
      }
      else
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndack!\n"));
	}
      msg_free(ack);
      sfree(ack);
    }
  else
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: can't create ACK request\n"));
    }
}

void cb_rcv2xx(transaction_t *tr,sip_t *sip)
{
  dialog_t *dialog;
  ua_core_t *ua_core;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcv2xx!\n"));

  ua_core = ua_core_get();
  dialog = ua_core_search_dialog_as_uac(ua_core, sip);

  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    {
      /* this method is called when the first 2xx of a call
	 is received! subsequent 2xx will be direclty sent
	 by the transport layer to the proper dialog (or
	 a new one).
	 TODO!当系统收到第一个2XX呼叫后，其后的2XX将被直接被传输层送到一个新的会话中
      */
      rcv2xx_for_invite(ua_core, dialog, tr, sip);
      uaapp_rcv2xx_invite(tr, sip);
    }
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_rcv2xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    {
      /* should be delete the dialog context here and when the BYE is sent? */
      uaapp_rcv2xx_bye(tr, sip);
    }
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_rcv2xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_rcv2xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_rcv2xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_rcv2xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_rcv2xx_subscribe(tr, sip);
  else
    uaapp_rcv2xx_unknown(tr, sip);
}

int
invite_remove_early_dialog(transaction_t *tr, sip_t *response)
{
  ua_core_t *ua_core;
  dialog_t *dialog;
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: Removing early dialog!\n"));

  ua_core = ua_core_get();
  dialog = ua_core_search_dialog_as_uac(ua_core, response);
  if (dialog!=NULL)
    {
      /* we delete the early dialog!在这里结束先期会话 */
      /* I guess it should always be an early dialog? */
      if (dialog->state==DIALOG_EARLY)
	{
	  ua_core_remove_dialog(ua_core, dialog);
	  dialog=NULL;
	}
      /* else 如果再次请求失败，怎样解决这个问题？所以应该保留会话
	 It was a re-invite that fails! How to deal with that???
	 I think, we should stay in the call
      */
    }
  return 0;
}

void cb_rcv3xx(transaction_t *tr,sip_t *sip)/*接收到3XX（重定向响应）时的判断*/
{
  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    {
      invite_remove_early_dialog(tr, sip);
      uaapp_rcv3xx_invite(tr, sip);
    }
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_rcv3xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    uaapp_rcv3xx_bye(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_rcv3xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_rcv3xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_rcv3xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_rcv3xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_rcv3xx_subscribe(tr, sip);
  else
    uaapp_rcv3xx_unknown(tr, sip);
}

void cb_rcv4xx(transaction_t *tr,sip_t *sip)/*接收到4XX（客户端出错）时的判断*/
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcv4xx!\n"));
  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    {
      invite_remove_early_dialog(tr, sip);
      uaapp_rcv4xx_invite(tr, sip);
    }
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_rcv4xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    uaapp_rcv4xx_bye(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_rcv4xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_rcv4xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_rcv4xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_rcv4xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_rcv4xx_subscribe(tr, sip);
  else
    uaapp_rcv4xx_unknown(tr, sip);
}

void cb_rcv5xx(transaction_t *tr,sip_t *sip)/*接收到5XX（服务器出错）时的判断*/
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcv5xx!\n"));
  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    {
      invite_remove_early_dialog(tr, sip);
      uaapp_rcv5xx_invite(tr, sip);
    }
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_rcv5xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    uaapp_rcv5xx_bye(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_rcv5xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_rcv5xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_rcv5xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_rcv5xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_rcv5xx_subscribe(tr, sip);
  else
    uaapp_rcv5xx_unknown(tr, sip);
}

void cb_rcv6xx(transaction_t *tr,sip_t *sip)/*接收到6XX（全局故障）时的判断*/
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcv6xx!\n"));
  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    {
      invite_remove_early_dialog(tr, sip);
      uaapp_rcv6xx_invite(tr, sip);
    }
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_rcv6xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    uaapp_rcv6xx_bye(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_rcv6xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_rcv6xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_rcv6xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_rcv6xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_rcv6xx_subscribe(tr, sip);
  else
    uaapp_rcv6xx_unknown(tr, sip);
}
