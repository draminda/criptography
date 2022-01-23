
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <omp.h>

using namespace std;
using namespace cv;

const size_t BUFFER_SIZE = 1024;
const int msgSize = 25 ;
const int charRandomSize = 255 ;//coz adding another 10 as random
const char charset[] = "0edfrgu99jnfrtyu89iujkhgt678TYUIOP{P:><LOP)(*&^%^TGBHJKIUYTF123456789ABCDEF12345678GHIJKLMNQWSDFGBNMZXCFVGHJKIOqwertyuikmnbgfdrtghb345670987hbnm,m098765tgbnklp[p;lkjhgfghjertyhnbvcfghOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz~`!@#$%^&*(<>?:{},./;[]|";

/**
 *
 * @param numbers
 * @param maxNumCanAssign
 * @param size
 */
void genRandomNumber(set<int> &numbers, int maxNumCanAssign, int size){
    numbers.insert(rand()%(maxNumCanAssign));
    if(numbers.size() == size){
        genRandomNumber(numbers, maxNumCanAssign, numbers.size());
    }
}
/**
 *
 * @param count
 * @param rows
 * @param cols
 * @return
 */
set<int> genRandomNumbers(int numberOfPositions, int rowsCount, int colsCount) {
    set<int> numbers;
    int i = 0 ;
    //2 for x and y direction
    while (i!=numberOfPositions){
        genRandomNumber(numbers, rowsCount, numbers.size());//for row
        genRandomNumber(numbers, colsCount, numbers.size());//for col
        i++;
    }
    return numbers;
}

/**
 *
 * @param img
 * @param col
 * @param row
 * @param decodedKeyFromMedia
 */
void decodeImageIndex(Mat imageToDecode,int col,int row,int frm,string &decodedKeyFromMedia){
    Point_<int> po;
    po.y = row;
    po.x = col;
    float keyAsInt = imageToDecode.at<cv::Vec2f>(po)[0];
    unsigned char frame = imageToDecode.at<cv::Vec2f>(po)[1];


    cout<<"frame "<<frm<<" decode pix x "<<po.x<< " pix y "<<po.y<<" key "<< (char)keyAsInt<< " frame "<<frame<<endl;

    decodedKeyFromMedia.push_back((unsigned char )keyAsInt);
}

/**
 * 
 * @param frame
 * @param decodedKey
 * @param index
 * @param decodedKeyFromMedia
 */
void decodeImage(Mat frame,string decodedKey,int index, string &decodedKeyFromMedia){
    int row = (unsigned char)decodedKey.at(index*6);//x
    row = row + (unsigned char) decodedKey.at(index*6+1)*charRandomSize;
    int col =  (unsigned char)decodedKey.at(index*6+2);//y
    col = col + (unsigned char)decodedKey.at(index*6+3)*charRandomSize;

    int frm =  (unsigned char)decodedKey.at(index*6+4);//f
    frm = frm+ (unsigned char)decodedKey.at(index*6+5)*charRandomSize;

    decodeImageIndex(frame, col, row,frm,decodedKeyFromMedia);
}

/**
 *
 * @param inputVideo
 * @param decodedKey
 * @param decodedKeyFromMedia
 */
void decrypt(VideoCapture inputVideo,string decodedKey,string &decodedKeyFromMedia){
    int i = -1;
    int numberOfHiddenFrames = decodedKey.size()/6;
#pragma omp parallel
#pragma omp while
    while (true) {
        i++;
        Mat frame;
        inputVideo >> frame;
        if (frame.empty() || i>=numberOfHiddenFrames)
            break;
        if(i<=numberOfHiddenFrames) {
            decodeImage( frame, decodedKey, i, decodedKeyFromMedia);
        }
    }
}


/**
 *
 * @param image
 * @param decodedKey
 * @param decodedKeyFromMedia
 */
void decrypt(Mat image,string decodedKey,string &decodedKeyFromMedia){
    int i = -1;
    int numberOfHiddenFrames = decodedKey.size()/6;
#pragma omp parallel
#pragma omp while
    while (true) {
        i++;
        if (i>=numberOfHiddenFrames)
            break;
        if(i<numberOfHiddenFrames) {
            decodeImage( image, decodedKey, i, decodedKeyFromMedia);
        }
    }
}

