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


#include <osip/port.h>
#include "rcfile.h"

static list_t *elements = NULL;

int josua_config_load(char *filename)/*SIP会议系统配置载入*/
{
  FILE *file;
  char *s; 
  configelt_t *configelt;

#ifdef __VXWORKS_OS__
  elements = (list_t *)smalloc(sizeof(list_t));
  list_init(elements);/*初始化元素清单*/

  configelt = (configelt_t *)smalloc(sizeof(configelt_t));
  if (0 <= josua_config_set_element((const char *)"displayname=Aymeric",/*显示名=Aymeric*/
configelt))
        list_add(elements, configelt, -1);

  configelt = (configelt_t *)smalloc(sizeof(configelt_t));
  if (0 <= josua_config_set_element((const char *)"username=jack", configelt))/*用户名=jack*/
        list_add(elements, configelt, -1);

  configelt = (configelt_t *)smalloc(sizeof(configelt_t));
  if (0 <= josua_config_set_element((const char *)"localip=90.0.0.1",/*本地边界=90.0.0.1*/
configelt))
        list_add(elements, configelt, -1);

  configelt = (configelt_t *)smalloc(sizeof(configelt_t));
  if (0 <= josua_config_set_element((const char
*)"global_contact=<sip:jack@90.0.0.1>", configelt))/*连接sip:jack@90.0.0.1*/
        list_add(elements, configelt, -1);

  configelt = (configelt_t *)smalloc(sizeof(configelt_t));
  if (0 <= josua_config_set_element((const char
*)"contact=<sip:jack@90.0.0.1>", configelt))
        list_add(elements, configelt, -1);

  configelt = (configelt_t *)smalloc(sizeof(configelt_t));
  if (0 <= josua_config_set_element((const char *)"from=<sip:jack@90.0.0.1>",/*从sip:jack@90.0.0.1发起*/
configelt))
        list_add(elements, configelt, -1);

  configelt = (configelt_t *)smalloc(sizeof(configelt_t));
  if (0 <= josua_config_set_element((const char *)"localport=5060", configelt))/*本地端口=5060*/
        list_add(elements, configelt, -1);

/*  configelt = (configelt_t *)smalloc(sizeof(configelt_t));
  if (0 <= josua_config_set_element((const char
*)"registrar=sip:192.168.1.114", configelt))
        list_add(elements, configelt, -1);*/

  configelt = (configelt_t *)smalloc(sizeof(configelt_t));
  if (0 <= josua_config_set_element((const char *)"networktype=IN", configelt))/*网络类型=IN*/
        list_add(elements, configelt, -1);

  configelt = (configelt_t *)smalloc(sizeof(configelt_t));
  if (0 <= josua_config_set_element((const char *)"addr_type=IP4", configelt))/*地址类型=IP4*/
        list_add(elements, configelt, -1);
#else

  elements = (list_t *) smalloc(sizeof(list_t));
  list_init(elements);
  file = fopen(filename, "r");
  if (file==NULL) return -1;
  s = (char *)smalloc(201*sizeof(char));
  while (NULL!=fgets(s, 200, file))
    {
    if ((strlen(s)==1)||(0==strncmp(s,"#",1)))
      ; /* 将之跳过 */
    else 
      {
      configelt = (configelt_t *)smalloc(sizeof(configelt_t));
      if (0<=josua_config_set_element((const char *)s,configelt))
	list_add(elements,configelt,-1);
      }
    }
  sfree(s);
  fclose(file);

#endif

  return 0; /* ok完成 */
}

int
josua_config_set_element(const char *s,configelt_t *configelt)
{
  char *tmp;
  tmp = strchr(s,61);  /* search "=" 61 */
  configelt->name = (char *) smalloc(tmp-s+1);
  configelt->value= (char *) smalloc(s+strlen(s)-tmp);
  sstrncpy(configelt->name,s,tmp-s);
  sstrncpy(configelt->value,tmp+1,s+strlen(s)-tmp-2);
  return 1;
}

char *
josua_config_get_element(char *name)
{
  configelt_t *tmp;
  int pos = 0;
  while (0==list_eol(elements,pos))
    {
    tmp = (configelt_t *)list_get(elements,pos);
    if (0==strcmp(tmp->name,name))
      return tmp->value;
    pos++;
    }
  return NULL;

}

void
josua_config_unload()/*SIP会议系统配置卸载*/
{
  configelt_t *tmp;
  if (elements==NULL) return;
  while (0==list_eol(elements,0))
    {
    tmp = (configelt_t *)list_get(elements,0);
    list_remove(elements,0);
    sfree(tmp->name);
    sfree(tmp->value);
    sfree(tmp);
    }

  sfree(elements);
  elements=NULL;
  return;
}
