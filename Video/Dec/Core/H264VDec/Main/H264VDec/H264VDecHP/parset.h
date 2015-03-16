
/*!
**************************************************************************************
* \file
*    parset.h
* \brief
*    Picture and Sequence Parameter Sets, decoder operations
*    This code reflects JVT version xxx
* \date 25 November 2002
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details) 
*      - Stephan Wenger        <stewe@cs.tu-berlin.de>
***************************************************************************************
*/
#ifndef _PARSET_H_
#define _PARSET_H_


#include "parsetcommon.h"
#include "nalucommon.h"

void Scaling_List PARGS3(UCHAR *scalingList, int sizeOfScalingList, BOOL *UseDefaultScalingMatrix);

void InitVUI(seq_parameter_set_rbsp_t *sps);
CREL_RETURN ReadVUI PARGS1(seq_parameter_set_rbsp_t *sps);
CREL_RETURN ReadVUIExtension PARGS1(seq_parameter_set_rbsp_t *sps);
CREL_RETURN ReadHRDParameters PARGS1(hrd_parameters_t *hrd);
#if 0
void PPSConsistencyCheck (pic_parameter_set_rbsp_t *pps);
void SPSConsistencyCheck (seq_parameter_set_rbsp_t *sps);
#endif
void MakePPSavailable PARGS2(int id, pic_parameter_set_rbsp_t *pps);
void MakeSPSavailable PARGS2(int id, seq_parameter_set_rbsp_t *sps);
void MakeSubsetSPSavailable PARGS2(int id, seq_parameter_set_rbsp_t *sps);
BOOL CheckSPS PARGS2(NALU_t *one_nalu, unsigned int view_id);
CREL_RETURN ProcessSPS PARGS1(NALU_t *one_nalu);
CREL_RETURN ProcessSPSSubset PARGS1(NALU_t *one_nalu);
CREL_RETURN ProcessPPS PARGS1(NALU_t *one_nalu);
CREL_RETURN  ProcessAUD PARGS2(NALU_t *one_nalu, int *primary_pic_type);
CREL_RETURN ProcessNaluExt PARGS1(NALU_t *one_nalu);

CREL_RETURN UseParameterSet PARGS1(int PicParsetId);
int GetBaseViewId PARGS0 ();
int GetViewIndex PARGS1(unsigned int view_id);

CREL_RETURN activate_sps PARGS2(seq_parameter_set_rbsp_t *sps, unsigned int view_id);
CREL_RETURN activate_pps PARGS2(pic_parameter_set_rbsp_t *pps, unsigned int view_index);

CREL_RETURN activate_global_sps PARGS2(seq_parameter_set_rbsp_t *sps, unsigned int view_index);
CREL_RETURN activate_global_pps PARGS2(pic_parameter_set_rbsp_t *pps, unsigned int view_index);

void FreeSPS_MVC_Related(seq_parameter_set_rbsp_t *sps);

#endif