/**
 *
 * @param key
 * @param encodedKey
 */
void addKey(int key,string &encodedKey){
    int keyRemainder = key%charRandomSize;
    encodedKey.push_back(int(keyRemainder));
    encodedKey.push_back(int(key/charRandomSize));
}
/**
 *
 * @param imageToEncode
 * @param frame
 * @param keyForEncode
 * @param encodedKey
 * @return
 */
void encodeImage(Mat &imageToEncode,int frame,string keyForEncode,string &encodedKey){
    if(keyForEncode!=""  && keyForEncode.size()>0) {
        set<int> numbers = genRandomNumbers(keyForEncode.size(), imageToEncode.rows, imageToEncode.cols);
#pragma omp parallel
#pragma omp parallel for
        for (int i = 0; i < keyForEncode.size(); i++) {
            Point_<int> po;
            po.x = *next(numbers.begin(), i * 2);
            po.y = *next(numbers.begin(), i * 2 + 1);

            //hide Key
            float keyAsInt = keyForEncode.at(i);
            imageToEncode.at<cv::Vec2f>(po)[0]=keyAsInt;
            imageToEncode.at<cv::Vec2f>(po)[1]=frame;


            cout<<"frame :"<< frame<<" encode pix x "<<po.x<< " pix y "<<po.y<<" keyAsInt "<<(unsigned char)keyAsInt<<endl;

            //gen key location
            addKey(po.y,encodedKey);
            addKey(po.x,encodedKey);
            addKey(frame,encodedKey);
        }
    }
}
/**
 *
 * @param inputVideo
 * @param outputVideo
 * @param keyToHide
 * @param finalKey
 */
void createVideo(VideoCapture inputVideo,VideoWriter &outputVideo,string keyForEncode,string &encodedKey){
    int i = 1;
    int frameNumber = inputVideo.get(CAP_PROP_FRAME_COUNT);
    int modOfFrames = keyForEncode.size()/frameNumber + 1;
#pragma omp parallel
#pragma omp while
    while (i++) {
        Mat frame;
        inputVideo >> frame;
        if (frame.empty())
            break;
        else if(modOfFrames*(i)<=keyForEncode.size()) {
            string key = keyForEncode.substr(i*modOfFrames, 1);
            encodeImage(frame, i - 1, key, encodedKey);
        }
        outputVideo<<frame;
        namedWindow("Creating Your video ", cv::WINDOW_AUTOSIZE);
        imshow("frames ", frame);
        waitKey(2);
    }
}
/**
 *
 * @param inputVideo
 * @param outputName
 * @param outputVideo
 * @return
 */
VideoWriter createVideoHeading(VideoCapture inputVideo, string  &outPutFileName,VideoWriter &outputVideo){
    Size S = Size((int) inputVideo.get(CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) inputVideo.get(CAP_PROP_FRAME_HEIGHT));
    int ex = static_cast<int>(inputVideo.get(CAP_PROP_FOURCC));     // Get Codec Type- Int form
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};

    outputVideo.open(outPutFileName, VideoWriter::fourcc('M', 'J', 'P', 'G') , inputVideo.get(CAP_PROP_FPS), S, true);
    //outputVideo.open(outPutFileName, inputVideo.get(CAP_PROP_BACKEND),VideoWriter::fourcc(EXT[0], EXT[1], EXT[2], EXT[3]), inputVideo.get(CAP_PROP_FPS), S, true);
    if (!outputVideo.isOpened()){
        cerr  << "Could not open the output video for write: " << outPutFileName << endl;

    }
    cout << "Input frame resolution: Width=" << S.width << "  Height=" << S.height
         << " of nr#: " << inputVideo.get(CAP_PROP_FRAME_COUNT) << endl;
    cout << "Input codec type: " << EXT << endl;
    return outputVideo;
}
/**
 *
 * @return random char from 0-245
 */
char getRandom() {
    return charset[ rand() % 240 ];
}

/**
 * @note adding  remaining chars
 * @param keyForEncode
 * @param first
 * @param size
 */
