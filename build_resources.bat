@echo off

php "..\builder\make_resource.php" ".\src\resource.hpp"
php "..\builder\make_locale.php" "Ip Lookup" "iplookup" ".\bin\i18n" ".\src\resource.hpp" ".\src\resource.rc" ".\bin\iplookup.lng"
copy /y ".\bin\iplookup.lng" ".\bin\32\iplookup.lng"
copy /y ".\bin\iplookup.lng" ".\bin\64\iplookup.lng"

pause
