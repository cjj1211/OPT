@echo off

REM MySQL 连接参数
set MYSQL_USER=root
set MYSQL_PASSWORD=admin123
set MYSQL_DATABASE=optimus
set MySQL_VERSION=0.11.0

REM 查询语句
set SQL_QUERY=SELECT DbVersion FROM optimus.versioninfo;

REM 执行查询并将结果保存到变量
for /f "tokens=*" %%a in ('mysql -u%MYSQL_USER% -p%MYSQL_PASSWORD% -e "%SQL_QUERY%" --skip-column-names %MYSQL_DATABASE%') do (
    set "RESULT=%%a"
)

REM 比较版本号
powershell -command "& {exit [version]'%RESULT%' -lt [version]'%MySQL_VERSION%'}"
if %ERRORLEVEL% equ 1 (
    goto :execute_backup
) else (
    goto :skip_backup
)

REM 执行备份
:execute_backup
echo Version is less than %MySQL_VERSION%, executing mysqldump...
mysqldump -u%MYSQL_USER% -p%MYSQL_PASSWORD% --databases %MYSQL_DATABASE% > %MYSQL_DATABASE%.sql
mysql -u%MYSQL_USER% -p%MYSQL_PASSWORD% %MYSQL_DATABASE% < %MySQL_VERSION%.sql
goto :end

REM 跳过备份
:skip_backup
echo Version is greater than or equal to %MySQL_VERSION%, skipping backup.

:end
REM 打印结果
echo %RESULT%
