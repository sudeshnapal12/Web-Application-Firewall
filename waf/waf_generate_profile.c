#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>


void  generate_param_file(request_rec *req){

    struct waf_paramfile_struct *param_struct = NULL;
    void *buffer = NULL;
    void *record_buf = NULL;
    FILE *param_file;
    int r;
    int temp;
    int ret;

    param_file = fopen(PARAM_PATH, "r+");
    if (param_file == NULL) {
        return;
    }

    buffer = malloc(sizeof(int));

    if (buffer == NULL) {
        goto out;
    }

    r = fread(buffer,1, sizeof(int), param_file);
    if (r == 0) {
        goto out;
    }

     while (r > 0) {
		memcpy(&temp, buffer, sizeof(int));

    record_buf = malloc(temp);

    if (record_buf == NULL) {
        break;
    }
    if (temp == 0) {
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

    param_struct->avg_param = param_struct->param_sum/param_struct->param_count;
	param_struct->sd_param = sqrt((param_struct->sd_sum/param_struct->param_count)-(param_struct->avg_param * param_struct->avg_param));
//    ap_rprintf( req,"new avg_param are %d\n", param_struct->avg_param);
//    ap_rprintf(req,"new sd param is %d\n", param_struct->sd_param);
    fseek(param_file, -temp, SEEK_CUR);
    ret = fwrite(param_struct, 1, temp, param_file);

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

out:
    if (param_file != NULL){
        free(param_file);
        param_file = NULL;
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
    return;
}