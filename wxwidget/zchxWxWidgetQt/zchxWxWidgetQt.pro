#-------------------------------------------------
#
# Project created by QtCreator 2019-08-16T12:22:21
#
#-------------------------------------------------

QT       += widgets opengl svg xml xmlpatterns

TARGET = zchxWxWidgetQt
TEMPLATE = lib

INCLUDEPATH += $${PWD}/include

DEFINES += ZCHXWXWIDGETQT_LIBRARY wxUSE_BITMAP_BASE

SOURCES += zchxwxwidgetqt.cpp \
    src/qt/dc.cpp \
    src/qt/dcclient.cpp \
    src/qt/dcmemory.cpp \
    src/qt/dcprint.cpp \
    src/qt/dcscreen.cpp \
    src/qt/accel.cpp \
    src/qt/anybutton.cpp \
    src/qt/app.cpp \
    src/qt/apptraits.cpp \
    src/qt/bitmap.cpp \
    src/qt/bmpbuttn.cpp \
    src/qt/brush.cpp \
    src/qt/button.cpp \
    src/qt/calctrl.cpp \
    src/qt/checkbox.cpp \
    src/qt/checklst.cpp \
    src/qt/choice.cpp \
    src/qt/clipbrd.cpp \
    src/qt/clrpicker.cpp \
    src/qt/colordlg.cpp \
    src/qt/colour.cpp \
    src/qt/combobox.cpp \
    src/qt/control.cpp \
    src/qt/converter.cpp \
    src/qt/ctrlsub.cpp \
    src/qt/cursor.cpp \
    src/qt/dataobj.cpp \
    src/qt/dataview.cpp \
    src/qt/defs.cpp \
    src/qt/dialog.cpp \
    src/qt/display.cpp \
    src/qt/dnd.cpp \
    src/qt/dvrenderer.cpp \
    src/qt/dvrenderers.cpp \
    src/qt/evtloop.cpp \
    src/qt/filedlg.cpp \
    src/qt/font.cpp \
    src/qt/fontdlg.cpp \
    src/qt/fontenum.cpp \
    src/qt/fontutil.cpp \
    src/qt/frame.cpp \
    src/qt/gauge.cpp \
    src/qt/glcanvas.cpp \
    src/qt/listbox.cpp \
    src/qt/listctrl.cpp \
    src/qt/mdi.cpp \
    src/qt/mediactrl.cpp \
    src/qt/menu.cpp \
    src/qt/menuitem.cpp \
    src/qt/minifram.cpp \
    src/qt/msgdlg.cpp \
    src/qt/nonownedwnd.cpp \
    src/qt/notebook.cpp \
    src/qt/palette.cpp \
    src/qt/pen.cpp \
    src/qt/popupwin.cpp \
    src/qt/printdlg.cpp \
    src/qt/printqt.cpp \
    src/qt/radiobox.cpp \
    src/qt/radiobut.cpp \
    src/qt/region.cpp \
    src/qt/scrolbar.cpp \
    src/qt/settings.cpp \
    src/qt/slider.cpp \
    src/qt/sockqt.cpp \
    src/qt/spinbutt.cpp \
    src/qt/spinctrl.cpp \
    src/qt/statbmp.cpp \
    src/qt/statbox.cpp \
    src/qt/statline.cpp \
    src/qt/stattext.cpp \
    src/qt/statusbar.cpp \
    src/qt/taskbar.cpp \
    src/qt/textctrl.cpp \
    src/qt/textentry.cpp \
    src/qt/tglbtn.cpp \
    src/qt/timer.cpp \
    src/qt/toolbar.cpp \
    src/qt/tooltip.cpp \
    src/qt/toplevel.cpp \
    src/qt/treectrl.cpp \
    src/qt/uiaction.cpp \
    src/qt/utils.cpp \
    src/qt/window.cpp \
    src/common/dcbase.cpp \
    src/common/bmpbase.cpp

HEADERS += zchxwxwidgetqt.h\
        zchxwxwidgetqt_global.h \
    include/wx/qt/accel.h \
    include/wx/qt/anybutton.h \
    include/wx/qt/app.h \
    include/wx/qt/bitmap.h \
    include/wx/qt/bmpbuttn.h \
    include/wx/qt/brush.h \
    include/wx/qt/button.h \
    include/wx/qt/calctrl.h \
    include/wx/qt/checkbox.h \
    include/wx/qt/checklst.h \
    include/wx/qt/choice.h \
    include/wx/qt/clipbrd.h \
    include/wx/qt/clrpicker.h \
    include/wx/qt/colordlg.h \
    include/wx/qt/colour.h \
    include/wx/qt/combobox.h \
    include/wx/qt/control.h \
    include/wx/qt/ctrlsub.h \
    include/wx/qt/cursor.h \
    include/wx/qt/dataform.h \
    include/wx/qt/dataobj.h \
    include/wx/qt/dataobj2.h \
    include/wx/qt/dataview.h \
    include/wx/qt/dc.h \
    include/wx/qt/dcclient.h \
    include/wx/qt/dcmemory.h \
    include/wx/qt/dcprint.h \
    include/wx/qt/dcscreen.h \
    include/wx/qt/defs.h \
    include/wx/qt/dialog.h \
    include/wx/qt/dirdlg.h \
    include/wx/qt/dnd.h \
    include/wx/qt/dvrenderer.h \
    include/wx/qt/dvrenderers.h \
    include/wx/qt/evtloop.h \
    include/wx/qt/filedlg.h \
    include/wx/qt/font.h \
    include/wx/qt/fontdlg.h \
    include/wx/qt/frame.h \
    include/wx/qt/gauge.h \
    include/wx/qt/glcanvas.h \
    include/wx/qt/listbox.h \
    include/wx/qt/listctrl.h \
    include/wx/qt/mdi.h \
    include/wx/qt/menu.h \
    include/wx/qt/menuitem.h \
    include/wx/qt/minifram.h \
    include/wx/qt/msgdlg.h \
    include/wx/qt/nonownedwnd.h \
    include/wx/qt/notebook.h \
    include/wx/qt/palette.h \
    include/wx/qt/pen.h \
    include/wx/qt/popupwin.h \
    include/wx/qt/printdlg.h \
    include/wx/qt/printqt.h \
    include/wx/qt/radiobox.h \
    include/wx/qt/radiobut.h \
    include/wx/qt/region.h \
    include/wx/qt/scrolbar.h \
    include/wx/qt/slider.h \
    include/wx/qt/spinbutt.h \
    include/wx/qt/spinctrl.h \
    include/wx/qt/statbmp.h \
    include/wx/qt/statbox.h \
    include/wx/qt/statline.h \
    include/wx/qt/stattext.h \
    include/wx/qt/statusbar.h \
    include/wx/qt/taskbar.h \
    include/wx/qt/textctrl.h \
    include/wx/qt/textentry.h \
    include/wx/qt/tglbtn.h \
    include/wx/qt/toolbar.h \
    include/wx/qt/tooltip.h \
    include/wx/qt/toplevel.h \
    include/wx/qt/treectrl.h \
    include/wx/qt/window.h \
    include/wx/qt/private/converter.h \
    include/wx/qt/private/pointer.h \
    include/wx/qt/private/timer.h \
    include/wx/qt/private/utils.h \
    include/wx/qt/private/winevent.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
