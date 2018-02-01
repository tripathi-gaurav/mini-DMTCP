#define MY_MAX_LEN 200
struct MemoryRegion
{
	void *startAddr;
	void *endAddr;
	int isReadable;
	int isWriteable;
	int isExecutabl;
	char location[150];
};
char* my_strconcat(char c1[], char c2[]);
char* getLine(int fd);
struct MemoryRegion* parseLineToMemoryRegion(char* lineToParse);
void* getAddressFromLine(char* lineToParse, int charactersRead, char delimeter);
int convertHexToLongLongInt(unsigned long long int *memoryAddressInLongLongInt, char* memoryAddressString);
void parseAndSetPermissions(char *lineToParse, int readFrom, struct MemoryRegion* memoryRegion);


char* getLine(int fileDescriptor){
	int i=0;
	char* lineToReturn = malloc( (sizeof(char) * (MY_MAX_LEN) ));
	char value;
	int bytesRead = 0;
	while(1){
		bytesRead = read(fileDescriptor, &value, 1);
		//printf("bytesRead = %d", bytesRead);
		if(bytesRead < 0){
			lineToReturn[i] = '\0';
			break;
		}else{
			lineToReturn[i++] = value;
		}
		if(bytesRead == 0 || value=='\n' || value=='\0'){
			lineToReturn[i] = '\n';
			if( value == -1){
				lineToReturn[0] = '\0';
			}
			//printf("val at 0=%c\n", lineToReturn[0]);
			break;
		}
	}
	return lineToReturn;
}



struct MemoryRegion* parseLineToMemoryRegion(char *lineToParse){
	struct MemoryRegion* memoryRegion = malloc(sizeof(struct MemoryRegion));
	void *address;
	int charactersRead = 0;

	/*
	int j=0;
	//TODO: strange behavior. lineToParse[j] cannot be compared to \n as it reaches a gibberish character before
	// therefore, comparing it to -1 instead to figure out the end of line;
	for(j=0;lineToParse[j] > 0; j++){
		printf("%c", lineToParse[j]);
	}
	printf("len=%d", j);
	*/

	if( (address = getAddressFromLine(lineToParse, charactersRead, '-')) != NULL ){
		memoryRegion->startAddr = address;
	}

	while(lineToParse[charactersRead++] != '-');
	//charactersRead += 1;

	if( (address = getAddressFromLine(lineToParse, charactersRead, ' ')) != NULL ){
		memoryRegion->endAddr = address;
	}

	while(lineToParse[charactersRead++] != ' ');
	//charactersRead += 1;

	parseAndSetPermissions(lineToParse, charactersRead, memoryRegion);

	while(lineToParse[charactersRead++] != ' ');
	charactersRead += 1;

	int i=0;
	char temp[150];
	//printf("chars read=%d\n", charactersRead);
	//TODO: same reason as mentioned in previous todo. :(
	//printf("!!note this!!\n");
	while(lineToParse[charactersRead] >0 ){
		//printf("char is: %c and charactersRead=%d \n", lineToParse[charactersRead], charactersRead);
		//printf("%c", lineToParse[charactersRead]);
		temp[i++] = lineToParse[charactersRead++];
	}
	temp[i] = '\0';
	//printf("\n");

	//printf("location should be=%s\n", temp);
	//memoryRegion->location = malloc(sizeof(char)*150);
	strcpy(memoryRegion->location,temp);
	//memoryRegion->location[] = temp;
	//printf("but location now =%s\n", memoryRegion->location);

	return memoryRegion;

}



void* getAddressFromLine(char *lineToParse, int charactersRead, char delimeter){
	void *address = NULL;
	char memoryAddressString[110];
	unsigned long long int memoryAddress;
	int i, converted;


	//printf("charactersRead++=%d", ++*charactersRead);

	if(lineToParse[charactersRead] == '\n'){
		return address;
	}

	for(i=0; lineToParse[charactersRead] != delimeter; i++){
		memoryAddressString[i] = lineToParse[charactersRead];
		charactersRead = (charactersRead) + 1;
	}
	//printf("*charactersRead=%d", charactersRead);
	memoryAddressString[i] = '\0';
	//printf("Memory address: %s\n", memoryAddressString);

	converted = convertHexToLongLongInt(&memoryAddress, memoryAddressString);
	if(converted < 0){
		printf("Failed to convert hex to int.\n");
		return NULL;
	}
	address = (void*) memoryAddress;
	//printf("address = %llu\n", (unsigned long long int) address);
	return address;
}

int convertHexToLongLongInt(unsigned long long int* memoryAddressInLongLongInt, char* memoryAddressString){

	int hexCharToIntTable[] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    	-1,-1, 0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,
    	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    	-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    };

	int success = -1;
	int longLongIntArray[100];
	int i,j;
	unsigned long long int currentValue, calculatedValue, placeValue = 1;

	calculatedValue = 0;

	for(i = 0; memoryAddressString[i] != '\0'; i++){

		//printf("ascii val at i= %d is = %d\n",i, (int)memoryAddressString[i]);
		success = hexCharToIntTable[(int)memoryAddressString[i]];
		if(success == -1){
			return success;
		}
		longLongIntArray[i] = success;
	}

	for(j=i-1; j>=0; j--){
		currentValue = longLongIntArray[j] * placeValue;
		calculatedValue += currentValue;
		placeValue *= 16;
	}
	*memoryAddressInLongLongInt = calculatedValue;
	success = 1;
	return success;
}

void parseAndSetPermissions(char *lineToParse, int readFrom, struct MemoryRegion* memoryRegion){
	//printf("chu giri. read=%c, write=%c, exec=%c", lineToParse[readFrom], lineToParse[readFrom+1], lineToParse[readFrom+2]);
	if(lineToParse[readFrom] == 'r'){
		memoryRegion->isReadable = 1;
	}else{
		memoryRegion->isReadable = 0;
	}

	if(lineToParse[readFrom+1] == 'w'){
		memoryRegion->isWriteable = 1;
	}else{
		memoryRegion->isWriteable = 0;
	}

	if(lineToParse[readFrom+2] == 'x'){
		memoryRegion->isExecutabl = 1;
	}else{
		memoryRegion->isExecutabl = 0;
	}
}

char* my_strconcat(char c1[], char c2[]){
	int i, j;
	i = j = 0;
	char * temp;

	while(c1[i] != '\0'){
		i++;
	}

	while( (c1[i++] = c2[j++]) != '\0' );
	temp = c1;
	return temp;
}
