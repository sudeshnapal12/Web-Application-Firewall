#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// #include "waf_file_operations.h"


int check_char_set(const char * cur, const char * seenChar){
    int len = strlen(cur);
    int i = 0;
    for (i=0; i<len; i++) {
        if (strchr(seenChar,cur[i])==NULL) {
            return 0;
        }
    }
    return 1;
}

int detect_param_file(request_rec *req,int len, char *param, char *uri, char *charset){

    struct waf_paramfile_struct *param_struct = NULL;
    void *buffer = NULL;
    void *record_buf = NULL;
    char *file_info  =NULL;
    FILE *param_file= NULL;
    char *param_name= NULL;
    char *uri_name= NULL;
    char *charset_name= NULL;
    int r;
    int temp;
    int ret = 0;

    param_file = fopen(PARAM_PATH, "r+");
    if (param_file == NULL) {
    //    ap_rprintf (req," param file is null \n");
        return -1;
	 }

    buffer = malloc(sizeof(int));

    if (buffer == NULL) {
        ret  = -1;
        goto out;
    }

    r = fread(buffer,1, sizeof(int), param_file);
    //ap_rprintf(req," r is %d\n", r);
    if (r == 0) {
        ret = -1;
        goto out;
    }

 while (r > 0) {

    memcpy(&temp, buffer, sizeof(int));

    record_buf = malloc(temp);

    if (record_buf == NULL) {
        ret = -1;
        break;
    }
    if (temp == 0) {
        ret = -1;
        break;
    }
    fseek(param_file, -sizeof(int), SEEK_CUR);
    r = fread(record_buf,1, temp, param_file);

    param_struct = malloc(temp + 1);
    memset(param_struct, 0, temp+1);

	if (param_struct == NULL) {
        ret = -1;
        break;
    }

    memcpy(param_struct, record_buf, temp);

    /*ap_rprintf(req," ==> Recorded Values <== \n");
    ap_rprintf(req," Recorded size       : %d\n", param_struct->rec_size);
    ap_rprintf(req," Recorded avg param       : %d\n", param_struct->avg_param);
    ap_rprintf(req," Recorded sd param          : %d\n", param_struct->sd_param);
    ap_rprintf(req," Recorded param count     : %d\n", param_struct->param_count);
    ap_rprintf(req," Recorded param len      : %d\n", param_struct->param_len);
    ap_rprintf(req," Recorded uri len   : %d\n", param_struct->uri_len);
    ap_rprintf(req," Recordedcharset len   : %d\n", param_struct->charset_len);
    ap_rprintf(req," Recorded bufinfo are :  %s\n", param_struct->bufinfo);*/

    param_name = malloc(param_struct->param_len + 1);
    memset(param_name, 0, param_struct->param_len + 1);
    memcpy(param_name, param_struct->bufinfo, param_struct->param_len);

    uri_name = malloc(param_struct->uri_len + 1);
    memset(uri_name, 0, param_struct->uri_len + 1);
    memcpy(uri_name, param_struct->bufinfo + param_struct->param_len, param_struct->uri_len);

    charset_name = malloc(param_struct->charset_len + 1);
    memset(charset_name, 0, param_struct->charset_len + 1);
    memcpy(charset_name, param_struct->bufinfo + param_struct->param_len + param_struct->uri_len, param_struct->charset_len);

//    ap_rprintf(req,"param_name is %s\n", param_name);
//    ap_rprintf(req,"uri_name is %s\n", uri_name);
//    ap_rprintf(req,"charset name is %s\n", charset_name);

    if (strcmp(uri, uri_name) == 0 && strcmp(param, param_name) == 0){
     //   printf("uri name matches avg_param is %d and temp is  %d\n", len, temp);
		if(len > (param_struct->avg_param + 3*param_struct->sd_param) || len <(param_struct->avg_param - 3*param_struct->sd_param)){
     //       ap_rprintf(req," param length illegal \n");
            ret = PARAM_LEN_ILLEGAL;
        }else if(!check_char_set(charset, charset_name)){
    //        ap_rprintf(req," all character not see \n");
            ret = CONTAINS_NO_SEEN_CHAR;
        }else {
     //       printf(" pass detection \n");
            ret  = PASS_DETECTION;
        }

        break;
    }

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    buffer = malloc(sizeof(int));

    if (buffer == NULL) {
        break;
    }

    r = fread(buffer,1, sizeof(int), param_file);
//    ap_rprintf(req," new r is %d\n", r);

    if (record_buf != NULL) {
        free(record_buf);
        record_buf = NULL;
    }
    if (param_struct != NULL) {
        free(param_struct);
        param_struct = NULL;
    }
	 }

    if (ret == 0) {
        ret = UNKNOWN_PARAM;
    }

out:
    if (buffer != NULL) {
        free(buffer);
    }
    if (record_buf != NULL) {
        free(record_buf);
    }
if (param_struct != NULL) {
        free(param_struct);
    }
    return ret;
}



