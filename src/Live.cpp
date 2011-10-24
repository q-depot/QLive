/*
 *  Live.cpp
 *  QUBO
 *
 *  Created by Q on 08/08/2011.
 *  Copyright 2011 . All rights reserved.
 *
 */


#include "cinder/Thread.h"
#include "cinder/Text.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "Logger.h"
#include "Live.h"
#include "Modulo.h"

using namespace ci;
using namespace ci::app;
using namespace std;



Live::Live(string osc_host, int osc_in_port, int osc_out_port, bool debugMode, bool init) 
: mOscHost(osc_host), mOscInPort(osc_in_port), mOscOutPort(osc_out_port), mDebugMode(debugMode), mOscListener(NULL), mOscSender(NULL), mSelectedTrack(NULL), mIsPlaying(false)
{	
	
	initOsc();
	
	mGetInfoRequestAt = - GET_INFO_MIN_DELAY * 2.0f;
	
	if ( init )
		getInfo();
	
	mColorWhite = ColorA( 1.0f, 1.0f, 1.0f, 1.0f );
	mColor1		= ColorA( 0.95f, 0.25f, 0.5f, 1.0f );
	mColor2		= ColorA( 0.39f, 0.89f, 0.92f, 1.0f );
	mColor3		= ColorA( 0.90, 0.88f, 0.56f, 1.0f );
	mColor4		= ColorA( 0.57, 0.88f, 0.36f, 1.0f );
	
	mFontSmall	= Font("Helvetica", 12);
	
	Logger::log("Live > Initialized!");
}


void Live::initOsc()
{
	if ( mOscListener )					// close OSC listener
	{
		mOscListener->shutdown();		
		delete mOscListener;
		mOscListener = NULL;
		ci::sleep(50);
	}
	
	if ( mOscSender )					// close OSC sender
	{
		delete mOscSender;
		mOscSender = NULL;
	}
	
	try {
		mOscSender = new osc::Sender();
		mOscSender->setup(mOscHost, mOscOutPort);
		Logger::log("LIVE: Initialized OSC sender " + toString(mOscHost) + ":" + toString(mOscOutPort) );
	} catch (...) {
		mOscSender = NULL;
		Logger::log("LIVE: Failed to bind OSC sender socket " + toString(mOscHost) + ":" + toString(mOscOutPort) );
	}
	
	try {
		mOscListener = new osc::Listener();
		mOscListener->setup(mOscInPort);
		Logger::log("LIVE: Initialized OSC listener " + toString(mOscInPort) );
	} catch (...) {
		mOscListener = NULL;
		Logger::log("LIVE: Failed to bind OSC listener socket " + toString(mOscInPort) );
	}
	
	thread receiveDataThread( &Live::receiveData, this);
}


void Live::shutdown() 
{
	if ( mOscListener )					// close OSC listener
	{
		mOscListener->shutdown();		
		delete mOscListener;
		mOscListener = NULL;
		ci::sleep(50);
	}

	if ( mOscSender )					// close OSC sender
	{
		delete mOscSender;
		mOscSender = NULL;
	}
	
	deleteObjects();					// delete objects and clear pointers
	
	Logger::log("Live > shutdown!");
}


bool Live::getInfo() 
{ 
	if ( getElapsedSeconds() - mGetInfoRequestAt < GET_INFO_MIN_DELAY )
		return false;
	
	Logger::log("Live > getInfo()");
	
	mSelectedTrack = NULL;
	
	mGetInfoRequestAt = getElapsedSeconds();

	deleteObjects();
	
	Logger::log("Live > getInfo()::Scenes");
	sendMessage("/live/name/scene");
	
	Logger::log("Live > getInfo()::Tracks");
	sendMessage("/live/name/track"); 

	ci::sleep(500);
	Logger::log("Live > getInfo()::Clips");
	sendMessage("/live/name/clip");
	return true;
}


void Live::deleteObjects()
{
	for(int k=0; k < mTracks.size(); k++) {
		delete mTracks[k];
	}
	mTracks.clear();
	
	
	for(int k=0; k < mScenes.size(); k++)
		delete mScenes[k];
	mScenes.clear();
	
	for(int k=0; k < mClips.size(); k++)
		delete mClips[k];
	mClips.clear();
	
	for(int k=0; k < mDevices.size(); k++)
		delete mDevices[k];
	mDevices.clear();
}