void populate(string &keyForEncode, bool first, int size){
    if(keyForEncode.size()<msgSize){
        char cch =  getRandom();
        if (first) {
            keyForEncode.push_back(cch);
            cch =  getRandom();
            keyForEncode.push_back(cch);
        } else {
            keyForEncode.push_back(cch);
            cch =  getRandom();
            keyForEncode.push_back(cch);
        }
        populate(keyForEncode,first,size);
    }
    else {
        cout<<"size "<<size<<endl;
        char cch =  getRandom();
        if (first) {
            keyForEncode.push_back(cch);
            keyForEncode.push_back(size);
        } else {
            keyForEncode.push_back(size);
            keyForEncode.push_back(cch);
        }
    }

}
/**
 *
 * @param msgToEncode
 * @param keyForEncode
 * @param typeOfMedia
 */
void generateKey(string msgToEncode, string &keyForEncode, char typeOfMedia){

    int index =  rand()%10; // add extra complexity +10
    bool first = rand()%2;//just need true false

    int size  = msgToEncode.size();
    if(size>msgSize){
        cerr<< "Msg is too long max size is "<< size<< endl;
        msgToEncode = msgToEncode.substr(0,msgSize);
        cout<< "Msg will converted to : "<< msgToEncode<< endl;
    }

    char cch = getRandom();
    if (first ) {
        keyForEncode.push_back(typeOfMedia);
        keyForEncode.push_back(cch);
    } else {
        keyForEncode.push_back(cch);
        keyForEncode.push_back(typeOfMedia);
    }

    for (int i=0; i<size; i++) {
        cch = getRandom();
        if (first) {
            keyForEncode.push_back(msgToEncode[i]);
            keyForEncode.push_back(cch + index);
        } else {
            keyForEncode.push_back(cch + index);
            keyForEncode.push_back(msgToEncode[i]);
        }
    }
    populate(keyForEncode, first, size);
    keyForEncode.push_back(first);//adding F/L functionality
    keyForEncode.push_back(index);//add adding index as char
}
/**
 *
 * @param eKey
 * @param msg
 */
void generateMsg(string decodedKeyFromMedia, string &decodedMsg){
    int size  = decodedKeyFromMedia.size();
    int amend  = 0 ;
    cout<<decodedKeyFromMedia.at(size-1);
    cout<<decodedKeyFromMedia.at(size-2);
    int index = int (decodedKeyFromMedia.at(size-1));
    bool first = bool (decodedKeyFromMedia.at(size-2));
    if(!first) {
        amend = 1;
    }
    int msgSize = int(decodedKeyFromMedia.at(size-3-(!first?-1:0)));
    if(msgSize>0)
    for(int i=0;i<msgSize*2+2;){
        if(i==0){
            cout <<"taken from "<<((((int) (decodedKeyFromMedia.at(amend + i)))==1)?"Image":"Video")<<endl;
        }
        else {
            decodedMsg.push_back(decodedKeyFromMedia.at(amend + i));
        }
        i+=2;
    }
}
/**
 *
 * @param argv
 * @param typeOfMedia
 * @param outPutFileName
 * @param decodeKeyLocation
 * @param decodedMsg
 */
void decryptMedia(char** argv,int typeOfMedia, string &outPutFileName, string decodeKeyLocation, string &decodedMsg){
    ifstream inFile;
    if (decodeKeyLocation == "*") {
        decodeKeyLocation = argv[3];
    }
    inFile.open(decodeKeyLocation);
    if (!inFile) {
        cerr << "Unable to open file "<< decodeKeyLocation<< endl;
    }
    else {
        string decodedKey = "";
        string decodedKeyFromMedia = "";
        if (inFile.is_open()) {
            fstream f(decodeKeyLocation);
            stringstream iss;
            iss << f.rdbuf();
            decodedKey = iss.str();
        }
        if(typeOfMedia == 1){
            if (outPutFileName == "*") {
                outPutFileName = argv[5];
            }
            Mat img = imread(outPutFileName);
            decrypt( img,decodedKey,decodedKeyFromMedia);
        }
        else{
            if (outPutFileName == "*") {
                outPutFileName = argv[4];
            }
            VideoCapture inputVideo(outPutFileName);
            decrypt( inputVideo, decodedKey,decodedKeyFromMedia);
        }
        generateMsg(decodedKeyFromMedia,decodedMsg);

    }
}
string findMedia(int typeOfMedia){

    return typeOfMedia==1?"Image":"Video";
}
/**
 *
 * @param argv
 * @param typeOfMedia
 * @param fileName
 * @param keyForEncode
 * @param encodedKey
 * @param msgToEncode
 */
