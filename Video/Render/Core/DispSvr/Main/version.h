#define IVI_LOGID			80745

#define IVI_MAJOR_VERSION 	4  //ComSDK-Video-DispSvr
#define IVI_MINOR_VERSION 	0  //ComSDK-Video-DispSvr
#define IVI_BUILD		0	//ComSDK-Video-DispSvr
#define IVI_FIX			2	//ComSDK-Video-DispSvr


#ifndef INSTALLSHIELD
// installshield does not process 2nd level of macros definition
#define STRINGIFY_1(x)				#x
#define STRINGIFY(x)  				STRINGIFY_1(x)
#define STRINGIFY2_1(x,y)			#x "." #y
#define STRINGIFY2(x,y)  			STRINGIFY2_1(x,y)
#define CUSTOMER(x)					CUSTOMER2(x)
#define CUSTOMER2(x)				#x

#endif
