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
        
        ~QLiveObject() {}
        
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
        friend class QLiveTrack;
        
    public:
        
        QLiveClip(int index, std::string name, int trackIndex, ci::Color color) : QLiveObject(index, name), mTrackIndex(trackIndex), mColor(color) 
        { 
            mState		= HAS_CLIP;
            mIsPlaying  = false;
        };
        
        ~QLiveClip() {}
        
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
        
        QLiveParam( int index, std::string name, float value, float minVal, float maxVal ) : QLiveObject(index, name), mValue(value), mMinValue(minVal), mMaxValue(maxVal) {}
        
        ~QLiveParam() {}
        
        float getValue() { return mValue; }
        
        float* getRef() { return &mValue; }
        
        float getMin() { return mMinValue; }
        
        float getMax() { return mMaxValue; }
        
    protected:
        
        float       mValue;
        float       mMinValue;
        float       mMaxValue;
        
    };


    class QLiveDevice : public QLiveObject {
        
        friend class QLive;
        friend class QLiveTrack;
        
    public:
        
        QLiveDevice(int index, std::string name, int trackIndex) : QLiveObject(index, name) { mTrackIndex = trackIndex; }
        
        ~QLiveDevice()
        {
            for( size_t k=0; k < mParams.size(); k++ )
                delete mParams[k];
            mParams.clear();
        }
        
        int getTrackIndex() { return mTrackIndex; }
        
        void addParam( int index, float value, std::string name, float minVal, float maxVal ) { mParams.push_back( new QLiveParam( index, name, value, minVal, maxVal ) ); }
        
        QLiveParam* getParamByName( std::string name )
        {
            QLiveParam *param = NULL;
            
            for( size_t k=0; k < mParams.size(); k++ )
                if( mParams[k]->mName == name )
                    return mParams[k];

            return param;
        }
        
        std::vector<QLiveParam*> getParams() { return mParams; }
        
        QLiveParam* getParam( int idx ) 
        { 
            QLiveParam* param = NULL;
            
            if ( idx < mParams.size() )
                return mParams[idx]; 
            else
                return param;
        }
        
        int getParamsN() { return mParams.size(); }
        
        float getParamValue( std::string name )
        {
            for( size_t k=0; k < mParams.size(); k++ )
                if ( mParams[k]->getName() == name )
                    return mParams[k]->getValue();
                    
            ci::app::console() << "Param " << name << " doesn't exists" << std::endl;
            exit(-1);
        }
        
        
        float* getParamRef( std::string name )
        {
            for( size_t k=0; k < mParams.size(); k++ )
                if ( mParams[k]->getName() == name )
                    return mParams[k]->getRef();
            
            ci::app::console() << "Param " << name << " doesn't exists" << std::endl;
            exit(-1);
        }
        
    protected:

        int                         mTrackIndex;
        std::vector<QLiveParam*>    mParams;

    };


    class QLiveTrack : public QLiveObject {
        
        friend class QLive;
        
    public:
     
        QLiveTrack( int index, std::string name, ci::Color color ) : QLiveObject(index, name), mColor(color), mVolume(0.0f) {}
        
        ~QLiveTrack() 
        {
            for(int k=0; k < mClips.size(); k++)
                delete mClips[k];
            mClips.clear();
            
            for(int k=0; k < mDevices.size(); k++)
                delete mDevices[k];
            mDevices.clear();
        }
        
        void addClip( QLiveClip *obj )		{ mClips.push_back(obj); }
        void addDevice( QLiveDevice *obj )	{ mDevices.push_back(obj); }
            
        
        std::vector<QLiveClip*> getClips() { return mClips; }
        
        std::vector<QLiveDevice*> getDevices() { return mDevices; }
//        int getClipsN() { return mClips.size(); }
//        int getDevicesN() { return mDevices.size(); }
        
        QLiveClip *getClip( int clipIdx ) 
        { 
            QLiveClip *clip = NULL;
            
            for( size_t k=0; k < mClips.size(); k++ )
                if ( mClips[k]->mIndex == clipIdx )
                    return mClips[clipIdx];
            
            return clip;
        }
        
        void clearPointers() { mClips.clear(); mDevices.clear(); }
        
        float getVolume() { return mVolume; }

        float* getVolumeRef() { return &mVolume; }

        QLiveDevice* getDevice( int idx ) 
        { 
            QLiveDevice* device = NULL;
            
            if ( idx < mDevices.size() )
                return mDevices[idx]; 
            else
                return device;
        }
        
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

        ~QLiveScene() {}
        
    };


}

#endif