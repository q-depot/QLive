/*
 *  QLiveGUI.cpp
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 *  Requires Gwen : https://github.com/garrynewman/GWEN
 *
 */

#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"

#include "QLiveGui.h"
#include "QLiveGuiTrackControl.h"

//#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace nocte;


void QLiveGUI::init()
{
    // TODO: find a better way..
//	fs::path rootPath = getAppPath().parent_path().parent_path().parent_path().parent_path().parent_path().parent_path();
//	addAssetDirectory( rootPath / "assets" );
    
	mRenderer = new cigwen::GwenRendererGl();
	mRenderer->Init();
    
	Gwen::Skin::TexturedBase* skin = new Gwen::Skin::TexturedBase( mRenderer );
	skin->Init( "DefaultSkinQLiveGui.png" );
//    skin->m_colBGDark = Gwen::Color( 15, 15, 15 );
    
	mCanvas = new Gwen::Controls::Canvas( skin );
//	mCanvas->SetSize( getWindowWidth(), getWindowHeight() );
    mCanvas->SetSize( getWindowWidth(), 0 );
	mGwenInput = cigwen::GwenInput::create( mCanvas );
    
    std::vector<QLiveTrackRef> tracks = mLive->getTracks();
    
    if ( tracks.empty() )
        return;
    
    int margin  = 15;
    
    int n = 0;
    for( auto k=0; k < tracks.size(); k++ )
        if ( !boost::starts_with( tracks[k]->getName(), "_") )          // IGNORE all the params in the tracks that starts with "_"
            n++;
    
    Vec2f size;
    size.x = ( getWindowWidth() - ( n + 1 ) * margin ) / n;
    size.y = 150;
    for( auto k=0; k < tracks.size(); k++ )
    {
        if ( boost::starts_with( tracks[k]->getName(), "_") )          // IGNORE all the params in the tracks that starts with "_"
            continue;
        
        mControls.push_back( QLiveGuiTrackControl::create( mLive, tracks[k], size, mCanvas ) );
        
        if ( mControls.back()->GetSize().y > mCanvas->GetSize().y )
            mCanvas->SetHeight( mControls.back()->GetSize().y );    
    }
    
}


void QLiveGUI::toggleParams()
{
    for( auto k=0; k < mControls.size(); k++ )
        mControls[k]->toggleParams();
}

