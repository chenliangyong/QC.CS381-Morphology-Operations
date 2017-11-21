//
//  morphology.cpp
//  Morphology Operations
//
//  Created by Nick on 2017/11/19.
//  Copyright © 2017年 Liangyong Chen. All rights reserved.
//
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

using namespace std;

class morphology{
private:
    int numRowsIMG;
    int numColsIMG;
    int minValIMG;
    int maxValIMG;
    
    int numRowsStrctElem;
    int numColsStrctElem;
    int minValStrctElem;
    int maxValStrctElem;
    int rowOrigin;
    int colOrigin;
    
    int rowFrameSize;
    int colFrameSize;
    
    int **imgAry;
    int **morphAry;
    int **tempAry;
    int **structElem;
    
    string iFile1,iFile2,oFile1,oFile2,oFile3,oFile4;
    
public:
    morphology(char **File){
        iFile1=File[1];
        iFile2=File[2];
        oFile1=File[3];
        oFile2=File[4];
        oFile3=File[5];
        oFile4=File[6];
        fstream finput;
        
        //load input2 head info
        finput.open(iFile2);
        if (!finput.is_open()) {
            throw "Error ! Open Input File Failed, try again...";
        }
        if (!finput.eof() && finput.peek() != EOF) {
            finput >> numRowsStrctElem;
            finput >> numColsStrctElem;
            finput >> minValStrctElem;
            finput >> maxValStrctElem;
            finput >> rowOrigin;
            finput >> colOrigin;
        }
        else {
            finput.close();
            throw "Error ! Input File is empty, try again...";
        }
        finput.close();
        
        // dynamically allocate structElem
        structElem = new int*[numRowsStrctElem];
        for (int i = 0; i < numRowsStrctElem; i++) {
            structElem[i] = new int[numColsStrctElem];
            for (int j = 0; j < numColsStrctElem; j++) {
                structElem[i][j] = 0;
            }
        }
        
        // load input1 to imgAry
        loadStruct();
        computeFremeSize();
        
        
        //load input1 head info
        
        finput.open(iFile1);
        if (!finput.is_open()) {
            throw "Error ! Open Input File Failed, try again...";
        }
        if (!finput.eof() && finput.peek() != EOF) {
            finput >> numRowsIMG;
            finput >> numColsIMG;
            finput >> minValIMG;
            finput >> maxValIMG;
        }
        else {
            finput.close();
            throw "Error ! Input File is empty, try again...";
        }
        finput.close();
        
        // dynamically allocate imgAry, morphAry
        imgAry = new int*[numRowsIMG + rowFrameSize*2];
        morphAry = new int*[numRowsIMG + rowFrameSize*2];
        tempAry = new int*[numRowsIMG + rowFrameSize*2];
        for (int i = 0; i < numRowsIMG + rowFrameSize*2; i++) {
            imgAry[i] = new int[numColsIMG + colFrameSize*2];
            morphAry[i] = new int[numColsIMG + colFrameSize*2];
            tempAry[i] = new int[numColsIMG + colFrameSize*2];
            for (int j = 0; j < numColsIMG + colFrameSize*2; j++) {
                imgAry[i][j] = 0;
                morphAry[i][j] = 0;
                tempAry[i][j] = 0;
                
            }
        }
        
        // load input1 to imgAry
        loadImage();
        
        // prepare Output file;
        fstream foutput1;
        fstream foutput2;
        fstream foutput3;
        fstream foutput4;
        foutput1.open(oFile1, ios::out);
        foutput2.open(oFile2, ios::out);
        foutput3.open(oFile3, ios::out);
        foutput4.open(oFile4, ios::out);
        foutput1 << numRowsIMG << " " << numColsIMG << " " << minValIMG << " " << maxValIMG << endl;
        foutput2 << numRowsIMG << " " << numColsIMG << " " << minValIMG << " " << maxValIMG << endl;
        foutput3 << numRowsIMG << " " << numColsIMG << " " << minValIMG << " " << maxValIMG << endl;
        foutput4 << numRowsIMG << " " << numColsIMG << " " << minValIMG << " " << maxValIMG << endl;
        foutput1.close();
        foutput2.close();
        foutput3.close();
        foutput4.close();
        
        prettyPrint(1);
        prettyPrint(2);
    }
    ~morphology(){
        for (int i = 0; i < numRowsIMG + rowFrameSize; i++) {
            delete[] imgAry[i];
            delete[] morphAry[i];
        }
        for (int i = 0; i < numColsStrctElem; i++){
            delete[] structElem[i];
        }
        delete[] imgAry;
        delete[] morphAry;
        delete[] structElem;
    }
    
