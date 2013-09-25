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
#include "QLiveModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;


QLiveModule::QLiveModule( QLiveRef live, QLiveTrackRef track, QLiveClipRef clip )
: mLive(live), mTrack(track), mClip(clip)
{
    updateBrightness();
    
    mOnClipSelectCb = mClip->connectOnSelect( std::bind( &QLiveModule::clipSelectCallback, this ) );
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


bool QLiveModule::isPlaying() 
{    
    return ( mClip->isPlaying() && mLive->isPlaying() ); 
}


bool QLiveModule::updateModule() 
{
    updateBrightness();

    if ( mClip->isSelected() )
    {
        // update local params
        std::map< std::string, boost::tuple<float,std::shared_ptr<float>,int,int> >::iterator it;
        for ( it=mParams.begin(); it != mParams.end(); it++ )
            boost::get<0>(it->second) = *(boost::get<1>(it->second));
    }
    
    return isPlaying();
}


void QLiveModule::clipSelectCallback()
{
    std::map< std::string, boost::tuple<float,std::shared_ptr<float>,int,int> >::iterator it;
    for ( it=mParams.begin(); it != mParams.end(); it++ )
        mLive->setParamByIndex( mTrack->getIndex(), boost::get<2>(it->second), boost::get<3>(it->second), boost::get<0>(it->second) );
}

