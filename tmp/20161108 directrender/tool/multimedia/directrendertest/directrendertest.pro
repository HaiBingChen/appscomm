#-------------------------------------------------
#
# Project created by QtCreator 2015-09-25T09:20:40
#
#-------------------------------------------------

TEMPLATE = subdirs


MY_WM_SYSTEM=$$(ATC_YOCTO_WM)
equals(MY_WM_SYSTEM, wayland) {
	SUBDIRS += dtdemo_bin.pro
} else {
	SUBDIRS += dtdemo_so.pro
	
}
