/*
 *  QLiveGUI.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 *  Requires ciUI : https://github.com/rezaali/ciUI
 *
 */

#pragma once

#include "cinder/app/AppBasic.h"

#include "QLive.h"
#include "ciUI.h"


namespace nocte {
    
    class QLiveGUI {
        
    public:
        
        QLiveGUI( QLive *live ) : mLive(live) 
        {
            initGUI();
        }
        
        void render() 
        {
            mGUI->draw();
        }
        
        void initGUI() 
        {
            Vec2f mModuleGUISize(120, 300);
         
            QLiveTrack      *track;
            QLiveDevice     *device;
            QLiveParam      *param;
            
            int                 tracksN = mLive->getTracksN();
            float               w       = mModuleGUISize.x - CI_UI_GLOBAL_WIDGET_SPACING * 2;
            float               h       = 8.0f;
            Vec2i               pos;
            ciUIWidget          *widget;
            char                roundedFloat[10];
            
            mGUI     =  new ciUICanvas( 0, getWindowHeight() - mModuleGUISize.y, 
                                        CI_UI_GLOBAL_WIDGET_SPACING + mModuleGUISize.x * tracksN, mModuleGUISize.y );
            
            mGUI->setFont(RES_GUI_FONT);
            mGUI->setFontSize( CI_UI_FONT_LARGE, 14 );
            mGUI->setFontSize( CI_UI_FONT_MEDIUM, 12 );
            mGUI->setFontSize( CI_UI_FONT_SMALL, 10 );
            
            for( int k=0; k < tracksN; k++ )
            {
                track   = mLive->getTrack(k);
                
                pos.x   = CI_UI_GLOBAL_WIDGET_SPACING + mModuleGUISize.x * k;
                pos.y   = CI_UI_GLOBAL_WIDGET_SPACING;
                
                // Label
                widget = new ciUILabel( pos.x, pos.y, track->getName(), CI_UI_FONT_MEDIUM );
                mGUI->addWidget( widget );
                pos.y += widget->getRect()->getHeight() + 3;

                // Volume                
                widget = new ciUISlider( pos.x, pos.y, w, h, 0, 10.0, 1.0f, "Master" );
                mGUI->addWidget( widget );
                pos.y += 24;
                
                mGUI->addWidget( new ciUISpacer( pos.x, pos.y, w, 1 ) );
                pos.y += 6;
                
                // Clips
                vector<string> clipNames;
                for( int i=0; i < track->getClipsN(); i++ )
                    clipNames.push_back( track->getClip(i)->getName() );

                widget = new ciUIRadio( pos.x, pos.y, 9, 9, "CLIPS_" + ci::toString(k), clipNames, CI_UI_ORIENTATION_VERTICAL );
                mGUI->addWidget( widget );
                pos.y += widget->getRect()->getHeight() - 3;
                
                mGUI->addWidget( new ciUISpacer( pos.x, pos.y, w, 1 ) );
                pos.y += 7;
                
                // Params
                for( int i=0; i < track->getDevicesN(); i++ )
                {
                    device = track->getDevice(i);
                    
                    for( int j=0; j < device->getParamsN(); j++ )
                    {
                        param = device->getParam(j);
                        sprintf (roundedFloat, "%.2f", param->getValue() );
                        widget = new ciUILabel( pos.x, pos.y, param->getName() + " " + roundedFloat, CI_UI_FONT_SMALL );
                        mGUI->addWidget( widget );
                        pos.y += widget->getRect()->getHeight() + 3;
                    }
                }                
            }
            
        }
        
    private:
        
        QLive       *mLive;
        ciUICanvas  *mGUI;
    };
    
}