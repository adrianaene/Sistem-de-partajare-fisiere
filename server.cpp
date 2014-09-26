#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <dirent.h>
#include <time.h>

#define MAX_CLIENTS	10
#define BUFLEN 1024

using namespace std;

//creez o structura de date cu informatii despre clienti, utile serverului

typedef struct client_info
{
	int port;	// Portul de ascultare al clientului 
	int sockfd_server;	// Socket pe care e conectat la server clientul 
	string nume_client;	// Numele clientului 	
	struct sockaddr_in adresa_ip;	// Adresa clientului clientului 
	string folder;	//folderul ce contine posibile fisiere partajate
	time_t timp_conectare;	//Timpul de conectare la server 
	vector<string> fisiere_partajate;	//lista fisierelor partajate
	vector<string> dimensiuni;	//lista ce contine dimensiunile fisierelor partajate

} client_info;

//voi pune toti clientii conectati la server intr-un vector
vector<client_info> clienti;


//multimea de citire folosita in select()
fd_set read_fds;

//valoare maxima file descriptor din multimea read_fds
int sockfd,fdmax;

//functie folosita in cazul erorilor

void error(char *msg)
{
	perror(msg);
	exit(1);
}

// functie de prelucreaza mesajul sau comanda primita de la un client
 
void prelucrare_mesaj(int socket, char* buffer, struct sockaddr_in adresa_client)
{
	int port;
	string temp = string(buffer), mesaj, nume;
	int pozitie = temp.find_first_of(" ");
	const char* folder;
	
	//verific daca mesajul are mai multe cuvinte
	if (pozitie <= 0)
		mesaj = temp.substr(0, strlen(buffer));
    	else
	       	mesaj = temp.substr(0, pozitie);
           
	//daca un client se conecteaza retin datele lui
	if (mesaj.compare("client") == 0)
	{
		client_info client1;
	        
		temp = temp.substr(pozitie + 1, temp.size());
                pozitie = temp.find_first_of(" ");
		nume = temp.substr(0, pozitie);
            	temp = temp.substr(pozitie + 1, temp.size());
		pozitie = temp.find_first_of(" ");
		folder = temp.substr(0, pozitie).c_str(); 
		
		client1.folder = string(folder);              
		
		temp = temp.substr(pozitie + 1, temp.size());
		
		port = atoi(temp.c_str());
                client1.port = atoi(temp.c_str());
                client1.sockfd_server = socket;
		client1.nume_client = string(nume);
		client1.adresa_ip = adresa_client;
		client1.fisiere_partajate.clear();
		client1.port = port;		
                time (&client1.timp_conectare);
            
		int j;
		for (j = 0; j < clienti.size(); j++)
       		{
			//numele unui client e unic, nu pot fi duplicate
                	if (clienti[j].nume_client.compare(client1.nume_client) == 0)
                	{
				printf("Mai exista un client cu acest numele");
		                // Anunt clientul ca nu va fi mai fi conectat 
                    		memset(buffer, 0, BUFLEN);
				sprintf(buffer, "Numele de client este deja folosit\n");

                    		if (send(socket, buffer, strlen(buffer), 0) < 0)
                        		error((char *)"ERROR");
                    		close(socket);

                    		// Scot socketul din multimea de citire 
                    		FD_CLR(socket, &read_fds);
                    		return;
               		 }
            	}

		// Atentionez clientul in cazul in care legatura este buna
                memset(buffer, 0, BUFLEN);
                sprintf(buffer, "Conectat");
                
                if (send(socket, buffer, strlen(buffer), 0) < 0)
                	error((char *)"ERROR");
		
		//adaug clientul in vectorul de clienti
                clienti.push_back(client1);
                return;
	}

	//retin tipul comenzii pe care trebuie sa o prelucrez
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
	
	switch(type)
	{
	
		//procesare pentru comanda listclients 
        	case 1 :
		{
        		string list = "";
			memset(buffer, 0, BUFLEN);                
			
			//creez lista de clienti 
			int j;               
			for (j = 0; j < clienti.size(); j++)
                		list += clienti[j].nume_client + "\n";      	
            	
			// Trimit lista de clienti            
			if(send(socket, list.c_str(), strlen(list.c_str()), 0) < 0)
	        	        error((char*)"Eroare la comanda listclients");
	        	return;
			break;
    		}
		
		//procesare pentru comanda infoclient
		case 2 :    
		{
        		temp = temp.substr(pozitie + 1, temp.size());
			int exist = 0;
			int j;
			//verific daca clientul exista			
			for(j = 0; j < clienti.size(); j++)
			{
            			if(clienti[j].nume_client.compare(temp) == 0)
				{
                 			exist = 1;
                			break;
				}
			}
			
			//daca exista transmit numele, adresa IP, portul 
			//si timpul de conectare la server a clientului
			if (exist == 1)
			{	
				
				struct tm *tm;
				tm = localtime(&clienti[j].timp_conectare);
				char t[40];
				strftime(t, 40, "%I:%M:%S %p\n", tm);
				
				memset(buffer, 0, BUFLEN);
				sprintf(buffer, "%s %s %d %s",clienti[j].nume_client.c_str(), 
				inet_ntoa(clienti[j].adresa_ip.sin_addr), clienti[j].port, t);
				
				// Trimit informatiile despre client 
				if ( send(socket, buffer, strlen(buffer), 0) < 0)
					error((char *)"Eroare la comanda infoclient");
				return;
				
			}
			
			else
			{
				// atentionez ca nu exista clientul 
				memset(buffer, 0, BUFLEN);
				sprintf(buffer, "-1 : Client inexistent\n");
				if (send(socket, buffer, strlen(buffer), 0) < 0)
					error((char *)"Eroare infoclient(client inexistent)");

				return;	
			}
			
			return;
			break;
		}

		// procesare pentru comanda getshare
		case 3 :
		{
			 
        		int exist = 0;
        		temp = temp.substr(pozitie + 1, temp.size());
			int j;
			//verific daca exista clientul respectiv
        		for(j = 0; j < clienti.size(); j++)
        		{
            			if(clienti[j].nume_client.compare(temp) == 0)
            			{
                			exist = 1;
                			break;
            			}
		        }
			
			//in cazul in care lista de fisiere partajate este goala			
			if(clienti[j].fisiere_partajate.size() == 0)
			{
				memset(buffer, 0, BUFLEN);
				sprintf(buffer, "-2 : Fisiere inexistente\n");
				if (send(socket, buffer, strlen(buffer), 0) < 0)
					error((char *)"eroare la tipul de mesaj getshare");
				return;
			}
			
			//daca clientul exista trmit informatiile necesare
		        if (exist == 1)
			{
				string fisiere = "";
				memset(buffer, 0, BUFLEN);
				int i = 0;				
				for( i = 0; i < clienti[j].fisiere_partajate.size(); i++) 				
                			fisiere += clienti[j].fisiere_partajate[i] + " " 
					+ clienti[j].dimensiuni[i] + "\n";   	
            			// Trimit lista de fisiere partajate            
				if (send(socket, fisiere.c_str(), strlen(fisiere.c_str()), 0) < 0)
					error((char *)"eroare la tipul de mesaj getshare");
				return;			
			}

			else
			{
				// in cazul in care clientul nu exista
				memset(buffer, 0, BUFLEN);
				sprintf(buffer, "-1 : Client inexistent\n");
				if (send(socket, buffer, strlen(buffer), 0) < 0)
					error((char *)"Eroare la mesajul getshare");
				return;	
			}
						
			return;
			break;
    		}

		// procesare pentru comanda infofile <nume_fisier>
		case 4 :
		{
			int exist = 0;
			string fisiere = "";
			temp = temp.substr(pozitie + 1, temp.size());
			int poz = temp.find_first_of(" ");
			
			//in file retin numele fisierului primit ca parametru
			string file = temp.substr(0, poz);
			int j, k, nr = 0;
			memset(buffer, 0, BUFLEN);
			//verific daca exista fisierul
			for(k = 0; k < clienti.size(); k++)
			{
				for(j = 0; j < clienti[k].fisiere_partajate.size(); j++)
        			{
					if(clienti[k].fisiere_partajate[j].compare(file) == 0)
					{
						//daca am gasit fisierul, retin informatiile acestuia
						exist = 1; 
                				fisiere += clienti[k].nume_client + " " + 
						clienti[k].fisiere_partajate[j] +
						" " + clienti[k].dimensiuni[j] + "\n";   	
					}
				}
			}
		
			if ( exist == 0)
			{
				// in cazul in care fisierul nu exista
				memset(buffer, 0, BUFLEN);
				sprintf(buffer, "-2 : Fisier inexistent\n");
				if (send(socket, buffer, strlen(buffer), 0) < 0)
					error((char *)"Eroare la mesajul infofile");
				return;	
			}
			else 
			{
				//trimit informatiile legate de fisier 
				if (send(socket, fisiere.c_str(), strlen(fisiere.c_str()), 0) < 0)
					error((char *)"eroare la tipul de mesaj infofile");
				return;
			}
			return;

			break;
    		}			

		// procesare pentru comanda sharefile <nume_fisier>
		case 5 :
		{
			//verific daca exista clientul
			int exist = 0;
			int j;
        		for(j = 0; j < clienti.size(); j++)
        		{
            			if(clienti[j].sockfd_server == socket)
            			{
               				exist = 1;
                			break;
            			}
		        }

			if (exist == 1)
			{
				char msg[50];
				strcpy(msg, temp.c_str());

				char* file = strtok(msg, " ");
				file = strtok(NULL, " ");
				char* size = strtok(NULL, " ");
				
				//verific daca exista fisierul
				for(int i=0; i<clienti[j].fisiere_partajate.size(); i++)
				{
					if(strcmp(clienti[j].fisiere_partajate[i].c_str(), file) == 0)
					return;
				}

				//adaug dimensiunea si numele fisierului in lista
				clienti[j].fisiere_partajate.push_back(string(file));
				clienti[j].dimensiuni.push_back(string(size));
			}

			int i;
			string fisiere = "";
			for( i = 0; i < clienti[j].fisiere_partajate.size(); i++) 				
                			fisiere += clienti[j].fisiere_partajate[i] + " " 
					+ clienti[j].dimensiuni[i] + "\n";            			
			return;
		}
		
		//procesare pentru comanda unsharefile <nume_fisier>
		case 6 :
		{
			//verific daca exista clientul
			int exist = 0;
			int j;
        		for(j = 0; j < clienti.size(); j++)
        		{
            			if (clienti[j].sockfd_server == socket )
            			{
               				exist = 1;
                			break;
            			}
		      	}
			
			if (exist == 1)
			{
				int fisier_gasit = 0;
				int i;
				
				char msg[50];
				strcpy(msg, temp.c_str());

				char* file = strtok(msg, " ");
				file = strtok(NULL, " ");
				char* size = strtok(NULL, " ");

				//gasesc indexul fisierului
				for(i=0; i<clienti[j].fisiere_partajate.size(); i++){
					if(strcmp(clienti[j].fisiere_partajate[i].c_str(), file) == 0){
						fisier_gasit = 1;
						break;
					}
				}
				
				if (fisier_gasit == 1){	
					//sterg dimensiunea si numele fisierului din lista
					clienti[j].fisiere_partajate.erase(clienti[j].fisiere_partajate.begin() + i);
					clienti[j].dimensiuni.erase(clienti[j].dimensiuni.begin() + i);			
					// Trimit mesaj inapoi la client ca fisierul s-a unshare-uit cu succes
					char raspuns[50];
					memset(raspuns, 0, 50);
					strcpy(raspuns, "Succes\n");
					send(socket, raspuns, strlen(raspuns), 0);				
					return;
				}
				else 
				{
					char raspuns[50];
					memset(raspuns, 0, 50);
					strcpy(raspuns, "-2 : Fisier inexistent pe server\n");
					send(socket, raspuns, strlen(raspuns), 0);
					return;
				}
					
			}
			else
				error((char *)"-1 : Clientul care unshare-uie nu exista");
			return;
			break;
		}

		//procesare pentru comanda getfile < nume_client > < nume_fisier >
		case 7 : 
		{
			
			int exist = 0;
			int j;
			temp = temp.substr(pozitie + 1, temp.size());
			int poz = temp.find_first_of(" ");
			//in file retin numele clientului
			string nume = temp.substr(0, poz);
			//in temp se va retine numele fisierului				
			temp = temp.substr(pozitie + 1, temp.size());
        	
			//verific daca exista clientul
			for(j = 0; j < clienti.size(); j++)
        		{
            			if (clienti[j].nume_client.compare(nume) == 0)
            			{
               				exist = 1;
                			break;
            			}
		      	}
			
			if (exist == 1)
			{
				int fisier_gasit = 0;
				int l;
				
				for ( l = 0; l < clienti[j].fisiere_partajate.size(); l++)
				{
					if (clienti[j].fisiere_partajate[l].compare(temp) == 0)
					{
						fisier_gasit = 1;
						break;
					}
				}
				
				if (fisier_gasit == 1)
				{	
					//trimit inapoi informatiile de conectare la client
					//ip si port"				
					
					memset(buffer, 0, BUFLEN);
					sprintf(buffer, " %s %d\n",
					inet_ntoa(clienti[j].adresa_ip.sin_addr), clienti[j].port);
					
					if (send(socket, buffer, strlen(buffer), 0) < 0)
						error((char *)"Eroare la mesajul de tip getfile");
					return;
				}
				else 
				{
					memset(buffer, 0, BUFLEN);
					sprintf(buffer, "%s", "-2 : Fisier inexistent\n");
					if (send(socket, buffer, strlen(buffer), 0) < 0)
						error((char *)"Eroare la mesajul de tip getfile");
					return;
				}
					
			}
			else
				error((char *)"-1 : Client inexistent\n");
			return;
			break;		
		}
	}
}

