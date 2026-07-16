-- company_count.sql
-- @param SERVER_UID   NUMERIC  default=0
-- @param FILTER_FIELD STRING   default=NAME
-- @param FILTER_VALUE STRING   default=''
--
-- Count companies matching optional filter

SELECT COUNT(*) AS "ROW_COUNT"
FROM company
WHERE "SERVER_UID" = :SERVER_UID
  AND $FILTER_FIELD LIKE '%' || :FILTER_VALUE || '%';
