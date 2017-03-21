#include <iostream>
#include "curlUtil.h"
#include <string>
using namespace std;
int main() {
	string site = "https://www.nd.edu";
	curlUtil c(site);
	c.get_curl();
	cout << c.get_data() << endl;
}
