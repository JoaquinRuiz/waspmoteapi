/*
 *  Copyright (C) 2012 Libelium Comunicaciones Distribuidas S.L.
 *  http://www.libelium.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
   
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Version:		0.1
 *  Design:		David Gascón
 *  Implementation:	Alejandro Gállego
 */
  

#ifndef __WPROGRAM_H__
//  #include <WProgram.h>
  #include "WaspClasses.h"
#endif

// Constructors ////////////////////////////////////////////////////////////////

WaspGPRS_Pro::WaspGPRS_Pro()
{
	_baudRate=GPRS_PRO_RATE;
	_pwrMode=GPRS_PRO_ON;
	_uart=1;
	not_ready=1;
}


// Private Methods /////////////////////////////////////////////////////////////

/* getIP() - Gets IP direction when configure a TCP/UDP profiles
 *
 * This function gets IP direction when configure a TCP/UDP profiles and stores it in 'IP_direction'
 *
 * Returns '1' on success and '0' if error
*/
uint8_t WaspGPRS_Pro::getIP(){
	int i;
	
	sprintf(theCommand, "AT%s\r\n",AT_IP_GET_IP);
	printString(theCommand,_uart);
	do{
		while(!serialAvailable(_uart));
	}while(serialRead(_uart)!='\n');
	do{
		while(!serialAvailable(_uart));
	}while(serialRead(_uart)!='\n');
	do{
		while(!serialAvailable(_uart));
	}while(serialRead(_uart)!='\n');
	i=0;
	do{
		while(!serialAvailable(_uart));
		IP_dir[i]=serialRead(_uart);
		i++;
	}while(IP_dir[i-1]!='\r');	
	IP_dir[i-1]='\0';
		
	if(IP_dir[0]=='E') return 0;
	
	return 1;
}

uint8_t WaspGPRS_Pro::parse(const char* data, const char* data_expected){
	
	int it=0;
	bool theSame=false;
	int cont=0;
	int cont2=0;
	int ok;
	
	while( data[it]!='\0' ) it++;

	while(cont<it)	
	{
		if (data[cont]==data_expected[cont2])
		{
			cont2=0;
			while(data_expected[cont2]!='\0')
			{
				if (data[cont+cont2]==data_expected[cont2])
				{    
					cont2++;
					ok=true; 
				}
				else
				{
					ok=false; 
					break;
				}
			}  
			if(ok)
			{
				return 1;
			} 
		}
		cont++;
		cont2=0;
	}	
	return 0;
}

/* getIfReady() - gets if GPRS module is ready or not
 *
 * This function gets if GPRS module is ready or not
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns nothing. It changes the value of 'not_ready'
*/
void WaspGPRS_Pro::getIfReady(){
	uint8_t answer=0;
	
	printString(AT_COMMAND,_uart);
	printByte('\r',_uart);
	printByte('\n',_uart);
	delay(10);
	answer=waitForData("OK",2,0,0);
	if(answer==1) not_ready=0;
	else not_ready=1;
}

/* checkGPRS() - checks if GPRS connection is OK
 *
 * This function checks if GPRS connection is OK
 *
 * Returns '1' on success and '0' if error
*/
uint8_t WaspGPRS_Pro::checkGPRS(){

	uint8_t answer=0;
	
	answer=sendATCommand(AT_GPRS_CHECK,AT_GPRS_CHECK_ON,AT_GPRS_CHECK_OFF);

	if(answer!=1) return 0;
	return 1;
}

/* checkIPstatus() - get the status of the IP connection
 *
 * This function gets the status of the IP connection
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns a number with the state
*/
uint8_t WaspGPRS_Pro::checkIPstatus(){
	uint8_t answer=10;
	char	ip_status[50];
	int it=0;
	
	for (int i = 0; i < 50; i++) ip_status[i] = '\0';
	
	flag &= ~(GPRS_ERROR_SOCKET);
		
	answer=sendATCommand(AT_IP_STATUS,AT_IP_STATUS_R);
	
	if(answer==1){
		do{
			ip_status[it]=serialRead(1);
			it++;
		}while(ip_status[it-1]!='\n');
		if(waitForData(ip_status,"IP INITIAL")) return 1;
		if(waitForData(ip_status,"IP START")) return 2;
		if(waitForData(ip_status,"IP CONFIG")) return 3;
		if(waitForData(ip_status,"IP GPRSACT")) return 4;
		if(waitForData(ip_status,"IP STATUS")||strcmp(ip_status,"IP PROCESSING")) return 5;
		if(waitForData(ip_status,"TCP CONNECTING")||strcmp(ip_status,"UDP CONNECTING")||strcmp(ip_status,"SERVER LISTENING")) return 6;
		if(waitForData(ip_status,"CONNECT OK")) return 7;
		if(waitForData(ip_status,"TCP CLOSING")||strcmp(ip_status,"UDP CLOSING")) return 8;
		if(waitForData(ip_status,"TCP CLOSED")||strcmp(ip_status,"UDP CLOSED")) return 9;
		if(waitForData(ip_status,"PDP DEACT")) return 10;
		answer=0;
	}
	
	switch(answer)
	{
		case 0:
			flag |= GPRS_ERROR_SOCKET;
			return 0;
			break;
	}
	if(flag & GPRS_ERROR_SOCKET) return 0;
	return 11;
}


// AT Comands ///////////////////////////////////////////////////////////////////

byte WaspGPRS_Pro::sendATCommand(const char* ATcommand, const char* expectedAnswer){
    return sendATCommand(ATcommand, expectedAnswer, 0);
}
  
byte WaspGPRS_Pro::sendATCommand(const char* ATcommand, const char* expectedAnswer, int sendOnce) {
  // command style: +CMGS=0736584317
  // this means, we gotta add the "AT"

    sprintf(theCommand, "AT%s", ATcommand);
    return sendCommand(theCommand, "\r\n", expectedAnswer, sendOnce);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, int endOfCommand, const char* expectedAnswer) {
    return sendCommand(theText, endOfCommand, expectedAnswer, 0);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, int endOfCommand, const char* expectedAnswer, int MAX_TIMEOUT, int sendOnce) {
    sprintf(theEnd, "%c", endOfCommand);
    return sendCommand(theText, theEnd, expectedAnswer, MAX_TIMEOUT, sendOnce);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, int endOfCommand, const char* expectedAnswer, int sendOnce) {
    sprintf(theEnd, "%c", endOfCommand);
    return sendCommand(theText, theEnd, expectedAnswer, sendOnce);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, char endOfCommand, const char* expectedAnswer) {
    return sendCommand(theText, (int) endOfCommand, expectedAnswer, 0);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, char endOfCommand, const char* expectedAnswer, int MAX_TIMEOUT) {
    return sendCommand(theText, (int) endOfCommand, expectedAnswer, MAX_TIMEOUT, 0);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, char endOfCommand, const char* expectedAnswer, int MAX_TIMEOUT, int sendOnce) {
    return sendCommand(theText, (int) endOfCommand, expectedAnswer, MAX_TIMEOUT, sendOnce);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, const char* endOfCommand, const char* expectedAnswer) {
    return sendCommand(theText, endOfCommand, expectedAnswer, DEFAULT_TIMEOUT, 0);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, const char* endOfCommand, const char* expectedAnswer, int sendOnce) {
    return sendCommand(theText, endOfCommand, expectedAnswer, DEFAULT_TIMEOUT, sendOnce);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, const char* endOfCommand, const char* expectedAnswer, int MAX_TIMEOUT, int sendOnce) {
    int timeout = 0;

    int length=sprintf(theCommand, "%s%s", theText,endOfCommand);
    

  // try sending the command
  // wait for serial response
    timeout = 0;
    serialFlush(_uart);
    while(!serialAvailable(_uart) && timeout < MAX_TIMEOUT) {
        if (!sendOnce || !timeout) {
            printString(theCommand,_uart);
        }
        delay(DELAY_ON_SEND);
        timeout++;
    };

    int answer= waitForData( expectedAnswer, MAX_TIMEOUT, timeout, 0);
    
    return answer;
}


//functions with two answers

byte WaspGPRS_Pro::sendATCommand(const char* ATcommand, const char* expectedAnswer1, const char* expectedAnswer2) {
    return sendATCommand(ATcommand, expectedAnswer1, expectedAnswer2, 0);
}

byte WaspGPRS_Pro::sendATCommand(const char* ATcommand, const char* expectedAnswer1, const char* expectedAnswer2, int sendOnce) {
  // command style: +CMGS=0736584317
  // this means, we gotta add the "AT"

    sprintf(theCommand, "AT%s", ATcommand);
    return sendCommand(theCommand, "\r\n", expectedAnswer1, expectedAnswer2, sendOnce);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, int endOfCommand, const char* expectedAnswer1, const char* expectedAnswer2) {
    return sendCommand(theText, endOfCommand, expectedAnswer1, expectedAnswer2, 0);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, int endOfCommand, const char* expectedAnswer1, const char* expectedAnswer2, int MAX_TIMEOUT, int sendOnce) {
    sprintf(theEnd, "%c", endOfCommand);
    return sendCommand(theText, theEnd, expectedAnswer1, expectedAnswer2, MAX_TIMEOUT, sendOnce);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, int endOfCommand, const char* expectedAnswer1, const char* expectedAnswer2, int sendOnce) {
    sprintf(theEnd, "%c", endOfCommand);
    return sendCommand(theText, theEnd, expectedAnswer1, expectedAnswer2, sendOnce);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, char endOfCommand, const char* expectedAnswer1, const char* expectedAnswer2) {
    return sendCommand(theText, (int) endOfCommand, expectedAnswer1, expectedAnswer2, 0);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, char endOfCommand, const char* expectedAnswer1, const char* expectedAnswer2, int MAX_TIMEOUT) {
    return sendCommand(theText, (int) endOfCommand, expectedAnswer1, expectedAnswer2, MAX_TIMEOUT, 0);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, char endOfCommand, const char* expectedAnswer1, const char* expectedAnswer2, int MAX_TIMEOUT, int sendOnce) {
    return sendCommand(theText, (int) endOfCommand, expectedAnswer1, expectedAnswer2, MAX_TIMEOUT, sendOnce);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, const char* endOfCommand, const char* expectedAnswer1, const char* expectedAnswer2) {
    return sendCommand(theText, endOfCommand, expectedAnswer1, expectedAnswer2, DEFAULT_TIMEOUT, 0);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, const char* endOfCommand, const char* expectedAnswer1, const char* expectedAnswer2, int sendOnce) {
    return sendCommand(theText, endOfCommand, expectedAnswer1, expectedAnswer2, DEFAULT_TIMEOUT, sendOnce);
}

byte WaspGPRS_Pro::sendCommand(const char* theText, const char* endOfCommand, const char* expectedAnswer1, const char* expectedAnswer2, int MAX_TIMEOUT, int sendOnce) {
    int timeout = 0;

    int length=sprintf(theCommand, "%s%s", theText,endOfCommand);
    

  // try sending the command
  // wait for serial response
    timeout = 0;
    serialFlush(_uart);
    while(!serialAvailable(_uart) && timeout < MAX_TIMEOUT) {
        if (!sendOnce || !timeout) {
            printString(theCommand,_uart);
        }
        delay(DELAY_ON_SEND);
        timeout++;
    };

    int answer= waitForData( expectedAnswer1, expectedAnswer2, MAX_TIMEOUT, timeout, 0);
    
    return answer;
}


