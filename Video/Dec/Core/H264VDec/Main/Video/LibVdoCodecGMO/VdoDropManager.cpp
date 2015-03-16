#include "VdoDropManager.h"

CVideoDropManager::CVideoDropManager()
{
	m_iContinuousBNums = 0;
	m_iSkipLevel = VDO_FRAME_DROP_LEVEL_NO_SKIP;
}

CVideoDropManager::~CVideoDropManager()
{

}

BOOL CVideoDropManager::CheckFrameDrop(int iPType)
{
	BOOL bDrop = FALSE;
	
	(iPType==E_VDO_FRAME_TYPE_B) ? ++m_iContinuousBNums : m_iContinuousBNums =0;

	switch(m_iSkipLevel)
	{
	case VDO_FRAME_DROP_LEVEL_SKIP_ALL_IPB: bDrop = TRUE;
		break;
	case VDO_FRAME_DROP_LEVEL_SKIP_ALL_B: bDrop = (iPType==E_VDO_FRAME_TYPE_B);
		break;
	case VDO_FRAME_DROP_LEVEL_SKIP_ALL_PB: bDrop = (iPType==E_VDO_FRAME_TYPE_B || iPType==E_VDO_FRAME_TYPE_P);
		break;
	case VDO_FRAME_DROP_LEVEL_SKIP_END_OF_B: bDrop = (iPType==E_VDO_FRAME_TYPE_B && (m_iContinuousBNums%2)==0);
		break;
	default:
	case VDO_FRAME_DROP_LEVEL_NO_SKIP: bDrop = FALSE;
		break;
	}

	return bDrop;
}
