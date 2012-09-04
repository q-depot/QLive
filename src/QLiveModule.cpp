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
        
        mClip->addStateUpdateCallback( std::bind( &QLiveModule::clipStateUpdateCallback, this ) );
        
        mParamsUpdatedAt = 0;
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
    
    
    void QLiveModule::sendLocalParamValues( const string &name ) // set Live value to Module local value is it's playing(is selected)
    {
        if ( mParams.count(name) == 0 )
            return;
        
        boost::tuple<float,float*,int,int> localParam = mParams[name];

        
        mLive->setParam( mTrack->getIndex(), boost::get<2>(localParam), boost::get<3>(localParam), boost::get<0>(localParam) );
        
        /////
        
//        if (!mDevice)
//            return;
//        
//        QLiveParam *param = mDevice->getParam(name);
//        
//        if ( param )
//            mLive->setParam( mTrack->getIndex(), mDevice->getIndex(), param->getIndex(), value );
    }
    
    
    bool QLiveModule::updateModule() 
    {
        updateBrightness();
        
        if ( mClip->isPlaying() && getElapsedSeconds() - mParamsUpdatedAt > 0.5f )
        {                
            // update local params
            std::map< std::string, boost::tuple<float,float*,int,int> >::iterator it;
            for ( it=mParams.begin(); it != mParams.end(); it++ )
                boost::get<0>(it->second) = *(boost::get<1>(it->second));
//                it->second.first = *(it->second.second);
        }
        
        return isPlaying();
    }
    
    
    void QLiveModule::clipStateUpdateCallback()
    { 
        if ( mClip->isPlaying() ) 
        {
            std::map< std::string, boost::tuple<float,float*,int,int> >::iterator it;
            for ( it=mParams.begin(); it != mParams.end(); it++ )
                sendLocalParamValues( it->first );
            
            //    sendLiveParamValue( it->first, it->second.first );
            
            mParamsUpdatedAt = getElapsedSeconds();
        }
    }

}
