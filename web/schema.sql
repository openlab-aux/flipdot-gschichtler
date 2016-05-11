drop table if exists queue;
create table queue (
  id integer primary key autoincrement,
  text text not null
);
