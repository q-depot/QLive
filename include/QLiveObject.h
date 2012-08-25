/*
 *  QLiveObject.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#ifndef QLIVE_OBJECT
#define QLIVE_OBJECT

namespace nocte {
    
    
enum ClipState {
	NO_CLIP,			// 0
	HAS_CLIP,			// 1
	CLIP_PLAYING,		// 2
	CLIP_TRIGGERED		// 3
};


class QLiveObject {
	
	friend class QLive;

public:
	
	QLiveObject(int index, std::string name) 
	{
		mIndex	= index;
		mName	= name;
	};
	
		
	int			getIndex() { return mIndex; };
	
	std::string getName() { return mName; };
	
protected:
    
	int			mIndex;
	std::string	mName;
	
};

/*
void TClassA::Wrapper_To_Call_Display(void* pt2Object, char* string)
{
	// explicitly cast to a pointer to TClassA
	TClassA* mySelf = (TClassA*) pt2Object;
	
	// call member
	mySelf->Display(string);
	*/

class QLiveClip : public QLiveObject {
	
	friend class QLive;
	
public:
    
	QLiveClip(int index, std::string name, int trackIndex, ci::Color color) : QLiveObject(index, name), mTrackIndex(trackIndex), mColor(color) 
	{ 
		mState		= HAS_CLIP;
        mIsPlaying  = false;
	};
	
	int getTrackIndex() { return mTrackIndex; };

	ClipState	getState() { return mState; };
	
	void		setState(ClipState state) 
    { 
        mState = state; 
        
        mIsPlaying = ( mState == CLIP_PLAYING )  ? true : false;
    };
	
    bool *getIsPlayingRef() { return &mIsPlaying; };
    
	bool		isPlaying() { return ( mState == CLIP_PLAYING ); };
	
	ci::ColorA	getColor() { return mColor; };
	
	
protected:
    
	int			mTrackIndex;
	ci::ColorA	mColor;
    
private:
    
    bool        mIsPlaying;
	ClipState	mState;
    
};


class QLiveParam : public QLiveObject {
    
    friend class QLive;
    friend class QLiveDevice;
    
public:
    
    QLiveParam( int index, std::string name, float value ) : QLiveObject(index, name), mValue(value) {}
    
        float getValue() { return mValue; };
    
protected:
    
    float mValue;
    
};


class QLiveDevice : public QLiveObject {
	
	friend class QLive;
	
public:
    
	QLiveDevice(int index, std::string name, int trackIndex) : QLiveObject(index, name) { mTrackIndex = trackIndex; };
	
	
	int getTrackIndex() { return mTrackIndex; };
	
    void addParam( int index, float value, std::string name )
    {
        mParams.push_back( new QLiveParam( index, name, value ) );
    }
    
    QLiveParam* getParamByName( std::string name )
    {
        QLiveParam *param = NULL;
        
        for( size_t k=0; k < mParams.size(); k++ )
            if( mParams[k]->mName == name )
                return mParams[k];

        return param;
    }
    
    QLiveParam* getParam( int idx ) { return mParams[idx]; }
    
    int getParamsN() { return mParams.size(); };
    
protected:

	int                         mTrackIndex;
    std::vector<QLiveParam*>    mParams;

};


class QLiveTrack : public QLiveObject {
	
	friend class QLive;
	
public:
 
	QLiveTrack( int index, std::string name, ci::Color color ) : QLiveObject(index, name), mColor(color), mVolume(0.0f) {}
	
	void addClip( QLiveClip *obj )		{ mClips.push_back(obj); }
	void addDevice( QLiveDevice *obj )	{ mDevices.push_back(obj); }
	
	int getClipsN() { return mClips.size(); }
	int getDevicesN() { return mDevices.size(); }
	
	QLiveClip *getClip(int n) { return mClips[n]; }
	
	void clearPointers() { mClips.clear(); mDevices.clear(); }
	
	float getVolume() { return mVolume; }

	float* getVolumeRef() { return &mVolume; }
    
	QLiveDevice* getDevice( int idx ) { return mDevices[idx]; }
    
	ci::ColorA	getColor() { return mColor; };
    
protected:
	
	std::vector<QLiveClip*>		mClips;
	std::vector<QLiveDevice*>	mDevices;
	float						mVolume;
    ci::ColorA                  mColor;
    
};


class QLiveScene : public QLiveObject {

	friend class QLive;
	
public:
    
	QLiveScene(int index, std::string name) : QLiveObject(index, name) {};

};


}

#endif