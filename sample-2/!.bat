@echo off
cls
pushd ..
call !
popd
if exist system.log (del system.log)
..\psxt -d -i -o asx/ -n asx lexicon.sx
