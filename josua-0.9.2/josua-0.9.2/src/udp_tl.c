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

#include <stdlib.h>
#include <stdio.h>

#ifndef __VXWORKS_OS__
#ifdef WIN32
#include <winsock.h>
#define close(s) closesocket(s)
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#endif
#endif

#ifdef __VXWORKS_OS__
#include "vxWorks.h" 
#include "sockLib.h" 
#include "inetLib.h" 
#endif

#include "josua.h"
#include <osip/port.h>
#include <osip/const.h>

udp_transport_layer_t *_udp_tl;/*UDP传输层*/

int
udp_transport_layer_init(udp_transport_layer_t **udp_tl,/*UDP传输层初始化*/
			 int in_port, int out_port)
{
  struct sockaddr_in  raddr;
  int option=1;

  *udp_tl = (udp_transport_layer_t*)smalloc(sizeof(udp_transport_layer_t));
  if (*udp_tl==NULL) return -1;
  
#ifndef WIN32
#ifdef __VXWORKS_OS__
  if (pipeDevCreate("/pipe/demo", 10, 100) == ERROR)
    {
      fprintf(stdout,"Error creating pipe!\n");
      exit(1);
    }
  (*udp_tl)->control_fds[0] = open("/pipe/demo", O_RDWR, 0644);
  (*udp_tl)->control_fds[1] = (*udp_tl)->control_fds[0];
#else                                         
  if (pipe((*udp_tl)->control_fds)!=0)
    {
      perror("Error creating pipe");
      exit(1);
    }
#endif
#endif

  /* 文件描述哪里写入控制堆栈the file descriptor where to write something to control the stack */
  
  (*udp_tl)->in_port = in_port;

  /* 现在不使用not used by now */
  (*udp_tl)->out_port = out_port;

  (*udp_tl)->in_socket = (int)socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  
  if ((*udp_tl)->in_socket==-1) {
    OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
			  "ERROR: UDP listener cannot create descriptor %i!\n",
			  (*udp_tl)->in_port));
    exit(0);
  }
  
  raddr.sin_addr.s_addr = htons(INADDR_ANY);
  raddr.sin_port = htons((short)(*udp_tl)->in_port);
  raddr.sin_family = AF_INET;
  
  if (bind((*udp_tl)->in_socket,/*绑定接口*/
	   (struct sockaddr *)&raddr, sizeof(raddr)) < 0) {
    OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: UDP listener bind failed %i!\n", (*udp_tl)->in_port));
    exit(0);
  }
  
  if (out_port==in_port)
    {
      (*udp_tl)->in_socket = (*udp_tl)->out_socket;
    }
  else
    {
      (*udp_tl)->out_socket = (int)socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if ((*udp_tl)->out_socket==-1)
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
				"ERROR: UDP sender cannot create descriptor %i!\n",
				(*udp_tl)->out_port));
	  return -1;
	}
      raddr.sin_addr.s_addr = htons(INADDR_ANY);
      raddr.sin_port = htons((short)(*udp_tl)->out_port);
      raddr.sin_family = AF_INET;
      
      if (bind((*udp_tl)->out_socket,
	       (struct sockaddr *)&raddr, sizeof(raddr)) < 0) {
	OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: UDP listener bind failed %i!\n", (*udp_tl)->out_port));
	exit(0);
      }
    }
  
  if (0!=setsockopt((*udp_tl)->in_socket,/*设置接口*/
		    SOL_SOCKET , SO_REUSEADDR, (void*)&option, sizeof(option)))
    {
     OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"WARNING: UDP listener SO_REUSE_ADDR failed %i!\n", (*udp_tl)->in_port));
    }
  _udp_tl = *udp_tl;
return 0;
}

void
udp_transport_layer_free(udp_transport_layer_t *udp_tl)/*释放UDP传输层*/
{
  sfree(udp_tl->thread);
  if (udp_tl->in_socket!=-1)
    close(udp_tl->in_socket);
  if (udp_tl->in_port==udp_tl->out_port)
    udp_tl->out_socket=-1;
  else if (udp_tl->out_socket!=-1)
    close(udp_tl->out_socket);
#ifndef WIN32
  close(udp_tl->control_fds[0]);
  close(udp_tl->control_fds[1]);
#endif
}

int
udp_transport_layer_set_in_port(udp_transport_layer_t *udp_tl, int port)/*设置UDP传输层输入端口*/
{
  if (udp_tl==NULL) return -1;
  udp_tl->in_port = port;
  return 0;
}

