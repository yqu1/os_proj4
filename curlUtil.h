#ifndef CURLUTIL_H
#define CURLUTIL_H

#include <curl/curl.h>
#include <string.h>
using namespace std;

class curlUtil {
	string data;
	size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up) {
    		data.clear();
    		for (size_t c = 0; c<size*nmemb; c++)
    		{
        		data.push_back(buf[c]);
    		}
    		return size*nmemb;
	}
	public:
		
}

void get_curl(string site) {
        CURL* curl;
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
     
      


#endif
