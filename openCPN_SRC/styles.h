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
#include <tinyxml.h>

#include "ocpn_types.h"
#include <QBitmap>
#include <QHash>

enum StyleToolIconTypes
{
      TOOLICON_NORMAL,
      TOOLICON_TOGGLED,
      TOOLICON_DISABLED,
      TOOLICON_ACTIVE
};

void bmdump(QBitmap bm, QString name);
QBitmap MergeBitmaps( QBitmap back, QBitmap front, QSize offset );
QBitmap ConvertTo24Bit( QColor bgColor, QBitmap front );

namespace ocpnStyle {

typedef QHash<QString, int> intHash;

class Tool {
public:
      QString name;
      QPoint iconLoc;
      QPoint rolloverLoc;
      QPoint disabledLoc;
      QPoint activeLoc;
      QBitmap icon;
      QBitmap rollover;
      QBitmap rolloverToggled;
      QBitmap disabled;
      QBitmap active;
      QBitmap toggled;
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
      QBitmap icon;
      bool loaded;

      void Unload(void) {
            loaded = false;
      }

      Icon(void) { Unload(); }
};

class Style {

public:
      Style( void );
      ~Style( void );

      QBitmap GetNormalBG();
      QBitmap GetActiveBG();
      QBitmap GetToggledBG();
      QBitmap GetToolbarStart();
      QBitmap GetToolbarEnd();
      bool HasBackground() const { return hasBackground; }
      void HasBackground( bool b ) { hasBackground = b; }
      QBitmap GetIcon(const QString & name, int width = -1, int height = -1, bool bforceReload = false);
      QBitmap GetToolIcon(const QString & toolname,
                           int iconType = TOOLICON_NORMAL, bool rollover = false,
                           int width = -1, int height = -1);
      QBitmap BuildPluginIcon( QBitmap &bm, int iconType, double scale = 1.0 );
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
      void DrawToolbarLineStart( QBitmap& bmp, double scale = 1.0 );
      void DrawToolbarLineEnd( QBitmap& bmp, double scale = 1.0 );

      static QBitmap SetBitmapBrightness( QBitmap& bitmap, ColorScheme cs );
      static QBitmap SetBitmapBrightnessAbs( QBitmap& bitmap, double level );
      
      void SetOrientation( long orient );
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
      wxArrayPtrVoid icons;
      intHash iconIndex;
      QBitmap* graphics;

      QColor consoleFontColor;
      QPoint consoleTextBackgroundLoc;
      QSize consoleTextBackgroundSize;
      QPoint toolbarStartLoc[2];
      QSize toolbarStartSize[2];
      QPoint toolbarEndLoc[2];
      QSize toolbarEndSize[2];
      QBitmap consoleTextBackground;
      QBitmap toolbarStart[2];
      QBitmap toolbarEnd[2];

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

class StyleManager {
public:
      StyleManager(void);
      ~StyleManager(void);
      StyleManager(const QString & configDir);

      bool IsOK() const { return isOK; }
      void Init(const QString & fromPath);
      void SetStyle(QString name);
      void SetStyleNextInvocation(const QString & name) { nextInvocationStyle = name; }
      const QString & GetStyleNextInvocation() const { return nextInvocationStyle; }
      Style* GetCurrentStyle();
      wxArrayPtrVoid GetArrayOfStyles() { return styles; };

private:
      bool isOK;
      wxArrayPtrVoid styles;
      Style* currentStyle;
      QString nextInvocationStyle;
};

}
