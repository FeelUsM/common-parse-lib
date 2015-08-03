#include <string>
#include <stdio.h>
#include "base_parse.h"
int main()
{
	char s[]="qwerty!asdfgh";
	const char * p=s;
	std::string str;
	read::until_char(&p,'!',&str);
	printf("%s\n",str.c_str());
}