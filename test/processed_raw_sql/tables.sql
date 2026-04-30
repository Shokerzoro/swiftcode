-- CodeGen: create_employee version 1. Need update (yes/no): no
INSERT INTO manager_employee (id, user_hash, display_name)
VALUES (:id, :user_hash, :display_name);

-- CodeGen: read_employee_by_id version 1. Need update (yes/no): no
SELECT id, display_name
FROM manager_employee
WHERE id = id_value AND user_hash = user_hash_value; -- replaceable: id_value, user_hash_value
