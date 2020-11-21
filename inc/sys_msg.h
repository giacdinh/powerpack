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

#ifndef __INCSYSMSGh
#define __INCSYSMSGh

#ifdef __cplusplus
extern "C"  {
#endif

#include <sys/msg.h>
#include <sys/ipc.h>

// Define each module message key value
#define WD_MSGQ_KEY		    0x00001111
#define CTRL_MSGQ_KEY		0x00002222
#define REMOTEM_MSGQ_KEY	0x00003333
#define CONFIG_MSGQ_KEY		0x00004444
#define SU_MSGQ_KEY			0x00005555

#define SYS_MSG_TYPE 0xf1a322

#define MSG_TIMEOUT 5 /* Try 5 second before read timeout */

typedef enum {
	REMOTEM_MODULE_ID,
	CTRL_MODULE_ID,
	CONFIG_MODULE_ID,
	SU_MODULE_ID,
    UNKNOWN_MODULE_ID
} MODULE_ID_ENUM;

typedef enum {
    MSG_BARK,
    MSG_TAG,
	MSG_REBOOT,
    MSG_INVALID
} MSG_TYPE_ENUM;

typedef struct {
    MODULE_ID_ENUM     moduleID;     /* sender's module ID */
    MSG_TYPE_ENUM      subType;      /* message command */
    unsigned long	   msgtime;	 /* For watchdog time keeper */
    int			   data;
} GENERIC_MSG_HEADER_T;

typedef struct {
    GENERIC_MSG_HEADER_T header;
} REMOTEM_MSG_T;

typedef struct {
    GENERIC_MSG_HEADER_T header;
} CTRL_MSG_T;

typedef struct {
    GENERIC_MSG_HEADER_T header;
} CONFIG_MSG_T;

typedef struct {
    GENERIC_MSG_HEADER_T header;
} SU_MSG_T;

typedef enum {
    DBG_ERROR = 0,
	DBG_DBG = 0,
    DBG_WARNING,
    DBG_EVENT,
    DBG_INFO,
    DBG_DETAILED,
    DBG_ALL
} LOG_LEVEL_ENUM_T;

typedef struct {
    unsigned int module_id;
    long     timer;
    int     reboot;
} WD_RESPONSE_T;

int create_msg(key_t p_key);
int send_msg(int msgid, void *msg, int size, int timeout);
int recv_msg(int msgid, void *msg, int size, int timeout);
void send_dog_bark(int);
int logging(int level, char *logstr, ...);
int open_msg(key_t key);
int get_str_json(int itemID, char *strdata);
char *unit_ID();

extern char *module_list[];
extern char *modname[];


#endif
