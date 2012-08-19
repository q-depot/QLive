/*
 *  Fuffa.h
 *  TheCompany
 *
 *  Created by Q on 09/09/2011.
 *  Copyright 2011 . All rights reserved.
 *
 */



class Modulo;
class Scene;
class FuffaBlob;



class Fuffa : public nocte::QLiveModule {
	
public:
	
	Fuffa(QLive *live);
	
	void update(float *values);
	
	void render();
	
private:
	
	bool containsPoint(ci::Vec2f point2f);
	
	ci::Vec2f				mPos;
	std::vector<FuffaBlob>	mBlobs;
	Scene					*mScene;
};