// functie ce prelucreaza comanda primita de la tastatura 
void prelucrare_comanda(char* buffer)
{
    	string comanda;
    	string temp = string(buffer);
	int pozitie = temp.find_first_of(" ");
	
	if(pozitie <= 0)
    		comanda = temp.substr(0, strlen(buffer) - 1);
    	else
		comanda = temp.substr(0, pozitie);       
   	int j;
	// cazul in care am primit comanda status 
        if(comanda.compare("status") == 0)
    	{
		for(j = 0; j < clienti.size(); j++)
	        {
	        	// afisez informatiile legate de clienti 
	        	printf("nume_client: %s\n", clienti[j].nume_client.c_str());
			printf("adresa_ip: %s\n", inet_ntoa(clienti[j].adresa_ip.sin_addr));
	        	printf("port: %d\n", clienti[j].port);	            	
	            	printf("\n");
	        }
        	return;
    	}
    
    	// cazul in care am primit comanda quit
    	if(comanda.compare("quit") == 0)
    	{
    		printf("Serverul a fost deconectat\n");
    		for ( j = 1; j <= fdmax; ++j)
			if (FD_ISSET(j, &read_fds))
			{
				memset(buffer, 0, BUFLEN);
				sprintf(buffer, "deconectat");
				// anunt clientului ca serverul a fost desconectat 
				if (send(j, buffer, strlen(buffer), 0) < 0)
					 error((char *)"ERROR: Serverul a fost deconectat!");
				close(j);
				//Scot socketul din multimea de citire 
				FD_CLR(j, &read_fds);
			}
		close(sockfd);
		exit(0);
    	}
    	printf("Comanda tastata este gresita.\n\n");
    	return;
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno, clilen;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, j;

	fd_set tmp_fds;		//multime folosita temporar
	if (argc < 2) {
		fprintf(stderr,"Usage : %s port\n", argv[0]);
		exit(1);
	}

	//golim multimea de descriptori de citire (read_fds) si multimea tmp_fds
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	//creare socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error((char*)"ERROR opening socket");

	portno = atoi(argv[1]);

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
	serv_addr.sin_port = htons(portno);

	//bind socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0)
		error((char*)"ERROR on binding");

	//ascultare pe socket
	listen(sockfd, MAX_CLIENTS);

	//adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfd, &read_fds);
	FD_SET(0, &read_fds);
	fdmax = sockfd;

	while (1)
	{
		tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1)
			error((char *)"ERROR in select");
		
		for(i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &tmp_fds))
			{
				if (i == sockfd)
				{
					// a sosit ceva pe socketul inactiv(cel cu listen) pentru o noua conexiune
					// verific accept()					
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen)) == -1)
						error((char *)"ERROR in accept");
					else
					{
						// Adaug noul socket la multimea descriptorilor de citire 
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax)
							fdmax = newsockfd;
					}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);					
                    			
				}
				
				else if (i == 0)
				{
					// citesc comanda primita de la tastatura 
					memset(buffer, 0 , BUFLEN);
					fgets(buffer, BUFLEN-1, stdin);
					// prelucrez comanda 
					prelucrare_comanda(buffer);
				}

				else
				{
					// am primit date de la clienti prin socketi 
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0)
					{
						if (n == 0)
						{
							// in cazul in care conexiunea s-a incheiat
							for (int j = 0; j < clienti.size(); j++)
								if (clienti[j].sockfd_server == i)
									printf("clientul %s de pe socket %d s-a deconectat\n", clienti[j].nume_client.c_str(), i);
						}
						else
							error((char *)"ERROR in recv");
						
						//sterg din lista de clienti, clientul care s-a deconectat
						for (int j = 0; j < clienti.size(); j++)
						{
							if (clienti[j].sockfd_server == i)
								clienti.erase(clienti.begin()+j);
						}
						close(i);
						
						// scot socketul din multimea de citire  
						FD_CLR(i, &read_fds);
					}

					else
					{   	//recv intoarce > 0
						//prelucrez mesajele sosite de la clienti
						 
						prelucrare_mesaj(i, buffer, cli_addr);
                        			printf ("Am primit un mesaj de la clientul %s\n",clienti[i - 4].nume_client.c_str());
					}
				}
			}
		}
	}
	
	close(sockfd);
	return 0;
}
