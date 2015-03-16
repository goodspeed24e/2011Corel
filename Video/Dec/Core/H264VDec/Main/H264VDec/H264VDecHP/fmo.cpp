
/*!
*****************************************************************************
*
* \file fmo.c
*
* \brief
*    Support for Flexible Macroblock Ordering (FMO)
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Stephan Wenger      stewe@cs.tu-berlin.de
*    - Karsten Suehring    suehring@hhi.de
******************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>

#include "global.h"
#include "elements.h"
#include "defines.h"
#include "header.h"
#include "fmo.h"
#include "clipping.h"

//#define PRINT_FMO_MAPS

static void FmoGenerateType0MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps, unsigned PicSizeInMapUnits );
static void FmoGenerateType1MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps, unsigned PicSizeInMapUnits );
static void FmoGenerateType2MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps, unsigned PicSizeInMapUnits );
static void FmoGenerateType3MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps, unsigned PicSizeInMapUnits );
static void FmoGenerateType4MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps, unsigned PicSizeInMapUnits );
static void FmoGenerateType5MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps, unsigned PicSizeInMapUnits );
static void FmoGenerateType6MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps, unsigned PicSizeInMapUnits );


/*!
************************************************************************
* \brief
*    Generates MapUnitToSliceGroupMap
*    Has to be called every time a new Picture Parameter Set is used
*
* \param pps
*    Picture Parameter set to be used for map generation
* \param sps
*    Sequence Parameter set to be used for map generation
*
************************************************************************
*/
static int FmoGenerateMapUnitToSliceGroupMap PARGS2(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps)
{
	unsigned int NumSliceGroupMapUnits;

	NumSliceGroupMapUnits = (sps->pic_height_in_map_units_minus1+1)* (sps->pic_width_in_mbs_minus1+1);

	if (pps->slice_group_map_type == 6)
	{
		if ((pps->num_slice_group_map_units_minus1+1) != NumSliceGroupMapUnits)
		{
			DEBUG_SHOW_ERROR_INFO ("[ERROR]wrong pps->num_slice_group_map_units_minus1 for used SPS and FMO type 6", 500);
		}
	}

	// allocate memory for MapUnitToSliceGroupMap
	if (MapUnitToSliceGroupMap)
	{
		_aligned_free (MapUnitToSliceGroupMap);
		MapUnitToSliceGroupMap = NULL;
	}
	if ((MapUnitToSliceGroupMap = (int *) _aligned_malloc ((NumSliceGroupMapUnits) * sizeof (int), 16)) == NULL)
	{
		DEBUG_SHOW_ERROR_INFO ("[ERROR]cannot allocate %d bytes for MapUnitToSliceGroupMap, exit\n", (pps->num_slice_group_map_units_minus1+1) * sizeof (int));
		exit (-1);
	}

	memset (MapUnitToSliceGroupMap, 0, NumSliceGroupMapUnits * sizeof (int));
	return 0;
	/*
	switch (pps->slice_group_map_type)
	{
	case 0:
	FmoGenerateType0MapUnitMap (pps, sps, NumSliceGroupMapUnits);
	break;
	case 1:
	FmoGenerateType1MapUnitMap (pps, sps, NumSliceGroupMapUnits);
	break;
	case 2:
	FmoGenerateType2MapUnitMap (pps, sps, NumSliceGroupMapUnits);
	break;
	case 3:
	FmoGenerateType3MapUnitMap (pps, sps, NumSliceGroupMapUnits);
	break;
	case 4:
	FmoGenerateType4MapUnitMap (pps, sps, NumSliceGroupMapUnits);
	break;
	case 5:
	FmoGenerateType5MapUnitMap (pps, sps, NumSliceGroupMapUnits);
	break;
	case 6:
	FmoGenerateType6MapUnitMap (pps, sps, NumSliceGroupMapUnits);
	break;
	default:
	DEBUG_SHOW_ERROR_INFO ("[ERROR]Illegal slice_group_map_type %d , exit \n", pps->slice_group_map_type);
	exit (-1);
	}
	return 0;
	*/
}