void Live::render() 
{
	if( mDebugMode )
		drawDebug();
}


void Live::drawDebug() 
{
	TextLayout textLayout = TextLayout();
	textLayout.clear( ColorA(0.0f, 0.0f, 0.0f, 1.0f) );
	textLayout.setLeadingOffset(3);
	textLayout.setFont( mFontSmall );
	textLayout.setColor( mColorWhite );
	
/*
	textLayout.addLine( "SCENES" );
	textLayout.addLine( "index\tname" );
	for (int k = 0; k < mScenes.size(); k++)
		textLayout.addLine( toString(mScenes[k]->mIndex) + "\t\t" + mScenes[k]->mName );
	textLayout.addLine( " " );
*/	
	
	textLayout.addLine( "TRACKS" );
	textLayout.addLine( "index\tname" );
	
	for (int k = 0; k < mTracks.size(); k++)
		textLayout.addLine( toString(mTracks[k]->mIndex) + "\t\t" + mTracks[k]->mName );
	textLayout.addLine( " " );
	
	
	textLayout.addLine( "CLIPS" );
	textLayout.addLine( "index\ttrack\t\tname" );
	for (int k = 0; k < mClips.size(); k++)
	{
		if ( mClips[k]->getState() == CLIP_PLAYING )
			textLayout.setColor( mColor1 );
		else
			textLayout.setColor( mColorWhite );
			
		textLayout.addLine( toString(mClips[k]->mIndex) + "\t\t" + toString(mClips[k]->mTrackIndex) + "\t\t" + mClips[k]->mName );
	}
	textLayout.addLine( " " );

	textLayout.setColor( mColorWhite );	
	
	textLayout.addLine( "DEVICES" );
	textLayout.addLine( "index\ttrack\t\tname" );
	for (int k = 0; k < mDevices.size(); k++)
		textLayout.addLine( toString(mDevices[k]->mIndex) + "\t\t" + toString(mDevices[k]->mTrackIndex) + "\t\t" + mDevices[k]->mName );
	textLayout.addLine( " " );
	
	gl::Texture tex = gl::Texture(textLayout.render(true));
	gl::draw( tex, Vec2f( getWindowWidth() - 450, 15) );
}


