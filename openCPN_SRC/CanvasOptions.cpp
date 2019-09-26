/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Canvas Options Window/Dialog
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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
 ***************************************************************************
 *
 *
 */

#include "dychart.h"

#include "chcanv.h"
#include "CanvasOptions.h"
#include "OCPNPlatform.h"

#include <QtSvg>

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif



//------------------------------------------------------------------------------
//    External Static Storage
//------------------------------------------------------------------------------


//  Helper utilities


//  Helper classes


//------------------------------------------------------------------------------
//    CanvasOptions
//------------------------------------------------------------------------------
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QGroupBox>
#include <QButtonGroup>

CanvasOptions::CanvasOptions( QWidget *parent) : QWidget(parent)

{
    this->setWindowFlags(Qt::SubWindow);
    this->setWindowTitle(tr("CanvasOptions"));
    m_ENCAvail = true;
    
    QVBoxLayout * total_layout = new QVBoxLayout(this);
    this->setLayout(total_layout);
    
    m_sWindow = new QScrollArea(this);
    m_sWindow->setWidget(new QWidget(this));
    total_layout->addWidget(m_sWindow);

    QVBoxLayout * scrolled_layout = new QVBoxLayout(this);
    scrolled_layout->setSpacing(10);
    m_sWindow->widget()->setLayout(scrolled_layout);
    //  Options Label
    QVBoxLayout* label_layout = new QVBoxLayout(this);
    scrolled_layout->addLayout(label_layout);
    label_layout->addWidget(new QLabel(tr("Chart Panel Options"), this));
    QFrame * line   = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    label_layout->addWidget(line);

    // 导航模式
    QGroupBox* navGroup = new QGroupBox(tr("Navigation Mode"), this);
    navGroup->setLayout(new QVBoxLayout(this));
    scrolled_layout->addWidget(navGroup);

    QHBoxLayout *navModeLayout = new QHBoxLayout(this);
    pCBNorthUp = new QRadioButton(tr("North Up"), this);
    pCBCourseUp = new QRadioButton(tr("Course Up"), this);
    navModeLayout->addWidget(pCBNorthUp);
    navModeLayout->addWidget(pCBCourseUp);
    ((QVBoxLayout*)(navGroup->layout()))->addLayout(navModeLayout);

    QButtonGroup* btngrp = new QButtonGroup(this);
    btngrp->addButton(pCBNorthUp);
    btngrp->addButton(pCBCourseUp);
    btngrp->setExclusive(true);
    connect(btngrp, SIGNAL(buttonClicked(int)), this, SLOT(OnOptionChange()));
    
    pCBLookAhead = new QCheckBox(tr("Look Ahead Mode"), this);
    ((QVBoxLayout*)(navGroup->layout()))->addWidget(pCBLookAhead);
    connect(pCBLookAhead, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));\


    // 显示选项
    QGroupBox*  display_option_grp = new QGroupBox(tr("Display Options"), this);
    display_option_grp->setLayout(new QVBoxLayout(this));
    scrolled_layout->addWidget(display_option_grp);
    
    pCDOQuilting = new QCheckBox(tr("Enable Chart Quilting"), this);
    display_option_grp->layout()->addWidget(pCDOQuilting);
    connect(pCDOQuilting, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));
    
    pSDisplayGrid = new QCheckBox(tr("Show Grid"), this);
    display_option_grp->layout()->addWidget(pSDisplayGrid);
    connect(pSDisplayGrid, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));
    
    pCDOOutlines = new QCheckBox(tr("Show Chart Outlines"), this);
    display_option_grp->layout()->addWidget(pCDOOutlines);
    connect(pCDOOutlines, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));

    pSDepthUnits = new QCheckBox(tr("Show Depth Units"), this);
    display_option_grp->layout()->addWidget(pSDepthUnits);
    connect(pSDepthUnits, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));

    // AIS Options
    QGroupBox*  ais_option_grp = new QGroupBox(tr("AIS"), this);
    ais_option_grp->setLayout(new QVBoxLayout(this));
    scrolled_layout->addWidget(ais_option_grp);
    
    pCBShowAIS = new QCheckBox(tr("Show AIS targets"), this);
    ais_option_grp->layout()->addWidget(pCBShowAIS);
    connect(pCBShowAIS, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));
    pCBAttenAIS = new QCheckBox(tr("Minimize less critical targets"), this);
    display_option_grp->layout()->addWidget(pCBAttenAIS);
    connect(pCBAttenAIS, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));

    // Tide/Current Options
    QGroupBox*  tc_option_grp = new QGroupBox(tr("Tides And Currents"), this);
    tc_option_grp->setLayout(new QVBoxLayout(this));
    scrolled_layout->addWidget(tc_option_grp);
    pCDOTides = new QCheckBox(tr("Show Tide stations"), this);
    tc_option_grp->layout()->addWidget(pCDOTides);
    connect(pCDOTides, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));
    pCDOCurrents = new QCheckBox(tr("Show Currents"), this);
    tc_option_grp->layout()->addWidget(pCDOCurrents);
    connect(pCDOCurrents, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));

 
    // ENC Options
    QGroupBox*  enc_option_grp = new QGroupBox(tr("Vector Charts"), this);
    enc_option_grp->setLayout(new QVBoxLayout(this));
    scrolled_layout->addWidget(enc_option_grp);
    pCDOENCText = new QCheckBox(tr("Show text"), this);
    enc_option_grp->layout()->addWidget(pCDOENCText);
    connect(pCDOENCText, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));

    pCBENCDepth = new QCheckBox(tr("Show depths"), this);
    enc_option_grp->layout()->addWidget(pCBENCDepth);
    connect(pCBENCDepth, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));
    
    pCBENCBuoyLabels = new QCheckBox(tr("Buoy/Light Labels"), this);
    enc_option_grp->layout()->addWidget(pCBENCBuoyLabels);
    connect(pCBENCBuoyLabels, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));
 
    pCBENCLights = new QCheckBox(tr("Lights"), this);
    enc_option_grp->layout()->addWidget(pCBENCLights);
    connect(pCBENCLights, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));
 
    pCBENCLightDesc = new QCheckBox(tr("Light Descriptions"), this);
    enc_option_grp->layout()->addWidget(pCBENCLightDesc);
    connect(pCBENCLightDesc, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));
    
    pCBENCAnchorDetails = new QCheckBox(tr("Anchoring Info"), this);
    enc_option_grp->layout()->addWidget(pCBENCAnchorDetails);
    connect(pCBENCAnchorDetails, SIGNAL(clicked(bool)), this, SLOT(OnOptionChange()));

    // display category
    enc_option_grp->layout()->addWidget(new QLabel(tr("Display Category"), this));
    QStringList pDispCatStrings;
    pDispCatStrings << tr("Base") << tr("Standard") << tr("All") << tr("User Standard");
    m_pDispCat = new QComboBox(this);
    enc_option_grp->layout()->addWidget(m_pDispCat);
    m_pDispCat->addItems(pDispCatStrings);
    connect(m_pDispCat, SIGNAL(currentIndexChanged(int)), this, SLOT(OnOptionChange()));

    RefreshControlValues();
}