byte WaspGPRS_Pro::waitForData(const char* expectedAnswer1, const char* expectedAnswer2, int MAX_TIMEOUT, int timeout, int seconds) {

	for (int i = 0; i < 100; i++) theCommand[i] = ' ';

	int theLength1 = 0;
	int theLength2 = 0;
	int it=0;
	bool theSame=false;	
	uint8_t first=1;
	int cont=0;
	int cont2=0;
	char aux;
    
	while( expectedAnswer1[theLength1]!='\0' ) theLength1++;
	while( expectedAnswer2[theLength2]!='\0' ) theLength2++;
		
	// if there is a heating time, then wait to see if you got
	// any data from the serial port
	while (seconds >0) {
		delay(1000);
		seconds--;
	}
		
	while(timeout < MAX_TIMEOUT) {
		while(!serialAvailable(_uart) && timeout < MAX_TIMEOUT) {
			timeout++;
			delay(1000);
		}
		
		while( serialAvailable(_uart))
		{
			if( first )
			{
				if(theLength1>=theLength2)
				{					
					for(it=0;it<theLength2;it++)
					{
						theCommand[it]=serialRead(_uart);
						delay(20);
					}
				}
				else
				{
					for(it=0;it<theLength1;it++)
					{
						theCommand[it]=serialRead(_uart);
						delay(20);
					}
				}
				first=0;
			}
			else
			{
				if(serialAvailable(_uart))
				{
					theCommand[it]=serialRead(_uart);
					it++;
				}
			}
			
			cont=0;
			cont2=0;
			boolean ok=false;
			while(cont<it)	//compares with expectedAnswer1
			{
				if(theCommand[cont]==expectedAnswer1[cont2])
				{
					cont2=0;
					while(expectedAnswer1[cont2]!='\0')
					{
						if (theCommand[cont+cont2]==expectedAnswer1[cont2])
						{    
							cont2++;
							ok=true; 
						}
						else
						{
							ok=false; 
							break;
						}
					}
					if(ok)
					{
						return 1;
					}
				}
			
				
				cont++;
				cont2=0;
			}
  
			cont=0;
			cont2=0;
			ok=false;
			
			while(cont<it)	//compares with expectedAnswer2
			{
				if (theCommand[cont]==expectedAnswer2[cont2])
				{
					cont2=0;
					while(expectedAnswer2[cont2]!='\0')
					{
						if (theCommand[cont+cont2]==expectedAnswer2[cont2])
						{    
							cont2++;
							ok=true; 
						}
						else
						{
							ok=false; 
							break;
						}
					}  
					if(ok)
					{
						/*USB.println("ERROR CME");
						if(expectedAnswer2==ERROR_CME){
							USB.println("ERROR ESPECIAL");
							while(serialRead(_uart)!=' ');
							
							CME_CMS_code=0;
							aux=serialRead(_uart);
							do{	//gets the length of the data string
								CME_CMS_code*=10;
								CME_CMS_code+=aux-0x30;
								aux=serialRead(_uart);
							}while(aux!='\r');
						}*/
						return 2;
					} 
				}
				cont++;
				cont2=0;
			}
  			
		}
	}
	return 0;
}

byte WaspGPRS_Pro::waitForData(const char* expectedAnswer, int MAX_TIMEOUT, int timeout, int seconds) {

	for (int i = 0; i < 100; i++) theCommand[i] = ' ';

	int theLength = 0;
	int it=0;
	bool theSame=false;	
	uint8_t first=1;
	uint8_t match=0;
    
	while( expectedAnswer[theLength]!='\0' ) theLength++;
		
	// if there is a heating time, then wait to see if you got
	// any data from the serial port
	while (seconds >0) {
		delay(1000);
		seconds--;
	}
		
	while(timeout < MAX_TIMEOUT) {
		while(!serialAvailable(_uart) && timeout < MAX_TIMEOUT) {
			timeout++;
			delay(1000);
		}
		
		while( serialAvailable(_uart) && !match )
		{
			if( first )
			{
				for(it=0;it<theLength;it++)
				{
					theCommand[it]=serialRead(_uart);
					delay(20);
				}
				first=0;
			}
			it=0;
		
			if( serialAvailable(_uart) )
			{
				theSame=true;
				for(it=0; it<theLength ; it++)
				{
					if(theCommand[it]!=expectedAnswer[it]){
						theSame= false;
						break;
					}
				}
				if( theSame ){
					match=1;
					return 1;
				}
				else
				{
					for(it=0; it<theLength-1 ; it++)
					{
						theCommand[it]=theCommand[it+1];
					}
					theCommand[it]=serialRead(_uart);
					delay(20);
				}
			}
		}
	}
	if( match ) return 1;
	else return 0;
}

uint16_t WaspGPRS_Pro::waitForData(const char* data, const char* expectedAnswer){
	uint16_t i=0;
	for (i = 0; i < 100; i++) theCommand[i] = ' ';
	int theLength = 0;
	int it=0;
	bool theSame=false;
	uint8_t first=1;
	uint8_t match=0;
	i=0;
    
	while( expectedAnswer[theLength]!='\0' ) theLength++;
	
	while( !match && data[i]!='\0' )
	{
		if( first )
		{
			for(it=0;it<theLength;it++)
			{
				theCommand[it]=data[i];
				i++;

			}
			first=0;
		}
		it=0;
		theSame=true;
		for(it=0; it<theLength ; it++)
		{
			if(theCommand[it]!=expectedAnswer[it]){
				theSame= false;
				break;
			}
		}
		if( theSame ) match=1;
		else
		{
			for(it=0; it<theLength-1 ; it++)
			{
				theCommand[it]=theCommand[it+1];
			}
			theCommand[it]=data[i];
			i++;
		}
	}
	
	if( !match ) i=0;
	return i;
}



//FTP private functions
int8_t WaspGPRS_Pro::sendDataFTP(const char* file, const char* path){
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	char aux='"';
	char* buffer_int = (char*) calloc(250,sizeof(char));
	if( command==NULL ) return -1;
	char* file_name = (char*) calloc(50,sizeof(char));
	if( command==NULL ) return -1;
	long previous=0;
	uint8_t answer=0;
	uint8_t end=0;
	uint32_t i,j=0;
	int max_FTP_data=0,aux2,n_bytes=0;

	Utils.strExplode(path,'/');	//Explores the destination file string
	i=0;
	while(path[i]!='\0')
	{
		if( path[i]== '/' ){ 
			j++;
			aux2=i;
		}
		buffer_int[i]=path[i];
		i++;
	}
	i=0;
	buffer_int[aux2+1]='\0';
	
	//Sets server path and name
	sprintf(command,"%s%c%s%c",AT_FTP_PUT_NAME,aux,Utils.arguments[j],aux);
	if(sendATCommand(command,AT_FTP_PUT_NAME_R,ERROR_CME)!=1) return 0;	
	sprintf(command,"%s%c%s%c",AT_FTP_PUT_PATH,aux,buffer_int,aux);
	if(sendATCommand(command,AT_FTP_PUT_PATH_R,ERROR_CME)!=1) return 0;

	//Opens the FTP put session
	sprintf(command,"AT%s1\r\n",AT_FTP_PUT);
	printString(command,_uart);
	previous=millis();
	while( (!serialAvailable(_uart)) && ((millis()-previous)<10000) );
	delay(10);
	answer=waitForData("+FTPPUT:1,1,",20,0,0);
	
	if(answer!=1) return 0;
	
	max_FTP_data=0;
	aux2=serialRead(_uart);
	do{	//gets the length of the data string
		max_FTP_data*=10;
		max_FTP_data+=aux2-0x30;
		aux2=serialRead(_uart);
	}while(aux2!='\r');


	serialFlush(_uart);
	Utils.strExplode(file,'/');	//Explores the origin file string
	i=0;
	j=0;
	while( file[i]!='\0' )
	{
		if( file[i]== '/' ) j++;
		i++;
	}
		
	i=1;
	
	Utils.strCp(Utils.arguments[j],file_name);
	
	SD.ON();	//Goes to the directory
	while( j>1 ){
		if(!SD.cd(Utils.arguments[i])){
			SD.OFF();
			free(command);
			free(buffer_int);
			free(file_name);
			return 0;
		}
		i++;
		j--;
	}
	i=0;
	j=0;
	n_bytes=0;
	end=0;
	do{	//Starts to send data
		//reads a fragment of the file
		if(max_FTP_data>250){
			buffer_int=SD.cat(file_name,n_bytes,250);
			n_bytes+=250;
		}else{
			buffer_int=SD.cat(file_name,n_bytes,max_FTP_data);
			n_bytes+=max_FTP_data;
		}
		while(buffer_int[j]!='\0' ) j++;//gets the length of the fragment
		if( j<249 ) end=1;
		
		sprintf(command,"AT%s2,%u\r\n",AT_FTP_PUT,j);
		printString(command,_uart);
		while(!serialAvailable(_uart));
		answer=waitForData("+FTPPUT:2,",20,0,0);
		if(answer!=1) return 0;
		
		printString(buffer_int,_uart);
		while(!serialAvailable(_uart));
		answer=waitForData("+FTPPUT:1,1,",20,0,0);	
		if(answer!=1) return 0;
		
		max_FTP_data=0;
		aux2=serialRead(_uart);
		do{	//gets the length of the data string
			max_FTP_data*=10;
			max_FTP_data+=aux2-0x30;
			aux2=serialRead(_uart);
		}while(aux2!='\r');
		j=0;
	}while(end==0);
	
	sprintf(command,"AT%s2,0\r\n",AT_FTP_PUT);
	printString(command,_uart);
	while(!serialAvailable(_uart));
	answer=waitForData("+FTPPUT:1,0",20,0,0);	
	free(command);
	free(buffer_int);
	free(file_name);
	if(answer!=1) return 0;
	
	SD.OFF();
	
	return 1;	
}

int8_t WaspGPRS_Pro::readDataFTP(const char* file, const char* path){
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	char aux='"';
	char* buffer_int = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	char* file_name = (char*) calloc(50,sizeof(char));
	if( command==NULL ) return -1;
	long previous=0;
	uint8_t answer=0;
	uint8_t end=0;
	uint32_t i,j=0;
	int FTP_data=0,aux2,n_bytes=0;
	
	
	Utils.strExplode(file,'/');	//Explores the origin file string
	i=0;
	while(file[i]!='\0')
	{
		if( file[i]== '/' ){ 
			j++;
			aux2=i;
		}
		buffer_int[i]=file[i];
		i++;
	}
	i=0;
	buffer_int[aux2+1]='\0';	
	
	//Sets server path and name
	sprintf(command,"%s%c%s%c",AT_FTP_GET_NAME,aux,Utils.arguments[j],aux);
	if(sendATCommand(command,AT_FTP_GET_NAME_R,ERROR_CME)!=1) return 2;	
	sprintf(command,"%s%c%s%c",AT_FTP_GET_PATH,aux,buffer_int,aux);
	if(sendATCommand(command,AT_FTP_GET_PATH_R,ERROR_CME)!=1) return 3;
	//Opens the FTP put session
	sprintf(command,"AT%s1\r\n",AT_FTP_GET);
	printString(command,_uart);
	previous=millis();
	while( (!serialAvailable(_uart)) && ((millis()-previous)<10000) );
	delay(10);
	answer=waitForData("+FTPGET:1,1",20,0,0);
	if(answer!=1) return 4;
	i=0;
	j=0;	
	
	Utils.strExplode(path,'/');
	while( path[i]!='\0' )
	{
		if( path[i]== '/' ) j++;
		i++;
	}
	i=1;
	
	SD.ON();
	Utils.strCp(Utils.arguments[i],file_name);
	while( j>1 )
	{
		if(!SD.cd(Utils.arguments[i])){
			SD.OFF();
			free(buffer_int);
			buffer_int=NULL; 
			return 5;
		}
		i++;
		j--;
	}
	Utils.strCp(Utils.arguments[i],file_name);
	SD.create(file_name);
		
	sprintf(command,"%s2,90",AT_FTP_GET);
	if(sendATCommand(command,"+FTPGET:2,",ERROR_CME)!=1) return 6;
	
	FTP_data=0;
	aux2=serialRead(_uart);
	do{	//gets the length of the data string
		FTP_data*=10;
		FTP_data+=aux2-0x30;
		aux2=serialRead(_uart);
	}while(aux2!='\r');
	serialRead(_uart);
		
	while(FTP_data!=0){
		
		while(serialAvailable(_uart)<FTP_data);
		
		for(i=0;i<FTP_data;i++){
			buffer_int[i]=serialRead(_uart);
		}
		
		if(!SD.append(file_name,buffer_int,FTP_data)){
			SD.OFF();
			free(buffer_int);
			buffer_int=NULL; 
			return 7;
		}
		
		sprintf(command,"%s2,90",AT_FTP_GET);
		if(sendATCommand(command,"+FTPGET:2,",ERROR_CME)!=1) return 8;
		
		FTP_data=0;
		aux2=serialRead(_uart);
		do{	//gets the length of the data string
			FTP_data*=10;
			FTP_data+=aux2-0x30;
			aux2=serialRead(_uart);
		}while(aux2!='\r');
		serialRead(_uart);
	}
	
	SD.ls();
		
	SD.OFF();
	
	free(command);
	free(buffer_int);
	free(file_name);
	if(answer!=1) return 0;
	return 1;	
}



