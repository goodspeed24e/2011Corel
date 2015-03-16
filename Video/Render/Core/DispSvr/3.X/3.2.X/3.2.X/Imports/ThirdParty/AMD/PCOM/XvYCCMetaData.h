/*******************************************************************************
*
*
* Copyright (c) 2008 Advanced Micro Devices, Inc. (unpublished)
*
* All rights reserved. This notice is intended as a precaution against
* inadvertent publication and does not imply publication or any waiver of
* confidentiality. The year included in the foregoing notice is the year of
* creation of the work.
*
*
*******************************************************************************/
#ifndef _XVYCCMETADATA_H
#define _XVYCCMETADATA_H
typedef struct _XVYCC_GAMUT_METADATA    
{
    unsigned int size;
    unsigned int Format_Flag : 1;
    unsigned int reserved : 2;
    unsigned int GBD_Color_Precision : 2;
    unsigned int GBD_Color_Space : 3;
    unsigned int Min_Red_Data : 12;
    unsigned int Max_Red_Data : 12;
    unsigned int Min_Green_Data : 12;
    unsigned int Max_Green_Data : 12;
    unsigned int Min_Blue_Data : 12;
    unsigned int Max_Blue_Data : 12;
    unsigned int reserved2 : 16;
} XVYCC_GAMUT_METADATA    , *PXVYCC_GAMUT_METADATA;
#endif