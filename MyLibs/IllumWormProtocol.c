/*
 * WormIllumProtocol.c
 *
 * This library is designed to have objects and functions used to
 * write, analyze and excecute illumination protocols written for worms.
 *
 * As such it relies on the high level WormAnalysis library.
 * But Other libraries like WriteOutWorm likely depend upon it.
 *
 *
 *  Created on: Nov 11, 2009
 *      Author: Andy
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

//OpenCV Headers
#include <cxcore.h>
#include <highgui.h>
#include <cv.h>

// Andy's Libraries

#include "AndysOpenCVLib.h"
#include "WormAnalysis.h"
#include "IllumWormProtocol.h"
#include "version.h"



/*******************************************/
/*
 * Protocol Objects
 */
/*******************************************/

/*
 * Create a Protocol Object and set all of its values to zero or NULL
 * The Protocol object has some descriptions and a CvSeq object that
 * points to sequences of polygons for illumination.
 * Each step can contain an arbitrary number of polygons.
 * Each polygon can contain an arbitrary number of points.
 *
 */
Protocol* CreateProtocolObject(){
	Protocol* MyProto= (Protocol*) malloc(sizeof(Protocol));
	MyProto->GridSize.height=0;
	MyProto->GridSize.width=0;
	MyProto->Filename=NULL;
	MyProto->Description=NULL;
	MyProto->Steps=NULL;
	MyProto->memory=cvCreateMemStorage();
	return MyProto;

}



/*
 * Write everything out to YAML
 *
 */
void WriteProtocolToYAML(Protocol* myP){
	/** Open file for writing **/
	printf("We are about to write out a protocol. Here is some info about the protocol.:\n");
	printf("The protocol has %d steps.\n",myP->Steps->total);


	CvFileStorage* fs=cvOpenFileStorage(myP->Filename,myP->memory,CV_STORAGE_WRITE);
	if (fs==0){
		printf("fs is zero! Could you have specified the wrong directory?\n");
		return;
	}
	printf("Writing to %s\n",myP->Filename);
	/** Write out Generic comments **/
	cvWriteComment(fs, "Illumination Protocol:",0);
	cvWriteComment(fs, "Generated by the IlluminationWormProtocol Library \nmade by leifer@fas.harvard.edu",0);
	cvWriteComment(fs, "\nSoftware Version Information:",0);
	cvWriteComment(fs, build_git_sha,0);
	cvWriteComment(fs, build_git_time,0);
	cvWriteComment(fs, "\n",0);
	printf("wrote comments\n");

	/** Write out Protocol **/
	cvStartWriteStruct(fs,"Protocol",CV_NODE_MAP,NULL);
		if (myP->Filename!=NULL) cvWriteString(fs,"Filename",myP->Filename);
		if (myP->Description!=NULL)  cvWriteString(fs,"Description",myP->Description);
		cvStartWriteStruct(fs,"GridSize",CV_NODE_MAP,NULL);
			cvWriteInt(fs,"height",myP->GridSize.height);
			cvWriteInt(fs,"width",myP->GridSize.width);
		cvEndWriteStruct(fs);
		printf("yo\n");
		/** Write Out Steps **/
		cvStartWriteStruct(fs,"Steps",CV_NODE_SEQ,NULL);
		int j;
		int jtot=myP->Steps->total;


		CvSeqReader StepReader;
		cvStartReadSeq(myP->Steps,&StepReader,0);
		for (j = 0; j < jtot; ++j) {
			printf("About to write step number %d\n",j);
			CvSeq** CurrentMontagePtr = (CvSeq**) StepReader.ptr;
			CvSeq* CurrentMontage=*CurrentMontagePtr;
			assert(CurrentMontage!=NULL);
			printf("ping\n");
			printf("CurrentMontage->total=%d",CurrentMontage->total);
			cvStartWriteStruct(fs,NULL,CV_NODE_SEQ,NULL);
			int k;
			int ktot=CurrentMontage->total;
			printf("ktot=%d\n",ktot);

			CvSeqReader MontageReader;
			cvStartReadSeq(CurrentMontage,&MontageReader);
			for (k = 0; k < ktot; ++k) {
				printf("About to write polygon number %d\n",k);
				WormPolygon** CurrentPolygonPtr= (WormPolygon**) MontageReader.ptr;
				WormPolygon* CurrentPolygon=*CurrentPolygonPtr;

				cvWrite(fs,NULL,CurrentPolygon->Points);

				CV_NEXT_SEQ_ELEM(CurrentMontage->elem_size,MontageReader);
			}
			CurrentMontagePtr=NULL;
			CurrentMontage=NULL;
			cvEndWriteStruct(fs);

			/** Loop to Next Step **/
			CV_NEXT_SEQ_ELEM(myP->Steps->elem_size,StepReader);

		}
		cvEndWriteStruct(fs);
	cvEndWriteStruct(fs);
	cvReleaseFileStorage(&fs);
}


void LoadProtocolWithFilename(const char* str, Protocol* myP){
	assert(myP!=NULL);
	myP->Filename = copyString(str);

}

