#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "http_core.h"
#include "http_request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "waf_file_operations.h"
#include "waf_train_mode.c"
#include "waf_generate_profile.c"
#include "waf_detection_mode.c"

typedef struct {
    const char *key;
    const char *value;
} kvPair;

kvPair *formData = NULL;
int params_size_post = 0;
int ap_args_to_table(request_rec *, apr_table_t **);
int getCharSetDetect(int, request_rec *, char *, char *);

typedef struct{
    const char * param_name;
    const char * param_value;
    int length;
}Params;

int get_params_request(request_rec *r, apr_off_t * params_size);

void getCharSet(int params_size, request_rec *r, char *name, char *value);

Params * get_params_request_detect(request_rec *r, apr_off_t * params_size);
Params * get_params_request_detect_post(request_rec *r, apr_off_t * postSize);

void trainModule(request_rec *r)
{
    apr_off_t params_size = 0;

    int params_request_size = get_params_request(r, &params_size);
   //ap_rprintf(r, "parameter size is %d", params_request_size);

    if(strcmp(r->method, "POST") == 0)
    {
         update_page_file(r,params_request_size, r->uri);
    } else if (strcmp(r->method, "GET") == 0)
    {
         update_page_file(r,params_request_size, r->uri);
    }

}

int get_params_request(request_rec *r, apr_off_t * params_size){
    Params * params;
    apr_table_t *table_req;   
    ap_args_to_table(r, &table_req);
    int param_size =0;
    
    const apr_array_header_t *array_req_params = apr_table_elts(table_req);
    const apr_table_entry_t * elements_req_params = (apr_table_entry_t*)array_req_params->elts;
    
    *params_size = array_req_params->nelts;
    
    params = apr_pcalloc(r->pool, sizeof(Params) * (*params_size + 1));
    int i = 0;
    if(strcmp(r->method, "GET")==0){
    for (i = 0; i < *params_size; i++) {


        params[i].param_name = elements_req_params[i].key;
        params[i].param_value = elements_req_params[i].val;
        params[i].length = strlen(params[i].param_value);
        
        if(params[i].length > 0){
            param_size++;
        //    ap_rprintf(r,"This is %s : %s : %d \n", params[i].param_name, params[i].param_value, params[i].length);
            getCharSet(params[i].length, r, (char *) params[i].param_name, (char *) params[i].param_value);
    }
   }
    }   else if(strcmp(r->method, "POST")==0){
        for (i = 0; i < params_size_post; i++) {


        //params[i].param_name = elements_req_params[i].key;
        //params[i].param_value = elements_req_params[i].val;
        //params[i].length = strlen(params[i].param_value);
        
            if(strlen(formData[i].value) > 0){
            param_size++;
        //    ap_rprintf(r,"This is %s : %s\n", formData[i].key, formData[i].value);
            getCharSet(strlen(formData[i].value), r, (char *) formData[i].key, (char *) formData[i].value);
            }

        }     
    // ap_rprintf(r,"This is %s : %s : %d \n", params[i].param_name, params[i].param_value, params[i].length);
    }
    return param_size;
}

void getCharSet(int param_length, request_rec *r, char *name, char *value)
{

    char* char_set = NULL;
    int bitmap[256] = {0,0,0,0};
    int count = 0, i, j = 0;
    while(*(value+j)!='\0')
    {
        if(bitmap[*(value+j)] == 0){
            if(islower(*(value+j))){
                for(i = 97; i<123;i++){
                    bitmap[i] = 1;
                    count++;
                }
            }else if(isupper(*(value+j))){
                for(i = 65; i<91;i++){
                    bitmap[i] = 1;
                    count++;
                }
            }else if(isdigit(*(value+j))){
                for(i = 48; i<58;i++){
                    bitmap[i] = 1;
                    count++;
                }
            }else{
                bitmap[*(value+j)] = 1;
                count++;                
            }            
        }
        j++;
    }

        count++;
        char_set = (char*) malloc(sizeof(char)*count);
        if(char_set == NULL){
            //printf("Not enough memory available");
            return;
        }
        count = 0;
        for(i=0;i<256;i++){
            if(bitmap[i] == 1){
                char_set[count] = i;
                count++;
            }
        }
        char_set[count] = '\0';

//     ap_rprintf(r, "Param length is %d, name: %s, uri: %s, char set: %s\n", param_length, name, r->uri, char_set);

       update_param_file(r,param_length, name, r->uri, char_set);

        i++;
}

