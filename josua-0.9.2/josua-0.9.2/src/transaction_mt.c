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

#ifdef OSIP_MT

void *
transaction_thread(transaction_t *transaction)
{
  sipevent_t *se;
#ifdef __VXWORKS_OS__
  taskSafe(); /* prevent from deletion */
#endif
  while (1)
    {
      se = (sipevent_t *)fifo_get(transaction->transactionff);/*SIP事件*/
      if (se==NULL)
	sthread_exit();
      if (transaction_execute(transaction,se)<1) /* 事务执行deletion asked */
	{
#ifdef __VXWORKS_OS__
	  taskUnsafe(); /* 安全删除线程thread is safe for deletion */
#endif
	  sthread_exit();
	}
  }
}


int
transaction_mt_init(transaction_mt_t **transaction_mt)/*初始化事务*/
{
  *transaction_mt = (transaction_mt_t *)smalloc(sizeof(transaction_mt_t));
  if (*transaction_mt==NULL)
    return -1;
  return 0;
}


void
transaction_mt_set_transaction(transaction_mt_t *transaction_mt,/*设置事务*/
			       transaction_t *transaction)
{
  transaction_mt->transaction = transaction;
}

int
transaction_mt_start_transaction(transaction_mt_t *transaction_mt)/*开启事务*/
{
  transaction_mt->transaction->your_instance = transaction_mt;

  transaction_mt->thread = sthread_create(20000,NULL,(void *(*)(void *))transaction_thread,(void *)transaction_mt->transaction);
  /* 尽量给这个任务优先权try to give a highest priority to this task     */
  /* this is only supported by some OS and currently */
  /* only used on VxWorks. */
  if (transaction_mt->thread==NULL) return -1;

  sthread_setpriority(transaction_mt->thread, 1);
  return 0;
}

void
transaction_mt_stop_transaction(transaction_mt_t *transaction_mt)/*停止事务*/
{
  susleep(1000000); /* 我想我们现在可以转移这个旧的条件I think we can now remove this old race condition hack? */
  if (transaction_mt->thread!=NULL)
    {
      ua_core_t *ua_core;
      sipevent_t *sipevent;
      sipevent = osip_new_event(KILL_TRANSACTION,transaction_mt->transaction->transactionid);
      fifo_add(transaction_mt->transaction->transactionff,sipevent);

      /* 开启另一个线程必须使用pthread_join()函数Another thread must call pthread_join() */
      ua_core = ua_core_get();
      fifo_add(ua_core->threads_to_join, transaction_mt);
    }
  else
    {
      printf("!!!!!!!!! How can I be there??? thread is broken??\n");
    }
}

#endif /* OSIP_MT */
