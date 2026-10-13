@echo off
cd /d "%~dp0"
start "" samshosen.exe --game_data_root="%~dp0extracted" --gpu_allow_invalid_fetch_constants=true %*
exit /b 0
