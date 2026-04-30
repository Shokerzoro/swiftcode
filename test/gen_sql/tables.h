#pragma once

namespace sql {

// CodeGen from create_employee version 1.
static constexpr const char* create_employee = R"SQL(
INSERT INTO manager_employee (id, user_hash, display_name)
VALUES (:id, :user_hash, :display_name);
)SQL";

// CodeGen from read_employee_by_id version 1.
static constexpr const char* read_employee_by_id = R"SQL(
SELECT id, display_name
FROM manager_employee
WHERE id = %ID_VALUE% AND user_hash = %USER_HASH_VALUE%;
)SQL";

} // namespace sql