int detect_page_file(request_rec *req,int max_param, char *uri){

    struct waf_pagefile_struct *page_struct = NULL;
    void *buffer = NULL;
    FILE *page_file;
    void *record_buf = NULL;
    char *uri_name;
    int max_param_all = -1;
    int r;
    int temp;
    int ret = 0;

    page_file = fopen(PAGE_PATH, "r+");
    if (page_file == NULL){
//		ap_rprintf(req," page_file is NULL \n");
        return -1;
    }

    buffer = malloc(sizeof(int));

    if (buffer == NULL) {
        return -1;
    }

    r = fread(buffer,1, sizeof(int), page_file);
//    ap_rprintf(req," r is %d\n", r);
    if (r == 0) {
        ret = -1;
        goto out;
    }

 while (r > 0) {

    memcpy(&temp, buffer, sizeof(int));
    record_buf = malloc(temp);

    if (record_buf == NULL) {
        ret = -1;
        break;
    }
    if (temp == 0) {
        ret = -1;
        break;
    }
    fseek(page_file, -sizeof(int), SEEK_CUR);
    r = fread( record_buf,1, temp, page_file);

    page_struct = malloc(temp + 1);
    memset(page_struct, 0, temp+1);
    if (page_struct == NULL) {
        ret = -1;
        break;
    }

    memcpy(page_struct, record_buf, temp);

    /*ap_rprintf(req," ==> Recorded Values <== \n");
    ap_rprintf(req," Recorded size       : %d\n", page_struct->rec_size);
    ap_rprintf(req," Recorded max param       : %d\n", page_struct->max_param);
    ap_rprintf(req," Recorded uri len          : %d\n", page_struct->uri_len);
    ap_rprintf(req," Recorded buf info     : %s\n", page_struct->bufinfo);*/

    if (page_struct->max_param > max_param_all){
        max_param_all = page_struct->max_param;
    }
    uri_name = malloc(page_struct->uri_len + 1);
    memset(uri_name, 0, page_struct->uri_len + 1);
    memcpy(uri_name, page_struct->bufinfo, page_struct->uri_len);
//    ap_rprintf(req,"uri_name is %s", uri_name);
//    ap_rprintf(req, " stored param is %d\n", page_struct->max_param);
//    ap_rprintf(req,"uri max_param is %d\n", max_param);
     if (strcmp(uri, uri_name) == 0){
//        ap_rprintf(req,"uri name matches max_param is %d\n", max_param);
        if(page_struct->max_param >=max_param){
            ret = MAX_PARAM_PASS;
        } else {
            ret = EXCEED_MAX_PARAM;
        }
        break;
    }

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    buffer = malloc(sizeof(int));
	if (buffer == NULL) {
        ret = -1;
        break;
     }

    r =fread(buffer,1, sizeof(int), page_file);
//    ap_rprintf(req,"r is %d", r);

    if (record_buf != NULL) {
        free(record_buf);
        record_buf = NULL;
    }
    if (page_struct != NULL) {
        free(page_struct);
        page_struct = NULL;
    }
    }

    if (ret == 0){
        if(max_param_all >= max_param){
            ret = PASS_DETECTION;
        }else{
            ret = EXCEED_ALL_MAX_PARAM;
        }
    }

out:
    if(page_file !=NULL){
        fclose(page_file);
        page_file = NULL;
    }
    if (buffer != NULL) {
        free(buffer);
    }
    if (record_buf != NULL) {
        free(record_buf);
	 }
    if (page_struct != NULL) {
        free(page_struct);
    }
    return ret ;
}