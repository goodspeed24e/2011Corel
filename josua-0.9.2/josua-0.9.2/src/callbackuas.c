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

/*用户代理服务器呼叫*/
#include "josua.h"
#include <osip/osip.h>

void cb_endoftransaction(transaction_t *tr)
{
#ifdef OSIP_MT
  transaction_mt_stop_transaction((transaction_mt_t *)tr->your_instance);
#endif
}

void cb_ist_kill_transaction(transaction_t *tr)
{
  static int ist_killed = 0;
  ist_killed++;
  if (ist_killed%100==0)
    { OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,"INFO: Number of killed IST transaction = %i\n", ist_killed)); }
#ifdef OSIP_MT
  transaction_mt_stop_transaction((transaction_mt_t *)tr->your_instance);
#endif
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,"INFO: cb_ist_kill_transaction!\n"));
}

void cb_nist_kill_transaction(transaction_t *tr)
{
  static int nist_killed = 0;
  nist_killed++;
  if (nist_killed%100==0)
    { OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,"INFO: Number of killed NIST transaction = %i\n", nist_killed)); }
#ifdef OSIP_MT
  transaction_mt_stop_transaction((transaction_mt_t *)tr->your_instance);
#endif
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,"INFO: cb_nist_kill_transaction!\n"));
}

void cb_sndresp_retransmission(transaction_t *tr, sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_sndresp_retransmission!\n"));
}

void cb_rcvreq_retransmission(transaction_t *tr, sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvreq_retransmission!\n"));
}

void cb_rcvinvite(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvinvite!\n"));
  /* look for the dialog! */  
  uaapp_rcv_invite(tr, sip);
}

void cb_rcvack(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvack!\n"));
  uaapp_rcv_ack(tr, sip);
}

void cb_rcvack2(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvack2!\n"));
  /*  uaapp_annouceincrequest(tr, sip);  */
}

void cb_rcvregister(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvregister!\n"));
  uaapp_rcv_register(tr, sip);
}

void cb_rcvbye(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvbye!\n"));
  uaapp_rcv_bye(tr, sip);
}

void cb_rcvcancel(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvcancel!\n"));
  uaapp_rcv_cancel(tr, sip);
}

void cb_rcvinfo(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvinfo!\n"));
  uaapp_rcv_info(tr, sip);
}

void cb_rcvoptions(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvoptions!\n"));
  uaapp_rcv_options(tr, sip);
}

void cb_rcvprack(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvprack!\n"));
  /*  uaapp_rcv_prack(tr, sip); */
}

void cb_rcvunkrequest(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_rcvunkrequest!\n"));
  uaapp_rcv_unknown(tr, sip);
}

void cb_snd1xx(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_snd1xx!\n"));
  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    {
      uaapp_snd1xx_invite(tr, sip);
    }
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_snd1xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    uaapp_snd1xx_bye(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_snd1xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_snd1xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_snd1xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_snd1xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_snd1xx_subscribe(tr, sip);
  else
    uaapp_snd1xx_unknown(tr, sip);
}

void cb_snd2xx(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_snd2xx!\n"));
  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    uaapp_snd2xx_invite(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_snd2xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    uaapp_snd2xx_bye(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_snd2xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_snd2xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_snd2xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_snd2xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_snd2xx_subscribe(tr, sip);
  else
    uaapp_snd2xx_unknown(tr, sip);

}

void cb_snd3xx(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_snd3xx!\n"));
  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    uaapp_snd3xx_invite(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_snd3xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    uaapp_snd3xx_bye(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_snd3xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_snd3xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_snd3xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_snd3xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_snd3xx_subscribe(tr, sip);
  else
    uaapp_snd3xx_unknown(tr, sip);
}

void cb_snd4xx(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_snd4xx!\n"));
  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    uaapp_snd4xx_invite(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_snd4xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    uaapp_snd4xx_bye(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_snd4xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_snd4xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_snd4xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_snd4xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_snd4xx_subscribe(tr, sip);
  else
    uaapp_snd4xx_unknown(tr, sip);
}

void cb_snd5xx(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_snd5xx!\n"));
  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    uaapp_snd5xx_invite(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_snd5xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    uaapp_snd5xx_bye(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_snd5xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_snd5xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_snd5xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_snd5xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_snd5xx_subscribe(tr, sip);
  else
    uaapp_snd5xx_unknown(tr, sip);

}

void cb_snd6xx(transaction_t *tr,sip_t *sip)
{
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: cb_snd6xx!\n"));
  if (MSG_IS_RESPONSEFOR(sip, "INVITE"))
    uaapp_snd6xx_invite(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "REGISTER"))
    uaapp_snd6xx_register(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "BYE"))
    uaapp_snd6xx_bye(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "OPTIONS"))
    uaapp_snd6xx_options(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "INFO"))
    uaapp_snd6xx_info(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "CANCEL"))
    uaapp_snd6xx_cancel(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "NOTIFY"))
    uaapp_snd6xx_notify(tr, sip);
  else if (MSG_IS_RESPONSEFOR(sip, "SUBSCRIBE"))
    uaapp_snd6xx_subscribe(tr, sip);
  else
    uaapp_snd6xx_unknown(tr, sip);
}
