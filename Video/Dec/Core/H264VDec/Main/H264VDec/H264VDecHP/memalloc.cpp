
/*!
************************************************************************
* \file  memalloc.c
*
* \brief
*    Memory allocation and free helper funtions
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
************************************************************************
*/
#pragma warning ( disable : 4995 )
#include <stdlib.h>
#include "global.h"
#include "memalloc.h"

/*!
************************************************************************
* \brief
*    Exit program if memory allocation failed (using error())
* \param where
*    string indicating which memory allocation failed
************************************************************************
*/
void no_mem_exit(char *where)
{
	DEBUG_SHOW_ERROR_INFO("[ERROR]Could not allocate memory: %s",where);

}
