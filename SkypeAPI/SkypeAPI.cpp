// SkypeAPI.cpp: implementation file
#include "stdafx.h"
#include "SkypeAPIApplication.h"
#include "SkypeAPI.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// 将字符串分开并添加到字符串数组中，如将字符串“aaa\nbbb\nccc”分开为：
// “aaa”、“bbb”、“ccc”添加到 StrAry 中
// 注意：这个函数里使用了 strtok() C库函数，strtok() 函数是不能嵌套使用的
// 所以，如果在调用 PartStringAndAddToStrAry() 的上层调用中已经使用了 strtok()
// 那么将不能调用本函数 PartStringAndAddToStrAry(),可以调用 PartStringAndAddToStrAry()
// 的另一版本, 它是简单地通过 strchr() 查找完成的.
//
int PutMSGToStrAry ( char *pStr, CStringArray &StrAry, char *seps/*="\t\r\n"*/ )
{
	StrAry.RemoveAll();
	char *token;
	token = strtok( pStr, seps );
	while( token != NULL )
	{
		StrAry.Add ( token );
		token = strtok( NULL, seps );
	}
	return StrAry.GetSize();
}

CSkypeAPI::CSkypeAPI ( Log_FUNC Log_In ): InsertMessage ( Log_In ), m_hWnd_Application ( NULL ), m_hWnd_Skype ( NULL ), m_MsgID_SkypeAttach ( 0 ), m_MsgID_SkypeDiscover ( 0 ), m_dwDialID ( 0 )
{
	ASSERT ( InsertMessage );
}

CSkypeAPI::~CSkypeAPI()
{

}

BOOL CSkypeAPI::InitialConnection ( HWND hWnd_MainWindow )           //CHwSkype的初始化函数
{
	m_hWnd_Application = hWnd_MainWindow;
	ASSERT ( ::IsWindow ( m_hWnd_Application ) );       //判断当前窗口句柄是否有效
	m_MsgID_SkypeAttach = RegisterWindowMessage("SkypeControlAPIAttach");          //注册自定义消息
	m_MsgID_SkypeDiscover = RegisterWindowMessage("SkypeControlAPIDiscover");      //注册自定义消息
	if ( m_MsgID_SkypeAttach==0 || m_MsgID_SkypeDiscover==0 )
	{
		return FALSE;                   //如果没有发现Skype程序或者没有连接倒Skype程序返回失败
	}
	
	SendMessage( HWND_BROADCAST, m_MsgID_SkypeDiscover, (WPARAM)m_hWnd_Application, 0);  //向当前前台窗体(Skype)发送m_MsgID_SkypeDiscover消息，该消息将被TranlateMessage解释并寻找到处理函数
	return TRUE;
}

BOOL CSkypeAPI::IsSkypeWindowOK()
{
	return ( ::IsWindow ( m_hWnd_Skype ) );                //m_hWnd_Skype是Skype的主窗口句柄，如果关闭主窗口则程序会报错（即使最小化到托盘也不行）
}

BOOL CSkypeAPI::SendComToSkype ( LPCTSTR lpszMsg, ... )           //...表示函数输入的为多个LPCTSTR类型的参数，这些参数通过va_list类型来传递的
{
	if ( !lpszMsg || strlen(lpszMsg) < 1 ) return FALSE;
	if ( !IsSkypeWindowOK() ) return FALSE;                       //如果Skype主窗口句柄不存在，则返回失败

	COPYDATASTRUCT CopyData = {0};                         
	
	//在Win32中，WM_COPYDATA消息主要目的是允许在进程间传递只读数据。
	//SDK文档推荐用户使用SendMessage()函数，接收方在数据复制完成前不返回，
	//这样发送方就不可能删除和修改数据。
	//这个函数的原型为：SendMessage(WM_COPYDATA,wParam,lParam)
	//其中wParam设置为包含数据的窗口句柄，lParam指向一个COPYDATASTRUCT的结构,
	//COPYDATASTRUCT结构中，dwData为自定义数据，cbData为数据大小，lpData为指向数据的指针
	
	char buf[1024] = {0};
	
	va_list  va;
	va_start ( va, lpszMsg );
	_vsnprintf  ( buf, sizeof(buf) - 1, (char*)lpszMsg, va);
	va_end(va);

	CopyData.dwData = 0;
	CopyData.lpData = (PVOID)buf;
	CopyData.cbData = strlen(buf)+1;
	if ( SendMessage ( m_hWnd_Skype, WM_COPYDATA, WPARAM(m_hWnd_Application),LPARAM(&CopyData)) == 0 )  //向Skype程序发送消息
	{
		InsertMessage ( "Send message [%s] failed", lpszMsg );                //SendMessage方法在消息发送出去后要求系统给与回复，如果没有成功则返回失败
		return FALSE;
	}
	return TRUE;       //如果发送成功，消息将由Skype去处理，返回成功
}

