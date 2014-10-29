/****************************************************************************
*                                                                             
*   This program is free software: you can redistribute it and/or modify     
*    it under the terms of the GNU General Public License as published by     
*    the Free Software Foundation, either version 3 of the License, or        
*    (at your option) any later version.                                      
*                                                                             
*    This program is distributed in the hope that it will be useful,          
*    but WITHOUT ANY WARRANTY; without even the implied warranty of           
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            
*    GNU General Public License for more details.                             
*                                                                             
*    You should have received a copy of the GNU General Public License        
*    along with this program. If not, see <http://www.gnu.org/licenses/>.     
*                                                                             
******************************************************************************/

/****************************************************************************
*
* main.cpp
*
* Purpose: Simple example illustrating use of BGS library.
*
* Author: Donovan Parks, June 2008
*
* Note: You will need to install the HUFFY codex at: 
*					http://neuron2.net/www.math.berkeley.edu/benrg/huffyuv.html
*
******************************************************************************/

#include <iostream>
#include <string>

#pragma warning ( disable : 4800 ) 

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#include "Bgs.hpp"
#include "AdaptiveMedianBGS.hpp"
#include "GrimsonGMM.hpp"
#include "ZivkovicAGMM.hpp"
#include "MeanBGS.hpp"
#include "WrenGA.hpp"
#include "PratiMediodBGS.hpp"
#include "Eigenbackground.hpp"

enum RESULT_TYPE { IMAGE_DMS, IMAGE_SM_WALLFLOWER, VIDEO };

