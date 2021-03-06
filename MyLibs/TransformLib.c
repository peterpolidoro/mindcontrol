/*
 * Copyright 2010 Andrew Leifer et al <leifer@fas.harvard.edu>
 * This file is part of MindControl.
 *
 * MindControl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MindControl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MindControl. If not, see <http://www.gnu.org/licenses/>.
 *
 * For the most up to date version of this software, see:
 * http://github.com/samuellab/mindcontrol
 *
 *
 *
 * NOTE: If you use any portion of this code in your research, kindly cite:
 * Leifer, A.M., Fang-Yen, C., Gershow, M., Alkema, M., and Samuel A. D.T.,
 * 	"Optogenetic manipulation of neural activity with high spatial resolution in
 *	freely moving Caenorhabditis elegans," Nature Methods, Submitted (2010).
 */


//OpenCV Headers
//#include <cxcore.h>
#include "opencv2/highgui/highgui_c.h"
//#include <cv.h>

#include <stdio.h>
#include <stdbool.h>

#include "AndysOpenCVLib.h"
#include "WormAnalysis.h"
#include "TransformLib.h"


/*************************************
 *
 * Private Function
 *
 */




/*
 * Create and allocate memory for the CalibData structure
 *
 * Currently SizeOfCCD is stored but not really used
 *
 */
CalibData* CreateCalibData( CvSize SizeOfDLP, CvSize SizeOfCCD){

	printf("Inside CreateCalibData()\nSizeOfDLP.height =%d,SizeOfDLP.width=%d\n",SizeOfDLP.height ,SizeOfDLP.width);
	CalibData* Calib=(CalibData*) malloc(sizeof(CalibData));
	Calib->CCD2DLPLookUp = (int *) malloc(2 * SizeOfDLP.height * SizeOfDLP.width* sizeof(int));
	Calib->SizeOfCCD=SizeOfCCD;
	Calib->SizeOfDLP=SizeOfDLP;
	return Calib;
}

/*
 * Deallocate memory for CalibData object
 */
void DestroyCalibData(CalibData* Calib){
	free(Calib->CCD2DLPLookUp);
	free(Calib);

}





/*
 * Read In Calibration Frome File
 *
 * Returns 1 if open failed.
 * Returns -1 if open succesfully but read fails.
 */

int LoadCalibFromFile(CalibData* Calib, char * filename){
	FILE *fp;
	int result;
	int FLAG=0;
	/*************** Read Calibration from File ****************/

	if ((fp = fopen(filename, "rb")) == NULL) {
		printf("Cannot open file.\n");
		FLAG=1;
	}
	result = 0;
	if (FLAG==0) result = fread(Calib->CCD2DLPLookUp, sizeof(int) * 2 * Calib->SizeOfDLP.height * Calib->SizeOfDLP.width , 1, fp);
	if (result != 1) {
		printf("Read error!\n");
	} else{
		printf("Read was successful.\n");
	}
	if (FLAG==0) fclose(fp);
	if (result ==0) FLAG=-1;
	return FLAG;
}


/*
 * Transform's the binary image from the frame in Cam and transforms it DLP space.
 * Copies it into the DLP frame and also converts it to IlpImage and copies that to the DLP
 * frame also.
 * DEPRECATED!! This is really really slow.
 * If possible transform the outline of the worm using TransformSegWormCam2DLP isntead of the whole frame.
 *
 */
int TransformFrameCam2DLP(Frame* Cam, Frame* DLP, CalibData* Calib) {
	int ret = 0;

	ret = ConvertCharArrayImageFromCam2DLP(Calib->CCD2DLPLookUp, Cam->binary,
			DLP->binary, Cam->size.width, Cam->size.height, DLP->size.width,
			DLP->size.height, 0);
//	return 0;
	if (ret == 0) /** This could be ommitted to save CPU cycles, at the cost of monitoring output **/
		ret = CopyCharArrayToIplImage(DLP->binary, DLP->iplimg,DLP->size.width,DLP->size.height);
	return ret;
}




/*
 *  This function takes a lookup table generated by the CalibrationTest() function in calibrate.c
 *  It takes pointers to pre-allocated memory for the images fromCCD and forDLP which are
 *  unsigned character arrays in the Y800 format as employed by the Discovery 4000 DLP and the
 *  ImagingSource Camera. These are allocated with a function such as
 *
 *  fromCCD = (unsigned char *) malloc(NSIZEX * NSIZEY * sizeof(unsigned char));
 *
 *  nsizex and nsizey are ints that contain the x & y dimensionf the camera and the DLP. This will all change when we purchase a new camera
 *  with different sizes.
 *
 *  Currently ccdsizex and ccdsizey are simply placeholders, they don't do anything.
 *
 *	If DEBUG_FLAG !=0, then print debugging information.
 *
 *
 */
