@ECHO OFF
REM Voix de Sting-Bot : XTTS-v2 sur le GPU local, vers le voice-server.
REM L'environnement Python est a part (Python 3.12, PyTorch ROCm) : voir
REM l'en-tete de tools\sting_voice_service.py pour l'installer.
SET COQUI_TOS_AGREED=1
IF "%STING_VOICE_PYTHON%"=="" SET STING_VOICE_PYTHON=D:\sting-voice\venv\Scripts\python.exe
"%STING_VOICE_PYTHON%" "%~dp0tools\sting_voice_service.py" %*