// Public Methods //////////////////////////////////////////////////////////////


//Power functions
/* ON(void) - opens UART1 and powers the SIM900 module
 *
 * It opens UART1 and powers the SIM900 module
 *
 * Returns nothing
*/
void WaspGPRS_Pro::ON(){
	disableInterrupts(HAI_INT);
	disableInterrupts(LAI_INT);
	if( !RTC.isON ) RTC.setMode(RTC_ON, RTC_I2C_MODE);
	begin();
	setMode(GPRS_PRO_ON);
}

/* begin(void) - initialize SIM900 module
 *
 * This function powers up the SIM900 module and open Serial Port at velocity selected by the user
 * By default, it will be at 38400bps
 *
 * Returns nothing
*/
void WaspGPRS_Pro::begin(){
	_baudRate=GPRS_PRO_RATE;
	Utils.setMuxGPRS();
	beginSerial(_baudRate,_uart);
}

/* close(void) - closes UART used by SIM900 module
 *
 * This function closes UART used by SIM900 module
 *
 * Returns nothing
*/
void WaspGPRS_Pro::close(){
	closeSerial(_uart);
	Utils.setMux(MUX_TO_LOW,MUX_TO_LOW);
}

/* OFF(void) - closes UART1 and powers off the SIM900 module
 *
 * This function closes UART1 and powers off the SIM900 module
 *
 * Returns nothing
*/
void WaspGPRS_Pro::OFF(){
    setMode(GPRS_PRO_OFF);
	close();
	if( RTC.isON==2 ){
		PWR.closeI2C();
		RTC.setMode(RTC_OFF, RTC_I2C_MODE);
	}
}

/* setMode(uint8_t) - Sets GPRS Power Mode
 *
 * This function selects the active power mode among: ON, SLEEP/HIBERNATE and OFF
 * It does not close the serial port, only sends the proper command to GPRS module
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success and '0' if error
*/
uint8_t WaspGPRS_Pro::setMode(uint8_t pwrMode){
	
	flag &= ~(GPRS_ERROR_POWER);
	uint8_t answer=0;
    
	switch(pwrMode){
		
		case GPRS_PRO_ON:
			getIfReady();
			
			// start pulse
			pinMode(GPRS_PW, OUTPUT);
			digitalWrite(GPRS_PW, HIGH);
			if(not_ready==1)
			{	
				//if the module is started skip the start pulse
				delay(2000);
				digitalWrite(GPRS_PW, LOW);
				delay(500);
			}
			while(not_ready==1) getIfReady();
			answer=sendATCommand(POWER_FULL,POWER_R,ERROR_CME);
			switch(answer)
			{				
				case 0:
					flag |= GPRS_ERROR_POWER;
					break;
					
				case 1:
					_pwrMode=1;
					answer=1;
					break;					
					
				case 2:
					flag |= GPRS_ERROR_POWER;
					break;
				
			}
			break;

		case GPRS_PRO_FULL:
			answer=sendATCommand(POWER_FULL,POWER_R,ERROR_CME);
			switch(answer){
				
				case 0:
					flag |= GPRS_ERROR_POWER;
					break;
					
				case 1:
					_pwrMode=pwrMode;
					answer=1;
					break;					
					
				case 2:
					flag |= GPRS_ERROR_POWER;
					break;
				
			}
			break;
			
		case GPRS_PRO_RF_OFF:
			answer=sendATCommand(POWER_RF_OFF,POWER_R,ERROR_CME);
			switch(answer){
				
				case 0:
					flag |= GPRS_ERROR_POWER;
					break;
					
				case 1:
					_pwrMode=pwrMode;
					answer=1;
					break;					
					
				case 2:
					flag |= GPRS_ERROR_POWER;
					break;
				
			}
			break;

		case GPRS_PRO_MIN:
			answer=sendATCommand(POWER_MIN,POWER_R,ERROR_CME);
			switch(answer){
				
				case 0:
					flag |= GPRS_ERROR_POWER;
					break;
							
				case 1:
					_pwrMode=pwrMode;
					answer=1;
					break;					
								
				case 2:
					flag |= GPRS_ERROR_POWER;
					break;
					
			}
			break;
			
			case GPRS_PRO_SLEEP:
			answer=sendATCommand(POWER_SLEEP_PRO,POWER_R,ERROR);
			switch(answer){
				
				case 0:
					flag |= GPRS_ERROR_POWER;
					break;
							
				case 1:
					_pwrMode=pwrMode;
					answer=1;
					break;					
								
				case 2:
					flag |= GPRS_ERROR_POWER;
					break;
					
			}
			break;

		case GPRS_PRO_OFF:
		
			// powering off pulse
			pinMode(GPRS_PW, OUTPUT);
			digitalWrite(GPRS_PW, HIGH);
			delay(2000);
			digitalWrite(GPRS_PW, LOW);
			delay(100);
			answer=1;
			_pwrMode=pwrMode;
			break;		
	}	
		
	return answer;
}

/* getMode(void) - Gets GPRS Power Mode
 *
 * This function gets the actual GPRS Power Mode. Possible values are ON, SLEEP/HIBERNATE and OFF
 *
 * Returns the power mode
*/
uint8_t	WaspGPRS_Pro::getMode(){
	return	_pwrMode;
}

/* check(void) - Checks if GPRS is connected to the network
 *
 * This function checks if GPRS module is connected to the network. If not, it has no sense working with GPRS.
 *
 * It sends a command to GPRS module DEFAULT_TIMEOUT times. If GPRS module does not connect within these tries, function
 * exits.
 *
 * Returns '1' when connected and '0' if not
*/
int8_t WaspGPRS_Pro::check(){
	uint8_t timeout=DEFAULT_TIMEOUT;
	char* command = (char*) calloc(10,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	while(timeout)
	{
		sprintf(command,"%s%c%c","AT+CREG?",'\r','\n');
		printString(command,_uart);
		answer=waitForData("+CREG: 0,1",1,0,0);
		switch(answer){
			case 0:
				break;
				
			case 1:
				connected = 1;
				free(command);
				return 1;
				break;
				
			case 2:
				break;
		}
		
		printString(command,_uart);
		answer=waitForData("+CREG: 0,5",1,0,0);
		switch(answer){
			case	0:
				break;
				
			case	1:
				connected = 1;
				free(command);
				return 1;
				break;
				
			case 	2:
				break;
		}
		delay(100);
		timeout--;
	}
	connected = 0;
	free(command);
	return 0;	
}



//SIM functions
/* setPIN(const char*) - sets PIN to the SIM
 *
 * This function sets the specified PIN to the SIM
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t	WaspGPRS_Pro::setPIN(const char* pin){
	char* command = (char*) calloc(20,sizeof(char));
	if( command==NULL ) return -1;
	flag &= ~(GPRS_ERROR_PIN);
	char aux='"';
	uint8_t answer=0;

	sprintf(command,"%s%c%s%c",AT_PIN,aux,pin,aux);
	answer=sendATCommand(command,AT_PIN_R,ERROR_CME,SEND_ONCE);
	free(command);
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_PIN;
					break;
		case	2	:	flag |= GPRS_ERROR_PIN;
					break;
	}
	if(flag & GPRS_ERROR_PIN) return 0;
	return 1;
}

/* changePIN(const char*, const char*) - changes PIN number to the SIM
 *
 * This function changes the PIN number to the SIM
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success and '0' if error
*/
uint8_t WaspGPRS_Pro::changePIN(const char* old_pin, const char* new_pin){
	
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	flag &= ~(GPRS_ERROR_PIN);
	uint8_t answer=0;
	
	sprintf(command,"%s\"SC\",\"%s\",\"%s\"",AT_CHANGE_PASSWORD,old_pin,new_pin);
	answer=sendATCommand(command, AT_CHANGE_PASSWORD_R, ERROR);
	free(command);
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_PIN;
					break;
		case	2	:	flag |= GPRS_ERROR_PIN;
					break;
	}
	if(flag & GPRS_ERROR_PIN) return 0;
	return 1;
	
}