/*!
************************************************************************
* \brief
*    Generates MbToSliceGroupMap from MapUnitToSliceGroupMap
*
* \param pps
*    Picture Parameter set to be used for map generation
* \param sps
*    Sequence Parameter set to be used for map generation
*
************************************************************************
*/
static int FmoGenerateMbToSliceGroupMap PARGS2(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps)
{
	int i;

	// allocate memory for MbToSliceGroupMap
	if (MbToSliceGroupMap)
	{
		_aligned_free (MbToSliceGroupMap);
		MbToSliceGroupMap = NULL;
	}

	if ((MbToSliceGroupMap = (int *) _aligned_malloc ((IMGPAR PicSizeInMbs) * sizeof (int), 16)) == NULL)
	{
		DEBUG_SHOW_ERROR_INFO ("[ERROR]cannot allocated %d bytes for MbToSliceGroupMap, exit\n", (IMGPAR PicSizeInMbs) * sizeof (int));
		exit (-1);
	}


	if ((sps->frame_mbs_only_flag)|| IMGPAR field_pic_flag)
	{
		for (i=0; i<IMGPAR PicSizeInMbs; i++)
		{
			MbToSliceGroupMap[i] = MapUnitToSliceGroupMap[i];
		}
	}
	else
		if (sps->mb_adaptive_frame_field_flag  &&  (!IMGPAR field_pic_flag))
		{
			for (i=0; i<IMGPAR PicSizeInMbs; i++)
			{
				MbToSliceGroupMap[i] = MapUnitToSliceGroupMap[i/2];
			}
		}
		else
		{
			for (i=0; i<IMGPAR PicSizeInMbs; i++)
			{
				MbToSliceGroupMap[i] = MapUnitToSliceGroupMap[(i/(2*IMGPAR PicWidthInMbs))*IMGPAR PicWidthInMbs+(i%IMGPAR PicWidthInMbs)];
			}
		}
		return 0;
}


/*!
************************************************************************
* \brief
*    FMO initialization: Generates MapUnitToSliceGroupMap and MbToSliceGroupMap.
*
* \param pps
*    Picture Parameter set to be used for map generation
* \param sps
*    Sequence Parameter set to be used for map generation
************************************************************************
*/
int FmoInit PARGS2(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps)
{
#ifdef PRINT_FMO_MAPS
	unsigned i,j;
#endif

	FmoGenerateMapUnitToSliceGroupMap ARGS2(pps, sps);
	FmoGenerateMbToSliceGroupMap ARGS2(pps, sps);

	//NumberOfSliceGroups = pps->num_slice_groups_minus1+1;
	NumberOfSliceGroups = 1;

#ifdef PRINT_FMO_MAPS
	DEBUG_SHOW_ERROR_INFO("[ERROR]\n");
	DEBUG_SHOW_ERROR_INFO("[ERROR]FMO Map (Units):\n");

	for (j=0; j<IMGPAR PicHeightInMapUnits; j++)
	{
		for (i=0; i<IMGPAR PicWidthInMbs; i++)
		{
			DEBUG_SHOW_ERROR_INFO("[ERROR]%c",48+MapUnitToSliceGroupMap[i+j*IMGPAR PicWidthInMbs]);
		}
		DEBUG_SHOW_ERROR_INFO("[ERROR]\n");
	}
	DEBUG_SHOW_ERROR_INFO("[ERROR]\n");
	DEBUG_SHOW_ERROR_INFO("[ERROR]FMO Map (Mb):\n");

	for (j=0; j<IMGPAR PicHeightInMbs; j++)
	{
		for (i=0; i<IMGPAR PicWidthInMbs; i++)
		{
			DEBUG_SHOW_ERROR_INFO("[ERROR]%c",48+MbToSliceGroupMap[i+j*IMGPAR PicWidthInMbs]);
		}
		DEBUG_SHOW_ERROR_INFO("[ERROR]\n");
	}
	DEBUG_SHOW_ERROR_INFO("[ERROR]\n");

#endif

	return 0;
}


/*!
************************************************************************
* \brief
*    Free memory allocated by FMO functions
************************************************************************
*/
int FmoFinit PARGS0()
{
	if (MbToSliceGroupMap)
	{
		_aligned_free (MbToSliceGroupMap);
		MbToSliceGroupMap = NULL;
	}
	if (MapUnitToSliceGroupMap)
	{
		_aligned_free (MapUnitToSliceGroupMap);
		MapUnitToSliceGroupMap = NULL; 
	}
	return 0;
}


/*!
************************************************************************
* \brief
*    FmoGetNumberOfSliceGroup() 
*
* \par Input:
*    None
************************************************************************
*/
int FmoGetNumberOfSliceGroup PARGS0()
{
	return NumberOfSliceGroups;
}


/*!
************************************************************************
* \brief
*    FmoGetLastMBOfPicture() 
*    returns the macroblock number of the last MB in a picture.  This
*    mb happens to be the last macroblock of the picture if there is only
*    one slice group
*
* \par Input:
*    None
************************************************************************
*/
int FmoGetLastMBOfPicture PARGS0()
{
	return FmoGetLastMBInSliceGroup ARGS1(FmoGetNumberOfSliceGroup ARGS0()-1);
}