void generateNormalityProfile(request_rec *r)
{
//  ap_rprintf(r, "calling generate profile\n");
    generate_param_file(r);
    return;
}

int operation_for_post(request_rec *r){

    int page_return;
    int param_return;
    int i = 0;
    int size =0;
  //  ap_rprintf(r, "param size sending %d\n", params_size_post);
   // return MAX_PARAM_PASS;

    for (i = 0; i < params_size_post; i++) {
                
        if(strlen(formData[i].value) > 0){
            size++;
        }
    }
    i=0;
  //  ap_rprintf(r, "opearation for postsize is %d", size);
    page_return = detect_page_file(r, size, r->uri);

        if(page_return == MAX_PARAM_PASS)
        {
            for (i = 0; i < params_size_post; i++) {
            //    ap_rprintf(r, " param length is  and name is%d ,%s", params[i].length, params[i].param_name);
                if(strlen(formData[i].value) ==0){
            //        ap_rprintf(r, "continue=> name is %s", params[i].param_name);
                    continue;
                }
        //    ap_rprintf(r, "forData val %d, %s, %s", strlen(formData[i].value), (char *) formData[i].key, (char *) formData[i].value) ;   
            param_return = getCharSetDetect(strlen(formData[i].value), r, (char *) formData[i].key, (char *) formData[i].value);

                if(param_return == PARAM_LEN_ILLEGAL)
                {
                    break;
                } else if(param_return == CONTAINS_NO_SEEN_CHAR)
                {
                    break;
                } else if(param_return == UNKNOWN_PARAM)
                {
                    break;
                }


            }
            return param_return;
        
        }
    
    else 
    {
        return page_return;
    }
    return PASS_DETECTION;

}

int detectAnomolies(request_rec *r)
{
    apr_off_t params_size = 0;
    Params * params;
    // kvPair *kvp;
    if(strcmp(r->method, "GET") == 0)
    {
     //   ap_rprintf(r, "in get\n");
        params = get_params_request_detect(r, &params_size);
    } else if (strcmp(r->method, "POST") == 0)
    {
    //    ap_rprintf(r, "in post\n");
     //   params = get_params_request_detect_post(r, &params_size);
        // params = readPostParamsNew(r);
       return operation_for_post(r);
    }

    int page_return;
    int param_return;
    int i = 0;
    int size =0;
//    ap_rprintf(r, "param size sending %d\n", params_size);

    for (i = 0; i < params_size; i++) {
                
        if(params[i].length > 0){
            size++;
        }
    }
    i=0;
//    ap_rprintf(r, "size is %d", size);
    page_return = detect_page_file(r, size, r->uri);

        if(page_return == MAX_PARAM_PASS)
        {
            for (i = 0; i < params_size; i++) {
            //    ap_rprintf(r, " param length is  and name is%d ,%s", params[i].length, params[i].param_name);
                if(params[i].length ==0){
            //        ap_rprintf(r, "continue=> name is %s", params[i].param_name);
                    continue;
                }
                
            param_return = getCharSetDetect(params[i].length, r, (char *) params[i].param_name, (char *) params[i].param_value);

                if(param_return == PARAM_LEN_ILLEGAL)
                {
                    break;
                } else if(param_return == CONTAINS_NO_SEEN_CHAR)
                {
                    break;
                } else if(param_return == UNKNOWN_PARAM)
                {
                    break;
                }


            }
            return param_return;
        
        }
    
    else 
    {
        return page_return;
    }
    return PASS_DETECTION;

}