/* getIMEI() - gets the IMEI from the SIM card
 *
 * This function gets the IMEI from the SIM card. It stores the IMEI into 'IMEI' variable.
 *
 * Returns '1' on success and '0' if error
*/
int8_t WaspGPRS_Pro::getIMEI(){
	char* command = (char*) calloc(15,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t* byteIN = (uint8_t*) calloc(20,sizeof(uint8_t));
	if( byteIN==NULL ) return -1;
	long previous=millis();
	int8_t a,b=0;
	
	serialFlush(_uart);
	sprintf(command,"%s\r\n",AT_GPRS_IMEI);
	printString(command,_uart);
	free(command);
	
	while( (!serialAvailable(_uart)) && ((millis()-previous)<3000) );
	previous=millis();
	a=-10;
	while( (millis()-previous) < 2000 )
	{
		while( serialAvailable(_uart) && (millis()-previous) < 2000 && (a<20) )
		{
			if(a<0)
			{
				serialRead(_uart);
			}
			else
			{
				byteIN[a]=serialRead(_uart);
			}
			a++;
		}
	}
	a=0;

	while( (byteIN[a]!='\r') && (byteIN[a]!='\n') && (a<20) )
	{
		a++;
	}
	if(a>=20){
		free(byteIN);
		return 0;
	}
	a=0;
	b=0;
	while( (byteIN[a]!='\r') && (a<20) )
	{
		IMEI[b]=byteIN[a];
		a++;
		b++;
	}
	IMEI[b]='\0';
	if(b<=10){
		free(byteIN);
		return 0;
	}
	free(byteIN);
	return 1;
}

/* getIMSI() - gets the IMSI from the SIM card
 *
 * This function gets the IMSI from the SIM card. It stores the IMSI into 'IMSI' variable.
 *
 * Returns '1' on success and '0' if error
*/
int8_t WaspGPRS_Pro::getIMSI(){
	char* command = (char*) calloc(15,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t* byteIN = (uint8_t*) calloc(20,sizeof(uint8_t));
	if( byteIN==NULL ) return -1;
	long previous=millis();
	int8_t a,b=0;
	
	serialFlush(_uart);
	sprintf(command,"%s\r\n",AT_GPRS_IMSI);
	printString(command,_uart);
	free(command);
	while( (!serialAvailable(_uart)) && ((millis()-previous)<3000) );
	previous=millis();
	a=-10;
	while( (millis()-previous) < 2000 )
	{
		while( serialAvailable(_uart) && (millis()-previous) < 2000 && (a<20) )
		{
			if(a<0)
			{
				serialRead(_uart);
			}
			else
			{
				byteIN[a]=serialRead(_uart);
			}
			a++;
		}
	}
	a=0;

		while( (byteIN[a]!='\r') && (byteIN[a]!='\n') && (a<20) )
		{
			a++;
		}
		if(a>=20){
			free(byteIN);
			return 0;
		}
		a++;

	if(a>=20){
			free(byteIN);
			return 0;
		}

	b=0;
	while( (byteIN[a]!='\r') && (a<20) )
	{
		IMSI[b]=byteIN[a];
		a++;
		b++;
	}
	IMSI[b]='\0';
	if(b<=10){
		free(byteIN);
		return 0;
	}
	free(byteIN);
	return 1;
}



//Call functions
/* makeCall(const char*) - makes a call to the seleted telephone number
 *
 * This function makes a call to the given telephone number.
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::makeCall(const char* tlfNumber){
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	flag &= ~(GPRS_ERROR_CALL);
	uint8_t answer=0;

	sprintf(command,"%s%s;",AT_CALL,tlfNumber);
	answer=sendATCommand(command,AT_CALL_R,SEND_ONCE);
	free(command);
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_CALL;
					break;
		case	2	:	flag |= GPRS_ERROR_CALL;
					break;
	}
	if(flag & GPRS_ERROR_CALL) return 0;
	return 1;
	
}

/* makelostCall(const char* , uint8_t) - makes a lost call to the seleted telephone number
 *
 * This function makes a call to the given telephone number and its duration is specified by the other input.
 * After 'timeCall' seconds, hang the call up.
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::makeLostCall(const char* tlfNumber, uint8_t timeCall){
	char* command = (char*) calloc(25,sizeof(char));
	if( command==NULL ) return -1;
	flag &= ~(GPRS_ERROR_CALL);
	uint8_t answer=0;

	sprintf(command,"%s%s;",AT_CALL,tlfNumber);
	answer=sendATCommand(command,AT_CALL_R,SEND_ONCE);
	free(command);
	switch(answer){
		case	0	:
			flag |= GPRS_ERROR_CALL;
			break;
		case	1	:
			for(int a=0; a<timeCall ; a++) delay(1000);
			hangUp();
			break;
		case	2	:
			flag |= GPRS_ERROR_CALL;
			break;
	}
	if(flag & GPRS_ERROR_CALL) return 0;
	return 1;
}

/* setInfoIncomingCall() - set the info shown when an incoming call is received
 *
 * This function sets the info shown when an incoming call is received
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success and '0' if error
 */
uint8_t	WaspGPRS_Pro::setInfoIncomingCall(){
	flag &= ~(GPRS_ERROR_CALLINFO);
	uint8_t answer=0;

	answer=sendATCommand(AT_ID_INCALL,AT_ID_INCALL_R,ERROR_CME,SEND_ONCE);
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_CALLINFO;
					break;
		case	2	:	flag |= GPRS_ERROR_CALLINFO;
					break;
	}
	if(flag & GPRS_ERROR_CALLINFO) return 0;
	return 1;
}

/* readCall(const char*) - set in 'tlfIN' variable the tlf number of the incoming call
 *
 * This function sets in 'tlfIN' variable the tlf number of the incoming call
 *
 * This function should be executed only inside 'manageIncomingData' function.
 *
 * 'data' must contains the tlfn number, something like that: aaaa"+34666999888"aaa
 *
 * Returns '1' on success and '0' if error
*/
uint8_t	WaspGPRS_Pro::readCall(const char* data){
	uint8_t a=0;
	uint8_t b=0;

    tlfIN[0]='\0';
	while( (data[a]!='"') && (a<45) )
	{
		a++;
	}
	a++;
	if(a>=45) return 0;
	while( (data[a]!='"') && (a<45)  && (b<15) )
	{
		tlfIN[b]=data[a];
		a++;
		b++;
	}
    if( b>=15 ) tlfIN[b-1]='\0';
    else tlfIN[b]='\0';
	return 1;
}

/* hangUp() - hangs the call up
 *
 * This function hangs all the active calls up.
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success and '0' if error
*/
uint8_t WaspGPRS_Pro::hangUp(){
	uint8_t answer=0;
	flag &= ~(GPRS_ERROR_CALL);
	
	answer=sendATCommand(AT_HANG,AT_HANG_R);
	
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_CALL;
					break;
		case	2	:	flag |= GPRS_ERROR_CALL;
					break;
	}
	if(flag & GPRS_ERROR_CALL) return 0;
	return 1;
}



//SMS functions
/* setTextModeSMS() - set the text mode to the sms
 *
 * This function sets the text mode to the sms
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success and '0' if error
 */
uint8_t WaspGPRS_Pro::setTextModeSMS(){
	flag &= ~(GPRS_ERROR_SMS);
	uint8_t answer=0;

	answer=sendATCommand(AT_SMS_MODE,AT_SMS_MODE_R);
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_SMS;
					return 0;
					break;
		case	2	:	flag |= GPRS_ERROR_SMS;
					return 0;
					break;
	}
	if(flag & GPRS_ERROR_SMS) return 0;
	return 1;
}

/* sendSMS(const char*, const char*) - sends an SMS to the specified number
 *
 * This function sends an SMS to the specified number.
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success,'0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::sendSMS(const char* smsText, const char* tlfNumber){
	flag &= ~(GPRS_ERROR_SMS);
	flag &= ~(GPRS_ERROR_SMS);
	char* command = (char*) calloc(25,sizeof(char));
	if( command==NULL ) return -1;
	char aux='"';
	uint8_t answer=0;

	sprintf(command,"%s%c%s%c",AT_SMS,aux,tlfNumber,aux);
	
	setTextModeSMS();
	answer=sendATCommand(command,AT_SMS_R);
	free(command);
	
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_SMS;
					return 0;
					break;
		case	2	:	flag |= GPRS_ERROR_SMS;
					return 0;
					break;
	}
	
	answer=sendCommand((char*)smsText,0x1A,AT_SMS_TEXT_R,ERROR_CMS,20,SEND_ONCE);
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_SMS;
					return 0;
					break;
		case	2	:	flag |= GPRS_ERROR_SMS;
					return 0;
					break;
	}
	
	if(flag & GPRS_ERROR_SMS) return 0;
	return 1;
}

/* setInfoIncomingSMS() - set the info shown when an incoming SMS is received
 *
 * This function sets the info shown when an incoming SMS is received
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success and '0' if error
 */
uint8_t WaspGPRS_Pro::setInfoIncomingSMS(){
	flag &= ~(GPRS_ERROR_SMSINFO);
	uint8_t answer=0;

	answer=sendATCommand(AT_SMS_INFO,AT_SMS_INFO_R,SEND_ONCE);
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_SMSINFO;
					break;
		case	2	:	flag |= GPRS_ERROR_SMSINFO;
					break;
	}
	if(flag & GPRS_ERROR_SMSINFO) return 0;
	return 1;
}

/* readSMS() - set in 'tlfIN' and 'sms' variables the tlf number and text of the incoming SMS
 *
 * This function sets in 'tlfIN' and 'sms' variables the tlf number and text of the incoming SMS
 *
 * This function should be executed only inside 'manageIncomingData' function.
 *
 * Returns '1' on success and '0' if error
*/
int8_t WaspGPRS_Pro::readSMS(const char* data){
	uint8_t a=0,b=0,c=0;
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t* byteIN = (uint8_t*) calloc(200,sizeof(uint8_t));
	if( byteIN==NULL ) return -1;
	long previous=0;
	uint8_t counter=0;
	
        tlfIN[0]='\0';
        sms[0]='\0';
        sms_index[0]='\0';
	while( (data[a]!=',') && (a<45) ) a++;
	a++;
	if(a>=45){
		free(command);
		free(byteIN); 
		return 0;
	}
	while( data[a]!=13 )
	{
		sms_index[b]=data[a];
		a++;
		b++;
	}
	sms_index[b]='\0';
	sprintf(command,"AT%s%s\r\n",AT_SMS_READ,sms_index);
	printString(command,_uart);
	while(!serialAvailable(_uart));
	previous=millis();
	a=0;
	while( (millis()-previous) < 2000 )
	{
		while( serialAvailable(_uart) && (millis()-previous) < 2000 )
		{
			byteIN[a]=serialRead(_uart);
			a++;
		}
	}
	a=0;
	while( counter <3 )
	{
		while( (byteIN[a]!='"') && (a<200) )
		{
			a++;
		}
		a++;
		counter++;
	}
	if(a>=200){
		free(command);
		free(byteIN); 
		return 0;
	}
	counter=0;
	while( (byteIN[a]!='"') && (a<200) && (c<15) )
	{
		tlfIN[c]=byteIN[a];
		a++;
		c++;
	}
        if( c>=15 ) tlfIN[c-1]='\0';
        else tlfIN[c]='\0';
	while( counter < 5 )
	{
		while( (byteIN[a]!='"') && (a<200) )
		{
			a++;
		}
		a++;
		counter++;
	}
	if(a>=200){
		free(command);
		free(byteIN); 
		return 0;
	}
	a++;
	a++;
	b=0;
	while( (byteIN[a]!=13) && (a<200) && (b<100) )
	{
		sms[b]=byteIN[a];
		b++;
		a++;
	}
	if(a>=200){
		free(command);
		free(byteIN); 
		return 0;
	}
        if( b>=100 )sms[b-1]='\0';
        else sms[b]='\0';
	free(command);
	free(byteIN); 
	return 1;
}

/* deleteSMS(uint8_t ) - deletes a sms in the memory
 *
 * This function deletes a sms in the memory selected by sms_index
 *
 * Returns '1' on success '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::deleteSMS(uint8_t sms_index){

	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	int8_t answer;
    
	sprintf(command,"%s%u",AT_SMS_DELETE,sms_index);
	answer=sendATCommand(command,AT_SMS_DELETE_R,ERROR_CME);
	
	free(command);
	return answer;
}



/* manageIncomingData() - manage incoming data from serial port, executing proper functions to store received data
 *
 * This function manages incoming data from serial port, executing proper functions to store received data
 *
 * Returns '1' for call, '2' for SMS, '3' for IP data and '0' for error or not data
*/
int8_t	WaspGPRS_Pro::manageIncomingData(){
	char* byteIN = (char*) calloc(100,sizeof(char));
	if( byteIN==NULL ) return -1;
	uint8_t a=0; //counter and auxiliar variable
	long previous=0;
	
	while(a<100){
		byteIN[a]=0;
		a++;
	}
	a=0;
	
	serialFlush(_uart);
	previous=millis();
	while(!serialAvailable(_uart) && (millis()-previous)<20000);
	previous=millis();
	while( (millis()-previous) < 2000 )
	{
		while( serialAvailable(_uart) && (millis()-previous) < 2000 )
		{
			byteIN[a]=serialRead(_uart);
			a++;
		}
	}
	if(a>0){
		if(parse(byteIN,"+CLIP")){
			readCall(byteIN);
			a=1;
		}else if(parse(byteIN,"+CMTI")){
			readSMS(byteIN);
			a=2;
		}else{
			readIPData(byteIN);
			a=3;
		}
	}else{
		a=0;
	}
	free(byteIN);
	if(a==0) return 0;
	return a;
}

/*switchtoDataMode() - switchs from command mode to data mode
 *
 * This function switchs from command mode to data modes
 *
 * Returns '1' on success, '0' if error and '2' if connection is not successfully resumed
*/
int8_t WaspGPRS_Pro::switchtoDataMode(){
	uint8_t answer=0;
	
	answer=sendATCommand(AT_DATA_MODE,AT_DATA_MODE_R,AT_DATA_MODE_FAIL);

	if(answer!=1) return 0;
	return 1;
}

/*switchtoCommandMode() - switchs from data mode to command mode
 *
 * This function switchs from data mode to command mode
 *
 * Returns '1' on success and '0' if error
*/
int8_t WaspGPRS_Pro::switchtoCommandMode(){

	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_CONF);
	
	delay(1000);	//waits one second after the last data send
	answer=sendCommand(AT_COMMAND_MODE,"",AT_COMMAND_MODE_R);
	
	if(answer!=1) return 0;
	return 1;
}