int
udp_transport_layer_set_out_port(udp_transport_layer_t *udp_tl, int port)/*设置UDP传输层输出端口*/
{
  if (udp_tl==NULL) return -1;
  udp_tl->out_port = port;
  return 0;
}

int udp_transport_layer_stop(udp_transport_layer_t *udp_tl)/*停止UDP传输层*/
{
  char a=0;
  if (udp_tl==NULL) return -1;
#ifndef WIN32
  if (udp_tl->control_fds[1])
    write(udp_tl->control_fds[1],&a,1);
#endif
  return 0;
}

int udp_transport_layer_close(udp_transport_layer_t *udp_tl)/*关闭UDP传输层*/
{
  char a=0;
  if (udp_tl==NULL) return -1;
#ifndef WIN32
  if (udp_tl->control_fds[1])
    write(udp_tl->control_fds[1],&a,1);
#endif
  close(udp_tl->in_socket);
  udp_tl->in_socket=-1;
  if (udp_tl->in_port==udp_tl->out_port)
    udp_tl->out_socket=-1;
  return 0;
}

/* max_analysed = maximum number of message when this method can parse
   messages whothout returning.
   -1 will make this method parse for ever.

   In any cases, you can stop this method with
   a call to udp_transport_layer_close(udp_tl);

   This method returns:
   -2 on error
   -1 on timeout
   0 on killed with udp_transport_layer_close(udp_tl);
   1 on max_analysed reached
*/
int
udp_transport_layer_execute(udp_transport_layer_t *udp_tl, osip_t *osip,
			    int sec_max, int usec_max, int max_analysed)
{
  fd_set memo_fdset;
  fd_set osip_fdset;
  char *buf;
  int max_fd;
  struct timeval tv;
  struct sockaddr_in sa;

#ifdef __linux
  socklen_t slen;
#else
  int slen;
#endif

  if (udp_tl==NULL) return -2;
  /* 等待？秒Wait up to ? seconds. */
  tv.tv_sec = sec_max;
  tv.tv_usec = usec_max;

  FD_ZERO(&memo_fdset);
#ifndef WIN32
  FD_SET(udp_tl->control_fds[0],&memo_fdset);
#endif
  FD_SET(udp_tl->in_socket, &memo_fdset);
  FD_SET(udp_tl->out_socket, &memo_fdset);

  max_fd=udp_tl->in_socket;
  if (max_fd<udp_tl->out_socket)
    max_fd=udp_tl->out_socket;
#ifndef WIN32
  if (max_fd<udp_tl->control_fds[0]) max_fd=udp_tl->control_fds[0];
#endif  

  buf = (char *)smalloc(SIP_MESSAGE_MAX_LENGTH*sizeof(char)+1);
  while(1)
    { /* SIP_MESSAGE_MAX_LENGTH必须设为66001以改善协作性能 should be set to 66001
	 to improve interopperability */
      int i;
      max_analysed--;
      osip_fdset = memo_fdset;

      max_fd=udp_tl->out_socket;
      if (max_fd<udp_tl->in_socket)
	max_fd=udp_tl->in_socket;
#ifndef WIN32
      if (max_fd<udp_tl->control_fds[0]) max_fd=udp_tl->control_fds[0];
#endif  

      if ((sec_max==-1)||(usec_max==-1))
	i = select(max_fd+1, &osip_fdset, NULL, NULL, NULL);
      else
	i = select(max_fd+1, &osip_fdset, NULL, NULL, &tv);
      /* 不要光看tv值Don't rely on the value of tv now! */
      
      if (0==i)
	{
	  sfree(buf);
	  return -1; /* 无消息：超时no message: timout expires */
	}
      if (-1==i)
	{
	  sfree(buf);
	  return -2; /* error */
	}
      i = 0;
      slen = sizeof (sa);
#ifndef WIN32
      /* 如果读入文件描述符，则退出线程if something to read on the control file descriptor, then exit thread*/
      if (FD_ISSET(udp_tl->control_fds[0],&osip_fdset))
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"Exiting thread!\n"));
	  sfree(buf); /* added by AMD 18/08/2001 */
	  return 0;
	}
      else
	
