#include "plugin.h"
#include "simplehttp.h"

#pragma comment(lib, "winhttp.lib")
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

//borrowed from http://naclports.googlecode.com/svn-history/r120/trunk/src/examples/scriptable/matrix_npapi/matrix_comp.cc
std::string CreateString(const NPString& npString) {
    std::string ret_value;
	for (unsigned int i = 0; i < npString.UTF8Length; ++i) {
	    ret_value += npString.UTF8Characters[i];
	}
	return ret_value;
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

	for( unsigned int i=0;i<argCount;i++){
		NPVariant arg = args[i];
		switch (i){
			case 0:
				type = CreateString(NPVARIANT_TO_STRING(arg));
				break;
			case 1:
				host = CreateString(NPVARIANT_TO_STRING(arg));
				break;
			case 2:
				port = NPVARIANT_TO_INT32(arg);
				break;
			case 3:
				uri = CreateString(NPVARIANT_TO_STRING(arg));
				break;
			case 4:
				useragent = CreateString(NPVARIANT_TO_STRING(arg));
				break;
			case 5:
				additionalheaders = CreateString(NPVARIANT_TO_STRING(arg));
				break;
			case 6:
				getdata = CreateString(NPVARIANT_TO_STRING(arg));
				break;
			case 7:
				postdata = CreateString(NPVARIANT_TO_STRING(arg));
				break;
			case 8:
				resolveTimeout = NPVARIANT_TO_INT32(arg);
				break;
			case 9:
				connectTimeout = NPVARIANT_TO_INT32(arg);
				break;
			case 10:
				sendTimeout = NPVARIANT_TO_INT32(arg);
				break;
			case 11:
				receiveTimeout = NPVARIANT_TO_INT32(arg);
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

	if(hSession){
		std::wstring hostTemp = s2ws(host);
		LPCWSTR whost = hostTemp.c_str();
		hConnect = WinHttpConnect( hSession,
			                                whost,
											port,
											0);
	}else{
		goto endHttpRequest;
	}

	if(hConnect){
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
		
	if(hRequest)
		bResults = WinHttpAddRequestHeaders( hRequest, 
													  L"Cookie:", 
                                                      -1, 
                                                      WINHTTP_ADDREQ_FLAG_REPLACE);
	if(hRequest)
		bResults = WinHttpAddRequestHeaders( hRequest,
													  s2ws(additionalheaders).c_str(),
													  -1,
													  WINHTTP_ADDREQ_FLAG_ADD);
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

	if(bResults){
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
	if (page == "") {
		char errString[20];
	    DWORD err = GetLastError();
		sprintf_s(errString,"%d",err);
		page.append(errString);
	}
	char* npOutString = (char *)npnfuncs->memalloc(page.size()+1);
	strcpy_s(npOutString,strlen(page.c_str())+1,page.c_str());
	STRINGZ_TO_NPVARIANT(npOutString,*result);
	if( hRequest ) WinHttpCloseHandle( hRequest );
    if( hConnect ) WinHttpCloseHandle( hConnect );
    if( hSession ) WinHttpCloseHandle( hSession );
	return true;
};