    void computeFremeSize(){
        rowFrameSize=numRowsStrctElem/2;
        colFrameSize=numColsStrctElem/2;
    }
    void loadImage(){
        //Read the original image
        fstream finput;
        finput.open(iFile1);
        if (!finput.is_open()) {
            throw "Error ! Open Input File1 Failed, try again...";
        }
        if (!finput.eof() && finput.peek() != EOF) {
            string temp;
            getline(finput, temp); // skip the image head
            int gVal = 0;
            for (int i = rowFrameSize; i < numRowsIMG + rowFrameSize; i++) {
                for (int j = colFrameSize; j < numColsIMG + colFrameSize; j++) {
                    if (!finput.eof() && finput.peek() != EOF) {
                        finput >> gVal;
                    }
                    else { break; }
                    imgAry[i][j] = gVal;
                }
            }
        }
        else {
            finput.close();
            throw "Error ! Input File1 is empty, try again...";
        }
        finput.close();
        //cout << "Finish Loading Original Image." << endl;
    }
    void loadStruct(){
        //Read the Structure image
        fstream finput;
        finput.open(iFile2);
        if (!finput.is_open()) {
            throw "Error ! Open Input File2 Failed, try again...";
        }
        if (!finput.eof() && finput.peek() != EOF) {
            string temp;
            getline(finput, temp); // skip the image head
            getline(finput, temp); // skip the origin info
            int gVal = 0;
            for (int i = 0; i < numRowsStrctElem; i++) {
                for (int j = 0; j < numColsStrctElem ; j++) {
                    if (!finput.eof() && finput.peek() != EOF) {
                        finput >> gVal;
                    }
                    else { break; }
                    structElem[i][j] = gVal;
                }
            }
        }
        else {
            finput.close();
            throw "Error ! Input File2 is empty, try again...";
        }
        finput.close();
        //cout << "Finish Loading Strcture Image." << endl;
    }
    
