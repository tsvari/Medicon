-- test.sql
-- @param Name         STRING   default='Givi'
-- @param BirthDate    DATE     default='2007-01-20'
-- @param WholeDateTime DATETIME default='2007-01-20 10:11:12'
-- @param BirthTime    TIME     default='10:11:12'
-- @param Height       NUMERIC  default=175
-- @param Money        NUMERIC  default=122.123000
--
-- Query selection users by user ID

Money=:Money,Height=:Height,BirthTime=:BirthTime,WholeDateTime=:WholeDateTime,BirthDate=:BirthDate,Name=:Name
