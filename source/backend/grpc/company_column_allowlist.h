#pragma once

#include "column_allowlist.h"

/**
 * @brief Allow-list of valid column names for the company table.
 *
 * Used by SqlTemplate to validate $NAME identifier parameters before inlining.
 * Any FILTER_FIELD value not in this list is rejected with a descriptive error.
 */
static constexpr auto COMPANY_COLUMNS = ColumnAllowList<9>({
    "UID",
    "SERVER_UID",
    "COMPANY_TYPE",
    "NAME",
    "ADDRESS",
    "REG_DATE",
    "JOINT_DATE",
    "LICENSE",
    "LOGO"
});
