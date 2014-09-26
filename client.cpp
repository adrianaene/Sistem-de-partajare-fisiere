#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <math.h>
#define MAX_CLIENTS 10
#define BUFLEN 1024

using namespace std;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

FILE* file;

//functie ce imi transforma dimensiunea in format user-friendly
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
// functie ce prelucreaza comanda primita de la tastatura 
void prelucrare_comanda(int socket, char* buffer, char* nume_client, char* nume_folder)
{
	int n;
	//printf("%s", nume_folder);
	string mesaj;
    	string temp = string(buffer);
    	int pozitie = temp.find_first_of(" ");
    	
        if(pozitie <= 0)
		mesaj = temp.substr(0, temp.size() - 1);
	else
	        mesaj = temp.substr(0, pozitie);
	
	int type = 0;
	if (mesaj.compare("listclients") == 0)
		type = 1;
	else if (mesaj.compare("infoclient") == 0)
		type = 2;
	else if (mesaj.compare("getshare") == 0)
		type = 3;
	else if (mesaj.compare("infofile") == 0)
		type = 4;
	else if (mesaj.compare("sharefile") == 0)
		type = 5;
	else if (mesaj.compare("unsharefile") == 0)
		type = 6;
	else if (mesaj.compare("getfile") == 0)
		type = 7;
	else if (mesaj.compare("quit") == 0)
		type = 8; 
	
	switch(type)
	{
	
		//procesare pentru comanda listclients 
        	case 1 :
		{
        		temp = temp.substr(mesaj.size() + 1, temp.size());
            		if(temp.size() > 0)
            		{
                		printf("Comanda eronata. Usage: listclients\n");
                		return;
            		}
			
			// trimit catre server mesajul listclients 
            		char* mesaj_send = (char*)malloc(BUFLEN*sizeof(char));
            		sprintf(mesaj_send, "listclients");
            		if (send(socket, mesaj_send, strlen(mesaj_send), 0) < 0)
                		error((char *)"ERROR writing to socket");
            		free(mesaj_send);

            		// primesc raspunsul de la server 
            		memset(buffer, 0, BUFLEN);
			if ((n = recv(socket, buffer, BUFLEN, 0)) <= 0)
            		{
                		if (n != 0)
					error((char *)"ERROR in recv");
				else
                    			printf("client: conexiunea s-a inchis\n");
                		close(socket);
                		error((char *)"Problem in connection with server.");
            		}
            		
			else
            		{
            		    	// afisez lista de clienti 
				fprintf(file, "> listclients\n");
				fprintf(file, "%s\n", buffer);
				fflush(file); 
				
				fflush(stdin);               		
                		printf("%s",buffer);
                		printf("\n");
				fflush(stdin);
            		}
			return;
			break;
		}

		//procesare pentru comanda infoclient < nume_client > 
		case 2 :
		{
			string info_client;
        		temp = temp.substr(mesaj.size(), temp.size()-1);
		
			if(temp.find_first_of(" ") == -1)            	
			{
                		printf("Comanda eronata. Usage: infoclient nume_client\n");
            			return;			
            		}
		
			else
			{
				info_client = temp.substr(1, temp.size() - 2);
            			// trimit server-ului mesajul "infoclient "nume_client"  
            			char* message_server = (char*)malloc(BUFLEN*sizeof(char));
            			sprintf(message_server, "infoclient %s", info_client.c_str());
            			if (send(socket, message_server, strlen(message_server), 0) < 0)
                			error((char *)"ERROR writing to socket");
            			free(message_server);
            			
				// primesc informatii de la server 
            			memset(buffer, 0, BUFLEN);
				if ((n = recv(socket, buffer, BUFLEN, 0)) <= 0)
            			{
                		if (n != 0)
					error((char *)"ERROR in recv");
				else
					// cazul in care conexiunea s-a inchis 
                    			printf("client: conexiunea s-a inchis\n");
                		close(socket);
                		error((char *)"Problem in connection with server.");
				}
				
				else
				{	// printez informatiile despre client 
					fprintf(file, "%s\n", ("> " + string(message_server)).c_str());
					fprintf(file, "%s\n", buffer);
					fflush(file);

					// afisez informatiile si la consola
					fflush(stdin); 
                			printf("%s\n",buffer);
					fflush(stdin); 
				}
				return;	
            		}
			return;
			break;
		}
		
		//procesare pentru comanda getshare < nume_client > 
		case 3 :
		{
			string info_client;
        		temp = temp.substr(mesaj.size(), temp.size()-1);
		
			if(temp.find_first_of(" ") == -1)            	
			{
                		printf("Comanda eronata. Usage: getshare < nume_client > \n");
            			return;			
            		}
		
			else
			{
				info_client = temp.substr(1, temp.size() - 2);
            			// trimit server-ului mesajul "infoclient "nume_client"  
            			char* message_server = (char*)malloc(BUFLEN*sizeof(char));
            			sprintf(message_server, "getshare %s", info_client.c_str());
            			if (send(socket, message_server, strlen(message_server), 0) < 0)
                			error((char *)"ERROR writing to socket");
            			free(message_server);
            			
				// primesc informatii de la server 
            			memset(buffer, 0, BUFLEN);
				if ((n = recv(socket, buffer, BUFLEN, 0)) <= 0)
            			{
                		if (n != 0)
					error((char *)"ERROR in recv");
				else
					// cazul in care conexiunea s-a inchis 
                    			printf("client: conexiunea s-a inchis\n");
                		close(socket);
                		error((char *)"Problem in connection with server.");
				}
				
				else
				{
					// printez informatiile despre client 
					fprintf(file, "%s\n",  ("> " + string(message_server)).c_str());
					fprintf(file, "%s\n", buffer);
					fflush(file);

					//  afisez informatiile si la consola
					fflush(stdin); 
                			printf("%s\n",buffer);
					fflush(stdin); 
				}
				return;	
            		}
			return;
			break;
		}
		
		//procesare pentru comanda infofile < nume_fisier >
		case 4 :
		{
			string info_client;
        		temp = temp.substr(mesaj.size(), temp.size()-1);
		
			if(temp.find_first_of(" ") == -1)            	
			{
                		printf("Comanda eronata. Usage: infofile < nume_fisier >\n");
            			return;			
            		}
		
			else
			{
				info_client = temp.substr(1, temp.size() - 2);
            			// trimit server-ului mesajul "infofile "nume_fisier"  
            			char* message_server = (char*)malloc(BUFLEN*sizeof(char));
				
            			sprintf(message_server, "infofile %s", info_client.c_str());
            			if (send(socket, message_server, strlen(message_server), 0) < 0)
                			error((char *)"ERROR writing to socket");
            			free(message_server);
            			
				// primesc informatii de la server 
            			memset(buffer, 0, BUFLEN);
				if ((n = recv(socket, buffer, BUFLEN, 0)) <= 0)
            			{
                		if (n != 0)
					error((char *)"ERROR in recv");
				else
					// cazul in care conexiunea s-a inchis 
                    			printf("client: conexiunea s-a inchis\n");
                		close(socket);
                		error((char *)"Problem in connection with server.");
				}
				
				else
				{
					// printez informatiile despre client 
					fprintf(file, "%s %s\n", "> infofile", info_client.c_str());
					fprintf(file, "%s\n", buffer);
					fflush(file);

					// afisez informatiile si la consola
					fflush(stdin); 
                			printf("%s\n",buffer);
					fflush(stdin); 
				}
				return;	
            		}
			return;
			break;
		}

		//procesare pentru comanda sharefile < nume_fisier >
		case 5 :
		{
			string info_client;
        		temp = temp.substr(mesaj.size(), temp.size()-1);
		
			if(temp.find_first_of(" ") == -1)            	
			{
                		printf("Comanda eronata. Usage: sharefile < nume_fisier >\n");
            			return;			
            		}
		
			else
			{
				info_client = temp.substr(1, temp.size() - 2);
            			// trimit server-ului mesajul "sharefile nume_fisier dimensiune_fisier"  
            			char* message_server = (char*)malloc(BUFLEN*sizeof(char));
				
				struct stat st; 
				int size;
				
				char* path = new char[100];
				strcpy(path, nume_folder);
				strcat(path,"/");
				strcat(path, info_client.c_str());
				
				int file_exists = stat(path, &st);
				
				size = (int)st.st_size;

            			sprintf(message_server, "sharefile %s %s", info_client.c_str(), printsize(size));
            			
				//afisez daca nu exista fisierul
				if(file_exists == -1)
				{
					fprintf(file, "%s %s\n", "> sharefile", info_client.c_str());
					fprintf(file, "-2 : Fisier inexistent\n\n");
					fflush(file);

					// afisez rezultatul si la consola
					fflush(stdin); 
                			printf("-2 : Fisier inexistent\n\n");
					fflush(stdin);					
					return;
				} 
				//trimit serverului informatii ca sa stie ca exista fisierul
				//si afisez Succes
				else
				{
					if(send(socket, message_server, strlen(message_server), 0) < 0)
                				error((char *)"ERROR writing to socket");
            				free(message_server);					

					fprintf(file, "%s %s\n", "> sharefile", info_client.c_str());
					fprintf(file, "Succes\n\n");
					fflush(file);

					// afisez rezultatul si la consola
					fflush(stdin); 
                			printf("Succes\n\n");
					fflush(stdin);					
					return;
				}
            			
				return;	
            		}
			return;
			break;
		}
		//procesare pentru comanda unsharefile <nume_fisier>
		case 6 :
		{
			string info_client;
        		temp = temp.substr(mesaj.size(), temp.size()-1);
		
			if(temp.find_first_of(" ") == -1)            	
			{
                		printf("Comanda eronata. Usage: unsharefile < nume_fisier >\n");
            			return;			
            		}
		
			else
			{
				info_client = temp.substr(1, temp.size() - 2);
            			// trimit server-ului mesajul "unsharefile nume_fisier"  
            			char* message_server = (char*)malloc(BUFLEN*sizeof(char));

				struct stat st; 
				char* path = new char[100];
				strcpy(path, nume_folder);
				strcat(path,"/");
				strcat(path, info_client.c_str());
				
				int file_exists = stat(path, &st);
				
				if(file_exists == -1)
				{
					fprintf(file, "> unsharefile %s\n", info_client.c_str());
					fprintf(file, "-2 : Fisier inexistent\n\n");
					fflush(file);

					// afisez rezultatul si la consola
					fflush(stdin); 
                			printf("-2 : Fisier inexistent\n\n");
					fflush(stdin);					
					return;
				} 

				//trimit serverului informatii ca sa stie ca exista fisierul
				
				else
				{
					sprintf(message_server, "unsharefile %s", info_client.c_str());
	            			if (send(socket, message_server, strlen(message_server), 0) < 0)
	                			error((char *)"ERROR writing to socket");
	            			free(message_server);
					
					char buffer[50];
					memset(buffer, 0, 50);
					recv(socket, buffer, sizeof(buffer), 0); 

					fprintf(file, "> unsharefile %s\n", info_client.c_str());
					fprintf(file, "%s\n", buffer);
					fflush(file);

					// afisez rezultatul si la consola
					fflush(stdin); 
                			printf("%s\n", buffer);
					fflush(stdin);					
					return;
				}            			
            		}
			return;
			break;
		}
		
		// procesare pentru comanda quit
		case 8 :
		{
			printf("Clientul %s s-a deconectat de la server\n",nume_client);
			close(socket);
			exit(0);
			break;
		}

		default :
		{
			printf("Comanda eronata.\n\n");
			return;
    			break;
		}
	}
	fclose(file);
}