BOOL CSkypeAPI::SkypeAPISyntaxInterpret(LPCTSTR lpszMsg)          //SkypeAPI语法解释函数
{
	if ( !lpszMsg || strlen(lpszMsg) < 1 )             //如果消息长度为0或者消息为False，返回失败
		return FALSE;
	ASSERT ( ::IsWindow(m_hWnd_Application) && IsSkypeWindowOK() );  //检查本程序和Skype当前窗口句柄是否有效

	CStringArray StrAry;                                     //用CStringArray类型存储消息的各个参数，方便后续处理
	int nStrNum = PutMSGToStrAry ( (char*)lpszMsg, StrAry, " ,;\t" );
	if ( nStrNum < 1 ) return FALSE;
	int nCmdPos = 0;
	// 好友列表
	if ( StrAry.GetAt (nCmdPos) == "USERS" )              //如果消息的第一个关键字为USERS，则将该消息发送给本程序
	{
		nCmdPos ++;
		SendMessage ( m_hWnd_Application, WM_SKYPEMSG_USERS, WPARAM(&StrAry),LPARAM(nCmdPos) );
	}
	// 呼叫返回结果
	else if ( StrAry.GetAt (nCmdPos) == "CALL" )       //如果消息的第一个关键字为CALL，则计数器加1取下一个关键字
	{
		nCmdPos ++;
		if ( StrAry.GetSize() > nCmdPos )
		{
			m_dwDialID = (DWORD) atoi ( StrAry.GetAt(nCmdPos) );   //取得播打的号码
			nCmdPos ++;
			// 呼叫状态
			if ( StrAry.GetSize() > nCmdPos && StrAry.GetAt (nCmdPos) == "STATUS" )    //如果第三个关键字为STATUS
			{
				nCmdPos ++;
				if ( StrAry.GetSize() > nCmdPos )
				{
					m_csCallStatus = StrAry.GetAt (nCmdPos);       //取得播打的状态
					// 呼叫结束
					if ( m_csCallStatus == "FAILED" || m_csCallStatus == "MISSED" )    //如果第四个关键字为FAILED或者MISSED  
					{
						m_dwDialID = 0;                            //将播打的号码回复为0
					}
				}
			}
			// SUBJECT
			else if ( StrAry.GetSize() > nCmdPos && StrAry.GetAt (nCmdPos) == "SUBJECT" )//如果第三个关键字为SUBJECT，不作处理
			{
			}
		}
	}
	StrAry.RemoveAll ();           //清空临时消息序列

	return TRUE;
}

void CSkypeAPI::EndCurrentCall()      // 结束当前通话
{
	if ( m_dwDialID < 1 ) return;    //如果播打号码为0，则直接返回成功，结束通话，如果号码存在向Skype发送消息终止通话
	SendComToSkype ( "SET CALL %u STATUS FINISHED", m_dwDialID );
}

BOOL CSkypeAPI::TranslateMessage ( UINT message, WPARAM wParam, LPARAM lParam )   //本程序消息解释函数
{
	if ( message == WM_COPYDATA && m_hWnd_Skype == (HWND)wParam )  //如果消息是由Skype发送过来的显示消息内容，立即返回
	{
		PCOPYDATASTRUCT poCopyData = (PCOPYDATASTRUCT)lParam;
		InsertMessage ( "<<： %.*s\n", poCopyData->cbData, poCopyData->lpData); 
		SkypeAPISyntaxInterpret ( LPCTSTR(poCopyData->lpData) );
		return TRUE;
	}
	
	if ( message != m_MsgID_SkypeAttach && message != m_MsgID_SkypeDiscover )  //如果消息不是Skype发送的消息，并且不是连接到Skype或者没有安装Skype程序则返回失败
	{
		return FALSE;
	}

	if ( message == m_MsgID_SkypeAttach )                        //如果是连接Skype的消息
	{
		switch ( lParam )
		{
		case SKYPECONTROLAPI_ATTACH_SUCCESS:                     //如果与Skype连接成功则
			{
				InsertMessage ("<<： Connected to Skype!;" );
				m_hWnd_Skype=(HWND)wParam;
				ASSERT ( ::IsWindow ( m_hWnd_Skype ) );          //判断当前Skype窗口句柄是否有效
				break;
			}
		case SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION:       //如果与Skype连接成功则
			InsertMessage ("<<： Waiting Skype authorization!" );
			break;
		case SKYPECONTROLAPI_ATTACH_REFUSED:                     //如果与Skype连接尚未得到授权
			InsertMessage ("<<： Connection to Skype has been refused!" );
			break;
		case SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE:               //如果连接SkypeAPI无效
			InsertMessage ("<<： Skype API not available!" );
			break;
		case SKYPECONTROLAPI_ATTACH_API_AVAILABLE:               //如果SkypeAPI有效
			InsertMessage ("<<： Try to connect Skype API now !" );
			break;
		}
	}
	else if ( message == m_MsgID_SkypeDiscover )                //如果收到的是Skype程序发现的消息
	{
		InsertMessage ( "<<： Skype Client Discovered!" );
	}
	
	return TRUE;
}

BOOL CSkypeAPI::KeyPressd(LPCTSTR lpszKey)  // 发送按键消息
{
	if ( !IsSkypeWindowOK() ) return FALSE;                           //如果未获得Skyoe当前窗口句柄返回失败
	if ( !lpszKey || strlen(lpszKey) < 1 ) return FALSE;       //如果按键消息为false或者按键消息长度为0怎返回失败
	if ( !SendComToSkype ( "BTN_PRESSED %s", lpszKey ) )
		return FALSE;
	if ( !SendComToSkype ( "BTN_RELEASED %s", lpszKey ) )
		return FALSE;
	return TRUE;
}	
//  %d 十进制整数 
//  %c 字符 
//  %s 字符串 
//  %f, %g, %e 十进制浮点数 
//  %p 指针 
//  %o 八进制 
//  %x 十六进制 
//  %u 无符号整数