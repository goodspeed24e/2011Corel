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

#include <osip/smsg.h>
#include "rcfile.h"
#include <osip/dialog.h>
#include <osip/sdp.h>
#include <osip/sdp_negoc.h>

int
osip_ul_sendmsg(transaction_t *transaction,sip_t *msg)
{
    sipevent_t *sipevent;
    
    sipevent = osip_new_outgoing_sipmessage(msg);
    sipevent->transactionid =  transaction->transactionid;

    /* uaapp_annouceoutrequest(sipevent->sip,transaction); */
#ifdef OSIP_MT
    fifo_add(transaction->transactionff,sipevent);
#else
    transaction_execute(transaction,sipevent);
#endif

    return 0;
}

int
generating_response_default(sip_t **dest, dialog_t *dialog,
			    int status, sip_t *request)
{
  generic_param_t *tag;
  sip_t *response;
  int pos;
  int i;

  i = msg_init(&response);
  /* initialise sip_t structure */
  /* yet done... */

  response->strtline->sipversion = (char *)smalloc(8*sizeof(char));
  sprintf(response->strtline->sipversion,"SIP/2.0");
  response->strtline->statuscode = (char *)smalloc(5*sizeof(char));
  sprintf(response->strtline->statuscode,"%i",status);
  response->strtline->reasonphrase = msg_getreason(status);

  response->strtline->rquri     = NULL;
  response->strtline->sipmethod = NULL;

  i = to_clone(request->to, &(response->to));
  if (i!=0) goto grd_error_1;


  i = to_get_tag(response->to,&tag);
  if (i!=0)
    {  /* we only add a tag if it does not already contains one! */
      if ((dialog!=NULL) && (dialog->local_tag!=NULL))
	/* it should contain the local TAG we created */
	{ to_set_tag(response->to, sgetcopy(dialog->local_tag)); }
      else
	{
	  if (status!=100)
	    to_set_tag(response->to, to_tag_new_random());
	}
    }

  i = from_clone(request->from, &(response->from));
  if (i!=0) goto grd_error_1;

  pos = 0;
  while (!list_eol(request->vias,pos))
    {
      via_t *via;
      via_t *via2;
      via = (via_t *)list_get(request->vias,pos);
      i = via_clone(via, &via2);
      if (i!=-0) goto grd_error_1;
      list_add(response->vias, via2, -1);
      pos++;
    }

  i = call_id_clone(request->call_id, &(response->call_id));
  if (i!=0) goto grd_error_1;
  i = cseq_clone(request->cseq, &(response->cseq));
  if (i!=0) goto grd_error_1;
    
  *dest = response;
  return 0;

 grd_error_1:
  msg_free(response);
  sfree(response);
  return -1;
}

int
complete_answer_that_establish_a_dialog(sip_t *response, sip_t *request)
{
  int i;
  int pos=0;
  /* 12.1.1:
     copy all record-route in response
     add a contact with global scope
  */
  while (!list_eol(request->record_routes, pos))
    {
      record_route_t *rr;
      record_route_t *rr2;
      rr = list_get(request->record_routes, pos);
      i = record_route_clone(rr, &rr2);
      if (i!=0) return -1;
      list_add(response->record_routes, rr2, -1);
      pos++;
    }
  msg_setcontact(response, josua_config_get_element("global_contact"));
  return 0;
}

char *
generating_sdp_answer(sip_t *request)
{
  sdp_context_t *context;
  sdp_t *remote_sdp;
  sdp_t *local_sdp = NULL;
  int i;
  char *local_body;
  local_body = NULL;
  if (MSG_IS_INVITE(request)||MSG_IS_OPTIONS(request))
    {
      body_t *body;
      body = (body_t *)list_get(request->bodies,0);
      
      /* remote_sdp = (sdp_t *) smalloc(sizeof(sdp_t)); */
      i = sdp_init(&remote_sdp);
      if (i!=0) return NULL;
      
      /* WE ASSUME IT IS A SDP BODY AND THAT    */
      /* IT IS THE ONLY ONE, OF COURSE, THIS IS */
      /* NOT TRUE */
      i = sdp_parse(remote_sdp,body->body);
      if (i!=0) return NULL;      

      i = sdp_context_init(&context);
      if (i!=0)
	{
	  sdp_free(remote_sdp);
	  sfree(remote_sdp);
	  return NULL;
	}
      i = sdp_context_set_remote_sdp(context, remote_sdp);

      i = sdp_context_execute_negociation(context);
      if (i==200)
	{
	  local_sdp = sdp_context_get_local_sdp(context);
	  i = sdp_2char(local_sdp, &local_body);

	  sdp_context_free(context);
	  sfree(context);
	  if (i!=0) {
	    OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: Could not parse local SDP answer %i\n",i));
	    return NULL;
	  }
	  return local_body;
	}
      else if (i==415)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"WARNING: Unsupported media %i\n",i));
	}
      else
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: while building answer to SDP (%i)\n",i));
	}
      sdp_context_free(context);
      sfree(context);
    } 
  return NULL;
}

