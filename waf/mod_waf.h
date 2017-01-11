#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "http_core.h"
#include "http_request.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "anomaly_detection_phase.c"
#include "waf_parse_signature.c"
/*#include "waf_file_operations.h"
#include "waf_train_mode.c"
#include "waf_generate_profile.c"
#include "waf_detection_mode.c"*/

/*typedef struct {
    const char *key;
    const char *value;
} kvPair;

static kvPair *formData= NULL;
int param_size_post = 0;*/

static int MODE_CURRENT = 999;

static int MODE_TRAIN = 998;

static int MODE_NORMALITY_PROFILE = 997;

static int MODE_DETECT = 999;

static int FLAG_GENERATE = 0;

static int NON_MALICIOUS = 11;

static int MALICIOUS = 12;

static char* modeSetUri ="/joomla/";

//static const char *ADMIN_CONFIG_FILE="/home/ankit/waf_git/waf_apache_module/waf/admin_config";
//static const char *SIGNITURE_CONFIG="/home/ankit/waf_git/waf_apache_module/waf/signatures";

static char * illegalStr;

int readCurrMode();

void read_admin_config_file(request_rec *r, char *usr, char *pwd);

int examineGetParams(request_rec *r, node * getSigs, int getSigLen);

int examinePostParams(request_rec *r, node * postSigs, int postSigsCount);

int isLegal(request_rec *r, const char* key, const char* value, node * list, int listSize);

void toLowerCase(char * str);