/*!
************************************************************************
* \brief
*    FmoGetLastMBInSliceGroup: Returns MB number of last MB in SG
*
* \par Input:
*    SliceGroupID (0 to 7)
************************************************************************
*/

int FmoGetLastMBInSliceGroup PARGS1(int SliceGroup)
{
	int i;

	for (i=IMGPAR PicSizeInMbs-1; i>=0; i--)
		if (FmoGetSliceGroupId ARGS1(i) == SliceGroup)
			return i;
	return -1;

};


/*!
************************************************************************
* \brief
*    Returns SliceGroupID for a given MB
*
* \param mb
*    Macroblock number (in scan order)
************************************************************************
*/
int FmoGetSliceGroupId PARGS1(int mb)
{
	assert (mb < (int)IMGPAR PicSizeInMbs);
	assert (MbToSliceGroupMap != NULL);
	return MbToSliceGroupMap[mb];
}


/*!
************************************************************************
* \brief
*    FmoGetNextMBBr: Returns the MB-Nr (in scan order) of the next
*    MB in the (scattered) Slice, -1 if the slice is finished
*
* \param CurrentMbNr
*    number of the current macroblock
************************************************************************
*/
int FmoGetNextMBNr PARGS1(int CurrentMbNr)
{
	int SliceGroup = FmoGetSliceGroupId ARGS1(CurrentMbNr);

	while (++CurrentMbNr<(int)IMGPAR PicSizeInMbs && MbToSliceGroupMap [CurrentMbNr] != SliceGroup)
		;

	if (CurrentMbNr >= (int)IMGPAR PicSizeInMbs)
		return -1;    // No further MB in this slice (could be end of picture)
	else
		return CurrentMbNr;
}


/*!
************************************************************************
* \brief
*    Generate interleaved slice group map type MapUnit map (type 0)
*
************************************************************************
*/
static void FmoGenerateType0MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps, unsigned PicSizeInMapUnits )
{
	unsigned iGroup, j;
	unsigned i = 0;
	do
	{
		for( iGroup = 0; i < PicSizeInMapUnits; i += pps->run_length_minus1[iGroup++] + 1 )
		{
			for( j = 0; j <= pps->run_length_minus1[ iGroup ] && i + j < PicSizeInMapUnits; j++ )
				MapUnitToSliceGroupMap[i+j] = iGroup;
		}
	}
	while( i < PicSizeInMapUnits );
}


/*!
************************************************************************
* \brief
*    Generate dispersed slice group map type MapUnit map (type 1)
*
************************************************************************
*/
static void FmoGenerateType1MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps,
																							seq_parameter_set_rbsp_t* sps,
																							unsigned PicSizeInMapUnits )
{
	unsigned i;
	for( i = 0; i < PicSizeInMapUnits; i++ )
	{
		MapUnitToSliceGroupMap[i] = (i%IMGPAR PicWidthInMbs);//+(((i/IMGPAR PicWidthInMbs)*(pps->num_slice_groups_minus1+1))/2))%(pps->num_slice_groups_minus1+1);
	}
}

/*!
************************************************************************
* \brief
*    Generate foreground with left-over slice group map type MapUnit map (type 2)
*
************************************************************************
*/
static void FmoGenerateType2MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps, unsigned PicSizeInMapUnits )
{
	unsigned i;

	for( i = 0; i < PicSizeInMapUnits; i++ )
		MapUnitToSliceGroupMap[ i ] = pps->num_slice_groups_minus1;


	//  memset(MapUnitToSliceGroupMap[0],0,PicSizeInMapUnits*sizeof(MapUnitToSliceGroupMap[0]));

}


