#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


void update_param_file(request_rec *r,int avg_param, char *param, char *uri, char *charset);

void waf_expand_page_struct(request_rec *r,FILE *file,
            struct waf_pagefile_struct *waf_page_data, char *bufinfo)
{

    struct waf_pagefile_struct *waf_page_info = NULL;
    int err = 0;

    waf_page_info = (struct waf_pagefile_struct *)malloc(sizeof(struct waf_pagefile_struct) + strlen(bufinfo) + 1);
    if (waf_page_info == NULL) {
        goto out;
    }
    memset(waf_page_info, 0, sizeof(struct waf_pagefile_struct) + strlen(bufinfo)+1);
    memcpy(waf_page_info, waf_page_data, sizeof(struct waf_pagefile_struct));
    memcpy(&waf_page_info->bufinfo, bufinfo, strlen(bufinfo));

    err = fwrite( (char *)waf_page_info,1,
            sizeof(struct waf_pagefile_struct) + strlen(bufinfo), file);
    if (err < 0) {
//    ap_rprintf("writing wafpage file failed \n");
    }
out:
 if (waf_page_info != NULL) {
        free(waf_page_info);
        waf_page_info = NULL;
    }
}

void waf_expand_param_struct(request_rec *req,FILE *file,
            struct waf_paramfile_struct *waf_data, char *bufinfo, int buf_length)
{

    struct waf_paramfile_struct  *waf_info = NULL;
    int err = 0;

    waf_info = (struct waf_paramfile_struct *)malloc(sizeof(struct waf_paramfile_struct) + buf_length+1 );
    memset(waf_info, 0,sizeof(struct waf_paramfile_struct) + buf_length+1 );
    if (waf_info == NULL) {
        goto out;
    }
    memcpy(waf_info, waf_data, sizeof(struct waf_paramfile_struct));
    memcpy(&waf_info->bufinfo, bufinfo, buf_length);

    err = fwrite((char *)waf_info,1,
            sizeof(struct waf_paramfile_struct) + buf_length,file);
    if (err < 0) {
//     ap_rprintf(req,"writing trace file failed \n");
    }

out:
    if (waf_info != NULL) {
        free(waf_info);
        waf_info = NULL;
    }
    return;
}

void record_page_file(FILE *page_file, request_rec *r,int max_param, char *uri){

    struct waf_pagefile_struct *waf_page = NULL;
    char *uri_buf = NULL;
    void *file_info = NULL;

    waf_page = (struct waf_pagefile_struct *)malloc(sizeof(struct waf_pagefile_struct));

    waf_page->max_param = max_param;

    waf_page->uri_len = strlen(uri);

    uri_buf = malloc(256);
    memset(uri_buf, 0,256);
    memcpy(uri_buf, uri, waf_page->uri_len);

    waf_page->rec_size = sizeof(struct waf_pagefile_struct) + strlen(uri);
    waf_expand_page_struct(r,page_file, waf_page, uri_buf);
out:
     if(waf_page != NULL){
        free(waf_page);
        waf_page = NULL;
    }
    if(uri_buf != NULL){
        free(uri_buf);
        uri_buf = NULL;
    }
    return;
}

void record_param_file(FILE *param_file,request_rec *req, int param_sum, char *param, char *uri, char *charset){

    struct waf_paramfile_struct *waf_param = NULL;
    void *file_info = NULL;
    int buf_length =  0;

    waf_param = (struct waf_paramfile_struct *)malloc(sizeof(struct waf_paramfile_struct));

    waf_param->param_sum = param_sum;

    waf_param->avg_param = 0;

    waf_param->sd_sum = param_sum * param_sum;

    waf_param->sd_param = 0;

    waf_param->param_count = 1;

    waf_param->param_len = strlen(param);

    waf_param->uri_len = strlen(uri);

	waf_param->charset_len = strlen(charset);

    file_info  = malloc(strlen(param) + strlen(uri) + 256);
    memset(file_info, 0, strlen(param) + strlen(uri) + 256);
    memcpy(file_info, param, strlen(param));
    memcpy(file_info + strlen(param), uri, strlen(uri));
    memcpy(file_info + strlen(param) + strlen(uri), charset, 256);
    buf_length = strlen(param) + strlen(uri) + 256;

    waf_param->rec_size = sizeof(struct waf_paramfile_struct) + buf_length;
    waf_expand_param_struct(req,param_file, waf_param, file_info, buf_length);

out:
	if(waf_param != NULL){
        free(waf_param);
        waf_param =NULL;
    }
    if(file_info != NULL){
        free(file_info);
        file_info = NULL;
    }
    return;
}

