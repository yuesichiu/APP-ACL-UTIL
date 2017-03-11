#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>  
#include <getopt.h>
#include <string.h>
#include <sqlite3.h>
#include "app-acl-util.h"
#include "pbxbase.h"
#include "md5.h"

#define APP_ACL_UTIL_REVISION 	"1.4.0"
#define APP_NAME 				"app-acl-util" 

struct pbxbase_flags acl_options;

/*! These are the options that set by default*/
#define ACL_DEFAULT_OPTIONS ACL_OPT_FLAG_NONE

#define ACL_DEFAULT_DB "/var/www/db/acl.db"
#define MAX_PAH_SIZE 32
#define MAX_BUFF_SIZE 512
#define FORMAT "%5s|%10s|%10s|%33s|%10s\n"

#define acl_opt_add   	pbxbase_test_flag(&acl_options, ACL_OPT_FLAG_ADD)
#define acl_opt_delete	pbxbase_test_flag(&acl_options, ACL_OPT_FLAG_DELETE)
#define acl_opt_list	pbxbase_test_flag(&acl_options, ACL_OPT_FLAG_LIST)
#define acl_opt_update	pbxbase_test_flag(&acl_options, ACL_OPT_FLAG_UPDATE)
#define acl_opt_version	pbxbase_test_flag(&acl_options, ACL_OPT_FLAG_SHOW_VERSION)
#define acl_opt_help	pbxbase_test_flag(&acl_options, ACL_OPT_FLAG_HELP)

typedef struct acl_opts{
	sqlite3 *db;
	char *sql;
	int uid;
	char *username;
	char *password;
	char *dbfile;

}acl_opts_t;

static struct option long_options[] = {
	{ "help",    	no_argument,		NULL, 'h' },
	{ "add",    	no_argument,		NULL, 'a' },
	{ "delete",		no_argument,		NULL, 'd' },
	{ "list",		no_argument,		NULL, 'l' },
	{ "update",		no_argument,		NULL, 'U' },
	{ "username",	required_argument,	NULL, 'u' },
	{ "passwd",		required_argument,	NULL, 'p' },
	{ "version",	required_argument,	NULL, 'v' },
	{ "uid",	required_argument,	NULL, 'i' },
	{ 0, 0, 0, 0 }
};

struct acl_opts * acl_opts_init(struct acl_opts *acl)
{
	int rc = 0;
	struct acl_opts * res = acl;
	
	if(acl == NULL){
		res =(struct acl_opts  *)malloc(sizeof(struct acl_opts));
		if(!res){
			printf("failed to alloc struct acl_opts!\n");
			return NULL;
		}
		bzero(res, sizeof(struct acl_opts));
	}
	acl = res;

	acl->db = NULL;
	acl->sql = NULL;
	acl->username = NULL;
	acl->password = NULL;
	
	acl->dbfile = strdup(ACL_DEFAULT_DB);

	rc = sqlite3_open(acl->dbfile, &(acl->db));
	if(rc){
		printf("Can't open database: %s", sqlite3_errmsg(acl->db));
		return NULL;
	}
	
	return acl;
}

void acl_opts_destory(struct acl_opts *acl)
{
	if(acl->dbfile != NULL)
		free(acl->dbfile);
	if(acl->username != NULL)
		free(acl->username);

	if(acl->password != NULL)
		free(acl->password);


	if(acl->sql != NULL)
		sqlite3_free(acl->sql);
	
	if(acl->db != NULL){
		sqlite3_close(acl->db);
	}
	
	free(acl);
}


