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
    
    struct GwenDeviceParamRef {
        Gwen::Controls::HorizontalSlider    *slider;
        int                                 trackIdx;
        int                                 deviceIdx;
        int                                 paramIdx;
        std::shared_ptr<float>              ref;
        float                               prevVal;
    };
    
    struct GwenClipRef {
        Gwen::Controls::LabeledRadioButton  *radioButton;
        int                                 trackIdx;
        int                                 clipIdx;
        bool                                *ref;
        bool                                prevVal;
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
        mVolume = new Gwen::Controls::HorizontalSlider( this );
        mVolume->Dock( Gwen::Pos::Top );
        mVolume->SetSize( size.x - 15, 20 );
        mVolume->SetRange( 0.0f, 1.0f );
        mVolume->SetFloatValue( 0.0f );
        mVolume->onValueChanged.Add( this, &QLiveGuiTrackControl::onVolumeChange );
//        mBrightnessGwenParamRef = { volume, mTrack->getIndex(), -1, -1, mTrack->getVolumeRef(), 0.0f };

        
        // Clips
        mClipsRadioController = new Gwen::Controls::RadioButtonController( this );
        for( auto i=0; i < clips.size(); i++ )
        {
            clip = clips[i];
            Gwen::Controls::LabeledRadioButton *radioBtn = mClipsRadioController->AddOption( clip->getName() );
            radioBtn->GetLabel()->SetTextColorOverride( cigwen::toGwen( clip->getColor() ) );
            radioBtn->SetName( std::to_string( clip->getIndex() ) );

            if ( clip->isPlaying() )
                radioBtn->GetRadioButton()->SetChecked( true );
            
            GwenClipRef c = { radioBtn, mTrack->getIndex(), clip->getIndex(), clip->getIsPlayingRef(), clip->isPlaying() };
            mGwenClips.push_back(c);
        }
        mClipsRadioController->SetSize( size.x, clips.size() * 22 );
        mClipsRadioController->Dock( Gwen::Pos::Top );
        mClipsRadioController->onSelectionChange.Add( this, &QLiveGuiTrackControl::onClipChange );
        
        
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
                std::string sliderName = std::to_string( mTrack->getIndex() ) + "_" + std::to_string( devices[i]->getIndex() ) + "_" + std::to_string( param->getIndex() );
                Gwen::Controls::HorizontalSlider *pSlider = new Gwen::Controls::HorizontalSlider( this );
                pSlider->SetName( sliderName );
                pSlider->SetSize( size.x, 20 );
                pSlider->Dock( Gwen::Pos::Top );
                pSlider->SetRange( param->getMin(), param->getMax() );
                pSlider->SetFloatValue( 0.0f );
                pSlider->onValueChanged.Add( this, &QLiveGuiTrackControl::onParamChange );

                mParamSliders.push_back( pSlider );
                
                // used in Render() to update the value
//                GwenDeviceParamRef p = { pSlider, track->getIndex(), devices[i]->getIndex(), param->getIndex(), param->getRef(), 0.0f };
//                mGwenParams.push_back(p);
            }
        }
    }
    
	virtual ~QLiveGuiTrackControl() {}

	virtual void Render( Gwen::Skin::Base* skin )
    {
        // Volume
        Gwen::Event::Caller	cb = mVolume->onValueChanged;
        mVolume->onValueChanged = Gwen::Event::Caller();
        mVolume->SetFloatValue( mTrack->getVolume() );
        mVolume->onValueChanged = cb;
        
        // Params
        std::vector<std::string> splitValues;
        int trackIdx, deviceIdx, paramIdx;
        
        for( auto k=0; k < mParamSliders.size(); k++ )
        {
            Gwen::Controls::HorizontalSlider *slider = mParamSliders[k];
            
            boost::split( splitValues, slider->GetName(), boost::is_any_of("_") );
            trackIdx    = boost::lexical_cast<int>( splitValues[0] );
            deviceIdx   = boost::lexical_cast<int>( splitValues[1] );
            paramIdx    = boost::lexical_cast<int>( splitValues[2] );
            
            Gwen::Event::Caller	cb = slider->onValueChanged;
            slider->onValueChanged = Gwen::Event::Caller();
            slider->SetFloatValue( mLive->getParamValue( trackIdx, deviceIdx, paramIdx ) );
            slider->onValueChanged = cb;
        }
        
        
        
        // Clips
        Gwen::Controls::LabeledRadioButton  *pSelected  = mClipsRadioController->GetSelected();
        nocte::QLiveClipRef                 playingClip = mTrack->getPlayingClip();
        
        if ( playingClip )
        {
            int selectedClipIdx = -1;
            
            if ( pSelected )
                selectedClipIdx = boost::lexical_cast<int>( pSelected->GetName() );

            if ( selectedClipIdx != playingClip->getIndex() )
            {
                Gwen::Controls::Base::List &children = mClipsRadioController->GetChildren();
                
                for ( Gwen::Controls::Base::List::iterator iter = children.begin(); iter != children.end(); ++iter )
                {
                    Gwen::Controls::LabeledRadioButton* pChild = gwen_cast<Gwen::Controls::LabeledRadioButton>( *iter );
                    selectedClipIdx = boost::lexical_cast<int>( pChild->GetName() );
                    
                    if ( playingClip->getIndex() == selectedClipIdx )
                        pChild->Select();
                }
            }
        }
        
        
        
        
        
        
        /*
        // this piece of code below is awful!
        // TODO: change this shit!
        
        // update params values, I can't use the fucking onValueChanged handler because it gets together with SetFloatValue and mess up with the OSC thread
        float sliderVal;
        float prevVal;
        float paramVal;
        
        for( auto k=0; k < mGwenParams.size(); k++ )
        {
            sliderVal   = mGwenParams[k].slider->GetFloatValue();
            prevVal     = mGwenParams[k].prevVal;
            paramVal    = *mGwenParams[k].ref.get();
            
            if (  sliderVal == paramVal )
                continue;

            if ( sliderVal != prevVal )
                mLive->setParam( mGwenParams[k].trackIdx, mGwenParams[k].deviceIdx, mGwenParams[k].paramIdx, sliderVal );
            
            else
                mGwenParams[k].slider->SetFloatValue( paramVal );

            mGwenParams[k].prevVal = *mGwenParams[k].ref.get();  // always get the latest value
        }
        
        // same thing for the brightness!        
        sliderVal   = mBrightnessGwenParamRef.slider->GetFloatValue();
        prevVal     = mBrightnessGwenParamRef.prevVal;
        paramVal    = *mBrightnessGwenParamRef.ref.get();
        
        if (  sliderVal != paramVal )
        {
            if ( sliderVal != prevVal )
                mLive->setTrackVolume( mBrightnessGwenParamRef.trackIdx, sliderVal );

            else
                mBrightnessGwenParamRef.slider->SetFloatValue( paramVal );
            
            mBrightnessGwenParamRef.prevVal = *mBrightnessGwenParamRef.ref.get();  // always get the latest value
        }
         */
        
//        std::vector<QLiveClipRef>   clips = mTrack->getClips();
//        QLiveClipRef                playingClip;
//        for( auto k=0; k < clips.size(); k++ )
//            if ( clips[k]->isPlaying() )
//            {
//                clip = clips[k];
//                break;
//            }

        /*
         // this is again in conflict with the OSC thread, I must implement a wrapper to manage variable binding and event handler, 
         // the handler should only be called when the value is manually change(aka mouse click)
        // clips
        bool radioVal, prevState, clipState;
        
        for( auto k=0; k < mGwenClips.size(); k++ )
        {
            radioVal    = mGwenClips[k].radioButton->GetRadioButton()->IsChecked();
            prevState   = mGwenClips[k].prevVal;
            clipState   = *mGwenClips[k].ref;
           
            
            if (  radioVal == clipState )
                continue;
            
            if ( radioVal != prevState )
            {
                if ( mClipsRadioController->GetSelected() )
                {
                    int idx = boost::lexical_cast<int>( mClipsRadioController->GetSelected()->GetName() );
                    mLive->playClip( mGwenClips[k].trackIdx, idx );
                }
//                    mLive->setParam( mGwenParams[k].trackIdx, mGwenParams[k].deviceIdx, mGwenParams[k].paramIdx, sliderVal );
            }
            
            else
            {
                if ( clipState )
                    mGwenClips[k].radioButton->Select();
            }
//                mGwenParams[k].slider->SetFloatValue( paramVal );
            
            mGwenClips[k].prevVal = *mGwenClips[k].ref;  // always get the latest value
        }
        */
    }
    
    
