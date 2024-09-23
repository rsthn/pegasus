@echo off
if exist system.log (del system.log)
..\..\psxt -d -i -oasx/ -nasx lexicon.sx >x
