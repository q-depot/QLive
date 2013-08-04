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

#include <boost/algorithm/string.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;


void QLiveGUI::init( ci::Rectf rect )   // rect doesn't make any fucking sense! TODO: remove this shit!
{
    std::vector<QLiveTrackRef> tracks = mLive->getTracks();
    
    if ( tracks.empty() )
        return;
    
    int margin  = 15;
    
    int n = 0;
    for( auto k=0; k < tracks.size(); k++ )
        if ( !boost::starts_with( tracks[k]->getName(), "_") )          // IGNORE all the params in the tracks that starts with "_"
            n++;
    
    Vec2f size;
    size.x = ( rect.getWidth() - ( n + 1 ) * margin ) / n;
    size.y = 150;
    for( auto k=0; k < tracks.size(); k++ )
    {
        if ( boost::starts_with( tracks[k]->getName(), "_") )          // IGNORE all the params in the tracks that starts with "_"
            continue;
        
        mControls.push_back( QLiveGuiTrackControl::create( mLive, tracks[k], size, mCanvas ) );
    }
}


void QLiveGUI::toggle()
{
    for( auto k=0; k < mControls.size(); k++ )
        mControls[k]->SetHidden( mControls[k]->Visible() );
}


void QLiveGUI::toggleParams()
{
    for( auto k=0; k < mControls.size(); k++ )
        mControls[k]->toggleParams();
}

