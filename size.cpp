#include<stdio.h>
#include<math.h>
#include<string.h>
#include <sys/stat.h>
char* printsize(int  size)
{                   
    static const char *SIZES[] = { "B", "KB", "MB", "GB" };
    int div = 0;
    int rem = 0;

    while (size >= 1024 && div < (sizeof SIZES / sizeof *SIZES)) {
        rem = (size % 1024);
        div++;   
        size /= 1024;
    }
	double integer, fractional, number;
	number = (float)size + (float)rem / 1024.0;
   	
	fractional = modf(number, &integer);
	char charray[100];
	sprintf(charray, "%g", integer);
	
	char* fo = new char[100];
	strcpy(fo, charray);
	strcat(fo, (char*)SIZES[div]);
	return fo;
}
int main(){
	
	struct stat st; 
	int size;	
	stat("client1.log", &st);
	size = (int)st.st_size;
				
	printf("%s\n", printsize(size));
	return 0;
}
