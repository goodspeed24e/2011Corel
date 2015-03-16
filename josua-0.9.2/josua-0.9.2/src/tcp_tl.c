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

/*TCP传输层的描述*/
int
tcp_transport_layer_init(tcp_transport_layer_t **tcp_tl,/*TCP传输层初始化*/
			 int in_port, int out_port)/*输入输出端口*/
{
  *tcp_tl = (tcp_transport_layer_t*)smalloc(sizeof(tcp_transport_layer_t));
  if (*tcp_tl==NULL) return -1;
  return 0;
#ifdef OSIP_TCP
  struct sockaddr_in  raddr;/*接口地址*/
  int option=1;

  *tcp_tl = (tcp_transport_layer_t*)smalloc(sizeof(tcp_transport_layer_t));
  if (*tcp_tl==NULL) return -1;/*TCP传输层不存在，则返回-1*/
  
  if (pipe((*tcp_tl)->control_fds)!=0)/*导入TCP传输层*/
  {
    perror("Error creating pipe");
    exit(1);
  }
  /* 在这里文件描述在哪写入堆栈控制the file descriptor where to write something to control the stack */
  
  (*tcp_tl)->in_port = in_port;

  /* 现在还不使用not used by now */
  (*tcp_tl)->out_port = out_port;

  (*tcp_tl)->out_socket = (int)socket(PF_INET, SOCK_DGRAM, IPPROTO_TCP);
  if ((*tcp_tl)->out_socket==-1)/*输出端口为-1*/
    {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
		  "ERROR: TCP sender cannot create descriptor %i!\n",
		  (*tcp_tl)->in_port));
      return -1;
    }

  (*tcp_tl)->in_socket = (int)socket(PF_INET, SOCK_DGRAM, IPPROTO_TCP);

  if ((*tcp_tl)->in_socket==-1) {/*同样输入端口为-1*/
    OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,
		"ERROR: TCP listener cannot create descriptor %i!\n",
		(*tcp_tl)->in_port));
    exit(0);
  }
  if (out_port==in_port)/*输入输出同一端口*/
    {
      (*tcp_tl)->in_socket = (*tcp_tl)->out_socket;
    }
  else
    {
      
      raddr.sin_addr.s_addr = htons(INADDR_ANY);
      raddr.sin_port = htons((short)(*tcp_tl)->in_port);
      raddr.sin_family = AF_INET;
      
      if (bind((*tcp_tl)->in_socket,/*绑定输入端口*/
	       (struct sockaddr *)&raddr, sizeof(raddr)) < 0) {
	TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: TCP listener bind failed %i!\n", (*tcp_tl)->in_port));
	exit(0);
      }
    }
  
  if (0==setsockopt((*tcp_tl)->in_socket,
		    SOL_SOCKET , SO_REUSEADDR, (void*)&option, sizeof(option)))
    {
     OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"WARNING: TCP listener SO_REUSE_ADDR failed %i!\n", (*tcp_tl)->in_port));
    }
#endif
return 0;
}

void
tcp_transport_layer_free(tcp_transport_layer_t *tcp_tl)/*释放TCP传输层*/
{
#ifdef OSIP_TCP
  if (tcp_tl->in_socket!=-1)/*关闭输入输出端口并关闭控制*/
    close(tcp_tl->in_socket);
  if (tcp_tl->out_socket!=-1)
    close(tcp_tl->out_socket);
  close(tcp_tl->control_fds[0]);
  close(tcp_tl->control_fds[1]);
#endif
}

int
tcp_transport_layer_set_in_port(tcp_transport_layer_t *tcp_tl, int port)/*设置输入端口*/
{
#ifdef OSIP_TCP
  if (tcp_tl==NULL) return -1;
  tcp_tl->in_port = port;
#endif
  return 0;
}

int
tcp_transport_layer_set_out_port(tcp_transport_layer_t *tcp_tl, int port)/*设置输出端口*/
{
#ifdef OSIP_TCP
  if (tcp_tl==NULL) return -1;
  tcp_tl->out_port = port;
#endif
  return 0;
}


int tcp_transport_layer_stop(tcp_transport_layer_t *tcp_tl)/*停止TCP传输层*/
{
#ifdef OSIP_TCP
  char a=0;
  if (tcp_tl==NULL) return -1;
  if (tcp_tl->control_fds[1])
    write(tcp_tl->control_fds[1],&a,1);
#endif
  return 0;
}