void Live::sendMessage(string address, string args)
{	
	
//	console() << "send message" << endl;
	osc::Message message;
	message.setAddress( address );
	message.setRemoteEndpoint(mOscHost, mOscOutPort);
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


void Live::parseScene( osc::Message message ) {
	
	int index	= message.getArgAsInt32(0);
	string name = message.getArgAsString(1);
	
	// update if the scene already exists
	for(int k=0; k < mScenes.size(); k++)
		if ( mScenes[k]->mIndex == index )
		{
			mScenes[k]->mName = name;
			return;
		}
	
	mScenes.push_back( new LiveScene( index, name ) );
	
	//console() << "parse scene: " << endl;
}


bool sortTracksByIndex(LiveTrack *a,LiveTrack *b) { return ( a->getIndex() < b->getIndex() ); };


void Live::parseTrack( osc::Message message ) {
	
	int index	= message.getArgAsInt32(0);
	string name = message.getArgAsString(1);
	
	bool has_track = false;
	
	for(int k=0; k < mTracks.size(); k++)
		if( mTracks[k]->mIndex == index )
		{
			mTracks[k]->mName = name;
			has_track = true;
			break;
		}
	
	if ( !has_track )
		mTracks.push_back( new LiveTrack( index, name ) );

	sort (mTracks.begin(), mTracks.end(), sortTracksByIndex); 

	sendMessage("/live/volume", "i" + toString(index) );	// get track volume

	listTrackDevices(index);
}


void Live::parseClip( osc::Message message ) {
	
	int trackIndex	= message.getArgAsInt32(0);
	int index		= message.getArgAsInt32(1);
	string name		= message.getArgAsString(2);
	int colorInt	= message.getArgAsInt32(3);
	
	// return if the clips already exists
	for(int k=0; k < mClips.size(); k++)
		if ( mClips[k]->mIndex == index && mClips[k]->mTrackIndex == trackIndex )
			return;

	ColorA color;
	color.r = (float)((colorInt >> 16) & 0xB0000FF) / 255.0f;
	color.g = (float)((colorInt >> 8) & 0xB0000FF) / 255.0f;
	color.b = (float)(colorInt & 0xB0000FF) / 255.0f;
	color.a	= 1.0f;
	
	LiveClip *clip = new LiveClip( index, name, trackIndex, color );
	
	mClips.push_back( clip );
	if ( trackIndex < mTracks.size() )
		mTracks[trackIndex]->addClip( clip );
	
	sendMessage("/live/clip/info", "i" + toString(trackIndex) + " i" + toString(index) );	// get clip info
}


void Live::parseClipInfo( osc::Message message ) {
	
	int trackIndex	= message.getArgAsInt32(0);
	int index		= message.getArgAsInt32(1);
	ClipState state	= (ClipState)message.getArgAsInt32(2);
	
	for(int k=0; k < mClips.size(); k++)
	{
		if ( mClips[k]->getTrackIndex() == trackIndex && mClips[k]->mIndex == index)
			mClips[k]->setState(state);
	}
	
	//console() << "parse clip info: " << trackIndex << " " << index << " " << state << endl;
}


void Live::parseDevice( osc::Message message ) {
   
	if ( message.getNumArgs() < 3 )	// seems there is an error in the APIs!
	   return;
	
	int			trackIndex = message.getArgAsInt32(0);
	int			index;
	string		name;
	LiveDevice	*dev;
	bool		deviceExists;
	
	for (int k=1; k < message.getNumArgs(); k+=2) {
		index		= message.getArgAsInt32(k);
		name		= message.getArgAsString(k+1);
		
		deviceExists = false;
		
		// update if the device already exists
		for(int k=0; k < mDevices.size(); k++)
			if ( mDevices[k]->mName == name && mDevices[k]->mTrackIndex == trackIndex )
			{
				mDevices[k]->mName = name;
				deviceExists = true;
			}
			
			
		if ( !deviceExists )
		{
			dev = new LiveDevice( index, name, trackIndex );	
			mDevices.push_back( dev );
			mTracks[trackIndex]->addDevice( dev );
		}
	// get device params	
	//	sendMessage("/live/device", "i" + toString(trackIndex) + " i" + toString(index) );
	}
}


void Live::receiveData(){

	while( mOscListener ) {

		while (mOscListener->hasWaitingMessages()) {
			osc::Message message;
			mOscListener->getNextMessage(&message);
			
			string	msgAddress = message.getAddress();

			// debug
			if (false)
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
			
		//	console() << "receive DATA! " << msgAddress << endl;
			
			// Parse Live objects
			if ( msgAddress == "/live/name/scene" ) {
				parseScene(message);
			}
			else if ( msgAddress == "/live/name/track" ) {
				parseTrack(message);
			}
			else if ( msgAddress == "/live/name/clip" ) {
				parseClip(message);	
			}
			else if ( msgAddress == "/live/clip/info" ) {
				parseClipInfo(message);
			}
			else if ( msgAddress == "/live/devicelist" ) {
				parseDevice(message);	
			}
		//	else if ( msgAddress == "/live/device/allparam" ) {
		//		parseDeviceParams(message);
		//	}
			else if ( msgAddress == "/live/play" )
			{
				mIsPlaying = ( (ClipState)message.getArgAsInt32(0) == 2 ) ? true : false;
			}

			else if ( msgAddress == "/live/volume" )
			{
				int trackIndex = message.getArgAsInt32(0);
				if ( trackIndex < mTracks.size() )
					mTracks[trackIndex]->mVolume = message.getArgAsFloat(1);
			}

			else if ( msgAddress == "/live/track" )
			{
				int trackIndex = message.getArgAsInt32(0) - 1; // doesn't take into account shift
				for(int k=0; k < mTracks.size(); k++)
					if ( mTracks[k]->mIndex == trackIndex )
					{
						mSelectedTrack = mTracks[k];
						break;
					}
			}
		}
		
		boost::this_thread::sleep(boost::posix_time::milliseconds(15));
		
	}
	Logger::log("Live > receiveData() thread exited!");
}

