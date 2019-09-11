OPENCPN_3RDPARTY_PATH = $${PWD}/3rdParty
IDE_APP_PATH = $$dirname(PWD)/bin
OPENCPN_3RD_INCLUDE_PATH = $${OPENCPN_3RDPARTY_PATH}/include
OPENCPN_3RD_STATIC_LIB_PATH = $${OPENCPN_3RDPARTY_PATH}/lib

defineReplace(qtLibraryName) {
   unset(LIBRARY_NAME)
   LIBRARY_NAME = $$1
   CONFIG(debug, debug|release) {
      !debug_and_release|build_pass {
          mac:RET = $$member(LIBRARY_NAME, 0)_debug
              else:win32:RET = $$member(LIBRARY_NAME, 0)d
      }
   }
   isEmpty(RET):RET = $$LIBRARY_NAME
   return($$RET)
}

#根据不同的编译清空生成不同的输出路径
CONFIG(release, debug|release) {
  DEFINES *= RELEASE _RELEASE NDEBUG
  CONFIG_NAME = Release
} else {
  DEFINES *= DEBUG _DEBUG
  CONFIG_NAME = Debug
  EXT_NAME = d
}

