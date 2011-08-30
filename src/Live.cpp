/*
 *  Live.cpp
 *  QUBO
 *
 *  Created by Q on 08/08/2011.
 *  Copyright 2011 . All rights reserved.
 *
 */

#include "Live.h"
#include "cinder/Thread.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "cinder/Text.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

#include "Modulo.h"

using namespace ci;
using namespace ci::app;
using namespace std;


Live::Live() 
{	
	mOscSender.setup(OSC_HOST, OSC_OUT_PORT);
	mOscListener.setup(OSC_IN_PORT);
	
	thread receiveDataThread( &Live::receiveData, this);
	
	mIsPlaying = false;
	
	getInfo();
	
	mFontSmall = Font("Helvetica", 12);
}

void Live::getInfo() 
{ 
	for(int k=0; k < mTracks.size(); k++) {
		mTracks[k]->clearPointers();
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
	
	sendMessage("/live/name/scene");
	sendMessage("/live/name/track"); 
	sendMessage("/live/name/clip"); 
}


void Live::drawDebug() 
{
	TextLayout textLayout = TextLayout();
	textLayout.clear( ColorA(0.0f, 0.0f, 0.0f, 1.0f) );
	textLayout.setBorder(15,15);
	
	textLayout.setFont( mFontSmall );

	textLayout.setColor( Color( 1.0f, 1.0f, 1.0f ) );
	textLayout.addLine( "SCENES" );
	textLayout.addLine( "index\tname" );
	for (int k = 0; k < mScenes.size(); k++)
		textLayout.addLine( toString(mScenes[k]->mIndex) + "\t\t" + mScenes[k]->mName );
	textLayout.addLine( " " );
	
	
	textLayout.addLine( "TRACKS" );
	textLayout.addLine( "index\tname" );
	for (int k = 0; k < mTracks.size(); k++)
		textLayout.addLine( toString(mTracks[k]->mIndex) + "\t\t" + mTracks[k]->mName );
	textLayout.addLine( " " );
	
	
	textLayout.addLine( "CLIPS" );
	textLayout.addLine( "index\tname\ttrack" );
	for (int k = 0; k < mClips.size(); k++)
		textLayout.addLine( toString(mClips[k]->mIndex) + "\t\t" + mClips[k]->mName + "\t\t" + toString(mClips[k]->mTrackIndex) );
	textLayout.addLine( " " );
	
	textLayout.addLine( "DEVICES" );
	textLayout.addLine( "index\tname\t\ttrack" );
	for (int k = 0; k < mDevices.size(); k++)
		textLayout.addLine( toString(mDevices[k]->mIndex) + "\t\t" + mDevices[k]->mName + "\t\t" + toString(mDevices[k]->mTrackIndex) );
	textLayout.addLine( " " );
	
	gl::Texture tex = gl::Texture(textLayout.render(true));
	gl::draw( tex, Vec2f::zero() );
}


void Live::sendMessage(string address, string args)
{	
	//console() << "SEND MSG: " << address << " >> " << args << endl;
	osc::Message message;
	message.setAddress( address );
	message.setRemoteEndpoint(OSC_HOST, OSC_OUT_PORT);
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
	
	mOscSender.sendMessage(message);
	
}


void Live::debug() 
{
	
	console() << "=== SCENES ===" << endl;
	console() << "index\tname" << endl;
	for (int k = 0; k < mScenes.size(); k++)
		console() << mScenes[k]->mIndex << "\t\t" << mScenes[k]->mName << endl;
	console() << "==============" << endl << endl;	
	
	console() << "=== TRACKS ===" << endl;
	console() << "index\tname" << endl;
	for (int k = 0; k < mTracks.size(); k++)
		console() << mTracks[k]->mIndex << "\t\t" << mTracks[k]->mName << endl;
	console() << "==============" << endl << endl;
	
	console() << "=== CLIPS ===" << endl;
	console() << "index\tname\ttrack" << endl;
	for (int k = 0; k < mClips.size(); k++)
		console() << mClips[k]->mIndex << "\t\t" << mClips[k]->mName << "\t\t" << mClips[k]->mTrackIndex << endl;
	console() << "==============" << endl << endl;
	
	console() << "=== DEVICES ===" << endl;
	console() << "index\tname\ttrack" << endl;
	for (int k = 0; k < mDevices.size(); k++)
		console() << mDevices[k]->mIndex << "\t\t" << mDevices[k]->mName << "\t\t" << mDevices[k]->mTrackIndex << endl;
	console() << "==============" << endl << endl;

}

void Live::parseScene( osc::Message message ) {
	
	int index	= message.getArgAsInt32(0);
	string name = message.getArgAsString(1);
	
	mScenes.push_back( new LiveScene( index, name ) );
	
	console() << "parse scene: " << endl;
}


void Live::parseTrack( osc::Message message ) {
	
	int index	= message.getArgAsInt32(0);
	string name = message.getArgAsString(1);
	
	mTracks.push_back( new LiveTrack( index, name ) );
	
	listTrackDevices(index);
	
	console() << "parse track: " << endl;
}


void Live::parseClip( osc::Message message ) {
	
	int trackIndex	= message.getArgAsInt32(0);
	int index		= message.getArgAsInt32(1);
	string name		= message.getArgAsString(2);
	
	LiveClip *clip = new LiveClip( index, name, trackIndex );
	
	mClips.push_back( clip );
	mTracks[trackIndex]->addClip( clip );
	
	sendMessage("/live/clip/info", "i" + toString(trackIndex) + " i" + toString(index) );	// get clip info
	
	console() << "parse clip: " << endl;
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
	
	console() << "parse clip info: " << trackIndex << " " << index << " " << state << endl;
}


void Live::parseDevice( osc::Message message ) {
   
	if ( message.getNumArgs() < 3 )	// seems there is an error in the APIs!
	   return;
	
	int			trackIndex = message.getArgAsInt32(0);
	int			index;
	string		name;
	LiveDevice	*dev;
	
	for (int k=1; k < message.getNumArgs(); k+=2) {
		index		= message.getArgAsInt32(k);
		name		= message.getArgAsString(k+1);
		
		dev = new LiveDevice( index, name, trackIndex );	
		mDevices.push_back( dev );
		mTracks[trackIndex]->addDevice( dev );
		
		sendMessage("/live/device", "i" + toString(trackIndex) + " i" + toString(index) );
	}
	
	console() << "parse device: " << endl;
}


void Live::parseDeviceParams( osc::Message message ) {
	return;
	/*
	for (int k=1; k < message.getNumArgs(); k+=2) {
		index		= message.getArgAsInt32(k);
		name		= message.getArgAsString(k+1);
		
		dev = new LiveDevice( index, name, trackIndex );	
		mDevices.push_back( dev );
		mTracks[trackIndex]->addDevice( dev );
		
		sendMessage("/live/device", "i" + toString(trackIndex) + " i" + toString(index) );
	}
	
	ADD DEVICE 3New message received
Address: /live/device/allparam
	Num Arg: 5
	-- Argument 0
	---- type: int32
	------ value: 0
	-- Argument 1
	---- type: int32
	------ value: 0
	-- Argument 2
	---- type: int32
	------ value: 0
	-- Argument 3
	---- type: float
	------ value: 1
	-- Argument 4
	---- type: string
	------ value: Device On
	 */
}


void Live::receiveData(){
	while( true ) {
		while (mOscListener.hasWaitingMessages()) {
			osc::Message message;
			mOscListener.getNextMessage(&message);
			
			string	msgAddress = message.getAddress();
/*

			console() << "New message received" << std::endl;
			console() << "Address: " << message.getAddress() << std::endl;
			console() << "Num Arg: " << message.getNumArgs() << std::endl;
			for (int i = 0; i < message.getNumArgs(); i++) {
				console() << "-- Argument " << i << std::endl;
				console() << "---- type: " << message.getArgTypeName(i) << std::endl;
				if (message.getArgType(i) == osc::TYPE_INT32){
					try {
						console() << "------ value: "<< message.getArgAsInt32(i) << std::endl;
					}
					catch (...) {
						console() << "Exception reading argument as int32" << std::endl;
					}
					
				}else if (message.getArgType(i) == osc::TYPE_FLOAT){
					try {
						console() << "------ value: " << message.getArgAsFloat(i) << std::endl;
					}
					catch (...) {
						console() << "Exception reading argument as float" << std::endl;
					}
				}else if (message.getArgType(i) == osc::TYPE_STRING){
					try {
						console() << "------ value: " << message.getArgAsString(i).c_str() << std::endl;
					}
					catch (...) {
						console() << "Exception reading argument as string" << std::endl;
					}
					
				}
			}
			console() << endl;
			if( message.getNumArgs() != 0 && message.getArgType( 0 ) == osc::TYPE_FLOAT )
			{
				//positionX = message.getArgAsFloat(0);
			}
		*/
	
			// Parse Live objects
			
			if ( msgAddress == "/live/name/scene" ) {
				parseScene(message);
				break;
			}
			else if ( msgAddress == "/live/name/track" ) {
				parseTrack(message);
				
				break;
			}
			else if ( msgAddress == "/live/name/clip" ) {
				parseClip(message);	
				break;
			}
			else if ( msgAddress == "/live/clip/info" ) {
				parseClipInfo(message);
				break;
			}
			
			else if ( msgAddress == "/live/devicelist" ) {
				parseDevice(message);	
				break;
			}
			else if ( msgAddress == "/live/device/allparam" ) {
				parseDeviceParams(message);
				break;
			}
			else if ( msgAddress == "/live/play" )
			{
				mIsPlaying = ( (ClipState)message.getArgAsInt32(0) == 2 ) ? true : false;
				break;
			}
			
			
		}
		
		
		
	}
}

