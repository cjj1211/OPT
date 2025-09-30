@echo off
::--------------增量备份mysql数据库---------------
::服务器数据库ip 用户名 密码 申明需要备份的数据库
:: 设置服务器数据库ip、用户名、密码、需要备份的数据库
set suser=%1
set suserpwd=%2
set databasesname=%3
:: 刷新MySQL日志
mysqldump -u%suser% -p%suserpwd% --databases %databasesname% --hex-blob --where="(UpdateDateTime BETWEEN '%4' AND '%5')" > %6
