#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <dirent.h>
int main(){	
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir ("tema/Folder2")) != NULL) {
  	/* print all the files and directories within directory */
 	 while ((ent = readdir (dir)) != NULL) {
    		printf ("%s\n", ent->d_name);
  	}
  	closedir (dir);
	} else {
  	/* could not open directory */
  	perror ("");
  	return EXIT_FAILURE;
	}

	return 0;
}
