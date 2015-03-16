#ifndef __IVIDEMUX_VERSION__
#define __IVIDEMUX_VERSION__

	#define IVI_MAJOR_VERSION 	4 //ComSDK-Video-H264VDec
	#define IVI_MINOR_VERSION 	5 //ComSDK-Video-H264VDec
	#define IVI_BUILD		0 //ComSDK-Video-H264VDec
	#define IVI_FIX			15 //ComSDK-Video-H264VDec
	
	#ifndef INSTALLSHIELD
		// installshield does not process 2nd level of macros definition
		#define STRINGIFY_1(x)				#x
		#define STRINGIFY(x)  				STRINGIFY_1(x)
		#define STRINGIFY2_1(x,y)			#x "." #y
		#define STRINGIFY2(x,y)  			STRINGIFY2_1(x,y)
		#define CUSTOMER(x)				CUSTOMER2(x)
		#define CUSTOMER2(x)				#x
	#endif

#endif //__IVIDEMUX_VERSION__
