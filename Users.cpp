#include "users.h"

BOOL isUserAdmin()
{
	BOOL flag;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	flag = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup);
	if (flag)
	{
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &flag))
		{
			flag = FALSE;
		}
		FreeSid(AdministratorsGroup);
	}

	return flag;
}