
/*!
**************************************************************************************
* \file
*    output.h
* \brief
*    Picture writing routine headers
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details) 
*      - Karsten Suehring        <suehring@hhi.de>
***************************************************************************************
*/


#ifdef __cplusplus
//extern "C" {
#endif

#ifndef _OUTPUT_H_
#define _OUTPUT_H_

void write_stored_frame PARGS2(FrameStore *fs, int out);
CREL_RETURN direct_output PARGS2(StorablePicture *p, int out);
void init_out_buffer PARGS0();
void uninit_out_buffer PARGS0();

#ifdef PAIR_FIELDS_IN_OUTPUT
void flush_pending_output(int out);
#endif

#endif //_OUTPUT_H_

#ifdef __cplusplus
//}
#endif
