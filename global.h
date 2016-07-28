#ifndef GLOBAL_H
#define GLOBAL_H

#include "fastdb/cli.h"

#define CASE_ERROR_CODE(x) \
	case x: \
		return #x; \
		break; \

static const char*
get_cli_error_msg(int cli_code) {
	switch(cli_code) {
		CASE_ERROR_CODE(cli_bad_address);
		CASE_ERROR_CODE(cli_connection_refused);
		CASE_ERROR_CODE(cli_database_not_found);
		CASE_ERROR_CODE(cli_bad_statement);
		CASE_ERROR_CODE(cli_parameter_not_found);
		CASE_ERROR_CODE(cli_unbound_parameter);
		CASE_ERROR_CODE(cli_column_not_found);
		CASE_ERROR_CODE(cli_incompatible_type);
		CASE_ERROR_CODE(cli_network_error);
		CASE_ERROR_CODE(cli_runtime_error);
		CASE_ERROR_CODE(cli_bad_descriptor);
		CASE_ERROR_CODE(cli_unsupported_type);
		CASE_ERROR_CODE(cli_not_found);
		CASE_ERROR_CODE(cli_not_update_mode);
		CASE_ERROR_CODE(cli_table_not_found);
		CASE_ERROR_CODE(cli_not_all_columns_specified);
		CASE_ERROR_CODE(cli_not_fetched);
		CASE_ERROR_CODE(cli_already_updated);
		CASE_ERROR_CODE(cli_table_already_exists);
		CASE_ERROR_CODE(cli_not_implemented);
		CASE_ERROR_CODE(cli_xml_parse_error);
		CASE_ERROR_CODE(cli_backup_failed);
		default:
			return "Unknown error";
			break;
	}
	return "";
}

#endif
