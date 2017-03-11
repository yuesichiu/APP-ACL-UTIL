#ifndef _APP_ACL_UTIL_H_
#define _APP_ACL_UTIL_H_

enum acl_option_flags{
	ACL_OPT_FLAG_NONE = (1 << 0),
	ACL_OPT_FLAG_ADD = (1 << 1),
	ACL_OPT_FLAG_DELETE = (1 << 2),
	ACL_OPT_FLAG_LIST = (1 << 3),
	ACL_OPT_FLAG_UPDATE = (1 << 4),
	ACL_OPT_FLAG_SHOW_VERSION = (1 << 5),
	ACL_OPT_FLAG_HELP = (1 << 6),
};

enum acl_result{
	ACL_DB_OK =0,
	ACL_DB_OPEN_ERR,
	ACL_DB_CONNECT_ERR,
	ACL_DB_QUERY_ERR,
	ACL_DB_INSERT_ERR,
	ACL_DB_DELETE_ERR,
	ACL_DB_MALLOC_ERR,
};

enum acl_param{
	ACL_PARAM_NONE=1,
	ACL_PARAM_INVALID,
};
void show_version(void);
void usage(void);


#endif