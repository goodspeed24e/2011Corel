#include "stdafx.h"

#ifdef _NO_USE_D3DXDLL
D3DXMATRIX* WINAPI D3DXMatrixMultiply( D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2 )
{
	int i,j;
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pOut->m[i][j] = pM1->m[i][0] * pM2->m[0][j] + pM1->m[i][1] * pM2->m[1][j] + pM1->m[i][2] * pM2->m[2][j] + pM1->m[i][3] * pM2->m[3][j];
		}
	}
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixTranspose(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM)
{
	int i,j;
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pOut->m[i][j] = pM->m[j][i];
		}
	}
	return pOut;
}
#endif