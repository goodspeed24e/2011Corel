

#include "stdafx.h"
#include "OpenFiles.h"
#include <stdio.h>

BYTE * OpenImageFile(CString szFilePathName, int *width, int *height, int nType)
{
	BYTE * pImageData = NULL;
 	switch(nType)
	{
 	case T_bmp:
 		pImageData = OpenBMP(szFilePathName,width,height);
		break;
    case T_yuv:
 		AfxMessageBox("对YUV序列编码");
		break;
	case T_raw:
 		pImageData = OpenRAW(szFilePathName,width,height);
 		break;
 	case T_sif: 	
 		pImageData = OpenSIF(szFilePathName,width,height);
 		break;
	default:
 		AfxMessageBox("无法打开的文件！");
 		break;
 	}
 	if(pImageData)
 		return pImageData;
 	else 
 		return NULL;
}

CString GetNextFileName(CString filename,BOOL flag)
{
	CString tempName = filename;
	CString tempStr;
	int ret = filename.Find(".",0);
	if(ret>0)
		tempStr = filename.Mid(ret-3,3);
	else
		tempStr = filename.Right(3);
	
	int tempInt = atoi(tempStr);

	//flag 逻辑值为真，文件名递增；否则文件名递减
	if(flag)
		tempInt++;
	else
		tempInt--;

	char temp[3];
	if(tempInt<0)
	{
		AfxMessageBox("The file name is WRONG !");
		return tempName;
	}
	else if(tempInt<10)
	{
		temp[0] = '0';
		temp[1] = '0';
		itoa(tempInt,&temp[2],10);
	}
	else if(tempInt<100)
	{
		temp[0] = '0';
		itoa(tempInt,&temp[1],10);
	}
	else
		itoa(tempInt,temp,10);

	tempName.Replace(tempStr,temp);
	return tempName;
}

BYTE * OpenSIF(CString fileName, int *width, int *height)
{
	BYTE * pData;
	if(fileName=="")
		return FALSE;

	*width = 352;//578;
	*height = 288;//385;
 	long lFileSize = (*width) * (*height);
	pData = (BYTE*)new char[lFileSize*3/2];
	if(!pData)
		return NULL;

	CFile file;
	if((file.Open(fileName,CFile::modeRead|CFile::shareDenyNone))==NULL)
	{
		AfxMessageBox("Can not open the file");
		return NULL;
	}
 	file.Read(pData,lFileSize);
	file.Close();
	memset(pData+lFileSize,128,lFileSize/2);
	return pData;
	free(pData);
}

BYTE * OpenBMP(CString fileName, int *width, int *height)
{
	BYTE * pData, * pData1;
	if(fileName=="")
		return NULL;

	BITMAPFILEHEADER bmpFileHead;
	BITMAPINFOHEADER bmpInfo;

	CFile file;
	if((file.Open(fileName,CFile::modeRead|CFile::shareDenyNone))==NULL)
	{
		AfxMessageBox("Can not open the file");
		return NULL;
	}
 
	file.SeekToBegin();

	file.Read(&bmpFileHead,sizeof(BITMAPFILEHEADER));
	file.Read(&bmpInfo,sizeof(BITMAPINFOHEADER));
	DWORD LineBytes=(DWORD)WIDTHBYTES(bmpInfo.biWidth*bmpInfo.biBitCount);
    DWORD ImageSize =(DWORD)LineBytes*bmpInfo.biHeight;
    int NumColors;
	if(bmpInfo.biClrUsed!=0)
       NumColors=(DWORD)bmpInfo.biClrUsed;
	else
		switch(bmpInfo.biBitCount){
		case 1:
			NumColors=2;
			break;
		case 4:
			NumColors=16;
			break;
		case 8:
			NumColors=256;
			break;
		case 24:
			NumColors=0;
			break;
		default:
			AfxMessageBox("Invalid color numbers!");
			file.Close();
			return NULL;
	}

	if(bmpFileHead.bfOffBits!=(DWORD)(NumColors*sizeof(RGBQUAD)+sizeof(BITMAPFILEHEADER)
							+sizeof(BITMAPINFOHEADER)))
	{
			
    	AfxMessageBox("offBits 和实际头长度不符");
		//	,"Error Message" ,MB_OK|MB_ICONEXCLAMATION);
		return NULL; 
	}

	pData = (BYTE*)new char[ImageSize];
	pData1 = (BYTE*)new char[ImageSize*3/2];
	file.Seek(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+NumColors*sizeof(RGBQUAD),SEEK_SET);
    file.Read(pData,ImageSize);
	*width = bmpInfo.biWidth;
	*height = bmpInfo.biHeight;

	file.Close();

	int h,w;
	h=*height;
	w=*width;
		for(int i=0;i<h;i++)
		{
			for(int j=0;j<w;j++)
				{
				BYTE * temp;
				temp=pData+(h-1-i)*w+j;
				pData1[i*w+j]=*temp;
				}
		}
	delete pData;
	memset(pData1+ImageSize,128,ImageSize/2);
 	return pData1;
	free(pData1);

}

BYTE * OpenRAW(CString fileName, int *width, int *height)
{
	BYTE * pData;
	if(fileName=="")
		return FALSE;
	*width = 352;//512;
	*height = 288;//240;512;
 
 	long lFileSize = (*width) * (*height);
	pData = (BYTE*)new char[lFileSize*3/2];

	CFile file;
	if((file.Open(fileName,CFile::modeRead|CFile::shareDenyNone))==NULL)
	{
		AfxMessageBox("Can not open the file");
		return NULL;
	}
 	 
	if(!pData)
		return NULL;
 	file.Read(pData,lFileSize);
	file.Close();
	memset(pData+lFileSize,128,lFileSize/2);
	return pData;
	free(pData);
}