void
generating_1xx_answer_to_options(dialog_t *dialog, transaction_t *tr, int code)
{
  sip_t *response;
  int i;

  i = generating_response_default(&response, dialog, code, tr->orig_request);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: Could not create response for invite\n"));
      return;
    }

  msg_setcontent_length(response, "0");
  /*  send message to transaction layer */

  osip_ul_sendmsg(tr,response);
  return ;
}

void
generating_2xx_answer_to_options(dialog_t *dialog, transaction_t *tr, int code)
{
  ua_core_t *ua_core;
  sip_t *response;
  int i;
  char *size;
  char *body;
  ua_core = ua_core_get();
  body = generating_sdp_answer(tr->orig_request);
  if (body==NULL)
    {
      code = 488;
    }
  i = generating_response_default(&response, dialog, code, tr->orig_request);

  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"ERROR: Could not create response for options\n"));
      code = 500; /* ? which code to use? */
      sfree(body); /* not used */
      return;
    }

  if (code==488)
    {
      msg_setcontent_length(response, "0");
      /*  send message to transaction layer */
      osip_ul_sendmsg(tr,response);
      sfree(body);      
      return;
    }

  i = msg_setbody(response, body);
  if (i!=0) {
    goto g2atii_error_1;
  }
  size = (char *) smalloc(6*sizeof(char));
  sprintf(size,"%i",strlen(body));
  i = msg_setcontent_length(response, size);
  sfree(size);
  if (i!=0) goto g2atii_error_1;
  i = msg_setheader(response, "content-type", "application/sdp");
  if (i!=0) goto g2atii_error_1;

  /* response should contains the allow and supported headers */
  msg_setallow(response, "INVITE");
  msg_setallow(response, "ACK");
  msg_setallow(response, "OPTIONS");
  msg_setallow(response, "CANCEL");
  msg_setallow(response, "BYE");

  if (josua_config_get_element("supported")!=NULL)
    msg_setsupported(response, josua_config_get_element("supported"));

  sfree(body);
  osip_ul_sendmsg(tr,response);
  return ;

 g2atii_error_1:
  sfree(body);
  msg_free(response);
  sfree(response);
  return ;
}

void
generating_3456xx_answer_to_options(dialog_t *dialog, transaction_t *tr, int code)
{
  ua_core_t *ua_core;
  sip_t *response;
  int i;
  ua_core = ua_core_get();
  i = generating_response_default(&response, dialog, code, tr->orig_request);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"ERROR: Could not create response for options\n"));
      return;
    }

  if (300<=code<=399)
    {
      /* Should add contact fields */
      /* ... */
    }

  msg_setcontent_length(response, "0");
  /*  send message to transaction layer */

  osip_ul_sendmsg(tr,response);
  return ;
}

void
generating_1xx_answer_to_invite(dialog_t *dialog, transaction_t *tr, int code)
{
  sip_t *response;
  int i;
  ua_core_t *ua_core;
  ua_core = ua_core_get();

  i = generating_response_default(&response, dialog, code, tr->orig_request);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: Could not create response for invite\n"));
      return;
    }

  msg_setcontent_length(response, "0");
  /*  send message to transaction layer */

  if (code>100)
    {
      /* request that estabish a dialog: */
      /* 12.1.1 UAS Behavior */
      i = complete_answer_that_establish_a_dialog(response, tr->orig_request);
      if (i!=0) return; /* ?? */
      if (dialog==NULL)
	{
	  /* create a dialog except for 100 Trying! */
	  i = dialog_init_as_uas(&dialog, tr->orig_request, response);
	  if (i!=0)	return; /* ?? */
	  /* add this new dialog so it's known by the UA_core.. */
	  ua_core_add_dialog(ua_core, dialog);
	}
    }

  osip_ul_sendmsg(tr,response);
  return ;
}

