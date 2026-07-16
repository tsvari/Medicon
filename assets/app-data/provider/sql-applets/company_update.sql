-- company_update.sql
-- @param UID          STRING   default=''
-- @param SERVER_UID   NUMERIC  default=0
-- @param COMPANY_TYPE NUMERIC  default=0
-- @param NAME         STRING   default=''
-- @param ADDRESS      STRING   default=''
-- @param REG_DATE     DATE     default='2007-01-20'
-- @param JOINT_DATE   DATE     default='2007-01-20'
-- @param LICENSE      STRING   default=''
--
-- Update an existing company record

UPDATE company
SET "SERVER_UID" = :SERVER_UID,
    "COMPANY_TYPE" = :COMPANY_TYPE,
    "NAME" = :NAME,
    "ADDRESS" = :ADDRESS,
    "REG_DATE" = :REG_DATE,
    "JOINT_DATE" = :JOINT_DATE,
    "LICENSE" = :LICENSE,
    "LOGO" = :LOGO
WHERE "UID" = :UID
RETURNING "UID";
