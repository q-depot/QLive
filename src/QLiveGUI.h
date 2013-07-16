/*
 *  QLiveGUI.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 *  Requires Gwen : https://github.com/garrynewman/GWEN
 *
 */


#ifndef QLIVE_GUI
#define QLIVE_GUI

#pragma once

#include "cinder/app/AppBasic.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "QLive.h"

#include "cigwen/GwenRendererGl.h"
#include "cigwen/GwenInput.h"
#include "cigwen/CinderGwen.h"


namespace nocte {
    
    class QLiveGUI;
    typedef std::shared_ptr<QLiveGUI> QLiveGUIRef;
    
    class QLiveGUI {
        
    public:
        
        static QLiveGUIRef create( QLiveRef live )
        {
            return QLiveGUIRef( new QLiveGUI( live ) );
        }

        ~QLiveGUI()
        {
//            delete mRenderer;
//            delete mCanvas;
        }
        
        void update() {}
        
        void render()
        {
            mCanvas->RenderCanvas();
        }
        
        void init();
        
        void toggleVisible() {}
        
        
    private:
        
        QLiveGUI( QLiveRef live ) : mLive(live)
        {
            init();
        }

        
    private:
        
        QLiveRef                    mLive;
        
        cigwen::GwenRendererGl      *mRenderer;
        cigwen::GwenInputRef        mGwenInput;
        Gwen::Controls::Canvas      *mCanvas;

    };
    
}

#endif