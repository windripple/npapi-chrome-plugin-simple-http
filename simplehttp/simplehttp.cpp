#include "plugin.h"
#include "simplehttp.h"

#pragma comment(lib, "winhttp")
#include <windows.h>
#include <winhttp.h>
#include <string>

std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

using namespace std;

bool HttpRequest(ScriptablePluginObject* obj, const NPVariant* args,
				unsigned int argCount, NPVariant* result){

	string type = "GET";
	string host = "";
	unsigned int port = INTERNET_DEFAULT_HTTP_PORT;
	string uri ="/";
	string useragent = "Mozilla/5.0 (Windows; U; Windows NT 6.0; en-US)";
	string additionalheaders = "";
	string getdata = "";
	string postdata = "";

	unsigned int resolveTimeout = 10000;
	unsigned int connectTimeout = 10000;
	unsigned int sendTimeout = 10000;
	unsigned int receiveTimeout = 10000;

	for( int i=0;i<argCount;i++){
		NPVariant arg = args[i];
		switch (i){
			case 0:
				type = arg.value.stringValue.UTF8Characters;
				break;
			case 1:
				host = arg.value.stringValue.UTF8Characters;
				break;
			case 2:
				port = arg.value.intValue;
				break;
			case 3:
				uri = arg.value.stringValue.UTF8Characters;
				break;
			case 4:
				useragent = arg.value.stringValue.UTF8Characters;
				break;
			case 5:
				additionalheaders = arg.value.stringValue.UTF8Characters;
				break;
			case 6:
				getdata = arg.value.stringValue.UTF8Characters;
				break;
			case 7:
				postdata = arg.value.stringValue.UTF8Characters;
				break;
			case 8:
				resolveTimeout = arg.value.intValue;
				break;
			case 9:
				connectTimeout = arg.value.intValue;
				break;
			case 10:
				sendTimeout = arg.value.intValue;
				break;
			case 11:
				receiveTimeout = arg.value.intValue;
				break;
		}
	}
	BOOL usePost = (type == "POST" && postdata != "");

	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	BOOL bResults = FALSE;
	string page = "";

	HINTERNET hSession = NULL,
			  hConnect = NULL,
			  hRequest = NULL;

	hSession = WinHttpOpen( s2ws(useragent).c_str(),
							WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
							WINHTTP_NO_PROXY_NAME,
							WINHTTP_NO_PROXY_BYPASS,
							0);
	if (!WinHttpSetTimeouts( hSession, resolveTimeout, connectTimeout, sendTimeout, receiveTimeout)){
		goto endHttpRequest;
	};

	if(hSession){//session建立不成功则退出
		hConnect = WinHttpConnect( hSession,
											s2ws(host).c_str(),
											port,
											0);
	}else{
		goto endHttpRequest;
	}

	if(hConnect){//connection建立不成功则退出
		if(!usePost){
			hRequest = WinHttpOpenRequest( hConnect,
									   s2ws(type).c_str(),
									   s2ws(uri.append(getdata)).c_str(),
									   NULL,
									   WINHTTP_NO_REFERER,
									   WINHTTP_DEFAULT_ACCEPT_TYPES,
									   WINHTTP_FLAG_REFRESH);
		}else{
			hRequest = WinHttpOpenRequest( hConnect,
									   s2ws(type).c_str(),
									   s2ws(uri).c_str(),
									   NULL,
									   WINHTTP_NO_REFERER,
									   WINHTTP_DEFAULT_ACCEPT_TYPES,
									   WINHTTP_FLAG_REFRESH);
		}

	}else{
		goto endHttpRequest;
	}
		
	//清除默认的cookie
	if(hRequest)
		bResults = WinHttpAddRequestHeaders( hRequest, 
													  L"Cookie:", 
                                                      -1, 
                                                      WINHTTP_ADDREQ_FLAG_REPLACE);
	//自定义header
	if(hRequest)
		bResults = WinHttpAddRequestHeaders( hRequest,
													  s2ws(additionalheaders).c_str(),
													  -1,
													  WINHTTP_ADDREQ_FLAG_ADD);
	//post数据
	if(hRequest)
	{
		if(usePost){
			bResults = WinHttpAddRequestHeaders( hRequest,
													  L"Content-Type: application/x-www-form-urlencoded",
													  -1,
													  WINHTTP_ADDREQ_FLAG_ADD);
			bResults = WinHttpAddRequestHeaders( hRequest,
													  L"Content-Length: "+ postdata.size(),
													  -1,
													  WINHTTP_ADDREQ_FLAG_ADD);
			bResults = WinHttpSendRequest( hRequest,
												WINHTTP_NO_ADDITIONAL_HEADERS,
												0,
												(LPVOID)postdata.c_str(),
												postdata.size(),
												postdata.size(),
												0);
		}else{
			bResults = WinHttpSendRequest( hRequest,
												WINHTTP_NO_ADDITIONAL_HEADERS,
												0,
												WINHTTP_NO_REQUEST_DATA,
												0,
												0,
												0);
		}
	}

	if(bResults){//发送http建立不成功则退出
		bResults = WinHttpReceiveResponse(hRequest,NULL);
	}else{
		goto endHttpRequest;
	}

	if(bResults){
		do{
			dwSize = 0;
			if(!WinHttpQueryDataAvailable( hRequest,&dwSize)){
				return false;
			}

			pszOutBuffer = new char[dwSize+1];

			if(!pszOutBuffer){
				dwSize = 0;
				return false;
			}else{
				ZeroMemory(pszOutBuffer,dwSize+1);

				if(!WinHttpReadData( hRequest,
									 (LPVOID)pszOutBuffer,
									 dwSize,
									 &dwDownloaded)){
					return false;
				}else{
					page.append(pszOutBuffer);
				}
				delete [] pszOutBuffer;
			}
		}while(dwSize>0);
	}

	endHttpRequest:
	char* npOutString = (char *)npnfuncs->memalloc(page.size()+1);
	strcpy(npOutString,page.c_str());
	STRINGZ_TO_NPVARIANT(npOutString,*result);
	if( hRequest ) WinHttpCloseHandle( hRequest );
    if( hConnect ) WinHttpCloseHandle( hConnect );
    if( hSession ) WinHttpCloseHandle( hSession );
	return true;
};