void add_char_set(const char * cur, char * charSet){
    int len = strlen(cur);
    int i = 0;
    int size = 0;
    for(i=0; i<len; i++){
        if (strchr(charSet, cur[i]) == NULL) {
            size = strlen(charSet);
            charSet[size]=cur[i];
            charSet[size+1]='\0';
        }
    }
}

void update_param_file(request_rec *req,int avg_param, char *param, char *uri, char *charset){

    struct waf_paramfile_struct *param_struct = NULL;
    struct waf_paramfile_struct *waf_param =NULL;
    void *buffer = NULL;
    void *record_buf = NULL;
    char *file_info  =NULL;
	FILE *param_file= NULL;
    bool is_record = false;
    char *param_name = NULL;
    char *uri_name= NULL;
    char *charset_name= NULL;
    int r;
    int temp;
    int ret;
    int buf_length = 0;

//    ap_rprintf(req,"------------------> update param starts <----------------\n");
    param_file = fopen(PARAM_PATH, "r+");
    if (param_file == NULL) {
 //       ap_rprintf (req," param file is null \n");
        record_param_file(param_file,req,avg_param, param, uri, charset);
        goto out;
    }

    buffer = malloc(sizeof(int));

    if (buffer == NULL) {
        return;
    }

    r = fread(buffer,1, sizeof(int), param_file);

    if (r == 0) {
        record_param_file(param_file,req,avg_param, param, uri, charset);
        goto out;
    }

 while (r > 0) {

    memcpy(&temp, buffer, sizeof(int));
	record_buf = malloc(temp);
 

    if (record_buf == NULL) {
        break;
    }
    if (temp == 0) {
        is_record = false;
        break;
    }
    fseek(param_file, -sizeof(int), SEEK_CUR);
    r = fread(record_buf,1, temp, param_file);

    param_struct = malloc(temp + 1);
    memset(param_struct, 0, temp+1);

    if (param_struct == NULL) {
        break;
    }

    memcpy(param_struct, record_buf, temp);

    param_name = malloc(param_struct->param_len + 1);
    memset(param_name, 0, param_struct->param_len + 1);
    memcpy(param_name, param_struct->bufinfo, param_struct->param_len);
	uri_name = malloc(param_struct->uri_len + 1);
    memset(uri_name, 0, param_struct->uri_len + 1);
    memcpy(uri_name, param_struct->bufinfo + param_struct->param_len, param_struct->uri_len);

    charset_name = malloc(param_struct->charset_len + 1);
    memset(charset_name, 0, param_struct->charset_len + 1);
    memcpy(charset_name, param_struct->bufinfo + param_struct->param_len + param_struct->uri_len, param_struct->charset_len);


    if ((strcmp(uri, uri_name) == 0) && (strcmp(param, param_name) == 0)){

    //    ap_rprintf(req,"uri name matches avg_param is %d\n", avg_param);

        add_char_set(charset, charset_name);

    //    ap_rprintf(req, "new charset is %s\n", charset_name);

        waf_param = (struct waf_paramfile_struct *)malloc(sizeof(struct waf_paramfile_struct));

        waf_param->param_sum = param_struct->param_sum + avg_param;

        waf_param->sd_sum = param_struct->sd_sum + (avg_param *avg_param);

        waf_param->avg_param = 0;

        waf_param->sd_param = 0;

        waf_param->param_count = param_struct->param_count +  1;

        waf_param->param_len = strlen(param_name);

        waf_param->uri_len = strlen(uri_name);

        waf_param->charset_len = strlen(charset_name);

        buf_length = strlen(param_name) + strlen(uri_name) + 256;

        file_info  = malloc(strlen(param_name) + strlen(uri_name) + 256);

        memset(file_info, 0, strlen(param_name) + strlen(uri_name) + 256);

		memcpy(file_info, param_name, strlen(param_name));

        memcpy(file_info + strlen(param_name), uri_name, strlen(uri_name));

        memcpy(file_info + strlen(param_name) + strlen(uri_name), charset_name, 256);

        waf_param->rec_size = sizeof(struct waf_paramfile_struct) + buf_length;

        fseek(param_file, -temp, SEEK_CUR);

        waf_expand_param_struct(req,param_file, waf_param, file_info, buf_length);

        is_record = true;

        break;
    }

    if (ret == -1) {
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

    if (record_buf != NULL) {
        free(record_buf);
        record_buf = NULL;
    }
    if (param_struct != NULL) {
        free(param_struct);
		param_struct = NULL;
    }
    }

    if (!is_record){
        record_param_file(param_file,req,avg_param, param, uri, charset);
    }
out:
    if(param_file != NULL){
        fclose(param_file);
        param_file= NULL;
    }
    if (buffer != NULL) {
        free(buffer);
    }
    if (record_buf != NULL) {
        free(record_buf);
    }
    if (param_struct != NULL) {
        free(param_struct);
    }
    if(waf_param != NULL){
        free(waf_param);
    }
    if(file_info != NULL){
        free(file_info);
    }
    return;
}


void update_page_file(request_rec *req,int max_param, char *uri){

    struct waf_pagefile_struct *page_struct = NULL;
    void *buffer = NULL;
    FILE *page_file;
	void *record_buf = NULL;
    bool is_record = false;
    char *uri_name;
    int r;
    int temp;
    int ret;

//    ap_rprintf(req,"---------> update page starts <--------------\n");
    page_file = fopen(PAGE_PATH, "r+");
    if (page_file == NULL){
        record_page_file(page_file,req,max_param,uri);
        goto out;
    }

    buffer = malloc(sizeof(int));

    if (buffer == NULL) {
        return ;
    }

    r = fread(buffer,1, sizeof(int), page_file);

    if (r == 0) {
        record_page_file(page_file,req,max_param, uri);
        goto out;
    }

 while (r > 0) {

    memcpy(&temp, buffer, sizeof(int));

    record_buf = malloc(temp);

    if (record_buf == NULL) {
		break;
    }
    if (temp == 0) {
        is_record = false;
        break;
    }
    fseek(page_file, -sizeof(int), SEEK_CUR);
    r = fread( record_buf,1, temp, page_file);

    page_struct = malloc(temp + 1);
    memset(page_struct, 0, temp+1);

    if (page_struct == NULL) {
        break;
    }

    memcpy(page_struct, record_buf, temp);

    uri_name = malloc(page_struct->uri_len + 1);
    memset(uri_name, 0, page_struct->uri_len + 1);
    memcpy(uri_name, page_struct->bufinfo, page_struct->uri_len);

     if (strcmp(uri, uri_name) == 0){
 //       ap_rprintf(req,"uri name matches max_param is %d\n", max_param);
        if (page_struct->max_param < max_param){
            page_struct->max_param = max_param;
            fseek(page_file, -temp, SEEK_CUR);
            ret = fwrite(page_struct, 1, temp, page_file);

        }
        is_record = true;
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

    r =fread(buffer,1, sizeof(int), page_file);

    if (record_buf != NULL) {
        free(record_buf);
        record_buf = NULL;
    }
    if (page_struct != NULL) {
        free(page_struct);
        page_struct = NULL;
    }
    }

    if (!is_record){
        record_page_file(page_file, req,max_param,uri);
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
    return ;
}

	