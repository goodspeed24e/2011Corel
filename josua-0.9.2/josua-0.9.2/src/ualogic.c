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

#include "josua.h"
#include "rcfile.h"
/* #include <unistd.h> */ 


void
generating_answer_to_invite(transaction_t *tr, int code)
{
  generic_param_t *tag;
  ua_core_t *ua_core;
  dialog_t *dialog;
  int i;

  ua_core = ua_core_get();
  dialog = ua_core_search_dialog_as_uas(ua_core, tr->orig_request);

  if (dialog==NULL)
    {
      i = to_get_tag(tr->orig_request->to,&tag);
      if (i==0)
	{ /* a to tag exist?? --> but no dialog*/
	  /* do not have recovery capability
	     --> answer with 481 Call Leg does not exist */
	  generating_3456xx_answer_to_invite(dialog, tr, 481);
	  return;
	}
    }

  if (dialog==NULL||dialog->state==DIALOG_EARLY)
    {
      /* estabilishment of a new session: 13.2... */
      if (code<200)
	{
	  generating_1xx_answer_to_invite(dialog, tr, code);
	  return;
	}
      else if (code==200)
	{
	  generating_2xx_answer_to_invite(dialog, tr, code);
	  return;
	}
      else if (code>299)
	{
	  generating_3456xx_answer_to_invite(dialog, tr, code);
	  return;
	}
    }
  /* session already established by previous transactions */
  /* if one of those steps fail, we MUST remain in the previous state */
    
  /* 12.2.2 MUST reject with 500 if cseq does is lower than previous one */
  if (dialog->remote_cseq!=-1) /* remote_cseq already set */
    {
      int remote_cseq_from_request = satoi(tr->orig_request->cseq->number);
      if (remote_cseq_from_request<dialog->remote_cseq)
	{ 
	  generating_3456xx_answer_to_invite(dialog, tr, 500);
	  return;
	}
    }

  if (code<200)
    {
      generating_1xx_answer_to_invite(dialog, tr, code);
      return;
    }
  else if (200<=code<=299)
    {
      generating_2xx_answer_to_invite(dialog, tr, code);
      return;
    }
  else if (code>299)
    {
      generating_3456xx_answer_to_invite(dialog, tr, code);
      return;
    }
  return;
}

void
generating_answer_to_options(transaction_t *tr, int code)
{
  generic_param_t *tag;
  ua_core_t *ua_core;
  dialog_t *dialog;
  int i;

  ua_core = ua_core_get();
  dialog = ua_core_search_dialog_as_uas(ua_core, tr->orig_request);

  if (dialog==NULL)
    {
      i = to_get_tag(tr->orig_request->to,&tag);
      if (i==0)
	{ /* a to tag exist?? --> but no dialog*/
	  /* do not have recovery capability
	     --> answer with 481 Call Leg does not exist */
	  generating_3456xx_answer_to_options(dialog, tr, 481);
	  return;
	}
    }

  if (dialog==NULL||dialog->state==DIALOG_EARLY)
    {
      /* estabilishment of a new session: 13.2... */
      if (code<200)
	{
	  generating_1xx_answer_to_options(dialog, tr, code);
	  return;
	}
      else if (code==200)
	{
	  generating_2xx_answer_to_options(dialog, tr, code);
	  return;
	}
      else if (code>299)
	{
	  generating_3456xx_answer_to_options(dialog, tr, code);
	  return;
	}
    }
  /* session already established by previous transactions */
  /* if one of those steps fail, we MUST remain in the previous state */
    
  /* 12.2.2 MUST reject with 500 if cseq does is lower than previous one */
  if (dialog->remote_cseq!=-1) /* remote_cseq already set */
    {
      int remote_cseq_from_request = satoi(tr->orig_request->cseq->number);
      if (remote_cseq_from_request<dialog->remote_cseq)
	{ 
	  generating_3456xx_answer_to_options(dialog, tr, 500);
	  return;
	}
    }

  if (code<200)
    {
      generating_1xx_answer_to_options(dialog, tr, code);
      return;
    }
  else if (200<=code<=299)
    {
      generating_2xx_answer_to_options(dialog, tr, code);
      return;
    }
  else if (code>299)
    {
      generating_3456xx_answer_to_options(dialog, tr, code);
      return;
    }
  return;
}

