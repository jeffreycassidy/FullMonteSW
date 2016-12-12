###### Setup basic BLI script (data path, VTK version)
SET(FULLMONTE_DATA_DIR ${CMAKE_SOURCE_DIR}/data)
CONFIGURE_FILE(basic_bli.tcl.in basic_bli.tcl)

SET(FULLMONTE_DATA_DIR ${CMAKE_INSTALL_PREFIX}/data)
CONFIGURE_FILE(basic_bli.tcl.in basic_bli_install.tcl)