void LoadProtocolWithDescription(const char* str, Protocol* myP){
	myP->Description= (char *) malloc(sizeof(char)*(strlen(str)+1));
	strcpy(myP->Description,str);
}





void DestroyProtocolObject(Protocol** MyProto){
	/** **/
	printf("in DestroyProtocolObject()\n");
	assert(MyProto!=NULL);
	if (*MyProto==NULL) return;
	if  ((*MyProto)->Filename!=NULL ) {
		free( &((*MyProto)->Filename));
		(*MyProto)->Filename=NULL;
	}

	printf("about to cycle through...\n");

	if ((*MyProto)->Steps!=NULL){
	//	printf("(*MyProto)->Steps!=NULL\n");
		int numsteps=(*MyProto)->Steps->total;
	//	printf("numsteps=%d\n",numsteps);
		for (int step= 0; step < numsteps ; ++step) {
			CvSeq** montagePtr=(CvSeq**) cvGetSeqElem((*MyProto)->Steps,step);
			CvSeq* montage=*montagePtr;
			if (montage!=NULL){
		//		printf("montage!=NULL\n");
				int numpolygons=montage->total;
			//	printf("numpolygons=%d\n",numpolygons);
				for (int k= 0; k< numpolygons; ++k) {
					WormPolygon** polygonPtr=(WormPolygon**) cvGetSeqElem(montage,k);
					WormPolygon* polygon=*polygonPtr;
		//			printf( "\tFound polygon with %d nubmer of points.\n",polygon->Points->total );
					DestroyWormPolygon(&polygon);
		//			printf("Destroying polygon...\n");

				}
			}

		}
	}


	if  ((*MyProto)->Description!=NULL ) {
		free( &((*MyProto)->Description) );
		(*MyProto)->Description=NULL;
	}

	cvReleaseMemStorage(&(*MyProto)->memory);
	free(MyProto);
	*MyProto=NULL;
}





/*******************************************/
/*
 * Steps Objects
 */
/*******************************************/


/*
 * A steps object is a cvSeq containing pointers to
 * Illumination Montages (whcih are in turn cvSeq's of pointers to Polygons).
 *
 */
CvSeq* CreateStepsObject(CvMemStorage* memory){
	if (memory==NULL) {
		printf("ERROR! memory is null in CreateStepsObject()!\n");
		return NULL;
	}
	CvSeq* mySteps=cvCreateSeq(CV_SEQ_ELTYPE_PTR,sizeof(CvSeq),sizeof(CvSeq*),memory);
	return mySteps;
}




/*******************************************/
/*
 * Illumination Objects
 */
/*******************************************/

/*
 * An illumination montage consists of a sequence of pointers to polygons
 *
 */
CvSeq* CreateIlluminationMontage(CvMemStorage* memory){
	CvSeq* myIllumMontage=cvCreateSeq(CV_SEQ_ELTYPE_PTR,sizeof(CvSeq),sizeof(WormPolygon*),memory);
	return myIllumMontage;
}



/*******************************************/
/*
 * Polygon Objects
 */
/*******************************************/

/*
 * Given a memory object, this will create a polygon object that is a CvSeq.
 *
 */
WormPolygon* CreateWormPolygon(CvMemStorage* memory,CvSize mySize){
	WormPolygon* myPoly=(WormPolygon*) malloc(sizeof(WormPolygon));
	myPoly->Points=cvCreateSeq(CV_SEQ_ELTYPE_POINT,sizeof(CvSeq),sizeof(CvPoint),memory);
	myPoly->GridSize=mySize;
	return myPoly;
}

/*
 *
 * Creates a worm polygon object from a CvSeq of Points.
 * This will clone the CvSeq and copy it into the memory storage
 * specified
 */
WormPolygon* CreateWormPolygonFromSeq(CvMemStorage* memory,CvSize GridSize,CvSeq* points){
	WormPolygon* myPoly=(WormPolygon*) malloc(sizeof(WormPolygon));
	myPoly->Points=cvCloneSeq(points,memory);
	myPoly->GridSize=GridSize;
	return myPoly;
}



/*
 * Destroys a polygon but doesn't free up the CvMemStorage that that polygon used
 *
 */
void DestroyWormPolygon(WormPolygon** myPoly){
	free(myPoly);
	*myPoly=NULL;
}

/*
 * Simple function to copy a string
 */
char *copyString (const char *src) {
	assert (src != NULL);
	char *dst = (char *) malloc (strlen(src)+ 1);;
	strcpy(   dst, src);
	return dst;
}




void PrintPointsOfSeq(CvSeq* seq){
	int numpts=seq->total;
	CvSeqReader PtReader;
	cvStartReadSeq(seq,&PtReader,0);
	int i;
	for (i = 0; i < numpts; ++i) {
		CvPoint* pt= (CvPoint*) PtReader.ptr;

		printf("[%d,",pt->x);
		printf("%d] ",pt->y);
		CV_NEXT_SEQ_ELEM(seq->elem_size,PtReader);
	}
	printf("\n");

}