void encryption(char** argv,int typeOfMedia,string fileName,string &keyForEncode,string &encodedKey,string msgToEncode,string &outPutFileName){

    generateKey(msgToEncode,keyForEncode,typeOfMedia);

    if (typeOfMedia == 1) {

        Mat imageToEncode = imread(fileName);
        if (imageToEncode.empty()) {
            cerr<< "Not a valid image file "<< fileName<< endl;
        } else {
            encodeImage(imageToEncode, 1,keyForEncode,encodedKey);
            imwrite(outPutFileName, imageToEncode);
            cout<<"create file   in "<<outPutFileName;
        }
    } else {

        VideoCapture inputVideo(fileName);
        if (!inputVideo.isOpened()) {
            cerr<< "Error opening video stream or file "<< fileName<< endl;
        } else {
            VideoWriter outputVideo;
            createVideoHeading(inputVideo, outPutFileName,outputVideo);
            createVideo(inputVideo,outputVideo,keyForEncode,encodedKey);

            outputVideo.release();
            inputVideo.release();
            cout<<"create file   in "<<outPutFileName;
        }
    }
}
/**
 *
 * @param argv
 * @param decodeKeyLocation
 * @param encodedKey
 */
void writeKeyToFile(char** argv,string &decodeKeyLocation,string encodedKey){
    if (decodeKeyLocation == "*") {
        decodeKeyLocation = argv[3];
    }
    ofstream out(decodeKeyLocation);
    out<< encodedKey;
    out.close();
    cout<<"create file  for key in"<<decodeKeyLocation;
}
/**
 *
 * @param outPutFileName
 * @param fileName
 */
void getOutPutFile(string &outPutFileName,string fileName ){
    string::size_type pAt = fileName.find_last_of('.');                  // Find extension point
    const string NAME = fileName.substr( pAt+1,fileName.size());   // Form the new name with container
    outPutFileName = "./out."+NAME;
}

void convolutionFrame(Mat &imageToConvolute,float matrix[3][3],float edge,int frame ){
#pragma omp parallel
#pragma omp parallel for reduction(+:i,j)
    for(int i=0;i<imageToConvolute.rows;i++){
        for(int j=0;j<imageToConvolute.cols;j++){
            Mat_<float> custom(3, 3);
            cout<<"I*J "<<i<<"*"<<j<<" frame "<<frame<< " b - "<< imageToConvolute.at<float>(i,j);
            imageToConvolute.at<float>(i,j)=
                    + i<1 && j<1? edge : imageToConvolute.at<float>(i-1,j-1)*matrix[0][0]
                    + i<1 ?       edge: imageToConvolute.at<float>(i-1,j)*matrix[1][0]
                    + i<1 ?       edge: imageToConvolute.at<float>(i-1,j+1)*matrix[2][0]
                    + j<1 ?       edge: imageToConvolute.at<float>(i,j-1)*matrix[0][1]
                    + imageToConvolute.at<float>(i,j)*matrix[1][1]
                    + imageToConvolute.at<float>(i,j+1)*matrix[2][1]
                    + j<1 ?       edge: imageToConvolute.at<float>(i+1,j-1)*matrix[0][2]
                    + imageToConvolute.at<float>(i+1,j)*matrix[1][2]
                    + imageToConvolute.at<float>(i+1,j+1)*matrix[2][2];
            cout<<" - " << imageToConvolute.at<float>(i,j)<<endl;

        }

    }

}

/**
 *
 * @param typeOfMedia
 * @param fileName
 * @param matrix
 * @param edge
 * @param outPutFileName
 */
