//////////////////////////////////////////////////////////////////////
//
// GetWeb.h: interface for the GetWeb class.
// 
// Author : Huawen nie
//
// Email : nie173@sohu.com
//
// Data Time ��2004 .5 .31
//////////////////////////////////////////////////////////////////////

#include "GetWeb.h"


//////////////////////////////////////////////////////////////////////////
//Globals

const TCHAR szHeaders[] =
_T("Accept: text/*\r\nUser-Agent: LCD's Infobay Http Client\r\n");
//////////////////////////////////////////////////////////////////////////



CGetWeb::CGetWeb()
{
   dwHttpRequestFlags = INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_AUTO_REDIRECT;
   
};
//Get web
// return TRUE was success
int CGetWeb::OnGetTheWeb(CString& sURL, CString &fileContent)
{

	dwRetcode=-1;
	dwServiceType =0;
	CInternetSession session;
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	try
	{
		
		if(!OnInitSession(session))
		{
			return FALSE;
		}
		if (!AfxParseURL( (LPCTSTR)sURL, dwServiceType, strServerName, strObject, nPort) ||
			dwServiceType != INTERNET_SERVICE_HTTP)
		{
			str_Error = "�Ƿ���URL";
			return FALSE;
		}
		pServer = session.GetHttpConnection(strServerName, nPort);
		if(pServer==NULL)
		{
			str_Error = "�޷����������������";
			return FALSE;
		}
		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET,strObject, NULL, 1, NULL, NULL, dwHttpRequestFlags);

		if(pFile==NULL)
		{
			str_Error = "�޷����������������";
			return FALSE;
		}
		//////////////////////////////////////////////////////////////////////////
        try
	   {
		if(!pFile->AddRequestHeaders(szHeaders)|| !pFile->SendRequest())
		{
            str_Error ="��������޷���������ͷ";
			return FALSE;
		}
	   }
	   catch (CInternetException* ex) 
	   {
		   fileContent.Empty();
		   str_Error="�޷�����http��ͷ,��������״��������";
		   ex->Delete();
		   return FALSE;		
	   }
	   //////////////////////////////////////////////////////////////////////////
	   
		if(!pFile->QueryInfoStatusCode(dwRetcode))
		{
			str_Error ="��������޷���ѯ��������";
			return FALSE;
		}

		if(dwRetcode>=200&&dwRetcode<300)
		{		    		
				
		    return OnGetData(session,pServer,pFile,fileContent);

		}
        else
		{
			//���¶���
			if( dwRetcode >= 300 && dwRetcode <= 306 )
			{
				
				pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strNewLocation);

				int nPlace = strNewLocation.Find(_T("Location: "));
				if (nPlace == -1)
				{
					str_Error ="Error: Site redirects with no new location";
					if (pFile != NULL)
						delete pFile;
					if (pServer != NULL)
						delete pServer;
					session.Close();
					return FALSE;
				}

				strNewLocation = strNewLocation.Mid(nPlace + 10);
				nPlace = strNewLocation.Find('\n');
				if (nPlace > 0)
				{
					strNewLocation = strNewLocation.Left(nPlace);
				}
			   
				if(strNewLocation.Find("http://",0)<0)
				{
					sURL = OnConversionURL(sURL,strNewLocation);
				}
				else
				{
					sURL = strNewLocation;
				}
				fileContent.Empty();
				pFile->Close();
				delete pFile;
				pServer->Close();
				delete pServer;
			return OnGetTheWeb( sURL, fileContent);
			}
			else//����һ�����
			{
				fileContent.Empty();
				return OnProcessError(dwRetcode,session,pServer,pFile);
			}

		}

	}
	catch (CInternetException* pEx)
	{
		/*TCHAR szErr[1024];
		pEx->GetErrorMessage(szErr, 1024);*/
		str_Error ="�������";
		
		pEx->Delete();   
		return FALSE;
	}
}
//////////////////////////////////////////////////////////////////////////
//��ѯ��ҳ������޸�ʱ��
BOOL CGetWeb::OnQuerLastMendTime(const COleDateTime& m_site_LSTime,COleDateTime& news_Time ,CHttpFile* pFile)
{
	///*OnFunctionsMoind("OnQuerLastMendTime");*/
	//SYSTEMTIME *ptime=NULL;
	//ptime = new SYSTEMTIME ;
	//try
	//{
	//	if(ptime!=NULL&&pFile->QueryInfo( HTTP_QUERY_LAST_MODIFIED  , ptime) )
	//	{
	//		news_Time.SetDateTime(ptime->wYear,ptime->wMonth,ptime->wDay,ptime->wHour,ptime->wMonth,ptime->wSecond );
	//	
	//		if(m_site_LSTime >= news_Time||odNowtime.GetDay()-news_Time.GetDay()>5)//no updata
	//		{
	//			if(ptime!=NULL)
	//			{
	//				delete ptime;
	//			}
	//			return FALSE;
	//		}
	//		else
	//		{
	//			if(ptime!=NULL)
	//			{
	//				delete ptime;
	//			}
	//			return TRUE;
	//		}
	//	}
	//	else
	//	{
	//		if(ptime!=NULL)
	//		{
	//			delete ptime;
	//		}
	//		return TRUE;
	//	}
	//}
	//catch(CInternetException* pEx)
	//{
	//	str_Error ="��ѯ�ļ�ʱ�䵼���������";
	//	pEx->Delete();  
	//	return FALSE;
	//}
	return true;
}
//////////////////////////////////////////////////////////////////////////
//��ʼ��CInternetSession
BOOL CGetWeb::OnInitSession(CInternetSession& session)
{
	
	//��ʱ���ú���Ҫ���������̫С�������������ʱ���������̫����������̹߳���
	//����������֮��ĵȴ�����ʱֵ�ں��뼶��
	//������������ʱ�䳬ʱֵ�������뼶�������������ʱ�䳬�������ʱֵ�����󽫱�ȡ����ȱʡ�ĳ�ʱֵ�����޵ġ�
	//��������������ʱ�����Դ��������һ��������ͼ��ָ�������Դ�������ʧ�ܣ�������ȡ����ȱʡֵΪ5��
	try
	{
		if(!session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 10000)||
		!session.SetOption(INTERNET_OPTION_CONNECT_BACKOFF, 1000)||
		!session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 3)||
		!session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT,60000)||	
		!session.EnableStatusCallback(TRUE))
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
	catch(CInternetException* pEx)
	{
		pEx->GetErrorMessage(str_Error.GetBuffer(0),1023);
		pEx->Delete();
		return FALSE;
	}
}
//process error
int CGetWeb::OnProcessError(int dwRetcode,CInternetSession& session,CHttpConnection* pServer,CHttpFile* pFile)
{

    switch( dwRetcode )
		{
		case 100:
			str_Error ="�ͻ�������-���� [Continue]";
			break;
			//
		case 101:
			str_Error ="�ͻ�������-����Э�� [witching Protocols]";
			break;
			//
		case 204:
			str_Error ="��ҳ����Ϊ�� [No Content]";
			break;
			//------------------------------------------------------------
		case 400:
			str_Error ="�������� [Bad Request]";
			break;
			//
		case 401:
			str_Error ="��ҳ��Ҫ��֤��Ϣ [Unauthorized]";
			break;
			//
		case 402:
			str_Error ="��ҳ��Ҫ���� [Payment Required]";
			break;
			//
		case 403:
			str_Error ="��ֹ���� [Forbidden]";
			break;
			//
		case 404://
			str_Error = "û���ҵ���ҳ [Not Found]";
			break;
			//
		case 405:
			str_Error ="������Http���ʸ��ļ� [Method Not Allowed]";
			break;
			//
		case 406:
			str_Error ="���ļ���������� [Not Acceptable]";
			break;
			//
		case 407:
			str_Error ="���ļ���Ҫ������֤ [Proxy Authentication Required]";
			break;
			//
		case 408:
			str_Error ="�Ը��ļ�����ʱ [Request Time-out]";
			break;
			//
		case 409:
			str_Error ="�Ը��ļ����ʳ�ͻ [Conflict]";
			break;
			//
		case 410:
			str_Error ="�Ը��ļ�����ʧ�� [Gone]";
			break;
			//
		case 411:
			str_Error ="���ļ���Ҫ������Ϣ [Length Required]";
			break;
			//
		case 412:
			str_Error ="��������ʧ�� [Precondition Failed]";
			break;
			//
		case 413:
			str_Error ="�����ļ�ʵ��̫�� [Request Entity Too Large]";
			break;
			//
		case 414:
			str_Error ="�����URI̫�� [Request-URI Too Large]";
			break;
			//
		case 415:
			str_Error ="��֧��ý������ [Unsupported Media Type]";
			break;
			//
		case 416:
			str_Error ="��������ʧ�� [Requested range not satisfiable]";
			break;
			//
		case 417:
			str_Error ="Ԥ��ʧ�� [Expectation Failed]";
			break;
			//--------------------------------------------------------------
		case 500:
			str_Error ="�������ڲ����� [Internal Server Error]";
			break;
			//
		case 501:
			str_Error ="δʵ������ [Not Implemented]";
			break;
			//
		case 502:
			str_Error ="����ʧ�� [Bad Gateway]";
			break;
			//
		case 503:
			str_Error ="û���ҵ������� [Service Unavailable]";
			break;
			//
		case 504:
			str_Error ="���س�ʱ [Gateway Time-out]";
			break;
			//
		case 505:
			str_Error ="��������֧��ϵͳʹ�õ�HTTP�汾 [HTTP Version not supported]";
			break;
			//
		}
	try
	{
	if (pFile != NULL)
		delete pFile;
	if (pServer != NULL)
		delete pServer;
	session.Close();
	return FALSE;
	}
    catch (CInternetException* pEx)
	{
	pEx->GetErrorMessage(str_Error.GetBuffer(0),1024);
	pEx->Delete();
    return FALSE;
	}
}
//RXD function
int CGetWeb::OnGetData(CInternetSession& session,CHttpConnection* pServer,CHttpFile* pFile,CString &fileContent)
{
	
	try
	{
       
		while( pFile->ReadString(str_FlagTemp))
		{				
			
			fileContent +=str_FlagTemp;
			fileContent +="\r\n";
			
		}

		if (pFile != NULL)
			delete pFile;
		if (pServer != NULL)
			delete pServer;
		session.Close();
		return TRUE;
	}
	catch (CInternetException* pEx)
	{
			str_Error ="�������ݴ���";
			pEx->Delete();
			return 0;
	}
}
//====================================================
//
// ../��ʾ����һ��
// /��ʾ��Ŀ¼�µ�
// XX.htm��ʾ��ǰĿ¼�µ�
//��URLת���ɾ��Ե�ַ
CString CGetWeb::OnConversionURL(CString sURL,CString str_fafURL)
{
	
	if(sURL.Find("/",8)<0)
	{
		sURL +="/";
	}
	CString str_activeURL;
	int int_j = 0;
	int i=0;
	str_activeURL = str_fafURL;
	if(str_fafURL.Find("../",0)!=-1&&str_fafURL[0]!='/')
	{
		while( i<=str_fafURL.GetLength() )
		{
			if( str_fafURL[i] == '.' && str_fafURL[i+1] == '.' && str_fafURL[i+2] == '/' )
			{ int_j++;}
			i++;
		}
		if(str_fafURL[0]=='/')
		{
			str_fafURL.Delete(0,1);
		}
		str_fafURL.Replace("../","");
		i=0;
		int int_i=0;
		while( i <= sURL.GetLength() )
		{
			if( sURL[i]=='/' )
			{ 
				int_i++;
			}
			i++;
		}
		int_i -= int_j;

		if( int_i<3 )
		{
			int_i = 3;
		}

		int int_cour=0;
		for( i=0; i<=sURL.GetLength(); i++)
		{
			if( sURL[i]=='/' )
			{ 
				int_cour++;
			}
			if( int_cour==int_i )
			{			 
				sURL= sURL.Left(i+1);
				break;
			}
		}
		//�ݴ���
		if( sURL[sURL.GetLength()-1]!='/' )
		{	
			sURL +="/";
		}
		sURL += str_fafURL;
		return sURL;
	}
	else
	{
		if( str_fafURL[0] =='/' )
		{
			int int_b = 0 ;
			for( int a=0; int_b<3 && a<sURL.GetLength(); a++)
			{
				if( sURL[a]=='/' )
			 {
				 int_b++;
			 }
			 if( int_b==3 )
			 {
				 sURL = sURL.Left(a);
				 break;
			 }
			}
			sURL += str_fafURL;
		}
		else
		{
			for( int i=sURL.GetLength() ; i> 0 ; i-- )
			{
				if( sURL[i] =='/' )
				{
					sURL = sURL.Left( i+1 );
					break;
				}
			}
			sURL += str_fafURL;
		}
		return sURL;
	}
}
//====================================================
//
//return errer content
CString CGetWeb::OnQueryErrer()
{
	return str_Error;
}