void
generating_2xx_answer_to_invite(dialog_t *dialog, transaction_t *tr, int code)
{
  ua_core_t *ua_core;
  sip_t *response;
  int i;
  char *size;
  char *body;
  ua_core = ua_core_get();
  body = generating_sdp_answer(tr->orig_request);
  if (body==NULL)
    {
      code = 488;
    }
  i = generating_response_default(&response, dialog, code, tr->orig_request);

  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"ERROR: Could not create response for invite\n"));
      code = 500; /* ? which code to use? */
      sfree(body); /* not used */
      return;
    }

  if (code==488)
    {
      /* remove the dialog if it exists */
      if (dialog!=NULL&&dialog->state==DIALOG_EARLY)
	{
	  /* delete the early dialog! */
	  ua_core_remove_dialog(ua_core, dialog);
	}
      msg_setcontent_length(response, "0");
      /*  send message to transaction layer */
      osip_ul_sendmsg(tr,response);
      sfree(body);      
      return;
    }

  i = msg_setbody(response, body);
  if (i!=0) {
    goto g2atii_error_1;
  }
  size = (char *) smalloc(6*sizeof(char));
  sprintf(size,"%i",strlen(body));
  i = msg_setcontent_length(response, size);
  sfree(size);
  if (i!=0) goto g2atii_error_1;
  i = msg_setheader(response, "content-type", "application/sdp");
  if (i!=0) goto g2atii_error_1;

  /* request that estabish a dialog: */
  /* 12.1.1 UAS Behavior */
  i = complete_answer_that_establish_a_dialog(response, tr->orig_request);
  if (i!=0) goto g2atii_error_1;; /* ?? */

  /* response should contains the allow and supported headers */
  msg_setallow(response, "INVITE");
  msg_setallow(response, "ACK");
  msg_setallow(response, "OPTIONS");
  msg_setallow(response, "CANCEL");
  msg_setallow(response, "BYE");

  if (josua_config_get_element("supported")!=NULL)
    msg_setsupported(response, josua_config_get_element("supported"));

  sfree(body);
  /* THIS RESPONSE MUST BE SENT RELIABILY until the final ACK is received !! */
  /* this response must be stored at the upper layer!!! (it will be destroyed*/
  /* right after beeing sent! */

  if (dialog==NULL)
    {
      /* create a dialog except for 100 Trying! */
      i = dialog_init_as_uas(&dialog, tr->orig_request, response);
      if (i!=0)	goto g2atii_error_1;; /* ?? */
      /* add this new dialog so it's known by the UA_core.. */
      ua_core_add_dialog(ua_core, dialog);
    }
  else
    { /* update the dialog! */
      dialog_set_state(dialog, DIALOG_CONFIRMED);
      dialog_update_cseq_as_uas(dialog, tr->orig_request);
      dialog_update_route_set_as_uas(dialog, tr->orig_request);
    }
  osip_ul_sendmsg(tr,response);
  return ;

 g2atii_error_1:
  sfree(body);
  msg_free(response);
  sfree(response);
  return ;
}

void
generating_3456xx_answer_to_invite(dialog_t *dialog, transaction_t *tr, int code)
{
  ua_core_t *ua_core;
  sip_t *response;
  int i;
  ua_core = ua_core_get();
  i = generating_response_default(&response, dialog, code, tr->orig_request);
  if (i!=0)
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"ERROR: Could not create response for invite\n"));
      return;
    }

  if (300<=code<=399)
    {
      /* Should add contact fields */
      /* ... */
    }

  msg_setcontent_length(response, "0");
  /*  send message to transaction layer */

  if (dialog!=NULL&&dialog->state==DIALOG_EARLY)
    {
      /* delete the early dialog! */
      ua_core_remove_dialog(ua_core, dialog);
    }

  osip_ul_sendmsg(tr,response);
  return ;
}