private:
    
    void onVolumeChange( Gwen::Controls::Base* pControl )
    {
        Gwen::Controls::HorizontalSlider* slider = (Gwen::Controls::HorizontalSlider*)pControl;
        mLive->setTrackVolume( mTrack->getIndex(), slider->GetFloatValue() );
    }
    
    
    void onParamChange( Gwen::Controls::Base* pControl )
    {
        std::vector<std::string> splitValues;
        boost::split( splitValues, pControl->GetName(), boost::is_any_of("_") );
        
        int trackIdx    = boost::lexical_cast<int>( splitValues[0] );
        int deviceIdx   = boost::lexical_cast<int>( splitValues[1] );
        int paramIdx    = boost::lexical_cast<int>( splitValues[2] );
        
        mLive->setParam( trackIdx, deviceIdx, paramIdx, ((Gwen::Controls::Slider*)pControl)->GetFloatValue() );
    }

    
    void onClipChange( Gwen::Controls::Base* pControl )
    {
        Gwen::Controls::RadioButtonController*  rc          = ( Gwen::Controls::RadioButtonController* ) pControl;
        Gwen::Controls::LabeledRadioButton*     pSelected   = rc->GetSelected();
        int                                     clipIdx     = boost::lexical_cast<int>( pSelected->GetName() );
        
        mLive->playClip( mTrack->getIndex(), clipIdx );
    }
    
private:
    
    nocte::QLiveRef                                 mLive;
    nocte::QLiveTrackRef                            mTrack;
    std::vector<GwenDeviceParamRef>                 mGwenParams;
    std::vector<GwenClipRef>                        mGwenClips;
    GwenDeviceParamRef                              mBrightnessGwenParamRef;
    Gwen::Controls::RadioButtonController           *mClipsRadioController;
    Gwen::Controls::HorizontalSlider                *mVolume;
    std::vector<Gwen::Controls::HorizontalSlider*>  mParamSliders;
    
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