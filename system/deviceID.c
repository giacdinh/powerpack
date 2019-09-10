#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <openssl/rsa.h>       /* SSLeay stuff */
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include "common.h"


SSL_CTX		*sm_global_ctx=NULL;
#define MAX_CERT_SIZE 2048
#define DEVICE_ID_STRLEN 8 

/* Default HW cryptodevice for the Flash project is the OCF cryptodev */
#define FLASH25_SM_HW_ENGINE	"cryptodev"
#define SM_CIPHER_STRING	"DES-CBC-SHA"

#define UNIT_CERT_LOC	"/mnt/sysdata/certs/unit-cert.pem"

#define DEFAULT_UNITID "00000000"


char *unit_ID()
{
	char 			*str,*ptr;
	unsigned char	buf[MAX_CERT_SIZE];
	FILE 			*fp;
	X509 			*x509;
	int 			len;
	unsigned int	id=0xFFFFFFFF;
	static unsigned char 	deviceid[DEVICE_ID_STRLEN+1];

	ERR_load_crypto_strings();

	fp = fopen(UNIT_CERT_LOC, "r");
	if(fp == NULL)
	{
		printf("Error open certificate file\n");
		return DEFAULT_UNITID;
	}

	x509 = PEM_read_X509(fp, NULL, NULL, NULL);

	fclose (fp);

	if (x509 == NULL)
	{
	  printf("error reading certificate\n");
	  return 0;
	}

	str = X509_NAME_oneline (X509_get_subject_name (x509), 0, 0);

	if (str)
	{
		ptr=strstr(str,"/CN=");

		if (ptr)
		{
			if (strlen(ptr)==4+DEVICE_ID_STRLEN)
			{
				strcpy(deviceid,ptr+4);
			}
			 else printf("error parsing unit certificate - invalid device id\n");
		}
		 else printf("error parsing unit certificate - can't find CN\n");

		OPENSSL_free (str);
	}
	 else printf("error parsing unit certificate - can't find subject\n");

	X509_free (x509);

	return (&deviceid[0]);
}

/*
 * vim:ft=c:fdm=marker:ff=unix:expandtab: tabstop=4: shiftwidth=4: autoindent: smartindent:
 */