void CanvasOptions::OnClose( )
{
}

void CanvasOptions::OnOptionChange()
{
    UpdateCanvasOptions();
}



void CanvasOptions::RefreshControlValues( void )
{
    ChartCanvas *parentCanvas = dynamic_cast<ChartCanvas*> (parentWidget());
    if(!parentCanvas) return;
    
    //  Display options
    pCDOQuilting->setChecked(parentCanvas->GetQuiltMode());
    pSDisplayGrid->setChecked(parentCanvas->GetShowGrid());
//    pCDOOutlines->setChecked(parentCanvas->GetShowOutlines());
//    pSDepthUnits->setChecked(parentCanvas->GetShowDepthUnits());
 
    // AIS Options
//    pCBShowAIS->setChecked(parentCanvas->GetShowAIS());
//    pCBAttenAIS->setChecked(parentCanvas->GetAttenAIS());
    
//    // Tide/Current
//    pCDOTides->setChecked(parentCanvas->GetbShowTide());
//    pCDOCurrents->setChecked(parentCanvas->GetbShowCurrent());;
    
    //ENC Options
    pCDOENCText->setChecked(parentCanvas->GetShowENCText());
    pCBENCDepth->setChecked(parentCanvas->GetShowENCDepth());
    pCBENCLightDesc->setChecked(parentCanvas->GetShowENCLightDesc());
    pCBENCBuoyLabels->setChecked(parentCanvas->GetShowENCBuoyLabels());
    pCBENCLights->setChecked(parentCanvas->GetShowENCLights());
    pCBENCAnchorDetails->setChecked(parentCanvas->GetShowENCAnchor());

    //pCBENCLightDesc->Enable(parentCanvas->GetShowENCLights());
    
    
    //  Display category
    int nset = 2;  // default OTHER
    switch (parentCanvas->GetENCDisplayCategory()) {
        case (DISPLAYBASE):
            nset = 0;
            break;
        case (STANDARD):
            nset = 1;
            break;
        case (OTHER):
            nset = 2;
            break;
        case (MARINERS_STANDARD):
            nset = 3;
            break;
        default:
            nset = 3;
            break;
    }
    m_pDispCat ->setCurrentIndex(nset);
    
    // If no ENCs are available in the current canvas group, then disable the ENC related options.
    pCDOENCText->setEnabled(m_ENCAvail);
    pCBENCDepth->setEnabled(m_ENCAvail);
    pCBENCLightDesc->setEnabled(m_ENCAvail && parentCanvas->GetShowENCLights());
    pCBENCBuoyLabels->setEnabled(m_ENCAvail);
    pCBENCLights->setEnabled(m_ENCAvail);
    
    //  Anchor conditions are only available if display category is "All" or "User Standard"
    pCBENCAnchorDetails->setEnabled(m_ENCAvail && (nset > 1));

    //  Many options are not valid if display category is "Base"
    if(nset == 0){
        pCDOENCText->setEnabled(false);
        pCBENCDepth->setEnabled(false);
        pCBENCLightDesc->setEnabled(false);
        pCBENCBuoyLabels->setEnabled(false);
        pCBENCLights->setEnabled(false);
    }
        
    m_pDispCat->setEnabled(m_ENCAvail);
    
}

