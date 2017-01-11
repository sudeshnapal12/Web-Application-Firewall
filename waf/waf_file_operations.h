
static const char *PAGE_PATH="/home/dexter/git_working/waf_apache_module/waf/WAF_Page_File";

static const char *PARAM_PATH="/home/dexter/git_working/waf_apache_module/waf/WAF_Param_File";

static int EXCEED_MAX_PARAM = 100;

static int PARAM_LEN_ILLEGAL = 101;

static int CONTAINS_NO_SEEN_CHAR = 102;

static int PASS_DETECTION = 103;

static int UNKNOWN_PARAM = 104;

static int EXCEED_ALL_MAX_PARAM = 105;

static int MAX_PARAM_PASS = 106;

struct waf_pagefile_struct {
    int rec_size;
    int max_param;
    int uri_len;
    char bufinfo[1];
};


struct waf_paramfile_struct {
    int rec_size;
    int param_sum;
    int avg_param;
    int sd_sum;
    int sd_param;
    int param_count;
    int param_len;
    int uri_len;
    int charset_len;
    char bufinfo[1];
};
