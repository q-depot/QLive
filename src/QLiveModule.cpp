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

using namespace ci;
using namespace ci::app;
using namespace std;


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
            
            for( int k=0; k < 2; k++ )
            {
                mFftBuffer[k] = new float[FFT_SIZE];
                mFftBuffer[k] = mLive->getFftBuffer(k);
            }
        }
        
        mDevice = NULL;
        
        mClip->addStateUpdateCallback( std::bind( &QLiveModule::clipStateUpdateCallback, this ) );
        
        mParamsUpdatedAt = 0;
    }
    
    bool QLiveModule::updateState() 
    {        
        mIsPlaying = ( mClip->isPlaying() && mLive->isPlaying() ) ? true : false;
        
        return mIsPlaying;
    }
    
    void QLiveModule::updateBrightness() 
    { 
        if ( !mTrack ) 
            return;
        
        mTrackVolume = pow( mTrack->getVolume(), 2); 
    }
    
    string QLiveModule::getName()
    {
        if ( mClip )
            return mClip->getName();
        else 
            return "";
    }
    
    
    void QLiveModule::sendLiveParamValue( const string &name, float value ) // set Live value to Module local value is it's playing(is selected)
    {
        if (!mDevice)
            return;
        
        QLiveParam *param = mDevice->getParam(name);
        
        if ( param )
            mLive->setParam( mTrack->getIndex(), mDevice->getIndex(), param->getIndex(), value );
    }
    
    
    bool QLiveModule::updateModule() 
    {
        updateBrightness();
        
        updateState();
        
        if ( mClip->isPlaying() && getElapsedSeconds() - mParamsUpdatedAt > 0.5f )
        {                
            // update local params
            std::map< std::string, std::pair<float,float*> >::iterator it;
            for ( it=mParams.begin(); it != mParams.end(); it++ )
                it->second.first = *(it->second.second);
        }
        
        return mIsPlaying;
    }
    
    
    void QLiveModule::clipStateUpdateCallback()
    { 
        if ( mClip->isPlaying() ) 
        {
//            console() << "clipStateUpdateCallback() " << getName() << " " << mClip->getState() << endl;
            std::map< std::string, std::pair<float,float*> >::iterator it;
            for ( it=mParams.begin(); it != mParams.end(); it++ )
                sendLiveParamValue( it->first, it->second.first );
            
            mParamsUpdatedAt = getElapsedSeconds();
        }
    }

}
