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




class QLiveGuiTrackControl;
typedef std::shared_ptr<QLiveGuiTrackControl> QLiveGuiTrackControlRef;


class QLiveGuiTrackControl : public Gwen::Controls::Base {
    
public:
    
    static QLiveGuiTrackControlRef create( QLiveRef live, QLiveTrackRef track, ci::Vec2f size, Gwen::Controls::Base *parent )
    {
        return QLiveGuiTrackControlRef( new QLiveGuiTrackControl( live, track, size, parent ) );
    }
    
    virtual ~QLiveGuiTrackControl() {}
    
    virtual void toggleParams()
    {
        for( auto k=0; k < mParamSliders.size(); k++ )
            mParamSliders[k]->SetHidden( mParamSliders[k]->Visible() );
    }
    
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
        QLiveClipRef                 playingClip = mTrack->getPlayingClip();
        
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
        
    }
    
    
private:
    
    QLiveGuiTrackControl( QLiveRef live, QLiveTrackRef track, ci::Vec2f size, Gwen::Controls::Base *parent )
    : Gwen::Controls::Base(parent), mLive(live), mTrack( track )
    {
        SetSize( size.x, 0 );
        SetMargin( Gwen::Margin( 15, 15 , 0, 0 ) ); // left, top, right, bottom
        Dock( Gwen::Pos::Left );
        
        std::vector<QLiveClipRef>    clips = track->getClips();
        QLiveClipRef                 clip;

        
        // Label
        Gwen::Controls::Label *label = new Gwen::Controls::Label( this );
        label->SetText( track->getName() );
        label->SizeToContents();
        label->Dock( Gwen::Pos::Top );
        label->SetTextColorOverride( cigwen::toGwen( track->getColor() ) );
        SetHeight( GetSize().y + 20 );
        
        // Brightness
        mVolume = new Gwen::Controls::HorizontalSlider( this );
        mVolume->Dock( Gwen::Pos::Top );
        mVolume->SetSize( size.x - 15, 20 );
        mVolume->SetRange( 0.0f, 1.0f );
        mVolume->SetFloatValue( 0.0f );
        mVolume->onValueChanged.Add( this, &QLiveGuiTrackControl::onVolumeChange );
        SetHeight( GetSize().y + 20 );
        
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

            SetHeight( GetSize().y + 20 );
        }
        mClipsRadioController->SetSize( size.x, clips.size() * 22 );
        mClipsRadioController->Dock( Gwen::Pos::Top );
        mClipsRadioController->onSelectionChange.Add( this, &QLiveGuiTrackControl::onClipChange );
        
        
        // Params
        std::vector<QLiveDeviceRef>  devices = track->getDevices();
        std::vector<QLiveParamRef>   params;
        QLiveParamRef                param;
        
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
                SetHeight( GetSize().y + 40 );
            }
        }
    }
    
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
    
    QLiveRef                                 mLive;
    QLiveTrackRef                            mTrack;
    Gwen::Controls::RadioButtonController           *mClipsRadioController;
    Gwen::Controls::HorizontalSlider                *mVolume;
    std::vector<Gwen::Controls::HorizontalSlider*>  mParamSliders;
    
};


#endif