    void resetMorphAry(){
        for (int i = 0; i < numRowsIMG + rowFrameSize*2; i++) {
            for (int j = 0; j < numColsIMG + colFrameSize*2; j++) {
                morphAry[i][j] = 0;
            }
        }
    }
    void copyMorphAry(){
        for (int i = 0; i < numRowsIMG + rowFrameSize*2; i++) {
            for (int j = 0; j < numColsIMG + colFrameSize*2; j++) {
                tempAry[i][j] = morphAry[i][j];
            }
        }
    }
    void dilationOP(){
        resetMorphAry();
        for (int i = rowFrameSize; i < numRowsIMG + rowFrameSize; i++){
            for (int j = colFrameSize; j < numColsIMG + rowFrameSize; j++){
                if (imgAry[i][j]>0){
                    dilation(i, j);
                }
            }
        }
    }
    void dilation(int i, int j){
        for(int p = 0; p < numRowsStrctElem; p++){
            for(int q = 0; q < numColsStrctElem; q++){
                morphAry[i-rowOrigin+p][j-colOrigin+q] = structElem[p][q];
            }
        }
    }
    void erosionOP(){
        resetMorphAry();
        for (int i = rowFrameSize; i < numRowsIMG + rowFrameSize; i++){
            for (int j = colFrameSize; j < numColsIMG + rowFrameSize; j++){
                if (imgAry[i][j]>0){
                    erosion(i, j, 0);
                }
            }
        }
    }
    void erosion(int i, int j, int opType){
        for(int p = 0; p < numRowsStrctElem; p++){
            for(int q = 0; q < numColsStrctElem; q++){
                if (opType == 0){ // opType to control which array need to compare;
                    if (imgAry[i-rowOrigin+p][j-colOrigin+q] != structElem[p][q]) return;
                }
                else{
                    if (tempAry[i-rowOrigin+p][j-colOrigin+q] != structElem[p][q]) return;
                }
            }
        }
        morphAry[i][j] = 1;
    }
    void closing(){
        resetMorphAry();
        dilationOP();
        copyMorphAry();
        resetMorphAry();
        for (int i = rowFrameSize; i < numRowsIMG + rowFrameSize; i++){
            for (int j = colFrameSize; j < numColsIMG + rowFrameSize; j++){
                if (tempAry[i][j]>0){
                    erosion(i, j, 1);
                }
            }
        }
    }
    void opening(){
        resetMorphAry();
        erosionOP();
        copyMorphAry();
        resetMorphAry();
        for (int i = rowFrameSize; i < numRowsIMG + rowFrameSize; i++){
            for (int j = colFrameSize; j < numColsIMG + rowFrameSize; j++){
                if (tempAry[i][j]>0){
                    dilation(i, j);
                }
            }
        }
    }
    void prettyPrint(int type){
        //type : 1 = imgAry; 2 = structElem; 3 = morphAry;
        switch (type) {
            case 1:
                cout << "Input image :" << endl;
                for (int i = rowFrameSize; i < numRowsIMG + rowFrameSize; i++) {
                    for (int j = colFrameSize; j < numColsIMG + colFrameSize; j++) {
                        if (imgAry[i][j]>0)
                            cout << imgAry[i][j] << " ";
                        else
                            cout << "  ";
                    }
                    cout << endl;
                }
                cout << endl;
                break;
                
            case 2:
                
                cout  << "Structuring Element :" << endl;
                for (int i = 0; i < numRowsStrctElem; i++){
                    for(int j = 0; j < numColsStrctElem; j++){
                        cout << structElem[i][j] << " ";
                    }
                    cout << endl;
                }
                cout << endl;
                break;
                
            case 3:
                
                for (int i = rowFrameSize; i < numRowsIMG + rowFrameSize; i++) {
                    for (int j = colFrameSize; j < numColsIMG + colFrameSize; j++) {
                        if (morphAry[i][j]>0)
                            cout << morphAry[i][j] << " ";
                        else
                            cout << "  ";
                    }
                    cout << endl;
                }
                cout << endl;
                break;
                
            default:
                break;
        }
    }
    void outPutResult(int outputType){
        // outputType : 1 - dilation, 2 - erosion, 3 - closing, 4 - opening
        fstream foutput;
        switch (outputType) {
            case 1:
                foutput.open(oFile1, ios::out | ios::app);
                break;
            case 2:
                foutput.open(oFile2, ios::out | ios::app);
                break;
            case 3:
                foutput.open(oFile3, ios::out | ios::app);
                break;
            case 4:
                foutput.open(oFile4, ios::out | ios::app);
                break;
                
            default:
                cout << "ouPutResult(int) - Parameter error.." << endl;
                return;
        }
        
        for (int i = rowFrameSize; i < numRowsIMG + rowFrameSize; i++) {
            for (int j = colFrameSize; j < numColsIMG + colFrameSize; j++) {
                    foutput << morphAry[i][j] << " ";
            }
            foutput << endl;
        }
        foutput.close();
        
    }
};

int main(int argc, char * argv[]) {
    try{
        if (argc != 7) {
            cout << "Parameters missing... ( Run as : ChainCode <input file1> <input file2> <output file1> <output file2> ).";
            return 0;
        }
        
        morphology MP(argv);
        cout << "Delation Result :" << endl;
        MP.dilationOP();// call normal dilation opation
        MP.prettyPrint(3);
        MP.outPutResult(1);
        
        cout << "Erosion Result :" << endl;
        MP.erosionOP();// call normal erosion opation
        MP.prettyPrint(3);
        MP.outPutResult(2);
        
        cout << "Opening Result :" << endl;
        MP.opening();
        MP.prettyPrint(3);
        MP.outPutResult(4);
        
        cout << "Closing Result :" << endl;
        MP.closing();
        MP.prettyPrint(3);
        MP.outPutResult(3);
        
    }
    catch (const char* msg) {
        cerr << msg << endl;
        return 0;
    }
    return 0;
}
