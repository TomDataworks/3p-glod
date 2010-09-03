@echo off
copy ..\src\api\Release\*.map .\
copy ..\samples\simple\Release\*.map .\
prep /ft simple.exe glod.dll

