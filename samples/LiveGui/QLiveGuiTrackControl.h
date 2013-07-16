/*
 *  QLiveGuiTrackControl.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QLIVEGUI_TRACK_CONTROL
#define QLIVEGUI_TRACK_CONTROL

#pragma once

#include "QLiveObject.h"

#include "Gwen/Controls/Label.h"
#include "Gwen/Controls/LabelClickable.h"
#include "Gwen/Controls/Button.h"
#include "Gwen/Controls/HorizontalSlider.h"
#include "Gwen/Controls/RadioButtonController.h"


class QLiveGuiTrackControl : public Gwen::Controls::Base {

public:
    
	QLiveGuiTrackControl( nocte::QLiveTrackRef track, ci::Vec2f size, Gwen::Controls::Base *parent )
    : Gwen::Controls::Base(parent)
    {
        SetSize( size.x, size.y );
        SetMargin( Gwen::Margin( 15, 15 , 0, 0 ) ); // left, top, right, bottom
        Dock( Gwen::Pos::Left );
        
        std::vector<nocte::QLiveClipRef>    clips = track->getClips();
        nocte::QLiveClipRef                 clip;
        
        
        // Label
        Gwen::Controls::Label *label = new Gwen::Controls::Label( this );
        label->SetText( track->getName() );
        label->SizeToContents();
        label->Dock( Gwen::Pos::Top );
        label->SetTextColorOverride( cigwen::toGwen( track->getColor() ) );
        
        
        // Brightness
        Gwen::Controls::HorizontalSlider *brightness = new Gwen::Controls::HorizontalSlider( this );
        brightness->SetSize( size.x, 20 );
        brightness->Dock( Gwen::Pos::Top );
        brightness->onValueChanged.Add( this, &QLiveGuiTrackControl::onBrightnessChange );

        
        // Clips
        Gwen::Controls::RadioButtonController* rc = new Gwen::Controls::RadioButtonController( this );
        for( auto i=0; i < clips.size(); i++ )
        {
            clip = clips[i];
            Gwen::Controls::LabeledRadioButton *radioBtn = rc->AddOption( clip->getName() );
//            radioBtn->SetTextColorOverride( cigwen::toGwen( clip->getColor() ) );
        }
        rc->SetSize( size.x, clips.size() * 22 );
        rc->Dock( Gwen::Pos::Top );
        rc->onSelectionChange.Add( this, &QLiveGuiTrackControl::onClipChange );
        
        
        // Params
        std::vector<nocte::QLiveDeviceRef>  devices = track->getDevices();
        std::vector<nocte::QLiveParamRef>   params;
        nocte::QLiveParamRef                param;
        
        for( auto i=0; i < devices.size(); i++ )
        {
            params = devices[i]->getParams();
            
            for( auto j=1; j < params.size(); j++ )       // starts from 1 to ignore "Device On"
            {
                param = params[j];
                
                // Param name
                Gwen::Controls::Label *pLabel = new Gwen::Controls::Label( this );
                pLabel->SetText( param->getName() );
                pLabel->SizeToContents();
                pLabel->SetMargin( Gwen::Margin( 0, 10, 0, 0 ) );
                pLabel->Dock( Gwen::Pos::Top );
                
                // Slider
                Gwen::Controls::HorizontalSlider *pSlider = new Gwen::Controls::HorizontalSlider( this );
                pSlider->SetValue( std::to_string( param->getValue() ) );
                pSlider->SetRange( param->getMin(), param->getMax() );
                pSlider->SetSize( size.x, 20 );
                pSlider->Dock( Gwen::Pos::Top );
                pSlider->onValueChanged.Add( this, &QLiveGuiTrackControl::onParamChange );

//                widget->setMeta( "param_" + toString( track->getIndex() ) + "_" + toString( device->getIndex() ) + "_" + toString( param->getIndex() ) );
            }
        }

    }
    
	virtual ~QLiveGuiTrackControl() {}

    
private:
    
    void onBrightnessChange( Gwen::Controls::Base* pControl )
    {
//        Gwen::Controls::HorizontalSlider* slider = ( Gwen::Controls::HorizontalSlider* ) pControl;
//        Gwen::Controls::LabeledRadioButton* pSelected = rc->GetSelected();
//        UnitPrint( Utility::Format( L"RadioButton changed (using 'OnChange' event)\n Chosen Item: '%ls'", pSelected->GetLabel()->GetText().GetUnicode().c_str() ) );
    }

    
    void onClipChange( Gwen::Controls::Base* pControl )
    {
//        Gwen::Controls::RadioButtonController* rc = ( Gwen::Controls::RadioButtonController* ) pControl;
//        Gwen::Controls::LabeledRadioButton* pSelected = rc->GetSelected();
//        UnitPrint( Utility::Format( L"RadioButton changed (using 'OnChange' event)\n Chosen Item: '%ls'", pSelected->GetLabel()->GetText().GetUnicode().c_str() ) );
    }
    
    void onParamChange( Gwen::Controls::Base* pControl )
    {
        
    }

};


#endif



