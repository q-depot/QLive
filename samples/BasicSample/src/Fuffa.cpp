/*
 *  Fuffa.cpp
 *  Fuffa
 *
 *  Created by Q on 30/08/2011.
 *  Copyright 2011 . All rights reserved.
 *
 */


#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include "QLive.h"
#include "Fuffa.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class FuffaPoint {
	
	friend class Fuffa;
	friend class FuffaBlob;
	
public:
	FuffaPoint( ci::Vec2f pos, int freq ) : mPos(pos), mVel(ci::Vec2f::zero()), mFreq(freq)
	{
		mDir	= mPos.safeNormalized();
	};
	
protected:
	ci::Vec2f	mPos;
	ci::Vec2f	mDir;
	ci::Vec2f	mVel;
	int			mFreq;
};


class FuffaBlob {
	
	friend class Fuffa;
	
public:
	
	FuffaBlob(Vec2f pos, int nPoints, int fftOffset, int fftSpacing) : mPos(pos)
	{
		for (int k=0; k < nPoints+1; k++) 
		{
			Vec2f pos = Vec2f( cos(k*6.28f/nPoints), sin(k*6.28f/nPoints) );
			//		int freq = math<float>::clamp( fftOffset + k + (fftSpacing * k), 0, FFT_SIZE - 1 );
			int freq = math<float>::clamp( fftOffset + (fftSpacing * k), 0, FFT_SIZE - 1 );
			mPoints.push_back( FuffaPoint(pos, freq) );
		}
	}
	
	void update() 
	{
		int freq;
		for (int k=0; k < mPoints.size(); k++) 
		{
			freq = math<int>::clamp( mPoints[k].mFreq + gLiveFuffaFreqOffset, 0, 256 );
			mPoints[k].mPos = mPoints[k].mDir * mAnalyzer->getFreqAmplitude(freq, 0) * gLiveFuffaForce * Vec2f(gLiveFuffaWidth, gLiveFuffaHeight);
		}
	};
	
	Vec2f	getPoint2f(int n) { return mPoints[n].mPos + mPos; };
	
	Vec3f	getPoint3f(int n, float height) { return Vec3f(mPos.x + mPoints[n].mPos.x, height, mPos.y + mPoints[n].mPos.y); };
	
protected:
	vector<FuffaPoint>	mPoints;
	Vec2f				mPos;
	Analyzer			*mAnalyzer;
	
};



Fuffa::Fuffa(Analyzer *analyzer, Scene *scene, Live *live, LiveClip *clip) : Modulo(live, clip), mScene(scene)
{
	mName = "Fuffa";
	
	// LiveClip format => NAME pointsN offset spacing posX posY 
	vector<string>	splitValues;
	string			clipName = clip->getName();
	boost::split( splitValues, clipName, boost::is_any_of(" ") );
	int nParameters = 5;
	
	if ( (splitValues.size() - 1) % nParameters != 0 )
	{
		Logger::log("ERROR > Fuffa, wrong number of parameters! format => NAME pointsN offset spacing posX posY");
		Logger::log(clipName);
		exit(-1);
	}
	int objN = (splitValues.size() - 1) / nParameters;
	
	for(int k=0; k < objN; k++)
	{
		Vec2f pos( boost::lexical_cast<float>(splitValues[k * nParameters + 4]), boost::lexical_cast<float>(splitValues[k * nParameters + 5]) );
		
		int nPoints = boost::lexical_cast<float>(splitValues[k * nParameters + 1]);
		int fftOffset	= boost::lexical_cast<float>(splitValues[k * nParameters + 2]);
		int fftSpacing = boost::lexical_cast<float>(splitValues[k * nParameters + 3]);
		
		mBlobs.push_back( FuffaBlob(pos, nPoints, fftOffset, fftSpacing, analyzer) );
		
		mSettingsStr += toString(pos) + " . n " + toString(nPoints) + " . o " + toString(fftOffset) + " . s " + toString(fftSpacing) + "\t";
	}
}


void Fuffa::update(float *values)
{
	if( !updateModulo() )
		return;
	
	// ********************************* //
	// Module implementation starts here 
	// ********************************* //
	
	for (int k=0; k < mBlobs.size(); k++)
		mBlobs[k].update();
	
	// update fixtures
	for (int k=0; k < mScene->mFixtures.size(); k++) {
		Fixture *fixture = mScene->mFixtures[k];
		float val = ( containsPoint( fixture->getPos2f() ) ) ? 1.0f : 0.0f; 
		values[k] += val * mBrightness;
	}
}


void Fuffa::render(float height) 
{
	if ( !mIsPlaying )
		return;
	
	// ********************************* //
	// Module implementation starts here 
	// ********************************* //
	
	
	gl::pushMatrices();
	
	gl::color( mClip->getColor() );
	
	for (int k=0; k < mBlobs.size(); k++)	
	{
		glBegin(GL_LINE_LOOP);
		for(int i=0; i < mBlobs[k].mPoints.size(); i++)
			gl::vertex( mBlobs[k].getPoint3f(i, height) );
		glEnd();
	}
	gl::popMatrices();
	
	gl::color( gColorWhite );
}


bool Fuffa::containsPoint(Vec2f pos2f)
{	
	int i, j, c = 0;
	Vec2f posJ, posI;

	for (int k=0; k < mBlobs.size(); k++)		
	{
		for (i = 0, j = mBlobs[k].mPoints.size()-1; i < mBlobs[k].mPoints.size(); j = i++) {
			posJ = mBlobs[k].getPoint2f(j);
			posI = mBlobs[k].getPoint2f(i);
			
			if ( ( (posI.y > pos2f.y) != (posJ.y > pos2f.y)) &&
				(pos2f.x < (posJ.x - posI.x) * (pos2f.y - posI.y) / (posJ.y-posI.y) + posI.x) )
				c = !c;
		}
		if (c)
			return true;
	}
	
	return c;
}