/* sendCommand(ATcommand) - sends any command to GPRS module
 *
 * This function sends any command to GPRS module
 *
 * It stores in 'answer_command' variable the answer returned by the GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::sendCommand(const char* ATcommand){
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t timeout=0;
	uint8_t i=0;
	
	sprintf(command, "AT%s%c%c", ATcommand,'\r','\n');

	serialFlush(_uart);
	while(!serialAvailable(_uart)) {
		printString(command,_uart);
		delay(DELAY_ON_SEND);
	}
	free(command);
	while( timeout < 5 )
	{
		while(!serialAvailable(_uart) && timeout < 5) {
			timeout++;
			delay(1000);
		}
		while(serialAvailable(_uart) && timeout < 5){
			answer_command[i] = serialRead(_uart);
			USB.print(char(answer_command[i]));
			delay(20);
			i++;
			timeout=0;
			if(i>=199) timeout=5;
		}
	}
	answer_command[i]='\0';
	if( i<5 ) return 0;
	return 1;
}



//HTTP and FTP functions
/* configureGPRS_HTTP_FTP(uint8_t) - configures GPRS connection with login, password and other parameters
 *
 * This function creates a GPRS connection with the carrier server to get access to the HTTP and FTP applications
 *
 * It can configures 3 different profiles (1-3)
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::configureGPRS_HTTP_FTP(uint8_t n_conf){
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	char aux='"';
	uint8_t answer=0;
	
	int i;
	
	flag &= ~(GPRS_ERROR_HTTP_FTP_PROFILE);
	
	sprintf(command,"%s3,%u,%c%s%c,%c%s%c",AT_GPRS_CFG,n_conf,aux,"Contype",aux,aux,"GPRS",aux);	//Configures the connection
	if(sendATCommand(command,AT_GPRS_CFG_R)!=1){
		flag |= GPRS_ERROR_HTTP_FTP_PROFILE;
		return 0;
	}
	sprintf(command,"%s3,%u,%c%s%c,%c%s%c",AT_GPRS_CFG,n_conf,aux,"APN",aux,aux,AT_GPRS_PRO_APN,aux);
	if(sendATCommand(command,AT_GPRS_CFG_R)!=1){
		flag |= GPRS_ERROR_HTTP_FTP_PROFILE;
		return 0;
	}
	sprintf(command,"%s3,%u,%c%s%c,%c%s%c",AT_GPRS_CFG,n_conf,aux,"USER",aux,aux,AT_GPRS_PRO_LOGIN,aux);
	if(sendATCommand(command,AT_GPRS_CFG_R)!=1){
		flag |= GPRS_ERROR_HTTP_FTP_PROFILE;
		return 0;
	}
	sprintf(command,"%s3,%u,%c%s%c,%c%s%c",AT_GPRS_CFG,n_conf,aux,"PWD",aux,aux,AT_GPRS_PRO_PASSW,aux);
	if(sendATCommand(command,AT_GPRS_CFG_R)!=1){
		flag |= GPRS_ERROR_HTTP_FTP_PROFILE;
		return 0;
	}
	sprintf(command,"%s5,%u",AT_GPRS_CFG,n_conf);	//Saves configuration
	if(sendATCommand(command,AT_GPRS_CFG_R)!=1){
		flag |= GPRS_ERROR_HTTP_FTP_PROFILE;
		return 0;
	}
	sprintf(command,"%s1,%u",AT_GPRS_CFG,n_conf);	//Opens connection
	if(sendATCommand(command,AT_GPRS_CFG_R)!=1){
		flag |= GPRS_ERROR_HTTP_FTP_PROFILE;
		return 0;
	}
	
	sprintf(theCommand, "AT%s2,%u\r\n",AT_GPRS_CFG,n_conf);//Gets IP direction
	printString(theCommand,_uart);
	do{
		while(!serialAvailable(_uart));
	}while(serialRead(_uart)!='"');
	i=0;
	do{
		while(!serialAvailable(_uart));
		IP_dir[i]=serialRead(_uart);
		i++;
	}while(IP_dir[i-1]!='"');	
	IP_dir[i-1]='\0';
		
	if(IP_dir[0]=='+'){
		flag |= GPRS_ERROR_HTTP_FTP_PROFILE;
		return 7;
	}
	
	free(command);
	
	if(setDNS()!=1){
		flag |= GPRS_ERROR_HTTP_FTP_PROFILE;
		return 0;
	}
	
	if(flag & GPRS_ERROR_HTTP_FTP_PROFILE) return 0;
	return 1;
}

/* uploadFile(const char*, const char*, const char*, const char*, const char* , const char*, uint8_t) - uploads a file to a FTP server
 *
 * This function uploads a file to a FTP server
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success and '0' if error
*/
int8_t WaspGPRS_Pro::uploadFile(const char* file, const char* path, const char* user, const char* passw, const char* ftp_server, const char* ftp_port, uint8_t n_conf){
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	long previous=0;
	uint8_t answer=0;
	char aux='"';
	uint8_t id=0;
	flag &= ~(GPRS_ERROR_FTP);

	// Connect to FTP Server
	sprintf(command,"%s%u",AT_FTP_ID,n_conf);
	answer=sendATCommand(command,AT_FTP_ID_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s1",AT_FTP_MODE);
	answer=sendATCommand(command,AT_FTP_MODE_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s%cI%c",AT_FTP_TYPE,aux,aux);
	answer=sendATCommand(command,AT_FTP_TYPE_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s%c%s%c",AT_FTP_SERVER,aux,ftp_server,aux);
	answer=sendATCommand(command,AT_FTP_SERVER_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s%s",AT_FTP_PORT,ftp_port);
	answer=sendATCommand(command,AT_FTP_PORT_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s%c%s%c",AT_FTP_UN,aux,user,aux);
	answer=sendATCommand(command,AT_FTP_UN_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s%c%s%c",AT_FTP_PW,aux,passw,aux);
	answer=sendATCommand(command,AT_FTP_PW_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	
	answer=sendDataFTP(file, path);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	
	if(flag & GPRS_ERROR_HTTP_FTP_PROFILE) return 0;
	return 1;
}

/* downloadFile(const char*, const char*, const char*, const char*, const char* , const char*, uint8_t) - downloads a file from a FTP server
 *
 * This function downloads a file from a FTP server
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success and '0' if error
*/
int8_t WaspGPRS_Pro::downloadFile(const char* file, const char* path, const char* user, const char* passw, const char* ftp_server, const char* ftp_port, uint8_t n_conf){
		char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	long previous=0;
	uint8_t answer=0;
	char aux='"';
	uint8_t id=0;
	flag &= ~(GPRS_ERROR_FTP);

	// Connect to FTP Server
	sprintf(command,"%s%u",AT_FTP_ID,n_conf);
	answer=sendATCommand(command,AT_FTP_ID_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s1",AT_FTP_MODE);
	answer=sendATCommand(command,AT_FTP_MODE_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s%cI%c",AT_FTP_TYPE,aux,aux);
	answer=sendATCommand(command,AT_FTP_TYPE_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s%c%s%c",AT_FTP_SERVER,aux,ftp_server,aux);
	answer=sendATCommand(command,AT_FTP_SERVER_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s%s",AT_FTP_PORT,ftp_port);
	answer=sendATCommand(command,AT_FTP_PORT_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s%c%s%c",AT_FTP_UN,aux,user,aux);
	answer=sendATCommand(command,AT_FTP_UN_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	sprintf(command,"%s%c%s%c",AT_FTP_PW,aux,passw,aux);
	answer=sendATCommand(command,AT_FTP_PW_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
		
	answer=readDataFTP(file, path);
	if(answer!=1){
		flag |= GPRS_ERROR_FTP;
		return 0;
	}
	
	if(flag & GPRS_ERROR_HTTP_FTP_PROFILE) return 0;
	return 1;
}

/* readURL(const char*, uint8_t) - access to the specified URL and stores the info read in 'data_URL' variable
 *
 * This function access to the specified URL and stores the info read in 'data_URL' variable
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success and '0' if error
*/
int8_t WaspGPRS_Pro::readURL(const char* url, uint8_t n_conf){
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	long HTTP_data;
	char aux='"';
	uint16_t counter=0;
	
	flag &= ~(GPRS_ERROR_HTTP);

	serialFlush(_uart);
	answer=sendATCommand(AT_HTTP_INIT,AT_HTTP_INIT_R,ERROR_CME);	//Initializes HTTP service
	if(answer!=1){
		flag |= GPRS_ERROR_HTTP;
		return 0;
	}
	
	sprintf(command,"%s%c%s%c,%u",AT_HTTP_PARAM,aux,"CID",aux,n_conf);		//Sets HTTP values
	answer=sendATCommand(command,AT_HTTP_PARAM_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_HTTP;
		return 0;
	}
	
	sprintf(command,"%s%c%s%c,%c%s%c",AT_HTTP_PARAM,aux,"URL",aux,aux,url,aux);
	answer=sendATCommand(command,AT_HTTP_PARAM_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_HTTP;
		return 0;
	}
	sprintf(command,"%s0",AT_HTTP_ACTION);		//Starts HTTP sesion
	answer=sendATCommand(command,AT_HTTP_ACTION_R,ERROR_CME);
	if(answer!=1){
		flag |= GPRS_ERROR_HTTP;
		return 0;
	}
	
	do{
		sprintf(command,"%s=%u,70",AT_HTTP_READ,counter);		//Reads url data
		answer=sendATCommand(command,AT_HTTP_READ_R,ERROR_CME);
		if(answer!=1){
			flag |= GPRS_ERROR_HTTP;
			return 0;
		}
		
		HTTP_data=0;
		aux=serialRead(_uart);
		do{	//gets the length of the data string
			HTTP_data*=10;
			HTTP_data+=aux-0x30;
			aux=serialRead(_uart);
		}while(aux!='\r');
		serialRead(_uart);
		HTTP_data-=0;
	
		while(serialAvailable(_uart)<HTTP_data);
		
		aux=0;
		do{	//gets the length of the data string
			data_URL[counter]=serialRead(_uart);
			aux++;
			counter++;
		}while(aux<HTTP_data);
	
		waitForData("OK",20,0,0);
	
	}while(HTTP_data==70);
	
	
	
	sprintf(command,"%s",AT_HTTP_TERM);		//Terminates HTTP sesion
	answer=sendATCommand(command,AT_HTTP_TERM_R,ERROR_CME);	
	free(command);
	if(answer!=1){
		flag |= GPRS_ERROR_HTTP;
		return 0;
	}
	return 1;
}



/* setMonitorVolume(uint8_t) - Sets the monitor speaker loudness
 *
 * This function sets the monitor speaker loudness from 0 to 9
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setMonitorVolume(uint8_t volume){
	
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	sprintf(command,"%s%u",AT_SPEAKER_VOLUME,volume);
	answer=sendATCommand(command,AT_SPEAKER_VOLUME_R,ERROR);
	free(command);
	if(answer!=1) return 0;
	return 1;	
}

/* setMonitorMode(uint8_t) - Sets the monitor speaker mode
 *
 * This function sets the monitor speaker mode from 0 to 9
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setMonitorMode(uint8_t mode){
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	sprintf(command,"%s%u",AT_SPEAKER_MODE,mode);
	answer=sendATCommand(command,AT_SPEAKER_MODE_R,ERROR);
	free(command);
	if(answer!=1) return 0;
	return 1;
	
}

/* setCLIPresentation(uint8_t) - Enables or disables the presentation of the incoming call 
 *
 * This function enables or disables the presentation of the incoming call
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setCLIPresentation(uint8_t mode){
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	sprintf(command,"%s%u",AT_CLIP_MODE,mode);
	answer=sendATCommand(command,AT_CLIP_MODE_R,ERROR);
	free(command);
	if(answer!=1) return 0;
	return 1;
	
}

/* setCLIRestriction(uint8_t) - Restricts or enables the presentation of the CLI to the called party when originating a call.
 *
 * This function restricts or enables the presentation of the CLI to the called party when originating a call.
 *
 * Allowed modes DEFAULT_CLIR='0', INVOKE_CLIR='1' or SUPPRESS_CLIR='2'
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setCLIRestriction(uint8_t mode){
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	sprintf(command,"%s%u",AT_CLIR_MODE,mode);
	answer=sendATCommand(command,AT_CLIR_MODE_R,ERROR);
	free(command);
	if(answer!=1) return 0;
	return 1;
	
}

/* getPhoneStatus() - Gets the phone activity status
 *
 *This function gets the phone activity status
 *
 *Return '0' for error, '1'= Ready, '2'= Unknown,'3'= Ringing, '4'= Call in progress
*/
uint8_t WaspGPRS_Pro::getPhoneActStatus(){
	uint8_t answer=0;
	
	answer=sendATCommand(AT_PHONE_ACTIVITY,AT_PHONE_ACTIVITY_R,ERROR);
	
	if(answer==0||answer==2){
		return 0;
	}else{
		serialRead(_uart);
		answer=serialRead(_uart)-0x30;
		if(answer==0) answer=1;
	}	
	return answer;
}

/* setLoudspeakerLevel() - Sets loudspeaker volume 
 *
 * This function sets loudspeaker volume level between 0-100
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setLoudspeakerLevel(uint8_t volume){
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	sprintf(command,"%s%u",AT_SPEAKER_LEVEL,volume);
	answer=sendATCommand(command,AT_SPEAKER_LEVEL_R,ERROR_CME);
	free(command);
	if(answer!=1) return 0;
	return 1;
}

/* setCallAlert(uint8_t) - Configures the mode of calls alert
 *
 * This function configures the mode of calls alert
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setCallAlert(uint8_t mode){
	return setCallAlert(mode,0,0);
}

/* setCallAlert(uint8_t,uint8_t, uint8_t) - Configures the mode of calls alert
 *
 * This function configures the mode of calls alert
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setCallAlert(uint8_t mode,uint8_t sound_type, uint8_t ring_level){
	
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	sprintf(command,"%s%u",AT_ALERT_SOUND_MODE,mode);
	answer=sendATCommand(command,AT_ALERT_SOUND_MODE_R,ERROR_CME);
	if(answer!=1||mode==1){
		free(command);
		return 0;
	}
	
	sprintf(command,"%s%u",AT_ALERT_SOUND_LEVEL,sound_type);
	answer=sendATCommand(command,AT_ALERT_SOUND_LEVEL_R,ERROR_CME);
	if(answer!=1){
		free(command);
		return 0;
	}
	
	sprintf(command,"%s%u",AT_RINGER_SOUND_LEVEL,ring_level);
	answer=sendATCommand(command,AT_RINGER_SOUND_LEVEL_R,ERROR_CME);	
	free(command);
	if(answer!=1){
		return 0;
	}
	return 1;
}

/* setMute(uint8_t) - Enables or disables mute during a call
 *
 * This function enables or disables mute during a call
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setMute(uint8_t mute_mode){
	
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	sprintf(command,"%s%u",AT_MUTE,mute_mode);
	answer=sendATCommand(command,AT_MUTE_R,ERROR_CME);
	free(command);
	if(answer!=1){
		return 0;
	}
	return 1;
}

/* getcurrentOperator() - Gets the currently selected operator from network
 *
 * This function gets the currently selected operator from network and stores it in 'operator_name'
 *
 * Returns '1' on success and '0' if error
*/
uint8_t WaspGPRS_Pro::getcurrentOperator(){
	
	uint8_t answer=0;
	int i;
	
	serialFlush(_uart);
	
	answer=sendATCommand(AT_GET_OPERATOR, AT_GET_OPERATOR_R, ERROR_CME);
	if(answer!=1) return 0;
	
	while(serialRead(_uart)!='"');	
	i=0;
	do{
		while(serialAvailable(_uart)==0);
		operator_name[i]=serialRead(_uart);
		i++;
	}while(operator_name[i-1]!='"');
	operator_name[i-1]='\0';
	
	return 1;
	
}

/* getAvailableOperators() - Gets the currently available operators from network
 *
 * This function gets the currently available operators from network and stores it in 'operators_list'
 *
 * Returns '1' on success and '0' if error
*/
uint8_t WaspGPRS_Pro::getAvailableOperators(){
	
	uint8_t answer=0;
	int i,j,aux;
	
	serialFlush(_uart);
	
	answer=sendATCommand(AT_OPERATOR_LIST, AT_OPERATOR_LIST_R, ERROR_CME);
	if(answer!=1) return 0;
	
	j=0;
	do{
		while(serialRead(_uart)!=',');	//gets format used
		i=0;
		do{
			while(serialAvailable(_uart)==0);
			operators_list[j].format=aux-0x30;			
			aux=serialRead(_uart);
		}while(aux!=',');
		
		while(serialRead(_uart)!='"');		//gets operator name
		i=0;
		do{
			while(serialAvailable(_uart)==0);
			operators_list[j].operator_name[i]=serialRead(_uart);
			i++;
		}while(operators_list[j].operator_name[i-1]!='"');
		operators_list[j].operator_name[i-1]='\0';
		j++;
	}while(waitForData("OK", AT_OPERATOR_LIST_R, 20, 0, 0)==2);
	return answer;	
}

/* setPreferredOperator(int, uint8_tt, const char*) - Sets the preferred operator in the operators list into GPRS module
 *
 * This function sets the preferred operator in the operators list into GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setPreferredOperator(int index, uint8_t format, const char* preferred_operator){
	
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	sprintf(command,"%s%u,%u,\"%s\"",AT_SET_PREF_OPERATOR,index,format,preferred_operator);
	answer=sendATCommand(command, AT_SET_PREF_OPERATOR_R, ERROR_CME);
	free(command);
	if(answer!=1){
		return 0;
	}
	return 1;
}

/* getCellInfo() - gets the information from the cell where the module is connected
 *
 * This function gets the information from the cell where the module is connected
 *
 * It stores in 'RSSI' and 'cellID' variables the information from the cell
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::getCellInfo(){
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t* byteIN = (uint8_t*) calloc(200,sizeof(uint8_t));
	if( byteIN==NULL ) return -1;
	long previous=millis();
	uint8_t counter=0;
	uint8_t a,b=0;
	int answer=0;
	

	sprintf(command,"%s=1",AT_GPRS_CELLID);
	answer=sendATCommand(command,"OK","ERROR");
	if(answer!=1){
		free(byteIN);
		free(command);
		return 0;
	}
	
	serialFlush(_uart);
	sprintf(command,"AT%s?\r\n",AT_GPRS_CELLID);
	printString(command,_uart);
	free(command);
	while( (!serialAvailable(_uart)) && ((millis()-previous)<3000) );
	previous=millis();
	a=0;
	while( (millis()-previous) < 2000 )
	{
		while( serialAvailable(_uart) && (millis()-previous) < 2000 && (a<200) )
		{
			byteIN[a]=serialRead(_uart);
			a++;
		}
	}
	a=0;
	
	while( counter < 3 )
	{
		while( (byteIN[a]!=',') && (a<200) )
		{
			a++;
		}
		a++;
		counter++;
	}
	if(a>=200){
		free(byteIN); 
		return 0;
	}
	RSSI=0;	//gets RSSI value, converts it from ASCII to -dBm
	while( (byteIN[a]!=',') && (a<200) )
	{
		RSSI*=10;
		RSSI+=(byteIN[a]-0x30);
		a++;
	}
	RSSI-=110;
	
	counter=0;
	while( counter < 5 )
	{
		while( (byteIN[a]!=',') && (a<200) )
		{
			a++;
		}
		a++;
		counter++;
	}
	//gets cellID
	b=0;
	while( (byteIN[a]!=',') && (a<200) )
	{
		cellID[b]=byteIN[a];
		a++;
		b++;
	}
	a++;
	
	free(byteIN);
	if(answer!=1){
		return 0;
	}
	return 1;
}



//TCP and UDP functions
/* configureGPRS_TCP_UDP(uint8_t) - configures GPRS connection with login, password and some other parameters to use TCP or UDP connections
 *
 * This function creates a GPRS connection with the carrier server to get access to the internet
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success and '0' if error
 */
int8_t WaspGPRS_Pro::configureGPRS_TCP_UDP(uint8_t mode){
	return(configureGPRS_TCP_UDP(mode,0));
}

/* configureGPRS_TCP_UDP(uint8_t, uint8_t) - configures GPRS connection with login, password and some other parameters to use TCP or UDP connections
 *
 * This function creates a GPRS connection with the carrier server to get access to the internet
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success and '0' if error
*/
int8_t WaspGPRS_Pro::configureGPRS_TCP_UDP(uint8_t mode, uint8_t app_mode){
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	char aux='"';
	uint8_t answer=0;
	uint8_t i;
	
	for (i = 0; i < 16; i++) IP_dir[i] = '\0';
	
	flag &= ~(GPRS_ERROR_TCP_UDP_PROFILE);
	
	//disconnects and connects the GPRS connection
	answer=sendATCommand(AT_IP_SHUT,AT_IP_SHUT_R,ERROR);		
	if(answer!=1){
		flag |= GPRS_ERROR_TCP_UDP_PROFILE;
		free(command);
		return 0;
	}
	
	//starts configuration
	if(app_mode==0){
		sprintf(command,"%s0",AT_IP_APP_MODE);
	}else{				
		sprintf(command,"%s1",AT_IP_APP_MODE);
	}
	answer=sendATCommand(command,AT_IP_APP_MODE_R,ERROR);
	
	switch(answer){
		case	0	:	flag |= GPRS_ERROR_TCP_UDP_PROFILE;
			return 0;
			break;
		case	2	:	flag |= GPRS_ERROR_TCP_UDP_PROFILE;
			return 0;
			break;
	}
	
	IP_app_mode=app_mode;
	
	answer=sendATCommand(AT_GPRS_ATT_ON,AT_GPRS_ATT_R);
	
	switch(answer){
		case 0 :
			flag |= GPRS_ERROR_TCP_UDP_PROFILE;
			free(command);
			return 0;
			break;
		case 2 :
			flag |= GPRS_ERROR_TCP_UDP_PROFILE;	
			free(command);
			return 0;
			break;
	}
		
	while(checkGPRS()!=1);	
	
	//sets the connection mode: single or multiple
	switch(mode){
		case 0:
			answer=sendATCommand(AT_SINGLE_CONN,AT_SINGLE_CONN_R);
			break;
		case 1:
			answer=sendATCommand(AT_MULTI_CONN,AT_MULTI_CONN_R);
			break;
	}
	
	switch(answer){
		case 0 :
			flag |= GPRS_ERROR_TCP_UDP_PROFILE;
			free(command);
			return 0;
			break;
		case 2 :
			flag |= GPRS_ERROR_TCP_UDP_PROFILE;
			free(command);
			return 0;
			break;
	}
	
	IP_mode=mode;	
	
	
	//sets apn, username and password for IP connection
	sprintf(command,"%s%c%s%c,%c%s%c,%c%s%c",AT_IP_SET_APN,aux,AT_GPRS_PRO_APN,aux,aux,AT_GPRS_PRO_LOGIN,aux,aux,AT_GPRS_PRO_PASSW,aux);
	answer=sendATCommand(command,AT_IP_SET_APN_R,ERROR);
	
	switch(answer){
		case 0 :
			flag |= GPRS_ERROR_TCP_UDP_PROFILE;
			free(command);
			return 0;
			break;
		case 2 :
			flag |= GPRS_ERROR_TCP_UDP_PROFILE;
			free(command);
			return 0;
			break;
	}
	
	//Brings up wireless connection	
	answer=sendATCommand(AT_IP_BRING,AT_IP_BRING_R,ERROR);
	
	switch(answer){
		case 0 :
			flag |= GPRS_ERROR_TCP_UDP_PROFILE;
			free(command);
			return 0;
			break;
		case 2 :
			flag |= GPRS_ERROR_TCP_UDP_PROFILE;
			free(command);
			return 0;
			break;
	}

	//Gets IP direction
	answer=getIP();
	free(command);
	
	if(answer==0){
			flag |= GPRS_ERROR_TCP_UDP_PROFILE;
			return 0;
	}
	
	if(setDNS()!=1){
		flag |= GPRS_ERROR_TCP_UDP_PROFILE;
			return 0;
	}
	
	if(flag & GPRS_ERROR_TCP_UDP_PROFILE) return 0;
	return 1;
}

/* setDNS() - Sets the directions of DNS servers from GPRS_Proconstants.h
 *
 * This function sets the directions of DNS servers from GPRS_Proconstants.h
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setDNS(){
	
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_EXTRA_PARAMETERS);
	
	sprintf(command,"%s\"%s\",\"%s\"",AT_IP_SET_DNS,AT_GPRS_PRO_DNS1,AT_GPRS_PRO_DNS2);
	answer=sendATCommand(command,AT_IP_SET_DNS_R,ERROR);
	free(command);
	if(answer!=1){
		flag |= GPRS_ERROR_EXTRA_PARAMETERS;
		return 0;
	}
	return 1;
}

/* setDNS(const char*) - Sets the direction of DNS server
 *
 * This function sets the direction of DNS server
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setDNS(const char* DNS_dir){
	
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_EXTRA_PARAMETERS);
	
	sprintf(command,"%s\"%s\"",AT_IP_SET_DNS,DNS_dir);
	answer=sendATCommand(command,AT_IP_SET_DNS_R,ERROR);
	free(command);
	if(answer!=1){
		flag |= GPRS_ERROR_EXTRA_PARAMETERS;
		return 0;
	}
	return 1;
}

/* setDNS(const char*, const char*) - Sets the directions of DNS servers
 *
 * This function sets the directions of DNS servers
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setDNS(const char* DNS_dir1, const char* DNS_dir2){
	
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_EXTRA_PARAMETERS);
	
	sprintf(command,"%s\"%s\",\"%s\"",AT_IP_SET_DNS,DNS_dir1,DNS_dir2);
	answer=sendATCommand(command,AT_IP_SET_DNS_R,ERROR);
	free(command);
	if(answer!=1){
		flag |= GPRS_ERROR_EXTRA_PARAMETERS;
		return 0;
	}
	return 1;
}

/* setLocalPort(const char*, uint16_t) - Sets the number of the internal port for TCP or UDP connections
 *
 * This function sets the number of the internal port for TCP or UDP connections
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::setLocalPort(const char* mode, uint16_t port){
		
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_EXTRA_PARAMETERS);
	
	sprintf(command,"%s\"%s\",%u",AT_IP_LOCAL_PORT,mode,port);
	answer=sendATCommand(command, AT_IP_LOCAL_PORT_R, ERROR);
	
	free(command);
	if(answer!=1){
		flag |= GPRS_ERROR_EXTRA_PARAMETERS;
		return 0;
	}
	return 1;
	
}

/* saveGPRS_TCP_UDPconfiguration() - Saves the configuration into the internal NVRAM of the GPRS module
 *
 * This function saves the configuration into the internal NVRAM of the GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::saveGPRS_TCP_UDPconfiguration(){
	return(sendATCommand(AT_IP_SAVE_CONF, AT_IP_SAVE_CONF_R));
}

/* createSocket(uint8_t, const char*) - creates a TCP/IP connection to the specified IP and PORT
 *
 * This function creates a TCP/IP connection to the specified IP and PORT
 *
 * In multi connection mode you must specify a number of connection (0-7).
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::createSocket( uint8_t working_mode, const char* port){
	return(createSocket( working_mode, 0, 0, port));
}

/* createSocket(uint8_t, const char*,const char*, uint8_t) - creates a TCP/IP connection to the specified IP and PORT
 *
 * This function creates a TCP/IP connection to the specified IP and PORT
 *
 * In multi connection mode you must specify a number of connection (0-7).
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
/*int8_t WaspGPRS_Pro::createSocket( uint8_t working_mode, const char* ip,const char* port, uint8_t n_connection){
	return(createSocket( working_mode, n_connection, ip, port));
}*/

/* createSocket(uint8_t, const char*,const char*) - creates a TCP/IP connection to the specified IP and PORT
 *
 * This function creates a TCP/IP connection to the specified IP and PORT
 *
 * In multi connection mode you must specify a number of connection (0-7).
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::createSocket( uint8_t working_mode, const char* ip,const char* port){
	return(createSocket( working_mode, 0, ip, port));
}

/* createSocket(uint8_t, uint8_t, const char*, const char*) - creates a TCP/IP connection to the specified IP and PORT
 *
 * This function creates a TCP/IP connection to the specified IP and PORT
 *
 * In multi connection mode you must specify a number of connection (0-7).
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::createSocket( uint8_t working_mode, uint8_t n_connection, const char* ip, const char* port){
	char* command = (char*) calloc(40,sizeof(char));
	if( command==NULL ) return -1;
	char aux='"';
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_SOCKET);
	
	switch(IP_mode){
		case 0:	//single mode
			switch(working_mode){		
				case UDP_CLIENT:
					sprintf(command,"%s0",AT_IP_UDP_EXTENDED);
					answer|=sendATCommand(command,AT_IP_UDP_EXTENDED_R,ERROR);
					sprintf(command,"%s%c%s%c,%c%s%c,%c%s%c",AT_IP_CLIENT,aux,AT_UDP,aux,aux,ip,aux,aux,port,aux);
					answer|=sendATCommand(command,AT_IP_CFG_R,ERROR_CME);
					break;
				case TCP_CLIENT:
					sprintf(command,"%s%c%s%c,%c%s%c,%c%s%c",AT_IP_CLIENT,aux,AT_TCP,aux,aux,ip,aux,aux,port,aux);
					answer=sendATCommand(command,AT_IP_CFG_R,ERROR_CME);
					break;
				case TCP_SERVER:
					sprintf(command,"%s1,%c%s%c",AT_IP_SERVER,aux,port,aux);
					answer=sendATCommand(command,AT_IP_CFG_R,ERROR);
					break;
				case UDP_EXTENDED:
					sprintf(command,"%s1",AT_IP_UDP_EXTENDED);
					answer|=sendATCommand(command,AT_IP_UDP_EXTENDED_R,ERROR);
					sprintf(command,"%s%c%s%c,%c%s%c,%c%s%c",AT_IP_CLIENT,aux,AT_UDP,aux,aux,ip,aux,aux,port,aux);
					answer|=sendATCommand(command,AT_IP_CFG_R,ERROR_CME);
					sprintf(command,"%s2,%c%s%c,%c%s%c",AT_IP_UDP_EXTENDED,aux,ip,aux,aux,port,aux);
					answer|=sendATCommand(command,AT_IP_UDP_EXTENDED_R,ERROR);	
					break;
			}
			break;			
		case 1:	//multi mode	
			switch(working_mode){		
				case UDP_CLIENT:
					sprintf(command,"%s%u,%c%s%c,%c%s%c,%c%s%c",AT_IP_CLIENT,n_connection,aux,AT_UDP,aux,aux,ip,aux,aux,port,aux);
					answer=sendATCommand(command,AT_IP_CFG_R,ERROR_CME);
					break;
				case TCP_CLIENT:
					sprintf(command,"%s%u,%c%s%c,%c%s%c,%c%s%c",AT_IP_CLIENT,n_connection,aux,AT_TCP,aux,aux,ip,aux,aux,port,aux);
					answer=sendATCommand(command,AT_IP_CFG_R,ERROR_CME);
					break;
				case TCP_SERVER:
					sprintf(command,"%s1,%c%s%c",AT_IP_SERVER,aux,port,aux);
					answer=sendATCommand(command,AT_IP_CFG_R,ERROR);
					break;
			}
			break;
	}
	
	free(command);	
	
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_SOCKET;
					return 0;
					break;
		case	2	:	flag |= GPRS_ERROR_SOCKET;
					return 0;
					break;
		case	3	:	flag |= GPRS_ERROR_SOCKET;
					return 0;
					break;
	}
	
	if(working_mode<2){
		waitForData(AT_CONNECTED_OK, AT_CONNECTED_FAIL, 20, 0, 5);
		switch(answer)
		{
			case	0	:	flag |= GPRS_ERROR_SOCKET;
						return 0;
						break;
			case	2	:	flag |= GPRS_ERROR_SOCKET;
						return 0;
						break;
		}
	}
	
	if(answer!=1) return 0;
	return 1;
}

/* sendData(const char*) - sends 'data' to the specified to 'n_connection'
 *
 * This function sends 'data' to the specified to 'n_connection'. In single connection not specifies 'n_connection'.
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::sendData(const char* data){
	return(sendData(data, NULL));
}

/* sendData(const char*, uint8_t) - sends 'data' to the specified to 'n_connection'
 *
 * This function sends 'data' to the specified to 'n_connection'. In single connection not specifies 'n_connection'.
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::sendData(const char* data, uint8_t n_connection){
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_DATA);
	
	if(IP_app_mode==0){//non transparent mode
		switch(IP_mode){
			case 0:	//single mode
				sprintf(command,"%s",AT_IP_SEND);
				break;
			case 1:	//multi mode
				sprintf(command,"%s=%c",AT_IP_SEND,n_connection+0x30);
				break;
		}


		//Wait the connection with the server to send data	
		answer=sendATCommand(command,">",ERROR_CME);

		switch(answer)
		{
			case 0:
				flag |= GPRS_ERROR_DATA;
				free(command);
				return 0;
				break;
			case 2 :
				flag |= GPRS_ERROR_DATA;
				free(command);
				return 0;
				break;
		}

		//sends data to the server
		sprintf(command,"%s%c",data,0x1A);
		printString(command,_uart);
		answer=waitForData(AT_IP_SEND_R,AT_IP_SEND_FAIL,20,0,0);

		free(command);

		switch(answer)
		{
			case 0:
				flag |= GPRS_ERROR_DATA;
				return 0;
				break;
			case 2 :
				flag |= GPRS_ERROR_DATA;
				return 0;
				break;
		}
		
		if(answer!=1) return 0;
		return 1;

	}else{	//transparent mode
		printString(data,_uart);
	}
	
	if(flag & GPRS_ERROR_DATA) return 0;
	return 1;
}

/* readIPData(char*) - Gets data receive from a TCP or UDP connection and stores it in 'IP_data'
 *
 * This function gets data receive from a TCP or UDP connection and stores it in 'IP_data'.
 *
 * In multi connection mode also stores the connection number in 'IP_data_from.
 *
 * This function should be executed only inside 'manageIncomingData' function.
 *
 * Returns '1' on success and '0' if error
*/
int8_t WaspGPRS_Pro::readIPData(char* dataIN){
	int IP_data_length=0,IP_data_from=0;
	int i=0,j;
	int8_t answer=0;
	for( j=0;j<100;j++){ IP_data[j]='\0';}
	
	if(IP_mode==0){//single connection mode
		if(IP_app_mode==0){	//non transparent mode
			if(parse(dataIN,"+CIPRXGET")){
				answer=GetDataManually(150,0);
			}else if(parse(dataIN,"+IPD")){
				do{
					i++;
				}while(dataIN[i]!=':');
				
				do{	//gets the length of the data string
				IP_data_length*=10;
				IP_data_length=dataIN[i];
				i++;
				}while(dataIN[i]!=',');
				
				do{
					i++;
				}while(dataIN[i]!=':');
				
				for(j=0;j<IP_data_length;j++){
					IP_data[j]=dataIN[i+j];
				}
				
			}else{
				Utils.strCp(dataIN, IP_data);
			}
				
		}else{	//transparent mode
			Utils.strCp(dataIN, IP_data);
		}
	}else{//multiconnection mode
		
		if(parse(dataIN,"+CIPRXGET")){
			while(dataIN[i]!=','){
				i++;
			}		
			IP_data_from=dataIN[i+1]-0x30;	//gets the connection number
			answer=GetDataManually(150,IP_data_from);
		}else{			
			while(dataIN[i]!=','){
				i++;
			}
		
			IP_data_from=dataIN[i+1]-0x30;	//gets the connection number

			i+=3;
			IP_data_length=0;
			do{	//gets the length of the data string
				IP_data_length*=10;
				IP_data_length=dataIN[i];
				i++;
			}while(dataIN[i]!='\r');
			i++;	//skips \n
	
			for(int x=0;x< IP_data_length; x++){
				IP_data[x]=dataIN[i+x];
			}
		}
	}
	
	if(answer!=1) return 0;
	return 1;
}

/* closeSocket() - closes the socket specified by 'socket'
 *
 * This function closes the connection specified by 'n_connection'.In single not specifies number of connenction. For server use 8
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::closeSocket(){
	return(closeSocket(NULL));
}

/* closeSocket(uint8_t) - closes the socket specified by 'socket'
 *
 * This function closes the connection specified by 'n_connection'.In single not specifies number of connenction. For server use 8
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::closeSocket(uint8_t n_connection){
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_CLOSE);

	if(n_connection==8){//closes TCP server
		sprintf(command,"%s0",AT_IP_SERVER);
	}else{	//closes TCP or UDP client
		switch(IP_mode){
			case 0://single mode
				sprintf(command,"%s0",AT_IP_CLOSE);
				break;
			case 1://multi mode
				sprintf(command,"%s%c,0",AT_IP_CLOSE,n_connection+0x30);
				break;
		}
	}
	
	answer=sendATCommand(command,AT_IP_CLOSE_R);
	
	switch(answer){
		case 0:
			flag |= GPRS_ERROR_CLOSE;
			free(command);
			return 0;
			break;
		case 2:
			flag |= GPRS_ERROR_CLOSE;
			free(command);
			return 0;
			break;
	}
	
	if(answer!=1) return 0;
	return 1;
}

/* QuickcloseSocket(socket) - Enables/disables to close the connection quickly
 *
 * This function enables/disables to close the connection quickly. 
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::QuickcloseSocket(uint8_t mode){
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_EXTRA_PARAMETERS);

	switch(mode){
		case 0:	
			sprintf(command,"%s0",AT_IP_QCLOSE);
			break;
		case 1:
			sprintf(command,"%s1",AT_IP_QCLOSE);
			break;
	}
	
	answer=sendATCommand(command,AT_IP_QCLOSE_R,ERROR);
	
	switch(answer){
		case	0	:	
			flag |= GPRS_ERROR_EXTRA_PARAMETERS;
					free(command);
					return 0;
					break;
		case	2	:	
			flag |= GPRS_ERROR_EXTRA_PARAMETERS;
					free(command);
					return 0;
					break;
	}
	
	if(answer!=1) return 0;
	return 1;
}

/* getIPfromDNS(const char*) - Gets the IP direction from a URL using DNS servers
 *
 * This function gets the IP direction from a URL using DNS servers
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
uint8_t WaspGPRS_Pro::getIPfromDNS(const char* IP_query){
	
	char* command = (char*) calloc(100,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	char aux;
	
	sprintf(command,"%s\"%s\"",AT_IP_QUERY_DNS,IP_query);
	answer=sendATCommand(command,AT_IP_QUERY_DNS_R,ERROR);
	free(command);
	if(answer==1){
		aux=serialRead(_uart);
		if(aux=='0'){ return 0;}
		aux=0;
		while(aux<3){
			while(serialRead(_uart)!='"');
			aux++;
		}
		aux=0;
		do{
			IP_data[aux]=serialRead(_uart);
			aux++;
		}while(IP_data[aux-1]!='"');
		IP_data[aux-1]='\0';		
	}
	if (answer!=0) return 0;
	return 1;
}

/* IPHeader(uint8_t) - Adds an IP head at the beginning of a package received
 *
 * This function adds an IP head at the beginning of a package received
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::IPHeader(uint8_t on_off){
	
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_EXTRA_PARAMETERS);

	sprintf(command,"%s%u",AT_IP_HEADER,on_off);
	
	answer=sendATCommand(command,AT_IP_HEADER_R,ERROR);
	
	switch(answer){
		case	0	:
			flag |= GPRS_ERROR_EXTRA_PARAMETERS;
			free(command);
			return 0;
			break;
		case	2	:
			flag |= GPRS_ERROR_EXTRA_PARAMETERS;
			free(command);
			return 0;
			break;
	}
	
	if(answer!=1) return 0;
	return 1;
}

/* SetAutoSendingTimer(uint8_t) - Sets a timer when module is sending data
 *
 * This function sets a timer when module is sending data
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::SetAutoSendingTimer(uint8_t mode){
	return(SetAutoSendingTimer(mode,0));
}

/* SetAutoSendingTimer(uint8_t, uint8_t) - Sets a timer when module is sending data
 *
 * This function sets a timer when module is sending data
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::SetAutoSendingTimer(uint8_t mode, uint8_t time){
	
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_EXTRA_PARAMETERS);

	switch(mode){
		case 0:
			sprintf(command,"%s0",AT_IP_AUTOSENDING);
			break;
		case 1:
			sprintf(command,"%s1,%u",AT_IP_AUTOSENDING, time);
			break;
	}
	
	answer=sendATCommand(command,AT_IP_AUTOSENDING_R,ERROR);
	
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_EXTRA_PARAMETERS;
					free(command);
					return 0;
					break;
		case	2	:	flag |= GPRS_ERROR_EXTRA_PARAMETERS;
					free(command);
					return 0;
					break;
	}
	
	if(answer!=1) return 0;
	return 1;
}

/* ShowRemoteIP(uint8_t) - Enables or disables to show remote IP address and port when received data
 *
 * This function enables or disables to show remote IP address and port when received data
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::ShowRemoteIP(uint8_t on_off){
	
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_EXTRA_PARAMETERS);

	sprintf(command,"%s%u",AT_IP_SHOW_REMOTE_IP,on_off);
	
	answer=sendATCommand(command,AT_IP_SHOW_REMOTE_IP_R,ERROR);
	
	switch(answer)
	{
		case	0	:	flag |= GPRS_ERROR_EXTRA_PARAMETERS;
					free(command);
					return 0;
					break;
		case	2	:	flag |= GPRS_ERROR_EXTRA_PARAMETERS;
					free(command);
					return 0;
					break;
	}
	
	if(answer!=1) return 0;
	return 1;
}

/* ShowProtocolHeader(uint8_t) - Enables or disables to show transfer protocol in IP head when received data
 *
 * This function enables or disables to show transfer protocol in IP head when received data
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::ShowProtocolHeader(uint8_t on_off){
	
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_EXTRA_PARAMETERS);
	
	sprintf(command,"%s%u",AT_IP_PROTOCOL_HEADER,on_off);
	
	answer=sendATCommand(command,AT_IP_PROTOCOL_HEADER_R,ERROR);
	
	switch(answer)	{
		case 0:
			flag |= GPRS_ERROR_EXTRA_PARAMETERS;
			free(command);
			return 0;
			break;
		case 2:
			flag |= GPRS_ERROR_EXTRA_PARAMETERS;
			free(command);
			return 0;
			break;
	}

	if(answer!=1) return 0;
	return 1;
}

/* DiscardInputATData(uint8_t) - Enables or disables to discard input AT data in TCP data send
 *
 * This function enables or disables show to discard input AT data in TCP data send
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::DiscardInputATData(uint8_t on_off){
	
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_EXTRA_PARAMETERS);
	
	sprintf(command,"%s%u",AT_IP_DISCARD_AT_DATA,on_off);
	
	answer=sendATCommand(command,AT_IP_DISCARD_AT_DATA_R,ERROR);
	
	switch(answer)	{
		case 0:
			flag |= GPRS_ERROR_EXTRA_PARAMETERS;
			free(command);
			return 0;
			break;
		case 2:
			flag |= GPRS_ERROR_EXTRA_PARAMETERS;
			free(command);
			return 0;
			break;
	}

	if(answer!=1) return 0;
	return 1;
}

/* SetDataManually(uint8_t) - Enables or disables to get data manually from a TCP or UDP connection
 *
 * This function enables or disables show to get data manually from a TCP or UDP connection
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::SetDataManually(uint8_t on_off){
	return SetDataManually(on_off,0);
}

/* SetDataManually(uint8_t, uint8_t) - Enables or disables to get data manually from a TCP or UDP connection
 *
 * This function enables or disables show to get data manually from a TCP or UDP connection
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::SetDataManually(uint8_t on_off, uint8_t id){
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	
	flag &= ~(GPRS_ERROR_EXTRA_PARAMETERS);
	
	if(IP_mode==0){
		sprintf(command,"%s=%u",AT_IP_GET_MANUALLY,on_off);
	}else{
		sprintf(command,"%s=%u,%u",AT_IP_GET_MANUALLY,on_off,id);
	}
	answer=sendATCommand(command,AT_IP_GET_MANUALLY_R,ERROR);
	
	switch(answer){
		case 0:
			flag |= GPRS_ERROR_EXTRA_PARAMETERS;
			free(command);
			return 0;
			break;
		case 2:
			flag |= GPRS_ERROR_EXTRA_PARAMETERS;
			free(command);
			return 0;
			break;
	}

	if(answer!=1) return 0;
	return 1;
}

/* GetDataManually(uint16_t) - Gets data manually from a TCP or UDP connection
 *
 * This function gets data manually from a TCP or UDP connection
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::GetDataManually(uint16_t data_length){
	return GetDataManually(data_length,0);
}

/* GetDataManually(uint16_t, uint8_t) - Gets data manually from a TCP or UDP connection
 *
 * This function gets data manually from a TCP or UDP connection
 *
 * It modifies 'flag' if expected answer is not received after sending a command to GPRS module
 *
 * Returns '1' on success, '0' if error and '-1' if no memory
*/
int8_t WaspGPRS_Pro::GetDataManually(uint16_t data_length, uint8_t id){
	
	char* command = (char*) calloc(30,sizeof(char));
	if( command==NULL ) return -1;
	uint8_t answer=0;
	uint8_t i=0,IP_data_length;
	char aux;
	
	
	flag &= ~(GPRS_ERROR_DATA);
	
	if(IP_mode==0){
		sprintf(command,"%s=2,%u",AT_IP_GET_MANUALLY,data_length);
	}else{
		sprintf(command,"%s=2,%u,%u",AT_IP_GET_MANUALLY,id,data_length);
	}
	answer=sendATCommand(command,"+CIPRXGET=2,",ERROR);
	
	switch(answer)
	{
		case 0:
			flag |= GPRS_ERROR_DATA;
			free(command);
			return 0;
			break;
		case 1:			
			do{
				while(serialAvailable(_uart)==0);
			}while(serialRead(_uart)!=':');
			do{
				while(serialAvailable(_uart)==0);
			}while(serialRead(_uart)!=',');
			
			if(IP_mode==1){
				do{
					while(serialAvailable(_uart)==0);
				}while(serialRead(_uart)!=',');
			}
			
			IP_data_length=0;
			aux=serialRead(_uart);
			do{	//gets the length of the data string
				IP_data_length*=10;
				IP_data_length+=(aux-0x30);
				aux=serialRead(_uart);
			}while(aux!=',');
		
			do{
				while(serialAvailable(_uart)==0);
			}while(serialRead(_uart)!='\n');
		
			do{
				while(serialAvailable(_uart)==0);
				IP_data[i]=serialRead(_uart);
				i++;
			}while(i<(IP_data_length-1));
			break;
			
		case 2:
			flag |= GPRS_ERROR_DATA;
			free(command);
			return 0;
			break;
	}

	if(answer!=1) return 0;
	return 1;
}


// Preinstantiate Objects //////////////////////////////////////////////////////

WaspGPRS_Pro GPRS_Pro = WaspGPRS_Pro();