/*!
************************************************************************
* \brief
*    Generate box-out slice group map type MapUnit map (type 3)
*
************************************************************************
*/
static void FmoGenerateType3MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps,
																							seq_parameter_set_rbsp_t* sps,
																							unsigned PicSizeInMapUnits )
{
	unsigned i, k;
	int leftBound, topBound, rightBound, bottomBound;
	int x, y, xDir, yDir;
	int mapUnitVacant;

	unsigned mapUnitsInSliceGroup0 = min((pps->slice_group_change_rate_minus1 + 1) * IMGPAR slice_group_change_cycle, PicSizeInMapUnits);

	for( i = 0; i < PicSizeInMapUnits; i++ )
		MapUnitToSliceGroupMap[ i ] = 2;

	x = ( IMGPAR PicWidthInMbs - pps->slice_group_change_direction_flag ) / 2;
	y = ( IMGPAR PicHeightInMapUnits - pps->slice_group_change_direction_flag ) / 2;

	leftBound   = x;
	topBound    = y;
	rightBound  = x;
	bottomBound = y;

	xDir =  pps->slice_group_change_direction_flag - 1;
	yDir =  pps->slice_group_change_direction_flag;

	for( k = 0; k < PicSizeInMapUnits; k += mapUnitVacant ) 
	{
		if( xDir  ==  -1  &&  x  ==  leftBound ) 
		{
			leftBound = max( leftBound - 1, 0 );
			x = leftBound;
			xDir = 0;
			yDir = 2 * pps->slice_group_change_direction_flag - 1;
		} 
		else 
			if( xDir  ==  1  &&  x  ==  rightBound ) 
			{
				rightBound = min( rightBound + 1, (int)IMGPAR PicWidthInMbs - 1 );
				x = rightBound;
				xDir = 0;
				yDir = 1 - 2 * pps->slice_group_change_direction_flag;
			} 
			else 
				if( yDir  ==  -1  &&  y  ==  topBound ) 
				{
					topBound = max( topBound - 1, 0 );
					y = topBound;
					xDir = 1 - 2 * pps->slice_group_change_direction_flag;
					yDir = 0;
				} 
				else 
					if( yDir  ==  1  &&  y  ==  bottomBound ) 
					{
						bottomBound = min( bottomBound + 1, (int)IMGPAR PicHeightInMapUnits - 1 );
						y = bottomBound;
						xDir = 2 * pps->slice_group_change_direction_flag - 1;
						yDir = 0;
					} 
					else
					{
						x = x + xDir;
						y = y + yDir;
					}
	}

}

/*!
************************************************************************
* \brief
*    Generate raster scan slice group map type MapUnit map (type 4)
*
************************************************************************
*/
static void FmoGenerateType4MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps,
																							seq_parameter_set_rbsp_t* sps,
																							unsigned PicSizeInMapUnits )
{

	unsigned mapUnitsInSliceGroup0 = min((pps->slice_group_change_rate_minus1 + 1) * IMGPAR slice_group_change_cycle, PicSizeInMapUnits);
	unsigned sizeOfUpperLeftGroup = pps->slice_group_change_direction_flag ? ( PicSizeInMapUnits - mapUnitsInSliceGroup0 ) : mapUnitsInSliceGroup0;

	unsigned i;

	for( i = 0; i < PicSizeInMapUnits; i++ )
		if( i < sizeOfUpperLeftGroup )
			MapUnitToSliceGroupMap[ i ] = pps->slice_group_change_direction_flag;
		else
			MapUnitToSliceGroupMap[ i ] = 1 - pps->slice_group_change_direction_flag;

}

/*!
************************************************************************
* \brief
*    Generate wipe slice group map type MapUnit map (type 5)
*
************************************************************************
*/
static void FmoGenerateType5MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps,
																							seq_parameter_set_rbsp_t* sps,
																							unsigned PicSizeInMapUnits )
{

	unsigned mapUnitsInSliceGroup0 = min((pps->slice_group_change_rate_minus1 + 1) * IMGPAR slice_group_change_cycle, PicSizeInMapUnits);
	unsigned sizeOfUpperLeftGroup = pps->slice_group_change_direction_flag ? ( PicSizeInMapUnits - mapUnitsInSliceGroup0 ) : mapUnitsInSliceGroup0;

	int i,j;
	unsigned k = 0;

	for( j = 0; j < IMGPAR PicWidthInMbs; j++ )
		for( i = 0; i < IMGPAR PicHeightInMapUnits; i++ )
			if( k++ < sizeOfUpperLeftGroup )
				MapUnitToSliceGroupMap[ i * IMGPAR PicWidthInMbs + j ] = 1 - pps->slice_group_change_direction_flag;
			else
				MapUnitToSliceGroupMap[ i * IMGPAR PicWidthInMbs + j ] = pps->slice_group_change_direction_flag;

}

/*!
************************************************************************
* \brief
*    Generate explicit slice group map type MapUnit map (type 6)
*
************************************************************************
*/
static void FmoGenerateType6MapUnitMap PARGS3(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps, unsigned PicSizeInMapUnits )
{
	unsigned i;
	for (i=0; i<PicSizeInMapUnits; i++)
	{
		MapUnitToSliceGroupMap[i] = pps->slice_group_id[i];
	}
}

