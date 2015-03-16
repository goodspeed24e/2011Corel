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

#ifndef _RCFILE_H_
#define _RCFILE_H_

#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#include <osip/port.h>

typedef struct {

  char *name;
  char *value;

} configelt_t ;

int      josua_config_load (char *filename);
int      josua_config_set_element(const char *s,configelt_t *configelt);
char*    josua_config_get_element(char *name);

#endif