void
generating_answer_to_bye(transaction_t *tr, int code)
{
  ua_core_t *ua_core;
  dialog_t *dialog;
  sip_t *response;
  int i;
  int rr_cseq;

  ua_core = ua_core_get();
  dialog = ua_core_search_dialog_as_uas(ua_core, tr->orig_request);

  if (dialog==NULL)
    code = 481; /* dialog does not exist */
  else
    { /* THIS MUST BE DONE FOR ALL REQUEST */
      rr_cseq = atoi(tr->orig_request->cseq->number);
      printf("remote_cseq in dialog %i\n", dialog->remote_cseq);
      printf("remote cseq in BYE request %i\n", rr_cseq);
      if (dialog->remote_cseq>=rr_cseq) code = 400; /* wrong cseq */
    }
  i = generating_response_default(&response, dialog, code, tr->orig_request);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: Could not create response for BYE\n"));
    }
  else
    {
      msg_setcontent_length(response, "0");

      /* let's destroy the dialog if any! */
      if (dialog!=NULL)
	ua_core_remove_dialog(ua_core, dialog);

      osip_ul_sendmsg(tr,response);
    }
}

void
generating_default_response(transaction_t *tr, int code)
{
  ua_core_t *ua_core;
  dialog_t *dialog;
  sip_t *response;
  int i;

  ua_core = ua_core_get();
  dialog = ua_core_search_dialog_as_uas(ua_core, tr->orig_request);

  i = generating_response_default(&response, dialog, code, tr->orig_request);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: Could not create response for current status\n"));
    }
  else
    {
      msg_setcontent_length(response, "0");
      osip_ul_sendmsg(tr,response);
    }
}


int
sip_invite(char *to, char *subject)
{
#ifdef OSIP_MT
  transaction_mt_t *transaction_mt;
#endif
  sip_t         *invite;
  transaction_t *transaction;
  int i;
  char *body;
  ua_core_t *ua_core;
  ua_core = ua_core_get();
    
  /* do something for proxy ?!*/

  body = (char *)smalloc(strlen("v=0\r\no=- 1890408335 1890408335 IN IP4 192.168.1.114\r\ns=session\r\nc=IN IP4 192.168.1.114\r\nt=3185086562 0\r\nm=audio 10060 RTP/AVP 0\r\na=rtpmap:0 PCMU/8000\r\n")+1);
  sprintf(body,"v=0\r\no=- 1890408335 1890408335 IN IP4 192.168.1.114\r\ns=session\r\nc=IN IP4 192.168.1.114\r\nt=3185086562 0\r\nm=audio 10060 RTP/AVP 0\r\na=rtpmap:0 PCMU/8000\r\n");

  i = generating_initial_invite(&invite, to, subject, "UDP", body);
  sfree(body);
  if (i!=0) return -1;
  transaction_init(&transaction,
		   ICT,
		   ua_core->config,
		   invite);
#ifdef OSIP_MT
  transaction_mt_init(&transaction_mt);
  transaction_mt_set_transaction(transaction_mt,transaction);
  transaction_mt_start_transaction(transaction_mt);
#endif

  osip_ul_sendmsg(transaction,invite);  
  
  return transaction->transactionid;
}

int
sip_bye(dialog_t *dialog)
{
  ua_core_t *ua_core;
#ifdef OSIP_MT
  transaction_mt_t *transaction_mt;
#endif
  sip_t         *bye;
  transaction_t *transaction;
  int i;

  ua_core = ua_core_get();
  i = generating_bye(&bye, dialog);
  if (i!=0) return -1;

  transaction_init(&transaction,
		   NICT,
		   ua_core->config,
		   bye);
#ifdef OSIP_MT
  transaction_mt_init(&transaction_mt);
  transaction_mt_set_transaction(transaction_mt,transaction);
  transaction_mt_start_transaction(transaction_mt);
#endif

  osip_ul_sendmsg(transaction,bye);

  /* at this point, the dialog is terminated! */
  /* removing it right now may be dangerous... An other transaction
     might be using this dialog??? Race conditions should be verified
     here... In the general case, we will receive a BYE from the remote
     UA... Will this BYE use (and delete) the dialog? */
  ua_core = ua_core_get();
  ua_core_remove_dialog(ua_core, dialog);

  return transaction->transactionid;
}

