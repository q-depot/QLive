/*
 *  QLiveModule.cpp
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#include "cinder/app/AppBasic.h"
#include "QLive.h"

namespace nocte {
    
    
    QLiveModule::QLiveModule( QLive *live, QLiveTrack *track, QLiveClip *clip )
    : mLive(live), mTrack(track), mClip(clip)
    {
        updateState();
        updateBrightness();
        
        // init Fft buffer
        if ( mLive )
        {   
            mFftBuffer = new float*[2];
            
            for( int k=0; k < 2; k++)
            {
                mFftBuffer[k] = new float[FFT_SIZE];
                mFftBuffer[k] = mLive->getFftBuffer(k);
            }
        }
    };
    
    bool QLiveModule::updateState() 
    {
        if ( !mClip )
            return false;
        
        ClipState state = mClip->getState();
        
        if ( state == HAS_CLIP )
            mIsClipPlaying = false;
        else if ( state == CLIP_PLAYING )
            mIsClipPlaying = true;
        
        mIsPlaying = mIsClipPlaying && mLive->isPlaying();
        
        return mIsPlaying;
    };
    
    bool QLiveModule::isPlaying() { return mIsClipPlaying && mLive->isPlaying(); };
    
    void QLiveModule::updateBrightness() 
    { 
        if ( !mTrack ) 
            return;
        
        mTrackVolume = pow( mTrack->getVolume(), 2); 
    }
    
    std::string QLiveModule::getName()
    {
        if ( mClip )
            return mClip->getName();
        else 
            return "";
    }
    
}