int getCharSetDetect(int param_length, request_rec *r, char *name, char *value)
{
    char* char_set = NULL;
    int bitmap[256] = {0,0,0,0};
    int count = 0, i, j = 0;
    int param_return = 0;

    while(*(value+j)!='\0')
    {
        if(bitmap[*(value+j)] == 0){
            if(islower(*(value+j))){
                for(i = 97; i<123;i++){
                    bitmap[i] = 1;
                    count++;
                }
            }else if(isupper(*(value+j))){
                for(i = 65; i<91;i++){
                    bitmap[i] = 1;
                    count++;
                }
            }else if(isdigit(*(value+j))){
                for(i = 48; i<58;i++){
                    bitmap[i] = 1;
                    count++;
                }
            }else{
                bitmap[*(value+j)] = 1;
                count++;                
            }            
        }
        j++;
    }

        count++;
        char_set = (char*) malloc(sizeof(char)*count);
        if(char_set == NULL){
            return param_return;
        }
        count = 0;
        for(i=0;i<256;i++){
            if(bitmap[i] == 1){
                char_set[count] = i;
                count++;
            }
        }
        char_set[count] = '\0';
       
        param_return = detect_param_file(r, param_length, name, r->uri, char_set);
      //ap_rprintf(r,"charset value is %s\n", char_set);
        i++;
        return param_return;
}

Params * get_params_request_detect(request_rec *r, apr_off_t * params_size){
    Params * params;
    apr_table_t *table_req;   
    ap_args_to_table(r, &table_req);
    char* char_set;
    
    const apr_array_header_t *array_req_params = apr_table_elts(table_req);
    const apr_table_entry_t * elements_req_params = (apr_table_entry_t*)array_req_params->elts;
    
    *params_size = array_req_params->nelts;
    
    params = apr_pcalloc(r->pool, sizeof(Params) * (*params_size + 1));
    int i = 0;
    for (i = 0; i < *params_size; i++) {
        params[i].param_name = elements_req_params[i].key;
        params[i].param_value = elements_req_params[i].val;
        params[i].length = strlen(params[i].param_value);
    }
    return params;
}

Params * get_params_request_detect_post(request_rec *r, apr_off_t * postSize)
{
    ap_rprintf(r, "r->params %s",r->args);
    apr_array_header_t *pairs = NULL;
    apr_off_t len;
    apr_size_t size;
    int res;
    int i = 0;
    char *buffer;
    Params *kvp = NULL;
    
    apr_table_t *t; 
    ap_args_to_table(r, &t);
    

    res = ap_parse_form_data(r, NULL, &pairs, -1, HUGE_STRING_LEN);
    if (res != OK || !pairs){
     //   ap_rprintf(r, "res is numm");
        return NULL; 
    } 
    kvp = apr_pcalloc(r->pool, sizeof(Params) * (pairs->nelts + 1));
 //   ap_rprintf(r, "Anomaly pairs->nelts %d",pairs->nelts);

    while (pairs && !apr_is_empty_array(pairs)) {

        ap_form_pair_t *pair = (ap_form_pair_t *) apr_array_pop(pairs);
        apr_brigade_length(pair->value, 1, &len);
        size = (apr_size_t) len;
        buffer = apr_palloc(r->pool, size + 1);
        apr_brigade_flatten(pair->value, buffer, &size);
        buffer[len] = 0;
        kvp[i].param_name = (char *)pair->name;
        kvp[i].param_value = (char *)buffer;
     //   ap_rprintf(r, " key is %s\n", (char *)kvp[i].param_name);
     //   ap_rprintf(r, " val is %s\n", (char *)kvp[i].param_value);
        kvp[i].length = strlen(kvp[i].param_value);
        i++;
    }
    *postSize = i;

    return kvp;
}