int ConvertCharArrayImageFromCam2DLP(int *CCD2DLPLookUp,
		unsigned char* fromCCD, unsigned char* forDLP, int nsizex, int nsizey,
		int ccdsizex, int ccdsizey, int DEBUG_FLAG) {
	if (nsizex != ccdsizex || nsizey != ccdsizey) {
		printf(
				"ERROR: Ignoring values of ccdsizex & ccdsizey. \nCurrently CCD must be the same size as the DLP.\n This functionality has yet to be coded up.");
	}
	if (CCD2DLPLookUp == NULL) {
		printf("ERROR! CCD2DLPLookUp==NULL!\n");
		return -1;
	}
	unsigned int tempx = 0;
	unsigned int tempy = 0;
	int XOUT = 0;
	int YOUT = 1;
	unsigned int newptx;
	unsigned int newpty;
	while (tempx < nsizex) {
		//printf("#");
		tempy = 0;
		while (tempy < nsizey) {
			//Actually Perform the LookUp and convert (tempx, tempy) in CCD coordinates to (newptx,newpty) in DLP coordinates
			// I= z*Nx*Ny+x*Ny+y
			if (XOUT * nsizey * nsizex + tempx * nsizey + tempy >= nsizex
					* nsizey * 2) {
				printf(" In accessing lookup table, we are out of bounds!!\n");
			}
			if (YOUT * nsizey * nsizex + tempx * nsizey + tempy >= nsizex
					* nsizey * 2) {
				printf(" In accessing lookup table, we are out of bounds!!\n");
			}
			newptx = CCD2DLPLookUp[XOUT * nsizey * nsizex + tempx * nsizey
					+ tempy];
			newpty = CCD2DLPLookUp[YOUT * nsizey * nsizex + tempx * nsizey
					+ tempy];
			if (newptx < 0 || newpty < 0 || newptx >= nsizex || newpty
					>= nsizey) {
				// Don't do anything because the pint is invalid
			} else { //If the new point is reasonable, go ahead and do the conversion
				//triple check that we're not actually accesssing an element outside of our array
				if (tempx * nsizey + newpty >= nsizex * nsizey || newpty
						* nsizex + newptx >= nsizex * nsizey) {
					printf(
							"In Accessing fromCCD or forDLP, we are out of bounds!!\n");
				}
				//actually copy the value of the pixel at the two points
				forDLP[newpty * nsizex + newptx] = fromCCD[tempy * nsizex
						+ tempx];
			}
			tempy++;
		}
		tempx++;
	}
	return 0;
}





/*
 * Converts a CvPoint (x,y) camera space to DLP space.
 * This uses the lookup table generated by the CalibrationTest() function in calibrate.c
 *
 *  Currently ccdsizex and ccdsizey are simply placeholders, they don't do anything.
 *  In other words this assumes that both the camera and DLP are the simesize, namely 1024x768
 *
 *	If DEBUG_FLAG !=0, then print debugging information.
 *
 *
 */
