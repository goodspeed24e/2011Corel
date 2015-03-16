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

#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <osip/port.h>
#include <osip/const.h>
#include <osip/smsg.h>
#include <osip/osip.h>

#include "josua.h"

extern udp_transport_layer_t *_udp_tl;

/* send a message to host:port on UDP.从UDP端口发送信息到主机          */
int
cb_udp_snd_message(transaction_t *tr, sip_t *sip, char *host,
		   int port, int out_socket)
{
  static int num = 0;
  int i;
  char *message;
  struct hostent     *hp;
#ifdef __linux /* ok for GNU/linux */
  struct hostent     **hp_ptr;
#else
#  ifdef __sun__ /* ok for solaris (Not yet tested MACRO??) */
  struct hostent     *hp_2;
#  else 
#    ifdef __unix
  struct hostent     *hp_2;
#    endif
#  endif
#endif
  struct sockaddr_in addr;
  unsigned long int  one_inet_addr;
  int sock;
  
  hp = NULL;

#ifdef __linux /* avoid warning before replacing the gethostbyname_r */
  hp_ptr = NULL;
#endif
  /* OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,stdout
      ,"<udp_send.c> sending message to %s on port %i\n"
      ,host,port)); */
  
  i = msg_2char(sip, &message);

  if (i!=0) return -1;
#ifdef SHOW_MESSAGE
  fprintf(stdout,"%s\n",message);
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2,NULL,
	      "\n%s\n", message));
#endif

  if (host==NULL)
    {
      host = sip->strtline->rquri->host;
      if (sip->strtline->rquri->port!=NULL)
	port = satoi(sip->strtline->rquri->port);
      else
	port = 5060;
    }

  if ((int)(one_inet_addr = inet_addr(host)) == -1)
    {
      /* TODO: have to resolv, but it should not be done here! */
    }
  else
    { 
      addr.sin_addr.s_addr = one_inet_addr;
    }

  addr.sin_port        = htons((short)port);
  addr.sin_family      = AF_INET;

  if (out_socket<=0)
    sock = _udp_tl->out_socket;
  else
    sock = out_socket;

  // connect(sock,(struct sockaddr *) &addr,sizeof(addr));

  if (0  > sendto (sock, (const void*) message, strlen (message), 0,
		 (struct sockaddr *) &addr,sizeof(addr))) 
    {
#ifdef WIN32
      if (WSAECONNREFUSED==WSAGetLastError())
#else
      if (ECONNREFUSED==errno)
#endif
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO3,stdout,
		"SIP_ECONNREFUSED - No remote server.\n"));
	  /* This can be considered as an error, but for the moment,
	     I prefer that the application continue to try sending
	     message again and again... so we are not in a error case.
	     Nevertheless, this error should be announced!
	     ALSO, UAS may not have any other options than retry always
	     on the same port.
	  */
	  sfree(message);
	  return 1;
	}
      else
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO3,stdout,
		"SIP_NETWORK_ERROR - Network error.\n"));
	  /* SIP_NETWORK_ERROR; */
	  sfree(message);
	  return -1;
	}
    }
  if (strncmp(message, "INVITE", 6)==0)
    {
      num++;
      fprintf(stdout, "number of message sent: %i\n", num);
    }

  /* #endif */
  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO2, NULL,
	      "SND UDP MESSAGE.\n"));
  sfree(message);
  return 0;
}
