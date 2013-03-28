REM A script to run Astyle for the sources

SET STYLE=--style=stroustrup --indent=spaces=4 --indent-namespaces --lineend=linux --min-conditional-indent=0
SET OPTIONS=--pad-header --unpad-paren --suffix=none --convert-tabs

astyle %STYLE% %OPTIONS% main.c

