#ifndef _VDO_DROP_MANAGER_H_
#define _VDO_DROP_MANAGER_H_

#include <wtypes.h>

enum E_VDO_FRAME_DROP_LEVEL
{
	VDO_FRAME_DROP_LEVEL_NOT_USE				=-1,// Not Use.
	VDO_FRAME_DROP_LEVEL_NO_SKIP				=0, //	Decode All Frames.
	VDO_FRAME_DROP_LEVEL_SKIP_END_OF_B			=1,	 // Always Skip EOB.
	VDO_FRAME_DROP_LEVEL_SKIP_ALL_B				=2,	 // Always Skip B Frames.
	VDO_FRAME_DROP_LEVEL_SKIP_ALL_PB			=3,	 // Always Skip PB Frames.
	VDO_FRAME_DROP_LEVEL_SKIP_ALL_IPB			=4,  // Always Skip IPB Frames.
};

enum E_VDO_FRAME_TYPE
{
	E_VDO_FRAME_TYPE_INVALID =0,
	E_VDO_FRAME_TYPE_I =1,
	E_VDO_FRAME_TYPE_P =2,
	E_VDO_FRAME_TYPE_B =3,
};

class CVideoDropManager
{
public:
	CVideoDropManager();
	virtual ~CVideoDropManager();

	void UpScaleLevel() { (m_iSkipLevel<VDO_FRAME_DROP_LEVEL_SKIP_ALL_IPB) ? m_iSkipLevel++ : 0;};
	void DownScaleLevel() { (m_iSkipLevel>VDO_FRAME_DROP_LEVEL_NO_SKIP) ? m_iSkipLevel-- : 0;};
	void SetSkipLevel(int SkipLevel) {m_iSkipLevel = SkipLevel; };
	int GetSkipLevel() { return m_iSkipLevel;};
	
	BOOL CheckFrameDrop(int iPType=E_VDO_FRAME_TYPE_B);

protected:
	int m_iContinuousBNums;
	int m_iSkipLevel;
};

#endif //_VDO_DROP_MANAGER_H_