void convolution(int typeOfMedia,string fileName,float matrix[3][3],float edge,string outPutFileName){
    Mat_<float> custom(3, 3);
    if (typeOfMedia == 1) {
        Mat imageToConvolute = imread(fileName);
        if (imageToConvolute.empty()) {
            cerr<< "Not a valid image file "<< fileName<< endl;
        } else {
            convolutionFrame(imageToConvolute,matrix,edge,1 );
            imwrite(outPutFileName, imageToConvolute);
            cout<<"create file   in "<<outPutFileName;
        }
    } else {
        VideoCapture videoToConvolute(fileName);
        if (!videoToConvolute.isOpened()) {
            cerr<< "Error opening video stream or file "<< fileName<< endl;
        } else {
            VideoWriter outputVideo;
            createVideoHeading(videoToConvolute, outPutFileName,outputVideo);
            int i=-1;
#pragma omp parallel
#pragma omp while
            while (true) {
                Mat frame;
                videoToConvolute >> frame;
                if (frame.empty())
                    break;
                convolutionFrame(frame,matrix,edge,i++ );
                outputVideo<<frame;
                namedWindow("Creating Your video ", cv::WINDOW_AUTOSIZE);
                imshow("frames ", frame);
                waitKey(2);
            }
            outputVideo.release();
            videoToConvolute.release();
            cout<<"create file   in "<<outPutFileName;
        }
    }
}

/**
 *
 * @param argv
 * @param fileName
 * @param typeOfMedia
 */
void getFileType(char** argv, string &fileName,int typeOfMedia){
    if (fileName == "*") {
        fileName = typeOfMedia==1?  argv[1]:argv[2];
    }
}

/**
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char** argv) {

    int option;//1 for encode any for decode
    int typeOfMedia;//1 is for Image any for video
    string fileName;//fileName relative or absolute path
    string outPutFileName;


    string msgToEncode = "";
    string decodedMsg = "";
    string decodeKeyLocation = "";
    string encodedKeyLocation = "";
    string decodeKey = "";
    string keyForDecode = "";
    string keyForEncode = "";
    string encodedKey = "";
    float matrix[3][3];

    cout<< "0 - for apply filter  1- Encode 0- Decode"<< endl;
    cin>> option;
    cout<< " 1 -Image any number - Video" << endl;
    cin>> typeOfMedia;
    cout<< "Enter media file Name (* for dft) for process" << endl;
    cin>> fileName;

    if(option == 0) {
        getFileType(argv, fileName, typeOfMedia);
        getOutPutFile( outPutFileName,fileName );
        float edge;
        cout << " elements for filter : "<<endl;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                cout << " element in " << i + 1 << " * " << j + 1 << " : ";
                cin >> matrix[i][j];
            }
        }
        cout << " elements for edge : "<<endl;
        cin >> edge;
        convolution( typeOfMedia, fileName, matrix, edge, outPutFileName);
    }
    else if(option == 1) {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout<< "Enter message to encrypt" << endl;
        getline(cin, msgToEncode);
        cout<< "Enter decryption key save location * for dft " << endl;
        getline(cin, decodeKeyLocation);

        getFileType( argv,  fileName, typeOfMedia );
        getOutPutFile( outPutFileName,fileName );

        encryption(argv, typeOfMedia, fileName,  keyForEncode, encodedKey, msgToEncode,outPutFileName);

        cout<<"key to hide "<<keyForEncode<<endl;
        cout<< "Key to print "<<encodedKey<<endl;
        cout<<"\n********************************************\n"<<endl;

        writeKeyToFile(argv,decodeKeyLocation, encodedKey);


        cout<<"\n********************************"<<outPutFileName<<"************\n"<<endl;
        decryptMedia(argv, typeOfMedia, outPutFileName, decodeKeyLocation,  decodedMsg) ;

        cout<<"Hidden msg  "<<msgToEncode<<endl<<endl;
        cout<<"Massage found "<<decodedMsg<<endl;
    }
    else if(option ==2 ){
        outPutFileName = fileName;
        cout<< "Enter decryption key location * for dft" << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, decodeKeyLocation);
        decryptMedia(argv, typeOfMedia, outPutFileName, decodeKeyLocation,  decodedMsg) ;
        cout<< "Massage : "<< decodedMsg<< endl;
    }

    destroyAllWindows();
    fileName = "";
    outPutFileName = "";
    msgToEncode = "";
    decodedMsg = "";
    decodeKeyLocation = "";
    encodedKeyLocation = "";
    decodeKey = "";
    keyForDecode = "";
    keyForEncode = "";
    encodedKey = "";
    argv=NULL;
    return 0;
}
