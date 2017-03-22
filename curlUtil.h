#ifndef CURLUTIL_H
#define CURLUTIL_H
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <curl/curl.h>
using namespace std;
struct MemoryStruct {
  char *memory;
  size_t size;
};

class curlUtil {

	string site;
	string data;	
	public:	
	curlUtil(string s) {
		site = s;
	}

	static size_t
	WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
  		size_t realsize = size * nmemb;
  		struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  		mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1);
  		if(mem->memory == NULL) {
    		/* out of memory! */
    		printf("not enough memory (realloc returned NULL)\n");
    		return 0;
  		}

  		memcpy(&(mem->memory[mem->size]), contents, realsize);
		
  		mem->size += realsize;
  		mem->memory[mem->size] = 0;

  		return realsize;
	}
	void get_curl() {
		CURL *curl_handle;
  		CURLcode res;
 
  		struct MemoryStruct chunk;
 		chunk.memory = (char *)malloc(1);  /* will be grown as needed by the realloc above */
                chunk.size = 0;    /* no data at this point */
		curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  		curl_handle = curl_easy_init();

  /* specify URL to get */
  		curl_easy_setopt(curl_handle, CURLOPT_URL, site.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
  /* send all data to this function  */
  		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  /* some servers don't like requests that are made without a user-agent
 *  *      field, so we provide one */
  		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* get it! */
  		res = curl_easy_perform(curl_handle);

  		/* check for errors */
  		if(res != CURLE_OK) {
    		fprintf(stderr, "curl_easy_perform() failed: %s\n",
            		curl_easy_strerror(res));
  		}
  		else {
  			
  			char* a = chunk.memory;
			string s(a);
			data = s;
		}
  		/* cleanup curl stuff */
		curl_easy_cleanup(curl_handle);
		free(chunk.memory);
		curl_global_cleanup();
	}
	string get_data() {
		return data;
	}
};
 

#endif
