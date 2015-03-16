#ifndef __CREL_RETURN_H__
#define __CREL_RETURN_H__

/***********************************************************************  
//	Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------+-------+---------------+
//	        | Major |     Minor     |General| Info  |   Specific    |
//          | Domain|     Domain    | Info  | Level |     Info      |
//
//  where
//
//      Sev - is the severity code
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag, always set to 1
//
//      R - is a reserved bit, set to 0
//
//      Facility - is the facility code 12 bits
//			Major Domain - first 4 bits to indicate 'codec', 'system' or other domain
//			Minor Domain - last 8 bits to indicate specific part in Major Domain
//
//      Code - is the facility's status code 16 bits
//			General Info  - general information or errors, 4bits
//			Error Level   - Warning or Info level, 4 bits
//			Specific Info - specific information or errors, 8bits
*************************************************************************/

typedef int CREL_RETURN;

/************************************************************************
* Define Success & GENERAL Failure code
************************************************************************/
#define CREL_OK						0x00000000
#define CREL_FAIL					0xE0000000

/************************************************************************
* Define Sev & C & R code, 4 bits
************************************************************************/
#define CREL_SEV_INFO				(0x6 << 28)
#define CREL_SEV_WARNING			(0xA << 28)
#define CREL_SEV_ERROR				(0xE << 28)

/************************************************************************
* Define Major Domain of Facility, 4 bits
************************************************************************/
#define CREL_MAJOR_GENERAL			(0x0 << 24)
#define CREL_MAJOR_CODEC			(0x1 << 24)
#define CREL_MAJOR_SYSTEM			(0x2 << 24)


/************************************************************************
* Define the facility code, 12 bits 
************************************************************************/

#define CREL_FAC_CODEC_GENERAL           (CREL_MAJOR_CODEC|0x00 << 16)
#define CREL_FAC_CODEC_H264              (CREL_MAJOR_CODEC|0x01 << 16)
#define CREL_FAC_CODEC_VC1	             (CREL_MAJOR_CODEC|0x02 << 16)
#define CREL_FAC_CODEC_MPEG4             (CREL_MAJOR_CODEC|0x03 << 16)
#define CREL_FAC_CODEC_H263              (CREL_MAJOR_CODEC|0x04 << 16)
#define CREL_FAC_CODEC_AMR               (CREL_MAJOR_CODEC|0x05 << 16) 
#define CREL_FAC_CODEC_AMRWB             (CREL_MAJOR_CODEC|0x06 << 16) 
#define CREL_FAC_CODEC_BSAC              (CREL_MAJOR_CODEC|0x07 << 16) 
#define CREL_FAC_CODEC_AAC               (CREL_MAJOR_CODEC|0x08 << 16)
#define CREL_FAC_CODEC_WMA               (CREL_MAJOR_CODEC|0x09 << 16)


#define CREL_FAC_SYSTEM_GENERAL		(CREL_MAJOR_SYSTEM|0x00 << 16)
#define CREL_FAC_SYSTEM_MUX			(CREL_MAJOR_SYSTEM|0x01 << 16)
#define CREL_FAC_SYSTEM_DEMUX		(CREL_MAJOR_SYSTEM|0x02 << 16)

/************************************************************************
* Define the prefix of return code 
************************************************************************/
#define CREL_ERR_CODEC_H264		    (CREL_SEV_ERROR|CREL_FAC_CODEC_H264)
#define CREL_WARNING_CODEC_H264		(CREL_SEV_WARNING|CREL_FAC_CODEC_H264)
#define CREL_INFO_CODEC_H264		(CREL_SEV_INFO|CREL_FAC_CODEC_H264)
#define CREL_OK_CODEC_H264			(CREL_OK|CREL_FAC_CODEC_H264)



/************************************************************************
* Define General Error of Code, 4 bits 
* Usage: ( ERR_PREFIX | GEN_ERROR )
************************************************************************/
#define CREL_GENERR_UNDEFINED		(0x00 << 12)	/* undefined or unsupport error */
#define CREL_GENERR_NOTSUPPORTED	(0x01 << 12)	/* feature is not supported in current version */
#define CREL_GENERR_NOMEMORY		(0x02 << 12)	/* no memory, failed of malloc */
#define CREL_GENERR_SYNCNOTFOUND	(0x03 << 12) /* Sync code not found in data buffer */
#define CREL_GENERR_ALIGNMENT		(0x04 << 12) /* Memory alignment condition not met */

#define CREL_GENINFO_INPUT_DATA_STATUS  (0x08 << 12) /* Get data function return status */