int main(int argc, const char* argv[])
{
    std::string inputVideo = "Videos/TestVideo/19Aug-Test-Crowd.avi";
    int algorithmIndex = 0;
    if (argc <= 1){
        std::cout << "Usage: ./bgs_test [path_to_video_file] [algorithm_number(between 0 and 6)]" << std::endl;
        std::cout << "Example: ./bgs_test Videos/video.avi 1" << std::endl;
        exit(1);
    }

    if (argc > 1){
        inputVideo = argv[1];
    }
    if (argc > 2){
        algorithmIndex = atoi(argv[2]);
    }

	// read data from AVI file
	CvCapture* readerAvi = cvCaptureFromAVI(inputVideo.c_str());
	if(readerAvi == NULL)
	{
		std::cerr << "Could not open AVI file." << std::endl;
		return 0;
	}

	// retrieve information about AVI file
	cvQueryFrame(readerAvi); 
	int width	= (int) cvGetCaptureProperty(readerAvi, CV_CAP_PROP_FRAME_WIDTH);
	int height = (int) cvGetCaptureProperty(readerAvi, CV_CAP_PROP_FRAME_HEIGHT);
	int fps = (int) cvGetCaptureProperty(readerAvi, CV_CAP_PROP_FPS);
	int num_frames = (unsigned int) cvGetCaptureProperty(readerAvi,  CV_CAP_PROP_FRAME_COUNT);

    std::cout << "After retrieving information about AVI file: width=" << width << ", height=" << height << ", fps=" << fps << ", num_frames=" << num_frames << std::endl;

	// setup marks to hold results of low and high thresholding
	BwImage low_threshold_mask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	low_threshold_mask.Ptr()->origin = IPL_ORIGIN_BL;

	BwImage high_threshold_mask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	high_threshold_mask.Ptr()->origin = IPL_ORIGIN_BL;

	// setup buffer to hold individual frames from video stream
	RgbImage frame_data;
	frame_data.ReleaseMemory(false);	// AVI frame data is released by with the AVI capture device

	// setup AVI writers (note: you will need to install the HUFFY codex at: 
	//   http://neuron2.net/www.math.berkeley.edu/benrg/huffyuv.html)
	//CvVideoWriter* writerAvi = cvCreateVideoWriter("output/results.avi", CV_FOURCC('H', 'F', 'Y', 'U'),	
	//														fps, cvSize(width, height), 1);
	//CvVideoWriter* writerAvi = cvCreateVideoWriter("results.avi", CV_FOURCC('D','I','V','X'),	
	//														fps, cvSize(width, height), 1);

	// setup background subtraction algorithm
	//*
    Algorithms::BackgroundSubtraction::Bgs *bgs = NULL;
    
    switch (algorithmIndex){
        case 0: 
        {
            std::cout << "[*]Using AdaptiveMedian method" << std::endl;
            Algorithms::BackgroundSubtraction::AdaptiveMedianParams params;
            params.SetFrameSize(width, height);
            params.LowThreshold() = 40;
            params.HighThreshold() = 2*params.LowThreshold();	// Note: high threshold is used by post-processing 
            params.SamplingRate() = 7;
            params.LearningFrames() = 30;

            //Algorithms::BackgroundSubtraction::AdaptiveMedianBGS bgs;
            bgs = new Algorithms::BackgroundSubtraction::AdaptiveMedianBGS();
            bgs->Initalize(params);
            break;
        };
        case 1: 
        {
            std::cout << "[*]Using Grimson method (GMM)" << std::endl;
            Algorithms::BackgroundSubtraction::GrimsonParams params;
            params.SetFrameSize(width, height);
            params.LowThreshold() = 3.0f*3.0f;
            params.HighThreshold() = 2*params.LowThreshold();	// Note: high threshold is used by post-processing 
            params.Alpha() = 0.001f;
            params.MaxModes() = 3;

            //Algorithms::BackgroundSubtraction::GrimsonGMM bgs;
            bgs = new Algorithms::BackgroundSubtraction::GrimsonGMM();
            bgs->Initalize(params);
            break;
        };
        case 2: 
        {
            std::cout << "[*]Using Zivkovic method (AGMM)" << std::endl;
            Algorithms::BackgroundSubtraction::ZivkovicParams params;
            params.SetFrameSize(width, height);
            params.LowThreshold() = 5.0f*5.0f;
            params.HighThreshold() = 2*params.LowThreshold();	// Note: high threshold is used by post-processing 
            params.Alpha() = 0.001f;
            params.MaxModes() = 3;

            //Algorithms::BackgroundSubtraction::ZivkovicAGMM bgs;
            bgs = new Algorithms::BackgroundSubtraction::ZivkovicAGMM();
            bgs->Initalize(params);
            break;
        };
        case 3: 
        {
            std::cout << "[*]Using Mean method" << std::endl;
            Algorithms::BackgroundSubtraction::MeanParams params;
            params.SetFrameSize(width, height);
            params.LowThreshold() = 3*30*30;
            params.HighThreshold() = 2*params.LowThreshold();	// Note: high threshold is used by post-processing 
            params.Alpha() = 1e-6f;
            params.LearningFrames() = 30;

            //Algorithms::BackgroundSubtraction::MeanBGS bgs;
            bgs = new Algorithms::BackgroundSubtraction::MeanBGS();
            bgs->Initalize(params);
            break;
        };
        case 4: 
        {
            std::cout << "[*]Using Wren method (GA)" << std::endl;
            Algorithms::BackgroundSubtraction::WrenParams params;
            params.SetFrameSize(width, height);
            params.LowThreshold() = 3.5f*3.5f;
            params.HighThreshold() = 2*params.LowThreshold();	// Note: high threshold is used by post-processing 
            params.Alpha() = 0.005f;
            params.LearningFrames() = 30;

            //Algorithms::BackgroundSubtraction::WrenGA bgs;
            bgs = new Algorithms::BackgroundSubtraction::WrenGA();
            bgs->Initalize(params);
            break;
        };
        case 5: 
        {
            std::cout << "[*]Using Prati method (mediod)" << std::endl;
            Algorithms::BackgroundSubtraction::PratiParams params;
            params.SetFrameSize(width, height);
            params.LowThreshold() = 30;
            params.HighThreshold() = 2*params.LowThreshold();	// Note: high threshold is used by post-processing 
            params.SamplingRate() = 5;
            params.HistorySize() = 16;
            params.Weight() = 5;

            //Algorithms::BackgroundSubtraction::PratiMediodBGS bgs;
            bgs = new Algorithms::BackgroundSubtraction::PratiMediodBGS();
            bgs->Initalize(params);
            break;
        };
        case 6: 
        {
            std::cout << "[*]Using EigenBackground method" << std::endl;
            Algorithms::BackgroundSubtraction::EigenbackgroundParams params;
            params.SetFrameSize(width, height);
            params.LowThreshold() = 15*15;
            params.HighThreshold() = 2*params.LowThreshold();	// Note: high threshold is used by post-processing 
            params.HistorySize() = 100;
            params.EmbeddedDim() = 20;

            //Algorithms::BackgroundSubtraction::Eigenbackground bgs;
            bgs = new Algorithms::BackgroundSubtraction::Eigenbackground();
            bgs->Initalize(params);
            break;
        }
    }

    cvNamedWindow("ImageORI", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("ImageBG_LOW", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("ImageBG_HIGH", CV_WINDOW_AUTOSIZE);

	// perform background subtraction of each frame 
	for(int i = 0; i < num_frames-1; ++i)
	{
		if(i % 100 == 0)
			std::cout << "Processing frame " << i << " of " << num_frames << "..." << std::endl;

		// grad next frame from input video stream
		if(!cvGrabFrame(readerAvi))
		{           
			std::cerr << "Could not grab AVI frame." << std::endl;
			return 0;
		}		
		frame_data = cvRetrieveFrame(readerAvi); 
		
		// initialize background model to first frame of video stream
		if (i == 0)
			bgs->InitModel(frame_data); 

		// perform background subtraction
		bgs->Subtract(i, frame_data, low_threshold_mask, high_threshold_mask);

		// save results
		//cvWriteFrame(writerAvi, low_threshold_mask.Ptr());

        //Show the result
        cvFlip(low_threshold_mask.Ptr(),NULL,0);
        cvFlip(high_threshold_mask.Ptr(),NULL,0);
        cvShowImage("ImageORI", frame_data.Ptr());
        cvShowImage("ImageBG_LOW", low_threshold_mask.Ptr());
        cvShowImage("ImageBG_HIGH", high_threshold_mask.Ptr());
        cvWaitKey(50);

		// update background subtraction
		low_threshold_mask.Clear();	// disable conditional updating
		bgs->Update(i, frame_data, low_threshold_mask);
	}

	cvReleaseCapture(&readerAvi);
	//cvReleaseVideoWriter(&writerAvi);
}