#ifdef ENABLE_DEBUG
int
sip_info(dialog_t *dialog)
{
  ua_core_t *ua_core;
#ifdef OSIP_MT
  transaction_mt_t *transaction_mt;
#endif
  sip_t         *info;
  transaction_t *transaction;
  int i;

  ua_core = ua_core_get();
  i = generating_info(&info, dialog);
  if (i!=0) return -1;

  transaction_init(&transaction,
		   NICT,
		   ua_core->config,
		   info);
#ifdef OSIP_MT
  transaction_mt_init(&transaction_mt);
  transaction_mt_set_transaction(transaction_mt,transaction);
  transaction_mt_start_transaction(transaction_mt);
#endif

  osip_ul_sendmsg(transaction,info);

  return transaction->transactionid;
}
#endif

int
sip_options(dialog_t *dialog, char *callee)
{
  ua_core_t *ua_core;
#ifdef OSIP_MT
  transaction_mt_t *transaction_mt;
#endif
  sip_t         *options;
  transaction_t *transaction;
  int i;
  char *body;

  if (dialog==NULL&&callee==NULL) {
    fprintf(stdout, "ERROR: Provide a callee url or a dialog for OPTIONS request!\n");
    return -1;
  }

  if (dialog!=NULL&&dialog->type==CALLEE)
    {
      fprintf(stdout, "SORRY: This version doesn't support OPTIONS as a callee!\n");
      return -1;
    }

  body = (char *)smalloc(strlen("v=0\r\no=- 1890408335 1890408335 IN IP4 192.168.1.114\r\ns=session\r\nc=IN IP4 192.168.1.114\r\nt=3185086562 0\r\nm=audio 10060 RTP/AVP 0\r\na=rtpmap:0 PCMU/8000\r\nm=application 10050 UDP wb\r\na=orient:portrait\r\na=recvonly\r\n")+1);
  sprintf(body,"v=0\r\no=- 1890408335 1890408335 IN IP4 192.168.1.114\r\ns=session\r\nc=IN IP4 192.168.1.114\r\nt=3185086562 0\r\nm=audio 10060 RTP/AVP 0\r\na=rtpmap:0 PCMU/8000\r\nm=application 10050 UDP wb\r\na=orient:portrait\r\na=recvonly\r\n");


  if (dialog!=NULL)
    i = generating_options_within_dialog(&options, dialog, body);
  else
    i = generating_options(&options, callee, "UDP", body);

  sfree(body);
  if (i!=0) return -1;

  ua_core = ua_core_get();
  transaction_init(&transaction,
		   NICT,
		   ua_core->config,
		   options);
#ifdef OSIP_MT
  transaction_mt_init(&transaction_mt);
  transaction_mt_set_transaction(transaction_mt,transaction);
  transaction_mt_start_transaction(transaction_mt);
#endif

  osip_ul_sendmsg(transaction,options);

  return transaction->transactionid;
}

void
sip_register()
{
  ua_core_t *ua_core;
#ifdef OSIP_MT
  transaction_mt_t *transaction_mt;
#endif
  sip_t         *sregister;
  transaction_t *transaction;
  int i;

  ua_core = ua_core_get();
  i = generating_register(&sregister, "UDP");
  if (i!=0) return;

  transaction_init(&transaction,
		   NICT,
		   ua_core->config,
		   sregister);
#ifdef OSIP_MT
  transaction_mt_init(&transaction_mt);
  transaction_mt_set_transaction(transaction_mt,transaction);
  transaction_mt_start_transaction(transaction_mt);
#endif
  osip_ul_sendmsg(transaction,sregister);

  return ;
}
