@echo off
REM pushd .
REM cd ..
REM make wclang
REM popd
if exist system.log (del system.log)
..\psxt -d -i -oasx/ -nasx lexicon.sx >x
if exist system.log (echo [91mFOUND ERRORS ON MEMORY ALLOCATION[0m)
