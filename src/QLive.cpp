/*
 *  QLive.cpp
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "cinder/Thread.h"
#include "cinder/Text.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Xml.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "QLive.h"

using namespace ci;
using namespace ci::app;
using namespace std;

    
QLive::QLive( string osc_host, int osc_live_in_port, int osc_live_out_port, bool initFromLive ) 
: mOscHost(osc_host), mOscLiveInPort(osc_live_in_port), mOscLiveOutPort(osc_live_out_port)
{
    mOscListener        = NULL;
    mOscSender          = NULL;
    mIsPlaying          = false;
    mIsReady            = false;
    
    mRunOscDataThread   = false;

    initOsc();
    
    // set LiveOsc peer: /remix/set_peer HOST PORT (null host > host is automatically set to the host that sent the request)
    sendMessage("/remix/set_peer", "s i" + ci::toString(mOscLiveInPort) );
    
    mGetInfoRequestAt = - GET_INFO_MIN_DELAY * 2.0f;
    
    if ( initFromLive )
        getInfo();
    
    mFontSmall	= Font( "Helvetica", 12 );
    mFontMedium	= Font( "Helvetica", 14 );
    
    console() << "Live > Initialized!" << endl;	
    
    mPingReceivedAt = getElapsedSeconds();
}


void QLive::initOsc()
{
    try {
        
        if ( mOscSender )					// close OSC sender
        {
            delete mOscSender;
            mOscSender = NULL;
        }
        
        mOscSender = new osc::Sender();
        mOscSender->setup(mOscHost, mOscLiveOutPort);
        
        console() << "LIVE: Initialized OSC sender " << toString(mOscHost) + ":" << toString(mOscLiveOutPort) << endl;
    } 
    catch (...) {
        mOscSender = NULL;
        console() << "LIVE: Failed to bind OSC sender socket " << toString(mOscHost) << ":" << toString(mOscLiveOutPort) << endl;
    }
    
    
    try {
        
        if ( mOscListener )					// close OSC listener
        {
            mOscListener->shutdown();
            delete mOscListener;
            mOscListener = NULL;
            ci::sleep(50);
        }
        
        mOscListener = new osc::Listener();
        mOscListener->setup(mOscLiveInPort);
        console() << "LIVE: Initialized OSC listener " << mOscLiveInPort << endl;
    } 
    catch (...) {
        mOscListener = NULL;
    }
    
    mReceiveOscDataThread = std::thread( &QLive::receiveData, this );
}

void QLive::shutdown() 
{
    mIsReady = false;
    
    mRunOscDataThread = false;
    mReceiveOscDataThread.join();
    mOscListener->shutdown();
    delete mOscListener;
    mOscListener = NULL;

    if ( mOscSender )					// close OSC sender
    {
        delete mOscSender;
        mOscSender = NULL;
    }
    
    clearObjects();					// delete objects and clear pointers
}


void QLive::clearObjects()
{
    mTracks.clear();
    mScenes.clear();
}


bool QLive::getInfo() 
{ 
    if ( getElapsedSeconds() - mGetInfoRequestAt < GET_INFO_MIN_DELAY )
        return false;
    
    mIsReady = false;
    
    clearObjects();
    
    mSelectedTrack = NULL;
    
    mGetInfoRequestAt = getElapsedSeconds();

    sendMessage("/live/name/scene");

    ci::sleep(50);
    sendMessage("/live/name/track");

    ci::sleep(200);
    sendMessage("/live/name/clip");    

    ci::sleep(200);
    sendMessage("/live/devicelist");

    ci::sleep(500);
    
    mIsReady = true;
    
    return mIsReady;
}


void QLive::renderDebug( bool renderScenes, bool renderTracks, bool renderClips, bool renderDevices )
{
    QLiveTrackRef   track;
    QLiveClipRef    clip;
    QLiveDeviceRef  device;

    gl::color( Color::white() );
    
    TextLayout textLayout = TextLayout();
    textLayout.clear( ColorA(0.0f, 0.0f, 0.0f, 0.0f) );
    textLayout.setLeadingOffset(3);
    textLayout.setColor( Color::white() );
    textLayout.setFont( mFontSmall );
    
    textLayout.addLine( "QLIVE" );
    textLayout.addLine( mOscHost + " / in " + toString(mOscLiveInPort) + " / out " + toString(mOscLiveOutPort) );
    textLayout.addLine( " " );
    
    if ( renderScenes )
    {
        textLayout.addLine( "SCENES" );
        textLayout.addLine( " " );
        
        for (int k = 0; k < mScenes.size(); k++)
            textLayout.addLine( toString(mScenes[k]->mIndex) + "\t\t" + mScenes[k]->mName );
        textLayout.addLine( " " );	
    }

    if ( renderTracks )
    {
        textLayout.addLine( "TRACKS" );
        textLayout.addLine( " " );
        
        for (int k = 0; k < mTracks.size(); k++)
        {
            track = mTracks[k];
            
            textLayout.addLine( toString(track->mIndex) + " - " + track->mName + "\t" + toString(track->mClips.size()) );

            // Sends
            if ( !track->mSends.empty() )
                textLayout.addLine( "\t" );
            
            for ( map<int,float>::iterator it=track->mSends.begin() ; it != track->mSends.end(); it++ )
                textLayout.addLine( toString(it->first) + ">" + toString(it->second) );
            
            textLayout.addLine( " " );

            if ( renderDevices )
            {
                for( int i = 0; i < track->mDevices.size(); i++ )
                {
                    device = track->mDevices[i];
                    textLayout.addLine( "\tDEVICE\t" + toString(device->mIndex) + "\t" + device->mName );
                }
                textLayout.addLine( " " );
            }
                
            if ( renderClips )
            {
                textLayout.setFont( mFontSmall );
                for( int i = 0; i < track->mClips.size(); i++ )
                {
                    clip = track->mClips[i];                
                    textLayout.addLine( "\tCLIP\t\t" + toString(clip->mIndex) + "\t" + clip->mName );
                }
                textLayout.addLine( " " );
            }
        }
    }
    textLayout.addLine( " " );
    
    gl::Texture tex = gl::Texture(textLayout.render(true));
    gl::draw( tex, Vec2f( getWindowWidth() - 450, 15) );

}


void QLive::sendMessage(string address, string args)
{	
    osc::Message message;
    message.setAddress( address );
    message.setRemoteEndpoint(mOscHost, mOscLiveOutPort);
    vector<string> splitValues;
    boost::split(splitValues, args, boost::is_any_of(" "));
    
    for (int k = 0; k < splitValues.size(); k++) {
        char argType = splitValues[k][0];
        splitValues[k].erase(0,1);
        
        if ( argType == 'i' )
            message.addIntArg( boost::lexical_cast<int>( splitValues[k] ) );
        else
            if ( argType == 'f' )
            message.addFloatArg( boost::lexical_cast<float>( splitValues[k] ) );
        else 
            if ( argType == 's' )
            message.addStringArg( splitValues[k] );
    }
    
    mOscSender->sendMessage(message);
}


void QLive::parseScene( osc::Message message )
{
    int     index       = message.getArgAsInt32(0);
    string  name        = message.getArgAsString(1);

    QLiveSceneRef scene = getScene( index );
    
    if ( scene )
    {
        scene->mName = name;
        return;
    }
    
    mScenes.push_back( QLiveScene::create( index, name ) );
}


bool sortTracksByIndex( QLiveTrackRef a, QLiveTrackRef b )
{
    return ( a->getIndex() < b->getIndex() );
}


void QLive::parseTrack( osc::Message message ) 
{          
    int     index	= message.getArgAsInt32(0);
    string  name    = message.getArgAsString(1);
    ColorA  color   = colorIntToColorA( message.getArgAsInt32(2) );
    
    QLiveTrackRef track = getTrack( index );
    
    if ( track )
        track->mName = name;
    else
        mTracks.push_back( QLiveTrack::create( index, name, color ) );

    sort( mTracks.begin(), mTracks.end(), sortTracksByIndex );

    sendMessage("/live/volume", "i" + toString(index) );	// get track volume

    listTrackDevices(index);
}


void QLive::parseClip( osc::Message message ) 
{
    int     trackIdx	= message.getArgAsInt32(0);
    int 	clipIdx		= message.getArgAsInt32(1);
    string 	name		= message.getArgAsString(2);
    ColorA  color       = colorIntToColorA( message.getArgAsInt32(3) );

    QLiveTrackRef   track = getTrack( trackIdx );
    QLiveClipRef    clip;
    
    if ( track )
    {
        clip = track->getClip(clipIdx);
        if ( clip )
        {
            clip->mName     = name;
            clip->mColor    = color;
            return;
        }
    }

    clip = QLiveClip::create( clipIdx, name, color );
    
    mTracks[trackIdx]->mClips.push_back( clip );
    
    sendMessage("/live/clip/info", "i" + toString(trackIdx) + " i" + toString(clipIdx) );       // get clip info
}


void QLive::parseClipInfo( osc::Message message ) 
{    
    int             trackIdx	= message.getArgAsInt32(0);
    int             clipIdx		= message.getArgAsInt32(1);
    ClipState       state       = (ClipState)message.getArgAsInt32(2);
    QLiveTrackRef   track       = getTrack( trackIdx );
    QLiveClipRef    clip;
    
    if ( !track )
        return;

    clip = track->getClip(clipIdx);
    if ( clip )
        clip->setState(state);
}


void QLive::parseDeviceList( osc::Message message ) 
{
    if ( message.getNumArgs() < 3 )	// seems there is an error in the APIs!
       return;
    
    int             deviceIdx;
    string          deviceName;

    QLiveTrackRef   track = getTrack( message.getArgAsInt32(0) );
    
    if ( track )
    {
        for( int k=1; k < message.getNumArgs(); k+=2 )
        {
            deviceIdx	= message.getArgAsInt32(k);
            deviceName  = message.getArgAsString(k+1);
                
            if ( !track->getDevice(deviceIdx) )
            {
                track->mDevices.push_back( QLiveDevice::create( deviceIdx, deviceName ) );
                
                // get device params	
                sendMessage("/live/device", "i" + toString(track->mIndex) + " i" + toString(deviceIdx) );
            }
        }
    }
}


void QLive::parseDeviceAllParams( osc::Message message )
{
    //	int trackIdx, int deviceIdx, int parameterIdx, int value,  str name, ...
    if ( message.getNumArgs() < 7 )
        return;
    
    int             trackIdx    = message.getArgAsInt32(0);
    int             deviceIdx   = message.getArgAsInt32(1);
    QLiveTrackRef	track       = getTrack(trackIdx);
    QLiveDeviceRef  device      = track->getDevice( deviceIdx );
    
    if ( !device )
        return;
    
    int         paramIdx;
    float       paramVal;
    float       paramMinVal;
    float       paramMaxVal;
    std::string paramName;
    
    for (int k=2; k < message.getNumArgs(); k+=5) 
    {
        paramIdx    = message.getArgAsInt32(k);
        paramVal    = message.getArgAsFloat(k+1);
        paramName   = message.getArgAsString(k+2);
        paramMinVal = message.getArgAsFloat(k+3);
        paramMaxVal = message.getArgAsFloat(k+4);

        device->addParam( paramIdx, paramVal, paramName, paramMinVal, paramMaxVal );
    }
}


void QLive::parseDeviceParam( osc::Message message )
{
    //	/live/device/param (int track, int device, int paarmeter, int value, str name)
    
    if ( message.getNumArgs() < 7 )
        return;
    
    int             trackIdx    = message.getArgAsInt32(0);
    int             deviceIdx   = message.getArgAsInt32(1);
//        int			paramIdx    = message.getArgAsInt32(2);
    float           paramValue  = ( message.getArgType(3) == osc::TYPE_INT32 ) ? message.getArgAsInt32(3) : message.getArgAsFloat(3);
    string          paramName   = message.getArgAsString(4);
    float           paramMin    = message.getArgAsFloat(5);
    float           paramMax    = message.getArgAsFloat(6);
    
    QLiveParamRef   param       = getParam( trackIdx, deviceIdx, paramName );
    
    if ( param )    // we assume the param exists, parseDeviceAllParams() creates one for each device param in Live
    {
        param->setRange( paramMin, paramMax );
        param->setValue( paramValue );
    }
}


void QLive::parseTrackSends( ci::osc::Message message )
{
    QLiveTrackRef track;

    track = getTrack( message.getArgAsInt32(0) );
    if ( track )
        for( int k=1; k < message.getNumArgs() - 1; k+=2 )
            track->mSends[ message.getArgAsInt32(k) ] = message.getArgAsFloat(k+1);
}


void QLive::receiveData()
{    
    mRunOscDataThread = true;

    while( mRunOscDataThread && mOscListener ) {

        while (mOscListener->hasWaitingMessages()) {
            osc::Message message;
            mOscListener->getNextMessage(&message);
            
            string	msgAddress = message.getAddress();

            // debug
//            if ( true && msgAddress != "/live/ping" && msgAddress != "/live/beat" )
//                debugOscMessage( message );

            // Parse Live objects
            if ( msgAddress == "/live/name/scene" )
                parseScene(message);
            
            else if ( msgAddress == "/live/name/track" )
                parseTrack(message);

            else if ( msgAddress == "/live/scene" )
                setSelectedScene( message.getArgAsInt32(0) - 1 );  // LiveOSC inconsistency
                
            else if ( msgAddress == "/live/track" )
                setSelectedTrack( message.getArgAsInt32(0) - 1 );  // LiveOSC inconsistency
                
            else if ( msgAddress == "/live/name/clip" )
                parseClip(message);
    
            else if ( msgAddress == "/live/clip/info" )
                parseClipInfo(message);
            
            else if ( msgAddress == "/live/devicelist" )
                parseDeviceList(message);	
            
            else if ( msgAddress == "/live/device/allparam" )
                parseDeviceAllParams(message);
        
            else if ( msgAddress == "/live/play" )
                mIsPlaying = ( (ClipState)message.getArgAsInt32(0) == 2 ) ? true : false;

            else if ( msgAddress == "/live/volume" )
            {
                int trackIndex = message.getArgAsInt32(0);
                if ( trackIndex < mTracks.size() )
                    mTracks[trackIndex]->setVolume( message.getArgAsFloat(1) );
            }

//            else if ( msgAddress == "/live/track" )
//            {
//                int trackIndex = message.getArgAsInt32(0) - 1;  // shift
//                for(int k=0; k < mTracks.size(); k++)
//                    if ( mTracks[k]->mIndex == trackIndex )
//                    {
//                        mSelectedTrack = mTracks[k];
//                        break;
//                    }
//            }
            
            else if ( msgAddress == "/live/device/param" )
                parseDeviceParam(message);
            
            else if ( msgAddress == "/live/ping" )
                mPingReceivedAt = getElapsedSeconds();
            
            else if ( msgAddress == "/live/send" )
                parseTrackSends(message);
        }
            
        sleep(15.0f);
//		boost::this_thread::sleep(boost::posix_time::milliseconds(15));
    }
    console() << "Live > receiveData() thread exited!" << endl;
}


void QLive::debugOscMessage( osc::Message message )
{
    console() << "LIVE: Address: " << message.getAddress() << " ";
    
    for (int i = 0; i < message.getNumArgs(); i++) {
        if (message.getArgType(i) == osc::TYPE_INT32){
            try {
                console() << message.getArgAsInt32(i) << " ";
            }
            catch (...) {
                console() << "Exception reading argument as int32" << std::endl;
            }
            
        }else if (message.getArgType(i) == osc::TYPE_FLOAT){
            try {
                console() << message.getArgAsFloat(i) << " ";
            }
            catch (...) {
                console() << "Exception reading argument as float" << std::endl;
            }
        }else if (message.getArgType(i) == osc::TYPE_STRING){
            try {
                console() << message.getArgAsString(i).c_str() << " ";
            }
            catch (...) {
                console() << "Exception reading argument as string" << std::endl;
            }
            
        }
    }
    console() << endl;
}


void QLive::saveSettings( ci::fs::path path )
{
    path.replace_extension( ".xml" );
    XmlTree liveSettings("QLiveSettings", "" );
    XmlTree scenes("scenes", "" );
    XmlTree tracks("tracks", "" );
    
    for( size_t k=0; k < mScenes.size(); k++ )
        scenes.push_back( mScenes[k]->getXmlNode() );
    
    for( size_t k=0; k < mTracks.size(); k++ )
        tracks.push_back( mTracks[k]->getXmlNode() );
    
    liveSettings.push_back( scenes );
    liveSettings.push_back( tracks );
  
    liveSettings.write( writeFile(path) );
}


void QLive::loadSettings( ci::fs::path path, bool forceXmlSettings )
{
    if ( forceXmlSettings )
        clearObjects();
    
    XmlTree liveSettings;
    
    try 
    {
        liveSettings = XmlTree( loadFile(path) );
    }
    catch ( ... )
    {
        console() << "ASOW_stageApp::loadSettings() > settings file " << path.filename().generic_string() << " not found!" << endl;
        return;
    }
    
    QLiveSceneRef   scene;
    QLiveTrackRef   track;
    int             index;
    string          name;
    
    // parse scenes
    for( XmlTree::Iter nodeIt = liveSettings.begin("QLiveSettings/scenes/scene"); nodeIt != liveSettings.end(); ++nodeIt )
    {
        index   = nodeIt->getAttributeValue<int>("index");
        name    = nodeIt->getAttributeValue<string>("name");
        scene   = getScene(name);
        
        if ( scene )
            scene->loadXmlNode( *nodeIt );
        
        else if ( !scene && forceXmlSettings )
        {
            scene = QLiveScene::create( index, name );
            scene->loadXmlNode( *nodeIt );
            mScenes.push_back( scene );
        }
    }
    
    // parase tracks
    for( XmlTree::Iter nodeIt = liveSettings.begin("QLiveSettings/tracks/track"); nodeIt != liveSettings.end(); ++nodeIt )
    {
        index   = nodeIt->getAttributeValue<int>("index");
        name    = nodeIt->getAttributeValue<string>("name");
        track   = getTrack(name);

        if ( track )
            track->loadXmlNode( *nodeIt, forceXmlSettings );
        
        else if ( !track && forceXmlSettings )
        {
            track = QLiveTrack::create( index, name );
            track->loadXmlNode( *nodeIt, forceXmlSettings );
            mTracks.push_back( track );
        }
    }
    
   // syncLiveClips();        // send clip state to Live
    
}


void QLive::setSelectedScene( int idx )
{
    if ( mSelectedScene )
        mSelectedScene->mIsSelected = false;
    
    mSelectedScene = getScene( idx );
    
    if ( !mSelectedScene )
        return;
    
    mSelectedScene->mIsSelected = true;

    setSelectedClip();
}


void QLive::setSelectedTrack( int idx )
{
    if ( mSelectedTrack )
        mSelectedTrack->mIsSelected = false;
    
    mSelectedTrack = getTrack( idx );

    if ( !mSelectedTrack )
        return;
    
    mSelectedTrack->mIsSelected = true;
    
    setSelectedClip();
}


void QLive::setSelectedClip()
{
    if ( mSelectedScene && mSelectedTrack )
    {
        QLiveClipRef clip = mSelectedTrack->getClip( mSelectedScene->mIndex );
        
        if ( clip && clip == mSelectedClip )
            return;
        
        if ( mSelectedClip )
            mSelectedClip->select(false);
        
        mSelectedClip = clip;

        if ( !mSelectedClip )
            return;
        
        mSelectedClip->select(true);
    }
}


void QLive::playClip( int trackIdx, int clipIdx )
{
    QLiveTrackRef track = getTrack( trackIdx );
    
    if ( !track )
        return;
    
    QLiveClipRef clip = track->getClip(clipIdx);
    
    if ( !clip )
        return;
    
    sendMessage("/live/play/clip", "i" + ci::toString(trackIdx) + " i" + ci::toString(clipIdx) );
    
    std::vector<QLiveClipRef> clips = track->getClips();
    for( size_t k=0; k < clips.size(); k++ )
        if ( clips[k] == clip )
            clip->setState( CLIP_PLAYING );
        else
            clips[k]->setState( HAS_CLIP );

    // TODO: if both track and scene change, the selected clip can be the wrong one for an instant between the calls
    setSelectedScene( clipIdx );
    setSelectedTrack( trackIdx );
}


void QLive::stopClip( int trackIdx, int clipIdx )
{
    QLiveTrackRef track = getTrack( trackIdx );
    
    if ( !track )
        return;
    
    QLiveClipRef clip = track->getClip(clipIdx);
    
    if ( !clip )
        return;
    
    sendMessage("/live/stop/clip", "i" + ci::toString(trackIdx) + " i" + ci::toString(clipIdx) );
    clip->setState( HAS_CLIP );
}

