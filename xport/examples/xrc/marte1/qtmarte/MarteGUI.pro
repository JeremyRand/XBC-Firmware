TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

HEADERS	+= serialstream.h \
	TaskDefs.h \
	martecomm.h \
	martestringtable.h \
	fancylistviewitem.h

SOURCES	+= main.cpp \
	serialstream.cpp \
	martecomm.cpp \
	martestringtable.cpp \
	fancylistviewitem.cpp

FORMS	= ./martemainform.ui \
	./motorinfo.ui \
	martecontrol.ui

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}



