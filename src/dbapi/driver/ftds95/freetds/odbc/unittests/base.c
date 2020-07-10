#include "common.h"

#include <common/test_assert.h>

/* TODO place comment here */

static char software_version[] = "$Id: base.c 487476 2015-12-17 19:48:39Z ucko $";
static void *no_unused_var_warn[] = { software_version, no_unused_var_warn };

int
main(int argc, char *argv[])
{
	/* TODO remove if not neeeded */
	odbc_use_version3 = 1;
	odbc_connect();

	/* TODO write your test */

	odbc_disconnect();
	return 0;
}
