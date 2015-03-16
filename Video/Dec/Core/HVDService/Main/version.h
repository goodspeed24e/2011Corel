#pragma once

#define IVI_MAJOR_VERSION 	3 //ComSDK-Video-HVDService
#define IVI_MINOR_VERSION 	0 //ComSDK-Video-HVDService
#define IVI_BUILD		0 //ComSDK-Video-HVDService
#define IVI_FIX			16 //ComSDK-Video-HVDService


#ifndef INSTALLSHIELD
// installshield does not process 2nd level of macros definition
#define STRINGIFY_1(x)				#x
#define STRINGIFY(x)  				STRINGIFY_1(x)
#define STRINGIFY2_1(x,y)			#x "." #y
#define STRINGIFY2(x,y)  			STRINGIFY2_1(x,y)
#define CUSTOMER(x)					CUSTOMER2(x)
#define CUSTOMER2(x)				#x

#endif
