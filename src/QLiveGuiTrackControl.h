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

class QLiveGuiParamControl;
typedef std::shared_ptr<QLiveGuiParamControl> QLiveGuiParamControlRef;


/* --------------------- */
/* --- Param Control --- */
/* --------------------- */

class QLiveGuiParamControl : public Gwen::Controls::Base {
    
public:
    
    static QLiveGuiParamControlRef create( QLiveRef live, QLiveTrackRef track, QLiveDeviceRef device, QLiveParamRef param, Gwen::Controls::Base *parent )
    {
        return QLiveGuiParamControlRef( new QLiveGuiParamControl( live, track, device, param, parent ) );
    }
    
    virtual ~QLiveGuiParamControl() {}
    
    virtual void Render( Gwen::Skin::Base* skin )
    {
        Gwen::Event::Caller	cb = mSlider->onValueChanged;
        mSlider->onValueChanged = Gwen::Event::Caller();
        mSlider->SetFloatValue( mParam->getValue() );
        mSlider->onValueChanged = cb;
    }
    
    
private:
    
    QLiveGuiParamControl( QLiveRef live, QLiveTrackRef track, QLiveDeviceRef device, QLiveParamRef param, Gwen::Controls::Base *parent )
    : Gwen::Controls::Base(parent), mLive(live)
    {
        SetSize( parent->GetSize().x, 40 );
        Dock( Gwen::Pos::Top );
        
        mParam = param;
        
        // Param name
        Gwen::Controls::Label *pLabel = new Gwen::Controls::Label( this );
        pLabel->SetText( param->getName() );
        pLabel->SizeToContents();
        pLabel->SetMargin( Gwen::Margin( 0, 10, 0, 0 ) );
        pLabel->Dock( Gwen::Pos::Top );
        pLabel->SetTextColorOverride( cigwen::toGwen( ci::Color::gray( 0.3f ) ) );
        
        // Slider
        mSlider = new Gwen::Controls::HorizontalSlider( this );
        mSlider->SetSize( parent->GetSize().x, 20 );
        mSlider->Dock( Gwen::Pos::Top );
        mSlider->SetRange( param->getMin(), param->getMax() );
        mSlider->SetFloatValue( 0.0f );
        mSlider->onValueChanged.Add( this, &QLiveGuiParamControl::onChange );
    }
    
    void onChange( Gwen::Controls::Base* pControl )
    {
        mParam->setValue( ((Gwen::Controls::Slider*)pControl)->GetFloatValue() );
    }
    
    
private:
    
    QLiveRef                            mLive;
    Gwen::Controls::HorizontalSlider    *mSlider;
    QLiveParamRef                       mParam;
    
};


/* --------------------- */
/* --- Track Control --- */
/* --------------------- */

class QLiveGuiTrackControl : public Gwen::Controls::Base {
    
public:
    
    static QLiveGuiTrackControlRef create( QLiveRef live, QLiveTrackRef track, ci::Vec2f size, Gwen::Controls::Base *parent )
    {
        return QLiveGuiTrackControlRef( new QLiveGuiTrackControl( live, track, size, parent ) );
    }
    
    virtual ~QLiveGuiTrackControl() {}
    
    virtual void toggleParams()
    {
        for( auto k=0; k < mParamControls.size(); k++ )
            mParamControls[k]->SetHidden( mParamControls[k]->Visible() );
    }
    
    virtual void Render( Gwen::Skin::Base* skin )
    {
        QLiveClipRef clip;
        
        ci::gl::pushMatrices();
        
        ci::gl::translate( ci::Vec2f( 0, 37 ) + cigwen::fromGwen( LocalPosToCanvas() ) );

        glBegin( GL_QUADS );
        int h = 8;
        int top;
        
        for( auto k=0; k < mClipLabels.size(); k++ )
        {
            clip    = mTrack->getClipByIndex( (int)atof( mClipLabels[k]->GetName().c_str() ) );
            top     = k * 20;
            
            if ( clip->isPlaying() )
                ci::gl::color( ci::Color( 0.05f, 1.0f, 0.1f ) );
            else
                ci::gl::color( ci::Color::gray( 0.3f ) );
            
            ci::gl::vertex( 0,  top );
            ci::gl::vertex( h,  top );
            ci::gl::vertex( h,  top + h );
            ci::gl::vertex( 0,  top + h );
        }
        
        glEnd();
        
        ci::gl::popMatrices();
        
        // Volume
        Gwen::Event::Caller	cb = mVolume->onValueChanged;
        mVolume->onValueChanged = Gwen::Event::Caller();
        mVolume->SetFloatValue( mTrack->getVolume() );
        mVolume->onValueChanged = cb;
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
        for( auto i=0; i < clips.size(); i++ )
        {
            clip = clips[i];
            Gwen::Controls::LabelClickable *label = new Gwen::Controls::LabelClickable( this );//( clip->getName() );
            label->Dock( Gwen::Pos::Top );
            label->SetTextColorOverride( cigwen::toGwen( clip->getColor() ) );
            label->SetText( clip->getName() );
            label->SetName( std::to_string( clip->getIndex() ) );
            label->SetMargin( Gwen::Margin( 10, 0, 0, 0 ) );
            label->onPress.Add( this, &QLiveGuiTrackControl::onClipPress );
            mClipLabels.push_back( label );
        }
        
        // Params
        std::vector<QLiveDeviceRef>  devices = track->getDevices();
        std::vector<QLiveParamRef>   params;
        QLiveParamRef                param;
        
        for( auto i=0; i < devices.size(); i++ )
        {
            params = devices[i]->getParams();
            
            for( auto j=1; j < params.size(); j++ )       // start from 1 to ignore "Device On"
            {
                QLiveGuiParamControlRef p = QLiveGuiParamControl::create( mLive, track, devices[i], params[j], this );
                
                mParamControls.push_back( p );
                
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
    
    void onClipPress( Gwen::Controls::Base* pControl )
    {
        int clipIdx = boost::lexical_cast<int>( pControl->GetName() );
        QLiveClipRef clip  = mLive->getClipByIndex( mTrack->getIndex(), clipIdx );

        if ( clip->isPlaying() )
            mLive->stopClip( mTrack->getIndex(), clipIdx );
        else
            mLive->playClip( mTrack->getIndex(), clipIdx );
    }

private:
    
    QLiveRef                                        mLive;
    QLiveTrackRef                                   mTrack;
    std::vector<Gwen::Controls::LabelClickable*>    mClipLabels;
    Gwen::Controls::HorizontalSlider                *mVolume;
    std::vector<QLiveGuiParamControlRef>            mParamControls;
    
};


#endif

