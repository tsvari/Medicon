-- company_delete.sql
-- @param UID STRING default=''
--
-- Delete a company by UID

DELETE FROM company WHERE "UID" = :UID RETURNING "UID";
