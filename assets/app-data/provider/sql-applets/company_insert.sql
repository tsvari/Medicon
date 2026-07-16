-- company_insert.sql
-- @param SERVER_UID   NUMERIC  default=0
-- @param COMPANY_TYPE NUMERIC  default=0
-- @param NAME         STRING   default=''
-- @param ADDRESS      STRING   default=''
-- @param REG_DATE     DATE     default='2007-01-20'
-- @param JOINT_DATE   DATE     default='2007-01-20'
-- @param LICENSE      STRING   default=''
--
-- Insert a new company record

INSERT INTO company("SERVER_UID", "COMPANY_TYPE", "NAME", "ADDRESS", "REG_DATE",
 "JOINT_DATE", "LICENSE", "LOGO")
VALUES (:SERVER_UID, :COMPANY_TYPE, :NAME, :ADDRESS, :REG_DATE, :JOINT_DATE,
 :LICENSE, :LOGO) RETURNING "UID";
