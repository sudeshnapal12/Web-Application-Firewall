#include<string.h>
#include<stdlib.h>
#include<stdio.h>

typedef struct{
	char *key;
	char *val;
}node;

static int BUFF_SIZE = 256;
static node *hList;
static node *gList;
static node *pList;
static int hNum = 0;
static int gNum = 0;
static int pNum = 0;

void sig_parse(char* str, request_rec *r){    
	char *delimeter, *res, *lVal, *rVal, *temp, *buffer;

	delimeter = "/*";
	res = strstr(str, delimeter);
	if(res != NULL){
		*res = '\0';
	}
	delimeter = ",CONTAINS:";
	res = strstr(str, delimeter);
	
	if(res == NULL){
		printf("Delimeter doesn't exist");
		return;
	}
	buffer = (char *)malloc(sizeof(char)*BUFF_SIZE);
	if(buffer == NULL){
		printf("Unable to allocate memory. Not enough space \n");
		return;
	}
	strncpy(buffer, str, res-str);
	lVal = buffer;
	temp = res+1+strlen(delimeter);
	
	res = strstr(temp, "\"");
	rVal = (char *)malloc(sizeof(char) * BUFF_SIZE);
	if(rVal == NULL){
		printf("Unable to allocate memory. Not enough space \n");
		return;
	}
	strncpy(rVal, temp, res-temp);
	
	temp = strstr(lVal, "HEADER:");
	if(temp != NULL){ 
		hList[hNum].key = temp + strlen("HEADER:");
		hList[hNum].val = rVal;
		//ap_rprintf(r,"\n%s",hList[hNum].key);
		//ap_rprintf(r,"\n%s",hList[hNum].val);
		hNum++;
		return;
	}
	
	temp = strstr(lVal, "REQUEST_METHOD:GET,PARAMETER:");
	if(temp != NULL){
		gList[gNum].key = temp + strlen("REQUEST_METHOD:GET,PARAMETER:");
		gList[gNum].val = rVal;
		//ap_rprintf(r,"\n%s",gList[gNum].key);
		//ap_rprintf(r,"\n%s",gList[gNum].val);
		gNum++;
		return;
	}
	
	temp = strstr(lVal, "REQUEST_METHOD:POST,PARAMETER:");
	if(temp != NULL){
		pList[pNum].key = temp + strlen("REQUEST_METHOD:POST,PARAMETER:");
		pList[pNum].val = rVal;
		//ap_rprintf(r,"\n%s",pList[pNum].key);
		//ap_rprintf(r,"\n%s",pList[pNum].val);
		pNum++;
		return;
	}
	
	if(res != NULL)
		free(res);
	if(rVal != NULL)
		free(rVal);
	if(buffer != NULL)
		free(buffer);
	return;  

}

void parseConfigFile(request_rec *r){ 
	FILE * file;
	int hCount, gCount, pCount;
	file = fopen("/home/dexter/git_working/waf_apache_module/waf/signatures", "r");
	if(file == NULL){
		printf("File not found");
		return;
	}
		
	hCount = gCount = pCount = 0;
	char* line = (char*)malloc(sizeof(char) * BUFF_SIZE);
	if(line == NULL){
		printf("Unable to allocate memory. Not enough space \n");
		return;
	}
	
	while(fgets(line, BUFF_SIZE, file) != NULL){
		if(strstr(line, "HEADER:") != NULL)
			hCount++;
		if(strstr(line, "REQUEST_METHOD:GET,PARAMETER:") != NULL)
			gCount++;
		if(strstr(line, "REQUEST_METHOD:POST,PARAMETER:") != NULL)
			pCount++;
	}
	fclose(file);
	hList = (node *)malloc(sizeof(node) * hCount);
	if(hList == NULL){
		printf("Unable to allocate memory. Not enough space \n");
		return;
	}
	gList = (node *)malloc(sizeof(node) * gCount);
	if(gList == NULL){
		printf("Unable to allocate memory. Not enough space \n");
		return;
	}
	pList = (node *)malloc(sizeof(node) * pCount);
	if(pList == NULL){
		printf("Unable to allocate memory. Not enough space \n");
		return;
	}	

	hCount = gCount = pCount = 0;
		
	file = fopen("/home/dexter/git_working/waf_apache_module/waf/signatures", "r");
	if(file == NULL){
		printf("File not found");
		return;
	}
	line = (char *)malloc(sizeof(char) * BUFF_SIZE);
	if(line == NULL){
		printf("Unable to allocate memory. Not enough space \n");
		return;
	}
	while(fgets(line, BUFF_SIZE, file) != NULL){
		sig_parse(line, r);	
	}
	fclose(file);
	free(line);    
}


