/*
 *  LiveObject.h
 *  QUBO
 *
 *  Created by Q on 08/08/2011.
 *  Copyright 2011 . All rights reserved.
 *
 */

enum ClipState {
	NO_CLIP,			// 0
	HAS_CLIP,			// 1
	CLIP_PLAYING,		// 2
	CLIP_TRIGGERED		// 3
};


class LiveObject {
	
	friend class Live;

public:
	
	LiveObject(int index, std::string name) 
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

class LiveClip : public LiveObject {
	
	friend class Live;
	
public:
	LiveClip(int index, std::string name, int trackIndex, ci::Color color) : LiveObject(index, name) 
	{ 
		mTrackIndex = trackIndex; 
		mState		= HAS_CLIP; 
		mColor		= color;
	};
	
	int getTrackIndex() { return mTrackIndex; };

	ClipState	getState() { return mState; };
	
	void		setState(ClipState state) { mState = state;	};
	
	bool		isPlaying() { return ( mState == CLIP_PLAYING ); };
	
	ci::ColorA	getColor() { return mColor; };
	
	
protected:
	int			mTrackIndex;
	ClipState	mState;
	ci::ColorA	mColor;
};


class LiveDevice : public LiveObject {
	
	friend class Live;
	
public:
	LiveDevice(int index, std::string name, int trackIndex) : LiveObject(index, name) { mTrackIndex = trackIndex; };
	
	
	int getTrackIndex() { return mTrackIndex; };
	
protected:
	int	mTrackIndex;
	
};


class LiveTrack : public LiveObject {
	
	friend class Live;
	
public:
	LiveTrack(int index, std::string name) : LiveObject(index, name), mVolume(0.0f) {};
	
	void addClip( LiveClip *obj )		{ mClips.push_back(obj); };
	void addDevice( LiveDevice *obj )	{ mDevices.push_back(obj); };
	
	int getClipsN() { return mClips.size(); };
	int getDevicesN() { return mDevices.size(); };
	
	LiveClip *getClip(int n) { return mClips[n]; };
	
	void clearPointers() { mClips.clear(); mDevices.clear(); };
	
	float getVolume() { return mVolume; };
	
	
protected:
	
	std::vector<LiveClip*>		mClips;
	std::vector<LiveDevice*>	mDevices;
	
	float						mVolume;
};


class LiveScene : public LiveObject {

	friend class Live;
	
public:
	LiveScene(int index, std::string name) : LiveObject(index, name) {};

};





