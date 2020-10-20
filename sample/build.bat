@echo off
pushd .
cd ..
call build
popd
if exist system.log (del system.log)
..\psxt -d -i -oasx/ -nasx lexicon.sx
if exist system.log (echo [91mFOUND ERRORS ON MEMORY ALLOCATION[0m)
