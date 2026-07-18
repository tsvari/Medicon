-- insert_user_test.sql
-- @param id   NUMERIC  default=0
-- @param name STRING   default=''
-- @param age  NUMERIC  default=0

INSERT INTO users(id, name, age) VALUES(:id, :name, :age)