int acl_user_delete(struct acl_opts *acl)
{
	int rc;
	char *zErrMsg = 0;

	if((!acl->username )|| (acl->password || acl->uid)){
		return ACL_PARAM_INVALID;
	}	

	acl->sql = sqlite3_mprintf("DELETE from acl_membership where acl_membership.id = (select id from acl_user where name = '%s');", acl->username);
	if(acl->sql == NULL){
		printf("failed to allocate size(func : %s, line: %d)!\n",__func__, __LINE__);
		return ACL_DB_MALLOC_ERR;
	}
	rc = sqlite3_exec(acl->db, acl->sql, NULL, NULL, &zErrMsg);
	if(rc != SQLITE_OK){
		printf("SQL error : %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	sqlite3_free(acl->sql);
	acl->sql = NULL;

	acl->sql = sqlite3_mprintf("DELETE from acl_user where name ='%s' ; ", acl->username);
	if(acl->sql == NULL){
		printf("failed to allocate size(func : %s, line: %d)!\n",__func__, __LINE__);
		return ACL_DB_MALLOC_ERR;
	}
	rc = sqlite3_exec(acl->db, acl->sql, NULL, NULL, &zErrMsg);
	if(rc != SQLITE_OK){
		printf("Delete user='%s'...\t\t[ %s Failed %s]\n", acl->username, RED, NONE);
		printf("SQL error reason: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	printf("Delete user='%s'...\t\t[ %s OK %s]\n", acl->username, GREEN, NONE);
	
	return ACL_DB_OK;
	
}


int acl_user_list_callback(void *data, int argc, char **argv, char **azColName)
{
	int i;
	int pos=0;
	char buff[MAX_BUFF_SIZE] = {0};
	
	for(i = 0; i < argc; i++){
		if(strcasecmp(azColName[i], "id") ==0)
			pos +=snprintf(buff+pos, MAX_BUFF_SIZE, "%5s|", argv[i] ? argv[i]:"NULL");
			//printf("%5s|", argv[i] ? argv[i]:"NULL");
		else 	if(strcasecmp(azColName[i], "name") ==0)
			pos +=snprintf(buff+pos, MAX_BUFF_SIZE, "%10s|", argv[i] ? argv[i]:"NULL");
			//printf("%10s|",argv[i] ? argv[i]:"NULL");
		else 	if(strcasecmp(azColName[i], "description") ==0)
			//printf("%10s|", argv[i] ? argv[i]:"NULL");
			pos +=snprintf(buff+pos, MAX_BUFF_SIZE, "%10s|", argv[i] ? argv[i]:"NULL");
		else 	if(strcasecmp(azColName[i], "md5_password") ==0)
			pos +=snprintf(buff+pos, MAX_BUFF_SIZE, "%33s|", argv[i] ? argv[i]:"NULL");
			//printf("%35s|", argv[i] ? argv[i]:"NULL");
		else 	if(strcasecmp(azColName[i], "extension") ==0)
			//printf("%10s", argv[i] ? argv[i]:"NULL");
			pos +=snprintf(buff+pos, MAX_BUFF_SIZE, "%10s|", argv[i] ? argv[i]:"NULL");
	}
	printf("%s\n",buff);

	return 0;

}

int acl_user_list(struct acl_opts *acl)
{
	int rc;	
	char *zErrMsg = 0;
	const char *data = "List records";

	acl->sql = sqlite3_mprintf ("select * from acl_user");
	if(acl->sql == NULL){
		printf("failed to allocate size(func : %s, line: %d)!\n",__func__, __LINE__);
		return ACL_DB_MALLOC_ERR;
	}
	printf("%s:\n", (const char *)data);
	printf("%s"FORMAT"%s", PURPLE, "ID", "Username", "Description", "Md5 password", "Extension",NONE);
	rc = sqlite3_exec(acl->db, acl->sql, acl_user_list_callback, (void *)data, &zErrMsg);
	if(rc != SQLITE_OK){
		printf("SQL error : %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	return ACL_DB_OK;
	
}
/************************************************************************
  * Copy from Asterisk-11 source code, fill md5 info buffer
  ************************************************************************/
  
/*
static int printdigest(const unsigned char *d)
{
	int x, pos;
	char buf[256]; // large enough so we don't have to worry 

	for (pos = 0, x = 0; x < 16; x++)
		pos += sprintf(buf + pos, "%02x", (unsigned)*d++);

	printf("md5 passwd: %s\n", buf);
	return 0;
}*/

int acl_user_update(struct acl_opts *acl)
{
	int rc;
	sqlite3 *db = NULL;
	char *zErrMsg = 0;

	if((!acl->username &&! acl->password) ||(acl->uid)){
		return ACL_PARAM_INVALID;
	}
	
	int i, pos;
	unsigned char decrypt[16];   
	char md5_passwd[33] = {0};

 	MD5_CTX md5;
	MD5Init(&md5);         		
	MD5Update(&md5, acl->password, strlen(acl->password));
	MD5Final(&md5, decrypt);        

	for(pos=0, i=0; i < 16; i++)
	{
		pos += sprintf(md5_passwd + pos, "%02x", (unsigned)decrypt[i]);
	}
	/* Must be free acl->sql */
	acl->sql = sqlite3_mprintf("UPDATE acl_user set md5_password ='%s' where name ='%s' ", md5_passwd, acl->username);
	if(acl->sql == NULL){
		printf("failed to allocate size(func : %s , line: %d)!\n",__func__, __LINE__);
		return ACL_DB_MALLOC_ERR;
	}
	rc = sqlite3_exec(acl->db, acl->sql, NULL, NULL, &zErrMsg);
	if(rc != SQLITE_OK){
		printf("Update user='%s' ...\t\t[ %s Failed %s]\n", acl->username,  RED, NONE);
		printf("SQL error : %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	printf("Update user '%s'...\t\t[ %sOK %s]\n", acl->username, GREEN, NONE);

	return ACL_DB_OK;

}

int acl_user_insert(struct acl_opts *acl)
{
	int rc;
	sqlite3 *db = NULL;
	char *zErrMsg = 0;

	if(!acl->username &&! acl->password &&! acl->uid){
		return ACL_PARAM_INVALID;
	}
	int i, pos;
	unsigned char decrypt[16];   
	char md5_passwd[33] = {0};

 	MD5_CTX md5;
	MD5Init(&md5);         		
	MD5Update(&md5, acl->password, strlen(acl->password));
	MD5Final(&md5, decrypt);        

	for(pos=0, i=0; i < 16; i++)
	{
		pos += sprintf(md5_passwd + pos, "%02x", (unsigned)decrypt[i]);
	}

	acl->sql = sqlite3_mprintf(" insert into acl_membership values(%d, '%s', 3);", acl->uid, acl->username);
	if(acl->sql == NULL){
		printf("failed to allocate size(func : %s, line: %d)!\n",__func__, __LINE__);
		return ACL_DB_MALLOC_ERR;
	}
		
	rc = sqlite3_exec(acl->db, acl->sql, NULL, NULL, &zErrMsg);
	if(rc != SQLITE_OK){
		printf("SQL error : %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	sqlite3_free(acl->sql);
	acl->sql = NULL;

	
	acl->sql = sqlite3_mprintf("INSERT or REPLACE  into acl_user values(%d,'%s', '%s', '%s', '%s')", acl->uid, acl->username,  acl->username, md5_passwd, acl->username);
	if(acl->sql == NULL){
		printf("failed to allocate size(func : %s, line: %d)!\n",__func__, __LINE__);
		return ACL_DB_MALLOC_ERR;
	}
	rc = sqlite3_exec(acl->db, acl->sql, NULL, NULL, &zErrMsg);
	if(rc != SQLITE_OK){
		printf("Insert user='%s'...\t\t[ %s Failed %s]\n", acl->username, RED, NONE);
		printf("SQL error : %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	printf("Insert user= '%s'...\t\t[ %s OK %s]\n", acl->username, GREEN, NONE);

	return ACL_DB_OK;
}

int parse_opts(struct acl_opts *acl, int argc, char *argv[])
{
	int ret = 0;
	
	while (1) {
		int c;

		c = getopt_long(argc, argv, "ahdlUi:u:p:v", long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
			case 'h':
				pbxbase_set_flag(&acl_options, ACL_OPT_FLAG_HELP);
				ret = 1;
				goto end;
			case 'l':
				pbxbase_set_flag(&acl_options, ACL_OPT_FLAG_LIST);
				break;
			case 'v':
				pbxbase_set_flag(&acl_options, ACL_OPT_FLAG_SHOW_VERSION);	
				break;
			case 'a':
				pbxbase_set_flag(&acl_options, ACL_OPT_FLAG_ADD);
				break;
			case 'p':
				acl->password = strdup(optarg);
				break;
			case 'i':
				acl->uid = atoi(optarg);
				break;
			case 'd':
				pbxbase_set_flag(&acl_options, ACL_OPT_FLAG_DELETE);
				break;
			case 'U':
				pbxbase_set_flag(&acl_options, ACL_OPT_FLAG_UPDATE);
				break;
			case 'u':
				acl->username = strdup(optarg);
				break;
			default:
				ret = -1;
				goto end;
		}
	}

	while (optind < argc) {
		fprintf(stderr, "invalid argument '%s'\n", argv[optind++]);
		ret++;
	}

end:
	return ret;
}

int main(int argc, char *argv[])
{
	int ret =0;

	struct acl_opts *acl = NULL;
		
	acl = acl_opts_init(NULL);

	if (argc <= 1) {
		usage();
		goto end;
	}
		
	ret = parse_opts(acl, argc, argv);
	if (ret) {
		usage();
		goto end;
	}
	
	if(acl_opt_help){
		usage();
		goto end;
	}
	
	if(acl_opt_list){
		acl_user_list(acl);
	}
	
	if(acl_opt_version){
		show_version();
		goto end;
	}
	
	if(acl_opt_delete && !acl_opt_add && !acl_opt_update){
		ret = acl_user_delete(acl);
	}
			

	if(acl_opt_update && !acl_opt_add && !acl_opt_delete){
		ret = acl_user_update(acl);
	}

	if( acl_opt_add && !acl_opt_delete && !acl_opt_update){
		ret = acl_user_insert(acl);		
	}

	if(ret != 0){
		printf("Invalid parameters!\n");
		usage();
		goto end;
	}

end:
	acl_opts_destory(acl);
	exit(0);
	
}


void show_version(void)
{
	printf("%s version : %s\n", APP_NAME, APP_ACL_UTIL_REVISION);
}

void usage(void)
{
	printf(
			"Usage:\n"
			"  %s -h\n"
			"  %s -a -i <ID> -u <username> -p <passwd>\n"
			"  %s -d -u <username>\n"		
			"  %s -U -u <username> -p <passwd>\n"
			"  %s -l\n"
			"Options:\n"
			"  -h                  Show help\n"
			"  -i                  Specify the user ID\n"
			"  -l                  List the current user in acl.db\n"
			"  -a, --add           Create a new account in acl.db\n"
			"  -d, --delete        Delete the user from acl.db\n"
			"  -U  --update        Reset the user's passwd\n"
			"  -v  --version       Show the version of app-acl-util\n"
			"  -u  --uid           Specify the user id to be operate\n"
			"  -u  --username      Specify the username to be operate\n"
			"  -p  --passwd        Set the user's passwd\n"
			"Examples:\n"
			"  %s -U -u admin -p admin       --Update the password of user('admin')\n"
			"  %s -d -u admin                --Delete the user of 'admin'\n"
			"  %s -a -i 111 -u 111 -p pbx111 --Add the user of '111'(user id='111', passwd='pbx111')\n"
			"  %s -l                         --List of ACL users\n"
			"\n",APP_NAME,APP_NAME,APP_NAME,APP_NAME,APP_NAME, APP_NAME,APP_NAME,APP_NAME,APP_NAME);
}
