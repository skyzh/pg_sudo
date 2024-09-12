#include "postgres.h"


#include "access/genam.h"
#include "access/htup_details.h"
#include "access/relation.h"
#include "access/sysattr.h"
#include "access/table.h"
#include "access/xact.h"
#include "catalog/catalog.h"
#include "catalog/dependency.h"
#include "catalog/indexing.h"
#include "catalog/namespace.h"
#include "catalog/objectaccess.h"
#include "catalog/pg_authid.h"
#include "catalog/pg_collation.h"
#include "catalog/pg_database.h"
#include "catalog/pg_depend.h"
#include "catalog/pg_extension.h"
#include "catalog/pg_namespace.h"
#include "catalog/pg_type.h"
#include "commands/alter.h"
#include "commands/comment.h"
#include "commands/defrem.h"
#include "commands/extension.h"
#include "commands/schemacmds.h"
#include "funcapi.h"
#include "mb/pg_wchar.h"
#include "miscadmin.h"
#include "nodes/makefuncs.h"
#include "parser/parse_func.h"
#include "storage/fd.h"
#include "tcop/utility.h"
#include "utils/acl.h"
#include "utils/builtins.h"
#include "utils/conffiles.h"
#include "utils/fmgroids.h"
#include "utils/fmgrtab.h"
#include "utils/lsyscache.h"
#include "utils/memutils.h"
#include "utils/rel.h"
#include "utils/snapmgr.h"
#include "utils/varlena.h"
#include "utils/regproc.h"

PG_MODULE_MAGIC;

PGDLLEXPORT Datum anon_start_dynamic_masking(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(anon_start_dynamic_masking);

PGDLLEXPORT Datum anon_stop_dynamic_masking(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(anon_stop_dynamic_masking);

static void
execute_sql_string(const char *sql)
{
	List	   *raw_parsetree_list;
	DestReceiver *dest;
	ListCell   *lc1;

	/*
	 * Parse the SQL string into a list of raw parse trees.
	 */
	raw_parsetree_list = pg_parse_query(sql);

	/* All output from SELECTs goes to the bit bucket */
	dest = CreateDestReceiver(DestNone);

	/*
	 * Do parse analysis, rule rewrite, planning, and execution for each raw
	 * parsetree.  We must fully execute each query before beginning parse
	 * analysis on the next one, since there may be interdependencies.
	 */
	foreach(lc1, raw_parsetree_list)
	{
		RawStmt    *parsetree = lfirst_node(RawStmt, lc1);
		MemoryContext per_parsetree_context,
					oldcontext;
		List	   *stmt_list;
		ListCell   *lc2;

		/*
		 * We do the work for each parsetree in a short-lived context, to
		 * limit the memory used when there are many commands in the string.
		 */
		per_parsetree_context =
			AllocSetContextCreate(CurrentMemoryContext,
								  "execute_sql_string per-statement context",
								  ALLOCSET_DEFAULT_SIZES);
		oldcontext = MemoryContextSwitchTo(per_parsetree_context);

		/* Be sure parser can see any DDL done so far */
		CommandCounterIncrement();

		stmt_list = pg_analyze_and_rewrite_fixedparams(parsetree,
													   sql,
													   NULL,
													   0,
													   NULL);
		stmt_list = pg_plan_queries(stmt_list, sql, CURSOR_OPT_PARALLEL_OK, NULL);

		foreach(lc2, stmt_list)
		{
			PlannedStmt *stmt = lfirst_node(PlannedStmt, lc2);

			CommandCounterIncrement();

			PushActiveSnapshot(GetTransactionSnapshot());

			if (stmt->utilityStmt == NULL)
			{
				QueryDesc  *qdesc;

				qdesc = CreateQueryDesc(stmt,
										sql,
										GetActiveSnapshot(), NULL,
										dest, NULL, NULL, 0);

				ExecutorStart(qdesc, 0);
				ExecutorRun(qdesc, ForwardScanDirection, 0, true);
				ExecutorFinish(qdesc);
				ExecutorEnd(qdesc);

				FreeQueryDesc(qdesc);
			}
			else
			{
				if (IsA(stmt->utilityStmt, TransactionStmt))
					ereport(ERROR,
							(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
							 errmsg("transaction control statements are not allowed within an extension script")));

				ProcessUtility(stmt,
							   sql,
							   false,
							   PROCESS_UTILITY_QUERY,
							   NULL,
							   NULL,
							   dest,
							   NULL);
			}

			PopActiveSnapshot();
		}

		/* Clean up per-parsetree context. */
		MemoryContextSwitchTo(oldcontext);
		MemoryContextDelete(per_parsetree_context);
	}

	/* Be sure to advance the command counter after the last script command */
	CommandCounterIncrement();
}

Datum anon_start_dynamic_masking(PG_FUNCTION_ARGS) {
  Oid			save_userid = 0;
	int			save_sec_context = 0;
  // Node	   *escontext = fcinfo->context;
  // Datum result;
  // List *funcname;
  // Oid funcoid;
  // funcname = stringToQualifiedNameList("start_dynamic_masking", escontext);
  // if (funcname == NULL) {
  //   ereport(ERROR,
  //           (errcode(ERRCODE_UNDEFINED_FUNCTION),
  //            errmsg("cannot parse function")));
  // }
  // funcoid = LookupFuncName(funcname, 0, NULL, false);
  // if (!OidIsValid(funcoid)) {
  //   ereport(ERROR,
  //           (errcode(ERRCODE_UNDEFINED_FUNCTION),
  //            errmsg("cannot find function")));
  // }
  GetUserIdAndSecContext(&save_userid, &save_sec_context);
  SetUserIdAndSecContext(BOOTSTRAP_SUPERUSERID,
                save_sec_context | SECURITY_LOCAL_USERID_CHANGE);
  // result = OidFunctionCall0Coll(funcoid, C_COLLATION_OID);
  execute_sql_string("SELECT anon.start_dynamic_masking()");
  SetUserIdAndSecContext(save_userid, save_sec_context);
  PG_RETURN_TEXT_P(cstring_to_text("operation successful"));
}


Datum anon_stop_dynamic_masking(PG_FUNCTION_ARGS) {
  Oid			save_userid = 0;
	int			save_sec_context = 0;
  GetUserIdAndSecContext(&save_userid, &save_sec_context);
  SetUserIdAndSecContext(BOOTSTRAP_SUPERUSERID,
                save_sec_context | SECURITY_LOCAL_USERID_CHANGE);
  execute_sql_string("SELECT anon.stop_dynamic_masking()");
  SetUserIdAndSecContext(save_userid, save_sec_context);
  PG_RETURN_TEXT_P(cstring_to_text("operation successful"));
}
