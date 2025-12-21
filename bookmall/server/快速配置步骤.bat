@echo off
echo ========================================
echo MySQL驱动快速配置脚本
echo ========================================
echo.
echo 请按以下步骤操作:
echo.
echo 1. 下载 MySQL Connector/C
echo    地址: https://cdn.mysql.com/Downloads/Connector-C/mysql-connector-c-6.1.11-winx64.zip
echo.
echo 2. 解压ZIP文件
echo    找到: lib\libmysql.dll
echo.
echo 3. 复制到以下位置:
echo    %~dp0debug\libmysql.dll
echo    %~dp0release\libmysql.dll
echo.
echo 4. 按任意键继续，脚本会检查文件是否存在...
pause > nul
echo.
echo 正在检查...
echo.

if exist "%~dp0debug\libmysql.dll" (
    echo [成功] debug\libmysql.dll 已存在
) else (
    echo [失败] debug\libmysql.dll 不存在
    echo 请复制 libmysql.dll 到 debug 文件夹
)

if exist "%~dp0release\libmysql.dll" (
    echo [成功] release\libmysql.dll 已存在
) else (
    echo [失败] release\libmysql.dll 不存在
    echo 请复制 libmysql.dll 到 release 文件夹
)

echo.
echo 配置完成后，在Qt Creator中:
echo 1. 构建 -^> 清理项目
echo 2. 构建 -^> 重新构建
echo 3. 运行
echo.
echo ========================================
pause

