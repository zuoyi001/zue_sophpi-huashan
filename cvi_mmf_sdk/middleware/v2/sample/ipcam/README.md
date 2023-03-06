# how to build and use ipcam 
1. cd ./middleware/sample/ipcam
2. make ; "make install" (optional)
3. copy ./ipcam/install binary and ./ipcam/parameter corresponding ini file to board
4. run APP on board; example: "./ipcam -i param_config_ai.ini &"
## Notes:
 * source build environment before build ipcam
 * make will output ipcam_mars or ipcam_mercury (automatic dependency platform) and cmdTest. cmdTest for test ipcam
 * most parameters can modify in *.ini files
 * ipcam include double source code, mercury folder upward compatible CV1835 and mars folder downward compatible Phobos
------------------------------------------------------------------------------------
# ipcam folder tree overview
| Folder/File | Description                                                  |
| ----------- | ------------------------------------------------------------ |
| cmd         | cmtTest binary for test ipcam by socket                      |
| ChangeLog   | version list                                                 |
| common      | include common headers, socket and OS API                    |
| Makefile    | build ipcam and cmdTest, ipcam output will dependent on ARCH |
| mars        | for mars platform                                            |
| mercury     | for mercury platform                                         |
| parameter   | parameter ini files                                          |
| README.md   | notes                                                        |
| resource    | include ai models , rtsp libs , video pipeline , and so on   |
| sensor.mk   | sensor support list                                          |


