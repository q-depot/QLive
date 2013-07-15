/*
 *  QLiveAnalyzer.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#ifndef QLIVE_ANALYSER
#define QLIVE_ANALYSER

#pragma once

#include "OscListener.h"

namespace nocte {
        
    #define FFT_SIZE                512
    #define HOLD_ON_SET_FOR         0.035f		// hold the value for N seconds
    #define HOLD_ON_BEAT_FOR        0.035f		// hold the value for N seconds
    #define AUDIO_LEFT_CHANNEL      0
    #define AUDIO_RIGHT_CHANNEL     1
        
    class QLive;
    class QLiveDevice;
    
    class QLiveAnalyzer {
        
        friend class QLive;
        
    public:
        
        QLiveAnalyzer();
        
        QLiveAnalyzer( int port, QLiveDevice *device );
        
        ~QLiveAnalyzer();
        
        void	render();
        
        float	getFreqAmplitude(int freq, int channel) { return mFft[channel][freq]; };
        
        bool	isOnBeat() { return mIsOnBeat; };
        bool	isOnSet() { return mIsOnSet; };
        
        bool	isConnected() { return mOscListener != NULL; };
        
        void	init( int port, QLiveDevice *device );
        
    protected:
        
        void	receiveData();
        
        void	update();
        
        void	parseRawFft(ci::osc::Message message, int channel);
      
    protected:
        
        QLiveDevice         *mDevice;
        ci::osc::Listener	*mOscListener;
        
        float				mAmplitude[2];
        float**             mFft;
        float				mFreqPeaks[2][FFT_SIZE];
        
        bool				mIsOnSet;
        bool				mIsOnBeat;
        double				mOnSetAt;
        double				mOnBeatAt;
        
        float               mAmplitudeGain;
        float               mFftGain;
        float               mFftOffset;
        float               mFftDumping;
        
        int					mOscInPort;
        
        ci::Font            mFontSmall;
        
        ci::ColorA          mLeftChannelCol;
        ci::ColorA          mRightChannelCol;
    };


}

#endif