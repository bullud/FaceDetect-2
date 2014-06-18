cd /d "c:\"
git clone https://github.com/face-detection/FaceDetect.git
Create a shortcut, "%ComSpec% /k "C:\FaceDetection\setup.cmd""
msbuild


================== Another way to build exe file ================
steps: 
1. Set system environment variable: OPENCV_DIR=x:\opencv\build 
2. Double-click "OpenCmd.bat"
3. cd FaceDetection
4. msbuild /p:Configuration=Release(Debug)
5. browse to \bin\Release(Debug)\Win32\FaceDetection to run the exe file. 
