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

private:
    
    struct GwenDeviceParam {
        Gwen::Controls::HorizontalSlider    *slider;
        nocte::QLiveParamRef                param;
    };
    
    
public:
    
    
	QLiveGuiTrackControl( nocte::QLiveRef live, nocte::QLiveTrackRef track, ci::Vec2f size, Gwen::Controls::Base *parent )
    : Gwen::Controls::Base(parent), mLive(live), mTrack( track )
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
        Gwen::Controls::HorizontalSlider *volume = new Gwen::Controls::HorizontalSlider( this );
//        volume->SetName( std::to_string( track->getIndex() ) );
        volume->SetSize( size.x - 15, 20 );
        volume->Dock( Gwen::Pos::Top );
        volume->SetRange( 0.0f, 1.0f );
        volume->SetFloatValue( track->getVolume() );
//        volume->SetFloatValue( 1.0f );
        volume->onValueChanged.Add( this, &QLiveGuiTrackControl::onBrightnessChange );
        
//        ci::app::console() << track->getName() << " " << track->getVolume() << " " << volume->GetFloatValue() << std::endl;
        
        // Clips
        Gwen::Controls::RadioButtonController* rc = new Gwen::Controls::RadioButtonController( this );
        for( auto i=0; i < clips.size(); i++ )
        {
            clip = clips[i];
            Gwen::Controls::LabeledRadioButton *radioBtn = rc->AddOption( clip->getName() );
            radioBtn->GetLabel()->SetTextColorOverride( cigwen::toGwen( clip->getColor() ) );
            radioBtn->SetName( std::to_string( clip->getIndex() ) );
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
                pLabel->SetTextColorOverride( cigwen::toGwen( ci::Color::gray( 0.3f ) ) );
                
                // Slider
                std::string sliderName = std::to_string( devices[i]->getIndex() ) + "_" + std::to_string( param->getIndex() );
                Gwen::Controls::HorizontalSlider *pSlider = new Gwen::Controls::HorizontalSlider( this );
                pSlider->SetName( sliderName );
                pSlider->SetSize( size.x, 20 );
                pSlider->Dock( Gwen::Pos::Top );
                pSlider->SetRange( param->getMin(), param->getMax() );
                pSlider->SetValue( std::to_string( param->getValue() ) );
                pSlider->onValueChanged.Add( this, &QLiveGuiTrackControl::onParamChange );

                // used in Render() to update the value
                GwenDeviceParam p = { pSlider, param };
                mGwenParams.push_back(p);
            }
        }

    }
    
	virtual ~QLiveGuiTrackControl() {}

	virtual void Render( Gwen::Skin::Base* skin )
    {
        for( auto k=0; k < mGwenParams.size(); k++ )
            mGwenParams[k].slider->SetFloatValue( mGwenParams[k].param->getValue() );
    }
    
    
private:
    
    void onBrightnessChange( Gwen::Controls::Base* pControl )
    {
        
//        Gwen::Controls::HorizontalSlider* slider = ( Gwen::Controls::HorizontalSlider* ) pControl;
//        int trackIdx = boost::lexical_cast<int>( slider->GetName() );
        
//        mLive->setTrackVolume( trackIdx, slider->getScaledValue() );
        
//        ci::app::console() << "trackIdx: " << trackIdx << " " << slider->GetFloatValue() << std::endl;
        
//        Gwen::Controls::LabeledRadioButton* pSelected = rc->GetSelected();
//        UnitPrint( Utility::Format( L"RadioButton changed (using 'OnChange' event)\n Chosen Item: '%ls'", pSelected->GetLabel()->GetText().GetUnicode().c_str() ) );
        
//        
//        int trackIdx = boost::lexical_cast<int>( event->widget->getMeta() );
//        ciUISlider *slider = (ciUISlider *) event->widget;
//        mLive->setTrackVolume( trackIdx, slider->getScaledValue() );
    }

    
    void onClipChange( Gwen::Controls::Base* pControl )
    {
        Gwen::Controls::RadioButtonController*  rc          = ( Gwen::Controls::RadioButtonController* ) pControl;
        Gwen::Controls::LabeledRadioButton*     pSelected   = rc->GetSelected();
        int                                     clipIdx     = boost::lexical_cast<int>( pSelected->GetName() );

        mLive->playClip( mTrack->getIndex(), clipIdx );
    }
    
    void onParamChange( Gwen::Controls::Base* pControl )
    {        
        std::vector<std::string> splitValues;
        boost::split( splitValues, pControl->GetName(), boost::is_any_of("_") );
        
        int deviceIdx   = boost::lexical_cast<int>( splitValues[0] );
        int paramIdx    = boost::lexical_cast<int>( splitValues[1] );
        
        mLive->setParam( mTrack->getIndex(), deviceIdx, paramIdx, ((Gwen::Controls::Slider*)pControl)->GetFloatValue() );
    }

private:
    
    nocte::QLiveRef                 mLive;
    nocte::QLiveTrackRef            mTrack;
    std::vector<GwenDeviceParam>    mGwenParams;
    
};


#endif




/*

void guiEvent(ciUIEvent *event)
{
    std::string name = event->widget->getName();
    std::string meta = event->widget->getMeta();
    
    if(name == "Master")
    {
        int trackIdx = boost::lexical_cast<int>( event->widget->getMeta() );
        ciUISlider *slider = (ciUISlider *) event->widget;
        mLive->setTrackVolume( trackIdx, slider->getScaledValue() );
    }
    
    else if ( boost::find_first( meta, "clip") )
    {
        ciUIToggle *toggle = (ciUIToggle *) event->widget;
        
        std::vector<std::string>    splitValues;
        std::string                 clipMeta;
        bool                        toggleVal   = toggle->getValue();
        
        boost::split( splitValues, meta, boost::is_any_of("_") );
        
        int trackIdx    = boost::lexical_cast<int>( splitValues[1] );
        int clipIdx     = boost::lexical_cast<int>( splitValues[2] );
        
        toggle->setValue( !toggle->getValue() );    // trigger back the toogle, QLive sets the value, this is to avoid flickering
        
        if ( toggleVal )
            mLive->playClip( trackIdx, clipIdx );               // play clip
        else
            mLive->stopClip( trackIdx, clipIdx );               // play clip
    } 
    
}




*/