int tcp_transport_layer_close(tcp_transport_layer_t *tcp_tl)/*关闭TCP传输层*/
{
#ifdef OSIP_TCP
  char a=0;
  if (tcp_tl==NULL) return -1;
  if (tcp_tl->control_fds[1])
    write(tcp_tl->control_fds[1],&a,1);
  close(tcp_tl->in_socket);
#endif
  return 0;
}

/* max_analysed = 当本方法无返回处理消息时能够处理的最大消息数量
-1 将使这一方法始终解析
在任何情况下你都能通过调用tcp_transport_layer_close(tcp_tl);来终止这个方法
关于这个方法返回的解释：
-2 错误
-1 超时
0  调用tcp_transport_layer_close(tcp_tl);结束方法
1  达到max_analysed*/
int
tcp_transport_layer_execute(tcp_transport_layer_t *tcp_tl, osip_t *osip,/*执行TCP传输层*/
			    int sec_max, int usec_max, int max_analysed)
{
#ifdef OSIP_TCP
  fd_set osip_fdset;
  char *buf;
  int max_fd;
  struct timeval tv;

  if (tcp_tl==NULL) return -2;
  /* 等待？秒 */
  tv.tv_sec = sec_max;
  tv.tv_usec = usec_max;

  FD_ZERO(&osip_fdset);

  max_fd=tcp_tl->in_socket;
  /* 建立一系列的文件描述符来控制堆栈*/

  if (max_fd<tcp_tl->control_fds[0]) max_fd=tcp_tl->control_fds[0];
  
  FD_SET(tcp_tl->in_socket, &osip_fdset);
  FD_SET(tcp_tl->control_fds[0],&osip_fdset);

  buf = (char *)smalloc(SIP_MESSAGE_MAX_LENGTH*sizeof(char)+1);
  while(1)
    { /* SIP_MESSAGE_MAX_LENGTH应该设成66001以改善协同性能*/
      int i;
      max_analysed--;
      if ((sec_max==-1)||(usec_max==-1))
	i = select(max_fd+1, &osip_fdset, NULL, NULL, NULL);
      else
	i = select(max_fd+1, &osip_fdset, NULL, NULL, &tv);
      /* 现在不要光看tv值! */
      
      if (0==i)
	{
	  sfree(buf);
	  return -1; /* 无消息：超时 */
	}
      if (-1==i)
	{
	  sfree(buf);
	  return -2; /* 出错 */
	}
      /* 如果读入控制文件描述符，则结束线程if something to read on the control file descriptor, then exit thread*/
      if (FD_ISSET(tcp_tl->control_fds[0],&osip_fdset))
	{
	  OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"INFO: TCP thread is exiting!\n"));
	  sfree(buf); /* added by AMD 18/08/2001 */
	  return 0;
	}
      
    OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO3,NULL,"INFO: WAITING FOR TCP MESSAGE\n"));
    i = recv(tcp_tl->in_socket, buf, SIP_MESSAGE_MAX_LENGTH ,  0);
    if( i > 0 )
      {
	/* 消息可能并不是以"\0"结束，但我们必须知道接收到的char的数量Message might not end with a "\0" but we know the number of */
	/* char received! */
	transaction_t *transaction;
	sipevent_t *sipevent;
	sstrncpy(buf+i,"\0",1);
	TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO3,NULL,"INFO: RCV TCP MESSAGE\n"));
#ifdef SHOW_MESSAGE
	fprintf(stdout,"%s\n",buf));
	TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO4,NULL,"%s\n",buf));
#endif
	sipevent = osip_parse(buf);
	transaction = NULL;
	if (sipevent!=NULL)
	  transaction = osip_distribute_event(osip, sipevent);/*osip分配事件*/

#ifdef OSIP_MT
      if (transaction!=NULL)
	{
	  if (transaction->your_instance==NULL)
	    {
	      transaction_mt_t *transaction_mt;
	      transaction_mt_init(&transaction_mt);
	      transaction_mt_set_transaction(transaction_mt,transaction);
	      transaction_mt_start_transaction(transaction_mt);
	    }
	}
#endif
      if (max_analysed==0) return 1;
      }
    else
      {
      OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_ERROR,NULL,"ERROR: TCP listener failed while receiving data!\n"));
      }
    }
  sfree(buf);/*释放内存*/
#endif
  return -2;
}