/************************************************************************
* Define Warning/Information Level, 4 bits 
************************************************************************/
/************************************************************************ 
* H264 Decoder Information level Return code 
************************************************************************/
#define CREL_WARNING_H264_STREAMERROR_LEVEL_3	(CREL_WARNING_CODEC_H264|0x100)	
#define CREL_WARNING_H264_STREAMERROR_LEVEL_2	(CREL_WARNING_CODEC_H264|0x200)	
#define CREL_WARNING_H264_STREAMERROR_LEVEL_1	(CREL_WARNING_CODEC_H264|0x300)	

#define WARNING_LEVEL_MASK		0x0F00

#define ISWARNINGLEVEL_1(hr) ( ( (CREL_RETURN)(hr) >= CREL_SEV_WARNING ) && ( (CREL_RETURN)(hr) < CREL_SEV_ERROR ) && ( ((CREL_RETURN)(hr) & WARNING_LEVEL_MASK) >= 0x0300 ) )
#define ISWARNINGLEVEL_3(hr) ( ( (CREL_RETURN)(hr) >= CREL_SEV_WARNING ) && ( (CREL_RETURN)(hr) < CREL_SEV_ERROR ) && ( ((CREL_RETURN)(hr) & WARNING_LEVEL_MASK) == 0x0100 ) )
#define ISWARNING(hr) ( ( (CREL_RETURN)(hr) >= CREL_SEV_WARNING ) && ( (CREL_RETURN)(hr) < CREL_SEV_ERROR ) )
#define ISERROR(hr) ( ( (CREL_RETURN)(hr) < 0 ) && ( (CREL_RETURN)(hr) >= CREL_SEV_ERROR ) )


/************************************************************************
* Define Specific Error of Code, 8 bits 
* Usage: Use it directly            
************************************************************************/

/************************************************************************ 
* H264 Decoder specific Return code 
************************************************************************/
#define CREL_ERROR_H264_UNDEFINED       (CREL_ERR_CODEC_H264|CREL_GENERR_UNDEFINED)
#define CREL_ERROR_H264_NOTSUPPORTED		(CREL_ERR_CODEC_H264|CREL_GENERR_NOTSUPPORTED)
#define CREL_ERROR_H264_SYNCNOTFOUND		(CREL_ERR_CODEC_H264|CREL_GENERR_SYNCNOTFOUND)
#define CREL_ERROR_H264_NOMEMORY		    (CREL_ERR_CODEC_H264|CREL_GENERR_NOMEMORY)
#define CREL_ERROR_H264_ALIGNMENT		    (CREL_ERR_CODEC_H264|CREL_GENERR_ALIGNMENT)


#define CREL_ERROR_H264_INPUT_DATA_BUFFER			(CREL_ERR_CODEC_H264|CREL_GENINFO_INPUT_DATA_STATUS)
#define CREL_INFO_H264_INPUT_DATA_BUFFER			(CREL_INFO_CODEC_H264|CREL_GENINFO_INPUT_DATA_STATUS)


//Specific error information should be combined with Information level Return code 
#define CREL_WARNING_H264_ERROR_DPB					0x01
#define CREL_WARNING_H264_ERROR_REFIDX			0x02
#define CREL_WARNING_H264_ERROR_CABAC				0x03
#define CREL_WARNING_H264_ERROR_SLICEHEAD		0x04
#define CREL_WARNING_H264_ERROR_SPS					0x05
#define CREL_WARNING_H264_ERROR_PPS					0x06
#define CREL_WARNING_H264_ERROR_SEI					0x07
#define CREL_WARNING_H264_HD_ProfileFault			0x8

#define CREL_ERROR_H264_HD_ProfileFault			0x01

//Specific get data status from upper level Return code
#define CREL_OK_H264_DATA_STATUS_SUCCESS                (CREL_INFO_H264_INPUT_DATA_BUFFER)
#define CREL_OK_H264_DATA_STATUS_SUCCESS_NO_SCRAMBLE    (CREL_INFO_H264_INPUT_DATA_BUFFER|0x01)

#define CREL_ERROR_H264_DATA_STATUS_NO_DATA             (CREL_ERROR_H264_INPUT_DATA_BUFFER|0x01)
#define CREL_ERROR_H264_DATA_STATUS_DATA_DISCONTINUITY  (CREL_ERROR_H264_INPUT_DATA_BUFFER|0x02)
#define CREL_ERROR_H264_DATA_STATUS_READ_ERROR          (CREL_ERROR_H264_INPUT_DATA_BUFFER|0x03)



#endif // __CREL_RETURN_H_