int main(int argc, char *argv[])
{
    	int n, accept_len, sockfd, newsockfd;
    	struct sockaddr_in serv_addr, listen_client_addr, accept_addr;
   	struct hostent *server;
	int fdmax;
   
	fd_set read_fds;	// multimea de citire folosita in select() 
    	fd_set tmp_fds;		// multime folosita temporar 

    	char buffer[BUFLEN];
    
	if (argc != 6) {
		fprintf(stderr,"Usage %s <nume_client> <nume_director> <port_client> <ip_server> <port_server>\n", argv[0]);
		exit(0);
	}
    
	// socket folosit pentru conectarea la server 
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error((char*)"ERROR opening socket");
    
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
    	
	// portul serverului 
    	serv_addr.sin_port = htons(atoi(argv[5]));
    	// adresa IP a serverului 
    	inet_aton(argv[4], &serv_addr.sin_addr);

    	if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0)
        	error((char*)"ERROR connecting");

    	// socket pentru ascultare conexiuni de la alti clienti 
	int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0)
		error((char *)"ERROR opening listening socket at client");

	memset((char *) &listen_client_addr, 0, sizeof(listen_client_addr));
	listen_client_addr.sin_family = AF_INET;
	
	// portul clientului 
	listen_client_addr.sin_port = htons(atoi(argv[3]));
	// adresa IP a masinii 
	listen_client_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listen_sock, (struct sockaddr *) &listen_client_addr, sizeof(struct sockaddr)) < 0)
		error((char *)"ERROR on binding");

	listen(listen_sock, MAX_CLIENTS);

	memset(buffer, 0, BUFLEN);
	
	char* folder = new char[100];
	strcpy(folder,argv[2]);
	// trimit informatia cu numele si portul clientului 
	sprintf(buffer,"client %s %s %d", argv[1], argv[2], atoi(argv[3]));
	
	if (send(sockfd,buffer,strlen(buffer), 0) < 0)
		 error((char *)"ERROR writing to socket");

	memset(buffer, 0, BUFLEN);
	
	if ((n = recv(sockfd, buffer, sizeof(buffer), 0)) <= 0)
	{
		if (n != 0)
			error((char *)"ERROR in recv");			
		else
			// conexiunea s-a inchis 
			printf("client: conexiune inchisa\n");
		
		close(sockfd);
		error((char *)"Problem in connection with server.");
	}
	else
	{
		
		if (strncmp(buffer,"disconnected",strlen("disconnected")) == 0)
		{
			// daca am primit instiintare de deconectare 
			printf("Client %s\n",buffer);
			close(sockfd);
			exit(0);
		}
		// daca am primit instiintare de conectare 
		printf("Client %s cu succes la server\n",buffer);	
	}
    	// Golim multimea de descriptori de citire (read_fds) si multimea (tmp_fds) 
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	// Adaug stdin in multimea read_fds 
	FD_SET(0, &read_fds); //aici
	// Adaug socketul pe care e conectat clientul la server in multimea read_fds 
	FD_SET(sockfd, &read_fds);
	// Adaug socketul pe care se asculta la read_fds 
	FD_SET(listen_sock, &read_fds);
	fdmax = listen_sock;

   	while(1)
	{
		tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1)
			error((char *)"ERROR in select");

		for (int i = 0; i <= fdmax; ++i)
		{
			if (FD_ISSET(i, &tmp_fds))
			{
				if (i == sockfd) 
				{
					// primesc mesaj de la server 
					memset(buffer, 0, BUFLEN);
					fflush(stdin);
					if ((n = recv(sockfd, buffer, sizeof(buffer), 0)) <= 0)
					{
						if (n != 0)
							error((char *)"ERROR in recv");
						
						else
						{
							printf("Deconectat de server\n");
							close(sockfd);
                            				close(listen_sock);
                            				exit(0);						
						}
					}					
				}
				
				else if (i == listen_sock)
				{			
					//primesc mesaj pe socketul de ascultare si astept accept 
					accept_len = sizeof(accept_addr);
					if ((newsockfd = accept(listen_sock, (struct sockaddr *)&accept_addr, (socklen_t *)&accept_len)) == -1)
						error((char *)"ERROR in accept");
					
					else
					{
						// adaug noul socket la multimea descriptorilor de citire 
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax)
							fdmax = newsockfd;
					}
				}
				
				else if (i == 0)
				{
					// citesc comanda de la tastatura 
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN-1, stdin);
					// prelucrez comanda primita 
					char* fo = new char[40];
					strcpy(fo, argv[1]);
					strcat(fo,".log");
					file = fopen(fo, "a+");
					prelucrare_comanda(sockfd, buffer, argv[1], folder);
				}
				
				else
				{
					// daca am primit mesaj de la alt client 
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0)
					{
                        			// inchid socketul folosit din read_fds 
						close(i);
						FD_CLR(i, &read_fds);
					}
				}
			}
		}

	}
	
	close(listen_sock);
	close(sockfd);
	return 0;
}
