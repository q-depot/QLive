/*
 *  QLiveAnalyzer.cpp
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIO.h"
#include "cinder/Thread.h"
#include "cinder/Utilities.h"
#include <boost/algorithm/string.hpp>

#include "QLiveAnalyzer.h"
#include "QLiveObject.h"

using namespace ci;
using namespace ci::app;
using namespace std;


namespace nocte {

QLiveAnalyzer::QLiveAnalyzer() : mDevice(NULL), mOscListener(NULL)
{
    mFft = new float*[2];
    
	for (int k = 0; k < 2; k++)
        mFft[k] = new float[FFT_SIZE];
    
	for (int k = 0; k < FFT_SIZE; k++)
	{
		mFft[AUDIO_LEFT_CHANNEL][k] = 0.0f;
		mFft[AUDIO_RIGHT_CHANNEL][k] = 0.0f;
	}
	
	mAmplitude[AUDIO_LEFT_CHANNEL]   = 0.0f;
	mAmplitude[AUDIO_RIGHT_CHANNEL]   = 0.0f;
	
	mIsOnSet        = false;
	mIsOnBeat       = false;
	
	mOnSetAt        = getElapsedSeconds();
	mOnBeatAt       = getElapsedSeconds();
	
    mAmplitudeGain  = 1.0f;
    
    mFftGain        = 1.0f;
    mFftOffset      = 0.0f;
    mFftDumping     = 0.95f;
    
	mFontSmall      = Font("Helvetica", 12);
    
    mLeftChannelCol     = ColorA( 0.92f, 0.18f, 0.28f, 0.7f );
    mRightChannelCol    = ColorA( 0.20f, 0.22f, 0.27f, 0.7f );
}
    
    
QLiveAnalyzer::QLiveAnalyzer( int port, QLiveDeviceRef device ) : mOscListener(NULL)
{
    QLiveAnalyzer();
    
	init( port, device );
}

    
QLiveAnalyzer::~QLiveAnalyzer()
{
    for (int k = 0; k < 2; k++)
        delete[] mFft[k];
    
    if ( mOscListener )
    {
        mOscListener->shutdown();
        delete mOscListener;
        mOscListener = NULL;
        ci::sleep(50);
    }
    
    //	Logger::log("Analyzer > shutdown!");
}

    
void QLiveAnalyzer::init( int port, QLiveDeviceRef device )
{
    mOscInPort  = port;
    mDevice     = device;
	if ( mOscListener )
	{
		mOscListener->shutdown();
		delete mOscListener;
		mOscListener = NULL;
		ci::sleep(50);
	}
	
	try {
		mOscListener = new osc::Listener();
		mOscListener->setup(mOscInPort);
//		Logger::log("ANALYZER: Initialized OSC listener " + toString(mOscInPort) );
	}
	catch (...) {
		mOscListener = NULL;
//		Logger::log("ANALYZER: Failed to bind OSC listener socket " + toString(mOscInPort) );
	}
	
	thread receiveDataThread( &QLiveAnalyzer::receiveData, this);
	thread updateThread( &QLiveAnalyzer::update, this);
}


void QLiveAnalyzer::parseRawFft(osc::Message message, int channel)
{
    float val;
	for (int k = 0; k < message.getNumArgs(); k++)
	{
		if ( k < FFT_SIZE )
		{
			val = math<float>::clamp( (message.getArgAsFloat(k) * log(k + 2) * mFftGain / 100.0f) - mFftOffset, 0.0f, 1.0f );
            
			if ( val > mFft[channel][k] )
				mFft[channel][k] = val;
		} else
			return;
	}
}


void QLiveAnalyzer::receiveData()
{
	while( mOscListener )
	{
		while (mOscListener->hasWaitingMessages()) {
			osc::Message message;
			mOscListener->getNextMessage(&message);
            
			string  address = message.getAddress();
			float   val;
            
			// Audio Analysis
			
			// Amplitude left channel
			if ( boost::find_first( address, "/amplitude/1" ) )
            {
                val = math<float>::clamp( message.getArgAsFloat(0) * mAmplitudeGain, 0.0f, 1.0f );   // <<< WHY DO I CLAMP THE VALUE???
                if ( val > mAmplitude[AUDIO_LEFT_CHANNEL] )
                    mAmplitude[AUDIO_LEFT_CHANNEL] = val;
			}
            
			// Amplitude right channel	
            else if ( boost::find_first( address, "/amplitude/2" ) )
            {
                val = math<float>::clamp( message.getArgAsFloat(0) * mAmplitudeGain, 0.0f, 1.0f );   // <<< WHY DO I CLAMP THE VALUE???
                if ( val > mAmplitude[AUDIO_RIGHT_CHANNEL] )
                    mAmplitude[AUDIO_RIGHT_CHANNEL] = val;
            }
			
			// Raw Fft left channel
			else if ( boost::find_first( address, "/fft/1" ) )
				parseRawFft(message, AUDIO_LEFT_CHANNEL);
            
			// Raw Fft right channel
            else if ( boost::find_first( address, "/fft/2" ) )
                parseRawFft(message, AUDIO_RIGHT_CHANNEL);
			
            // Amplitude gain
            if ( boost::find_first( address, "/amplitude/gain" ) )
                mAmplitudeGain = message.getArgAsFloat(0);
            
            // Fft gain
            if ( boost::find_first( address, "/fft/gain" ) )
                mFftGain = message.getArgAsFloat(0);
            
            // Fft dumping
            if ( boost::find_first( address, "/fft/dumping" ) )
                mFftDumping = message.getArgAsFloat(0);
            
            // Fft offset
            if ( boost::find_first( address, "/fft/offset" ) )
                mFftOffset = message.getArgAsFloat(0);
            
                
			// On set detector
			else if ( boost::find_first( address, "/bonk" ) )
			{
				mIsOnSet = true;
				mOnSetAt = getElapsedSeconds();
			}
			
			// On beat
			else if ( boost::find_first( address, "/ezbeat" ) )
			{
				mIsOnBeat = true;
				mOnBeatAt = getElapsedSeconds();
			}
            
			// debug
			if (false) { 
				console() << "ANALYZER Address: " << message.getAddress() << " " << endl;
				for (int i = 0; i < message.getNumArgs(); i++) {
                    
					if (message.getArgType(i) == osc::TYPE_INT32){
						try {
							console() <<  message.getArgAsInt32(i) << " ";
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
            
		}
	
        sleep(15.0f);
//		boost::this_thread::sleep(boost::posix_time::milliseconds(15));
	}
	
//	Logger::log("Analyzer > receiveData() exited!");
}

void QLiveAnalyzer::update() 
{
    while( true )
    {
        mAmplitudeGain  = mDevice->getParamValue("A Gain");
        mFftGain        = mDevice->getParamValue("Gain");
    	mFftOffset      = mDevice->getParamValue("Offset");
    	mFftDumping     = mDevice->getParamValue("Dumping");
        
        // dump Fft values nicely
        for(int k=0; k < FFT_SIZE; k++) {
            //		mFft[0][k] = max( mFft[0][k] - mFftDumping, 0.0f );
            //		mFft[1][k] = max( mFft[1][k] - mFftDumping, 0.0f );
            mFft[AUDIO_LEFT_CHANNEL][k] *= mFftDumping;
            mFft[AUDIO_RIGHT_CHANNEL][k] *= mFftDumping;
        }
        
        if ( mIsOnSet && getElapsedSeconds() - mOnSetAt > HOLD_ON_SET_FOR )
            mIsOnSet = false;
        
        if ( mIsOnBeat && getElapsedSeconds() - mOnBeatAt > HOLD_ON_BEAT_FOR )
            mIsOnBeat = false;

        // dump amplitude
        mAmplitude[AUDIO_LEFT_CHANNEL] *= mFftDumping;
        mAmplitude[AUDIO_RIGHT_CHANNEL] *= mFftDumping;
        
        sleep(15.0f);
    }
}


void QLiveAnalyzer::render()
{
	int amplitudeHeight = 100;
	
	gl::color(Color(1, 1, 1));
	gl::pushMatrices();
	gl::translate( Vec2f( getWindowWidth() - 35  - FFT_SIZE, getWindowHeight() - 15 ) );
    
	// signal amplitude
	gl::color( mLeftChannelCol );
	gl::drawSolidRect( Rectf( 0, 0, 5, - mAmplitude[AUDIO_LEFT_CHANNEL] * amplitudeHeight ) );
	gl::color( mRightChannelCol );
	gl::drawSolidRect( Rectf( 8, 0, 13, - mAmplitude[AUDIO_RIGHT_CHANNEL] * amplitudeHeight ) );
	
	gl::translate( 20, 0 );
    
	PolyLine<Vec2f>	leftChannelLine;
	PolyLine<Vec2f>	rightChannelLine;
	
    
	for( int i = 0; i < FFT_SIZE; i++ ) {
		float y = ( -mFft[AUDIO_LEFT_CHANNEL][i] );
		leftChannelLine.push_back( Vec2f( i , y * amplitudeHeight ) );
		y = ( -mFft[AUDIO_RIGHT_CHANNEL][i] );
		rightChannelLine.push_back( Vec2f( i , y * amplitudeHeight ) );
	}
	
	gl::color( mLeftChannelCol );
	gl::draw( leftChannelLine );
	gl::color( mRightChannelCol );
	gl::draw( rightChannelLine );
    
	// draw top line
	gl::color( Color::white() * 0.5 );
	glBegin(GL_LINES);
	gl::vertex( Vec2f(0, - amplitudeHeight) );
	gl::vertex( Vec2f(FFT_SIZE, - amplitudeHeight) );
	glEnd();
	
	
	// Draw on set
	if ( mIsOnSet ) gl::color( Color(1.0f, 0.0f, 0.0f) ); else gl::color( Color::white() * 0.7 );
//	gl::draw( mOnSetTex, Vec2f(FFT_SIZE - 67, -amplitudeHeight + 10) );
    
	// Draw on beat
	if ( mIsOnBeat ) gl::color( Color(1.0f, 0.0f, 0.0f) ); else gl::color( Color::white() * 0.7 );
//	gl::draw( mOnBeatTex, Vec2f(FFT_SIZE - 67, -amplitudeHeight + 25) );
    
	// draw db range and gain labels
	char amplitudeGain[8]; sprintf (amplitudeGain, "%.3f", mAmplitudeGain);
	char fftGain[8]; sprintf (fftGain, "%.3f", mFftGain);
	char fftOffset[8]; sprintf (fftOffset, "%.3f", mFftOffset);
	
	gl::drawString(	"Amplitude gain: "	+ (string)amplitudeGain,	Vec2f(110, -amplitudeHeight - 12),                Color::white(), mFontSmall );
	gl::drawString(	"Fft gain: "        + (string)fftGain,          Vec2f(0, -amplitudeHeight - 12),                Color::white(), mFontSmall );
	gl::drawString(	"Offset: "          + (string)fftOffset,        Vec2f(FFT_SIZE - 69, -amplitudeHeight - 12),    Color::white(), mFontSmall );
	
    gl::color( Color::white() );
	
	gl::popMatrices();
    
}

}