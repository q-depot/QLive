/*
 *  BasicModule.cpp
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


class QLiveModule;

class BasicModule : public nocte::QLiveModule {
	
public:
	
	BasicModule( nocte::QLive *live, nocte::QLiveClip *clip = NULL );
	
	void update( float *values );
	
	void render();
	
private:
    
    std::vector<ci::Vec2f>  mPoints;
	std::vector<int>        mFreqs;
    
};
