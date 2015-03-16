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

#include "josua.h"

int audio_ctxt_init(audio_ctxt_t **ctx)/*音频初始化*/
{
  *ctx = (audio_ctxt_t*)smalloc(sizeof(audio_ctxt_t));
  if (*ctx==NULL) return -1;
  return 0;
}

void audio_ctxt_free(audio_ctxt_t *ctx)/*音频释放*/
{

}

int video_ctxt_init(video_ctxt_t **ctx)/*视频初始化*/
{
  *ctx = (video_ctxt_t*)smalloc(sizeof(video_ctxt_t));
  if (*ctx==NULL) return -1;
  return 0;
}

void video_ctxt_free(video_ctxt_t *ctx)/*视频释放*/
{

}

int application_ctxt_init(application_ctxt_t **ctx)/*应用初始化*/
{
  *ctx = (application_ctxt_t*)smalloc(sizeof(application_ctxt_t));
  if (*ctx==NULL) return -1;
  return 0;
}

void application_ctxt_free(application_ctxt_t *ctx)/*应用释放*/
{

}
