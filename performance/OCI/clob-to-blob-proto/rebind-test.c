
#include "ocilib.h"
#include <stdio.h>

#define OCI_REBINDING_ENABLED TRUE

void err_handler(OCI_Error *err) {
    fprintf(stderr, "OCILib Error: %s\n", OCI_ErrorGetString(err));
}

int main(void) {
    const char *db = "lestrade/orcl1901";        // Change as needed
    const char *user = "jkstill";   // Change as needed
    const char *pwd = "grok";    // Change as needed

    if (!OCI_Initialize(err_handler, NULL, OCI_ENV_DEFAULT)) {
        return EXIT_FAILURE;
    }

    OCI_Connection *cn = OCI_ConnectionCreate(db, user, pwd, OCI_SESSION_DEFAULT);

    if (!cn) {
		 printf("Connection failed for %s/%s@%s\n", user,pwd,db);
		 OCI_Cleanup();
		 return EXIT_FAILURE;
	 }

    OCI_Statement *st = OCI_StatementCreate(cn);
    OCI_Resultset *rs = NULL;
    int id = 1;

    // Allow rebinding
    OCI_AllowRebinding(st, OCI_REBINDING_ENABLED);

    OCI_Prepare(st, "SELECT name FROM test_table WHERE id = :id");

    // First binding and execution
    OCI_BindInt(st, ":id", &id);

    if (OCI_Execute(st)) {
        rs = OCI_GetResultset(st);
        if (OCI_FetchNext(rs)) {
            printf("First lookup (id=1): %s\n", OCI_GetString(rs, 1));
        }
    } else {
        printf("Execute failed on first lookup\n");
    }

    // Change id
    id = 2;
    OCI_BindInt(st, ":id", &id);

    // Second execution — with rebinding allowed
    if (OCI_Execute(st)) {
        rs = OCI_GetResultset(st);
        if (OCI_FetchNext(rs)) {
            printf("Second lookup (id=2): %s\n", OCI_GetString(rs, 1));
        }
    } else {
        printf("Execute failed on second lookup\n");
    }

    OCI_StatementFree(st);
    OCI_ConnectionFree(cn);
    OCI_Cleanup();
    return EXIT_SUCCESS;
}

