-- company_select.sql
-- @param SERVER_UID   NUMERIC  default=0
-- @param FILTER_FIELD STRING   default=NAME
-- @param FILTER_VALUE STRING   default=''
-- @param OFFSET       NUMERIC  default=0
-- @param LIMIT        NUMERIC  default=100
--
-- Select companies with optional filter and pagination

SELECT "UID", "SERVER_UID", "COMPANY_TYPE", "NAME", "ADDRESS", "REG_DATE",
 "JOINT_DATE", "LICENSE", "LOGO"
FROM company
WHERE "SERVER_UID" = :SERVER_UID
  AND $FILTER_FIELD LIKE '%' || :FILTER_VALUE || '%'
OFFSET :OFFSET
LIMIT :LIMIT;