int VerifyProtocol(Protocol* p){
	printf("\n\n========== VERIFYING PROTOCOL============\n");
	if (p==NULL){
		printf("Protocol is NULL\n");
		return -1;
	}

	printf("Protocol description: %s\n", p->Description);
	printf("Filename= %s\n",p->Filename);
	printf("Total number of steps: p->Steps->total=%d\n",p->Steps->total);

	CvSeqReader StepReader;
	cvStartReadSeq(p->Steps, &StepReader,0);
	int numsteps=p->Steps->total;
	/** Let's loop through all of the steps **/
		for (int i= 0; i< numsteps; ++i) {
			printf("Step i=%d\n",i);

			CvSeq* CurrMontage= *( (CvSeq**) StepReader.ptr);
			printf("\tCurrMontage has %d polygons\n",CurrMontage->total);
			int numPolygons=CurrMontage->total;
			int j;


			/** Let's loop through the polygons **/

			CvSeqReader MontageReader;
			cvStartReadSeq(CurrMontage, &MontageReader);
			for (j = 0; j < numPolygons; ++j) {
				WormPolygon*  poly= *( (WormPolygon**) MontageReader.ptr );
				int numpts=poly->Points->total;
				printf(" numpts=%d\n",numpts);




				PrintPointsOfSeq(poly->Points);




				CV_NEXT_SEQ_ELEM( CurrMontage->elem_size,MontageReader);
			}




			/** Progress to the next step **/
			CV_NEXT_SEQ_ELEM( p->Steps->elem_size, StepReader );
		}


	printf("========================================\n");
	return 0;
}



/************************************
 *
 * Illumination Routines
 *
 */


/*
 * This function makes a black image.
 *
 */
IplImage* GenerateRectangleWorm(CvSize size){
	/** Create a rectangular worm with the correct dimensions **/
	IplImage* rectWorm=cvCreateImage(size,IPL_DEPTH_8U,1);
	cvZero(rectWorm);
	return rectWorm;
}




/*
 * Returns the pointer to a montage of polygons corresponding
 * to a specific step of a protocol
 */
CvSeq* GetMontageFromProtocol(Protocol* p, int step){
	CvSeq** montagePtr=(CvSeq**) cvGetSeqElem(p->Steps,step);
	return *montagePtr;
}

/*
 * Given a Montage CvSeq of WormPolygon objects,
 * This function allocates memory for an array of
 * CvPoints and returns that.
 *
 * When using this function, don't forget to release the allocated
 * memory.
 */
int CreatePointArrFromMontage(CvPoint** polyArr,CvSeq* montage,int polygonNum){
	if (polygonNum >= montage->total){
		printf("ERROR! GetPointArrFromMontage() was asked to fetch the %dth polygon, but this montage only has %d polygons\n",polygonNum,montage->total);
		return -1;
	}
	WormPolygon** polygonPtr=(WormPolygon**) cvGetSeqElem(montage,polygonNum);
	WormPolygon* polygon=*polygonPtr;

	/** Allocate memory for a CvPoint array of points **/
	*polyArr= (CvPoint*) malloc( sizeof(CvPoint)*polygon->Points->total);
	cvCvtSeqToArray(polygon->Points,*polyArr,CV_WHOLE_SEQ);

	return polygon->Points->total;
}

void DisplayPtArr(CvPoint* PtArr,int numPts){
	int k=0;
	for (k = 0; k < numPts; ++k) {
				printf(" (%d,",PtArr[k].x);
				printf(" %d)\n",PtArr[k].y);


	}
}

/*
 * Given an array of CvPoint(s), go through and add an offset
 * to all of the x or y values.
 *
 * Use XorY=0 to change the offset of the x values.
 * Use XorY=1 to change the offset of the y values.
 *
 */
void OffsetPtArray(CvPoint** Pts,int numPts,int offset,int XorY){
	if (*Pts==NULL){
		printf("Error! Pts is NULL.\n");
		return;
	}

	/** lPts is just a shortcut for *Pts **/
	CvPoint* lPts=*Pts;

	/** Loop through Array **/
	int k;
	for (k = 0; k < numPts; ++k) {
		if (XorY==0){
			lPts[k].x=lPts[k].x+offset;
		} else {
			lPts[k].y=lPts[k].y+offset;
		}
	}

}

/*
 * Illuminate a rectangle worm
 */
void IllumRectWorm(IplImage* rectWorm,Protocol* p,int step){
	CvSeq* montage= GetMontageFromProtocol(p,step);
	int numOfPolys=montage->total;
	int numPtsInCurrPoly;
	CvPoint* currPolyPts=NULL;
	int poly;
	for (poly = 0; poly < numOfPolys; ++poly) {
		printf("==poly=%d==\n",poly);
		numPtsInCurrPoly=CreatePointArrFromMontage(&currPolyPts,montage,poly);
		DisplayPtArr(currPolyPts,numPtsInCurrPoly);

		OffsetPtArray(&currPolyPts,numPtsInCurrPoly,100,0);

		cvFillConvexPoly(rectWorm,currPolyPts,numPtsInCurrPoly,cvScalar(255,255,255),CV_AA);
		free(currPolyPts);
	}
}