int cvtPtCam2DLP(CvPoint camPt, CvPoint* DLPpt,CalibData* Calib) {


	int nsizex= Calib->SizeOfDLP.width;
	int nsizey=Calib->SizeOfDLP.height;
	int ccdsizex=Calib->SizeOfCCD.width;
	int ccdsizey=Calib->SizeOfCCD.height;

	if (nsizex != ccdsizex || nsizey != ccdsizey) {
		printf(
				"ERROR: Ignoring values of ccdsizex & ccdsizey. \nCurrently CCD must be the same size as the DLP.\n This functionality has yet to be coded up.");
	}
	if (Calib->CCD2DLPLookUp == NULL) {
		printf("ERROR! CCD2DLPLookUp==NULL!\n");
		return -1;
	}
//	int* CCD2DLPLookUp=Calib->CCD2DLPLookUp;

	const int XOUT = 0;
	const int YOUT = 1;
	
	/* Looking back at this code three years later, it appears that the format
	/* for the CCD2DLPLookup is such:
	/* It is a 1 x n array. The First nsizey*nsizex eleemtns represent X values 
	/* the second nsizex*nsizex elemetns represent Y values
	/*
	/* By looking at the equations below you can see how things are indexed   */
	/* To get the DLP x coordinate out for a a camera pt (camPt.x, camPt.y)
	/* DLP.x = CCD2DLPLookup[camPt.x*nsizey+camPt.y]
	/* DLP.y = CCD2DLPLookup[nsizey * nsizex +  camPt.x*nsizey+camPt.y]  */
	
	
	if (XOUT * nsizey * nsizex + camPt.x * nsizey + camPt.y >= nsizex* nsizey * 2) {
		printf(" In accessing lookup table, we are out of bounds!!\n");
		return 0;
	}

	if (YOUT * nsizey * nsizex + camPt.x * nsizey + camPt.y >= nsizex* nsizey * 2) {
		printf(" In accessing lookup table, we are out of bounds!!\n");
		return 0;
	}

	/** Actually convert the camPt to the DLPpt **/
	DLPpt->x = Calib->CCD2DLPLookUp[XOUT * nsizey * nsizex + camPt.x * nsizey + camPt.y];
	DLPpt->y = Calib->CCD2DLPLookUp[YOUT * nsizey * nsizex + camPt.x * nsizey + camPt.y];



	/* I am commenting this out, because this is a low level enough function, 
	that it is often useful to convert into DLP space even if we are out of range
	of the mirrors on the DLP. For examople, the head of the worm might be out of 
	range of the DLP, but it can still be important to know where the head is so 
	that one can draw an illumination pattern accurately, because there will ceraintly 
	be cases where the head of hte worm is out of bounds but the illumination pattern 
	itself is well within bounds. 
	
	Instead I should add in an error to let the user know when the illumination pattern
	is out of range of the DLP. */
	
//	if (DLPpt->x < 0 || DLPpt->y < 0 || DLPpt->x >= nsizex || DLPpt->y >= nsizey) {
//		printf ("ERROR: Illumination pattern is out of the field of the DLP.\n");
//		return 0;
//	}


	return 1;
}







/*
 * Transform's a sequence from Cameraspace to DLP space
 * This is an internal function only.
 */
int TransformSeqCam2DLP(CvSeq* camSeq, CvSeq* DLPseq, CalibData* Calib){
	if (camSeq==NULL || DLPseq==NULL) {
		printf ("ERROR! TransformSeqCam2DLP() was given NULL sequences\n");
		return -1;
	}
	/** Clear the points in the destination **/
	cvClearSeq(DLPseq);

	/** Setup CvSeq Reader **/
	CvSeqReader reader;
	cvStartReadSeq(camSeq,&reader,0);

	/**Setup CvSeq Writer **/
	CvSeqWriter writer;
	cvStartAppendToSeq(DLPseq, &writer);

	/** Temp points **/
	CvPoint DLPpt;
	CvPoint camPt;
	CvPoint* camPtptr;
	int numpts=camSeq->total;
	int j;
	for (j = 0; j < numpts; ++j) {

		camPtptr= (CvPoint*) reader.ptr;
		camPt=*camPtptr;

		/** Actually do the conversion **/
		cvtPtCam2DLP(camPt,&DLPpt,Calib);


		CV_WRITE_SEQ_ELEM( DLPpt, writer);
		CV_NEXT_SEQ_ELEM(camSeq->elem_size,reader);

	}
	cvEndWriteSeq(&writer);
	return 1;
}

/*
 * Takes a SegmentedWorm and transforms all of the points from Camera to DLP coordinates
 *
 */
int TransformSegWormCam2DLP(SegmentedWorm* camWorm, SegmentedWorm* dlpWorm,CalibData* Calib){
	if (camWorm==NULL || dlpWorm==NULL || Calib ==NULL ){
		printf("ERROR! TransformSegWormCAm2DLP passed NULL value.\n");
		return -1;
	}

	/** Transform points on centerline, right and left bounds**/
	ClearSegmentedInfo(dlpWorm);
	TransformSeqCam2DLP(camWorm->Centerline, dlpWorm->Centerline, Calib);
	TransformSeqCam2DLP(camWorm->RightBound, dlpWorm->RightBound, Calib);
	TransformSeqCam2DLP(camWorm->LeftBound, dlpWorm->LeftBound, Calib);


	/** Transform points on Head and Tail **/
	cvtPtCam2DLP(*(camWorm->Head),dlpWorm->Head,Calib);
	cvtPtCam2DLP(*(camWorm->Tail),dlpWorm->Tail,Calib);

	dlpWorm->NumSegments=camWorm->NumSegments;

	return 1;
}

