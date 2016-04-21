
//#include "string.h"
#include "switchd.h"


void* jw_switchd_get_context(char *name, struct jw_switch_policy *policy_tbl, int item_max)
{
    int i = 0;

    //printf("[%s][%d]: msg type = [%s]\n", __func__, __LINE__, blobmsg_get_string(attr));

    for (i = 0; i < item_max; i++) {
        //printf("[%s][%d]: policy name = [%s]\n", __func__, __LINE__, policy_tbl[i].name);
        if (!strcmp(name, policy_tbl[i].name)) {
            return &policy_tbl[i];
        }
    }

    return NULL;
}



