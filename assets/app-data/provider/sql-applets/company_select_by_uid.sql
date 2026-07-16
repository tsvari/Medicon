-- company_select_by_uid.sql
-- @param UID STRING default=''
--
-- Select a single company by its UID

SELECT "UID", "SERVER_UID", "COMPANY_TYPE", "NAME", "ADDRESS", "REG_DATE",
 "JOINT_DATE", "LICENSE", "LOGO"
FROM company
WHERE "UID" = :UID;