#endif
	if (FD_ISSET(udp_tl->out_socket,&osip_fdset))
	  {
	    OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"WAITING FOR UDP MESSAGE\n"));
	    
	    i = recvfrom(udp_tl->out_socket, buf, SIP_MESSAGE_MAX_LENGTH ,  0,
			 (struct sockaddr *) &sa, &slen);
	  }
	else if (FD_ISSET(udp_tl->in_socket,&osip_fdset))
	  {
	    OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"WAITING FOR UDP MESSAGE\n"));
	    
	    i = recvfrom(udp_tl->in_socket, buf, SIP_MESSAGE_MAX_LENGTH ,  0,
			 (struct sockaddr *) &sa, &slen);
	  }
      
    if( i > 0 )
      {
	/* 消息可能不以"\0"结束，但我们必须知道接收到的char的数量Message might not end with a "\0" but we know the number of */
	/* char received! */
	transaction_t *transaction = NULL;
	sipevent_t *sipevent;
	sstrncpy(buf+i,"\0",1);
	OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO3,
			      NULL,"RCV MESSAGE from %s:%i\n",
			      inet_ntoa (sa.sin_addr), sa.sin_port));
#ifdef SHOW_MESSAGE
	OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO3,NULL,"\n%s\n",buf));
	fprintf(stdout, "\n%s\n",buf);
#endif
	sipevent = osip_parse(buf);
	transaction = NULL;
	if (sipevent!=NULL&&sipevent->sip!=NULL)
	  {
	    i = osip_find_transaction_and_add_event(osip, sipevent);
	    if (i!=0)
	      { /* 找不到事务can't find a transaction! */
		dialog_t *dialog;
		ua_core_t *ua_core;
		dialog=NULL;
		if (MSG_IS_INVITE(sipevent->sip))
		  { /* 可能发了200个OK信息后我们才接收到INVITE It could be a late INVITE coming a long time after a 200 OK
		       has been sent! */
		    ua_core = ua_core_get();
		    dialog = ua_core_search_dialog_as_uas(ua_core, sipevent->sip);
		  }
		/* 我们必须检查CSeq以发现别的请求。但它们必须在同一个会话中we should also check the CSeq for other request. But as
		   they don't initiate dialogs, we'll only detect those
		   that are within a dialog!
		*/
		if (dialog!=NULL &&
		    dialog->remote_cseq == satoi(sipevent->sip->cseq->number))
		  { /* 这个请求已经有了一个对应的会话a dialog already exist for this request! */
		    /* 丢弃转发的INVITE discard this INVITE retransmission */
		    printf("Discard this very late INVITE retransmission!\n");
		    msg_free(sipevent->sip);
		    sfree(sipevent->sip);
		    sfree(sipevent);
		  }
		else if (MSG_IS_REQUEST(sipevent->sip)
		    && MSG_IS_ACK(sipevent->sip))
		  { /* ACK200用于INVITE邀请 ACK for 200 for INVITE! */
		    static int number_of_ack;
		    number_of_ack++;
		    printf("TODO: ACK for a 200 INVITE (%i)?\n", number_of_ack);
		    msg_free(sipevent->sip);
		    sfree(sipevent->sip);
		    sfree(sipevent);
		  }
		else if (MSG_IS_RESPONSE(sipevent->sip)
			 && MSG_IS_STATUS_2XX(sipevent->sip))
		  {
		    printf("TODO: RETRANSMISSION OF A 200 for INVITE?\n");
		    msg_free(sipevent->sip);
		    sfree(sipevent->sip);
		    sfree(sipevent);
		  }
		else if (MSG_IS_RESPONSE(sipevent->sip))
		  {
		    printf("ERROR: RETRANSMISSION OF A RESPONSE>200?\n");
		    msg_free(sipevent->sip);
		    sfree(sipevent->sip);
		    sfree(sipevent);
		  }
		else
		  {
		    transaction = osip_create_transaction(osip, sipevent);
#ifdef OSIP_MT
		    if (transaction!=NULL)
		      {
			if (transaction->your_instance==NULL)
			  {
			    transaction_mt_t *transaction_mt;
			    transaction_mt_init(&transaction_mt);
			    transaction_mt_set_transaction(transaction_mt,transaction);
			    fifo_add(transaction->transactionff, sipevent);
			    transaction_mt_start_transaction(transaction_mt);
			  }
		      }
		    else
		      {
			msg_free(sipevent->sip);
			sfree(sipevent->sip);
			sfree(sipevent);
		      }
#endif
		  }
	      }

	  }
      if (max_analysed==0) return 1;
      }
    else
      {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: UDP listener failed while receiving data!\n"));
      }
    }
  sfree(buf);
  return -2;
}