void CanvasOptions::SetENCAvailable( bool avail )
{ 
    m_ENCAvail = avail;
    RefreshControlValues();
}

void CanvasOptions::UpdateCanvasOptions( void )
{
    ChartCanvas *parentCanvas = dynamic_cast<ChartCanvas*> (parentWidget());
    if(!parentCanvas) return;
    
    bool b_needRefresh = false;
    bool b_needReLoad = false;
    
//     if(pCBToolbar->GetValue() != parentCanvas->GetToolbarEnable()){
//         parentCanvas->SetToolbarEnable( pCBToolbar->GetValue() );
//         b_needRefresh = true;
//     }
    
    if(pCDOQuilting->isChecked() != parentCanvas->GetQuiltMode()){
        parentCanvas->ToggleCanvasQuiltMode();
    }
    
    if(pSDisplayGrid->isChecked() != parentCanvas->GetShowGrid()){
        parentCanvas->SetShowGrid(pSDisplayGrid->isChecked());
        b_needRefresh = true;
    }
    
//    if(pCDOOutlines->isChecked() != parentCanvas->GetShowOutlines()){
//        parentCanvas->SetShowOutlines(pCDOOutlines->isChecked());
//        b_needRefresh = true;
//    }
//    if(pSDepthUnits->isChecked() != parentCanvas->GetShowDepthUnits()){
//        parentCanvas->SetShowDepthUnits(pSDepthUnits->isChecked());
//        b_needRefresh = true;
//    }

//    if(pCBShowAIS->isChecked() != parentCanvas->GetShowAIS()){
//        parentCanvas->SetShowAIS(pCBShowAIS->isChecked());
//        b_needRefresh = true;
//    }
    
//    if(pCBAttenAIS->isChecked() != parentCanvas->GetAttenAIS()){
//        parentCanvas->SetAttenAIS(pCBAttenAIS->isChecked());
//        b_needRefresh = true;
//    }
    
//    if(pCDOTides->isChecked() != parentCanvas->GetbShowTide()){
//        parentCanvas->ShowTides(pCDOTides->isChecked());
//        b_needRefresh = true;
//    }
//    if(pCDOCurrents->isChecked() != parentCanvas->GetbShowCurrent()){
//        parentCanvas->ShowCurrents(pCDOCurrents->isChecked());
//        b_needRefresh = true;
//    }

    //  ENC Options
    if(pCDOENCText->isChecked() != parentCanvas->GetShowENCText()){
        parentCanvas->SetShowENCText(pCDOENCText->isChecked());
        b_needReLoad = true;
    }

    if(pCBENCDepth->isChecked() != parentCanvas->GetShowENCDepth()){
        parentCanvas->SetShowENCDepth(pCBENCDepth->isChecked());
        b_needReLoad = true;
    }
    
    if(pCBENCLightDesc->isChecked() != parentCanvas->GetShowENCLightDesc()){
        parentCanvas->SetShowENCLightDesc(pCBENCLightDesc->isChecked());
        b_needReLoad = true;
    }
    
    if(pCBENCBuoyLabels->isChecked() != parentCanvas->GetShowENCBuoyLabels()){
        parentCanvas->SetShowENCBuoyLabels(pCBENCBuoyLabels->isChecked());
        b_needReLoad = true;
    }

    if(pCBENCLights->isChecked() != parentCanvas->GetShowENCLights()){
        parentCanvas->SetShowENCLights(pCBENCLights->isChecked());
        b_needReLoad = true;
    }
    
    if(pCBENCAnchorDetails->isChecked() != parentCanvas->GetShowENCAnchor()){
        parentCanvas->SetShowENCAnchor(pCBENCAnchorDetails->isChecked());
        b_needReLoad = true;
    }

    
    int nset = 2;
    switch (parentCanvas->GetENCDisplayCategory()) {
        case (DISPLAYBASE): nset = 0; break;
        case (STANDARD): nset = 1; break;
        case (OTHER): nset = 2; break;
        case (MARINERS_STANDARD): nset = 3; break;
        default: nset = 2; break;
    }
    
    
    if(m_pDispCat->currentIndex() != nset){
        int valSet = STANDARD;
        int newSet = m_pDispCat->currentIndex();
        switch(newSet){
            case 0: valSet = DISPLAYBASE; break;
            case 1: valSet = STANDARD; break;
            case 2: valSet = OTHER; break;
            case 3: valSet = MARINERS_STANDARD; break;
            default: valSet = STANDARD; break;
        }
        parentCanvas->SetENCDisplayCategory( valSet);
        b_needReLoad = true;
        
        //  Anchor conditions are only available if display category is "All" or "User Standard"
        pCBENCAnchorDetails->setEnabled(newSet > 1);

    }
    
    if(b_needReLoad){
        parentCanvas->ReloadVP();
    }
    else if (b_needRefresh){
        parentCanvas->Refresh(true);
//        parentCanvas->InvalidateGL();
    }
    
    RefreshControlValues();
        
}



