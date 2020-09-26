/*************************************************************************
 * 
 * Bac Son Technologies  
 * __________________
 * 
 *  [2019] Bac Son Technologies LLC 
 *  All Rights Reserved.
 * 
 * NOTICE:  All information contained herein is, and remains
 * the property of Bac Son Technologies LLC and its suppliers,
 * if any.  The intellectual and technical concepts contained
 * herein are proprietary to Bac Son Technologies LLC 
 * and its suppliers and may be covered by U.S. and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Bac Son Technologies LLC.
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <json-c/json.h>
#include <json-c/json_tokener.h>
#include <sys_msg.h>

#define JSON_TRUE	1
#define JSON_FALSE	0
#define NULL_STRING "N/A"

const char *process_json_data(char *ptr, char *search_key, int  *getint)
{
    char *tmp;
    json_object * jobj = json_tokener_parse(ptr);
    enum json_type type;

    json_object_object_foreach(jobj, key, val)
    {
        //logger_cloud("Search_Key: %s  -- Key: %s", search_key, key);
        if(0 == strcmp((char *) search_key, (char *) key))
        {
            type = json_object_get_type(val);
            switch (type) {
                case json_type_int: 
			*getint = json_object_get_int(val);
		    return NULL; 
                break;
                case json_type_boolean: 
			*getint = json_object_get_boolean(val);
		    return NULL; 
                break;
                case json_type_string:
		    return json_object_get_string(val);
                break;
            }
        }
    }
    logging(DBG_DBG,"%s: no match for key search: %s", __FUNCTION__, search_key);
    return NULL_STRING;
}

