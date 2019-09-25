/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Symbols
 * Author:   Jesper Weissglas
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   bdbcat@yahoo.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#pragma once
#include "tinyxml/tinyxml.h"

#include "_def.h"
#include "bitmap.h"
#include <QHash>

enum StyleToolIconTypes
{
      TOOLICON_NORMAL,
      TOOLICON_TOGGLED,
      TOOLICON_DISABLED,
      TOOLICON_ACTIVE
};

void bmdump(wxBitmap bm, QString name);
wxBitmap MergeBitmaps( wxBitmap back, wxBitmap front, QSize offset );
wxBitmap ConvertTo24Bit( QColor bgColor, wxBitmap front );

#define  StyleMgrIns        ocpnStyle::StyleManager::instance()

namespace ocpnStyle {

typedef QHash<QString, int> intHash;

class Tool {
public:
      QString name;
      QPoint iconLoc;
      QPoint rolloverLoc;
      QPoint disabledLoc;
      QPoint activeLoc;
      wxBitmap icon;
      wxBitmap rollover;
      wxBitmap rolloverToggled;
      wxBitmap disabled;
      wxBitmap active;
      wxBitmap toggled;
      bool iconLoaded;
      bool rolloverLoaded;
      bool rolloverToggledLoaded;
      bool disabledLoaded;
      bool activeLoaded;
      bool toggledLoaded;
      QSize customSize;

      void Unload(void) {
            iconLoaded= false;
            rolloverLoaded = false;
            rolloverToggledLoaded = false;
            disabledLoaded = false;
            activeLoaded =false;
            toggledLoaded =false;
            if(customSize.width() == 0 && customSize.height() == 0)
            customSize = QSize( 32, 32 );
      }

      Tool(void) {
          Unload();
      }
};

class Icon {
public:
      QString name;
      QPoint iconLoc;
      QSize size;
      wxBitmap icon;
      bool loaded;

      void Unload(void) {
            loaded = false;
//            size = QSize(0, 0);
      }

      bool operator ==(const Icon& other) const
      {
          return other.name == this->name;
      }

      Icon(void) { Unload(); }
};

class Style {

public:
    enum Direction{
        Hortizontal = 0,
        Vertical,
    };
      Style( void );
      ~Style( void );

      wxBitmap GetNormalBG();
      wxBitmap GetActiveBG();
      wxBitmap GetToggledBG();
      wxBitmap GetToolbarStart();
      wxBitmap GetToolbarEnd();
      bool HasBackground() const { return hasBackground; }
      void HasBackground( bool b ) { hasBackground = b; }
      wxBitmap GetIcon(const QString & name, int width = -1, int height = -1, bool bforceReload = false);
      wxBitmap GetToolIcon(const QString & toolname,
                           int iconType = TOOLICON_NORMAL, bool rollover = false,
                           int width = -1, int height = -1);
      wxBitmap BuildPluginIcon( wxBitmap &bm, int iconType, double scale = 1.0 );
      bool NativeToolIconExists(const QString & name);
      
      int GetTopMargin() const { return toolMarginTop[currentOrientation]; }
      int GetRightMargin() const { return toolMarginRight[currentOrientation]; }
      int GetBottomMargin() const { return toolMarginBottom[currentOrientation]; }
      int GetLeftMargin() const { return toolMarginLeft[currentOrientation]; }
      int GetToolbarCornerRadius();

      int GetCompassTopMargin() const { return compassMarginTop; }
      int GetCompassRightMargin() const { return compassMarginRight; }
      int GetCompassBottomMargin() const { return compassMarginBottom; }
      int GetCompassLeftMargin() const { return compassMarginLeft; }
      int GetCompassCornerRadius() const { return compasscornerRadius; }
      int GetCompassXOffset() const { return compassXoffset; }
      int GetCompassYOffset() const { return compassYoffset; }

      int GetToolSeparation() const { return toolSeparation[currentOrientation]; }
      QSize GetToolSize() const { return toolSize[currentOrientation]; }
      QSize GetToggledToolSize() const { return toggledBGSize[currentOrientation]; }

      bool HasToolbarStart() const { return toolbarStartLoc[currentOrientation] != QPoint(0,0); }
      bool HasToolbarEnd() const { return toolbarEndLoc[currentOrientation] != QPoint(0,0); }
      void DrawToolbarLineStart( wxBitmap& bmp, double scale = 1.0 );
      void DrawToolbarLineEnd( wxBitmap& bmp, double scale = 1.0 );

      static wxBitmap SetBitmapBrightness( wxBitmap& bitmap, ColorScheme cs );
      static wxBitmap SetBitmapBrightnessAbs( wxBitmap& bitmap, double level );
      
      void SetOrientation( Direction orient );
      int GetOrientation();
      void SetColorScheme( ColorScheme cs );
      void Unload();

      QString name;
      QString sysname;
      QString description;
      QString graphicsFile;
      int toolMarginTop[2];
      int toolMarginRight[2];
      int toolMarginBottom[2];
      int toolMarginLeft[2];
      int toolSeparation[2];
      int cornerRadius[2];
      int compassMarginTop;
      int compassMarginRight;
      int compassMarginBottom;
      int compassMarginLeft;
      int compasscornerRadius;
      int compassXoffset;
      int compassYoffset;

      QSize toolSize[2];
      QSize toggledBGSize[2];
      QPoint toggledBGlocation[2];
      QPoint activeBGlocation[2];
      QPoint normalBGlocation[2];
      QSize verticalIconOffset;
      wxArrayPtrVoid tools;
      intHash toolIndex;
      QList<Icon*> icons;
      intHash iconIndex;
      wxBitmap* graphics;

      QColor consoleFontColor;
      QPoint consoleTextBackgroundLoc;
      QSize consoleTextBackgroundSize;
      QPoint toolbarStartLoc[2];
      QSize toolbarStartSize[2];
      QPoint toolbarEndLoc[2];
      QSize toolbarEndSize[2];
      wxBitmap consoleTextBackground;
      wxBitmap toolbarStart[2];
      wxBitmap toolbarEnd[2];

      bool marginsInvisible;

      int chartStatusIconWidth;
      bool chartStatusWindowTransparent;

      QString embossFont;
      int embossHeight;

      QString myConfigFileDir;

private:
      int currentOrientation;
      ColorScheme colorscheme;
      bool hasBackground;
};

class StyleManager
{
private:
      StyleManager(void);
      StyleManager(const QString & configDir);
public:
      static StyleManager* instance();
      ~StyleManager(void);
      bool IsOK() const { return isOK; }
      void Init(const QString & fromPath);
      void SetStyle(QString name);
      void SetStyleNextInvocation(const QString & name) { nextInvocationStyle = name; }
      const QString & GetStyleNextInvocation() const { return nextInvocationStyle; }
      Style* GetCurrentStyle();
      QList<Style*> GetArrayOfStyles() { return styles; }

private:
    static StyleManager     *mInstance;

    class MGarbage
    {
    public:
        ~MGarbage()
        {
            if (StyleManager::mInstance)
                delete StyleManager::mInstance;
        }
    };
    static MGarbage Garbage;

private:
      bool isOK;
      QList<Style*> styles;
      Style* currentStyle;
      QString nextInvocationStyle;
};

}
