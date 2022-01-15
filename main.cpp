
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz~`!@#$%^&*(<>?:{},./;[]|";
/**
 *
 * @param numbers
 * @param num
 * @param size
 * @return
 */
set<int> genNumber(set<int> numbers, int num, int size){
    numbers.insert(rand()%num);
    if(numbers.size()==size){
        return genNumber(numbers,num,numbers.size());
    }
    return numbers;
}
/**
 *
 * @param count
 * @param rows
 * @param cols
 * @return
 */
set<int> genNumbers (int count,int rows ,int cols) {
    set<int> numbers;
    int i = 0 ;
    while (i!=count){
        numbers = genNumber(numbers,rows,numbers.size());
        numbers = genNumber(numbers,cols,numbers.size());
        i++;
    }
    return numbers;
}
/**
 *
 * @param img
 * @param frame
 * @param keyToHide
 * @param finalKey
 * @return
 */
Mat showImage(Mat img,int frame,string keyToHide,string &finalKey){
    cout <<"Frame : "<<frame<< " Rows :" << img.rows << " Cols : " << img.cols <<" key to hide "<<keyToHide<< endl;
    set<int> numbers = genNumbers(keyToHide.size(),img.rows,img.cols);
    cout<<" -- Hidden locations --"<<endl;
    for(int i=0;i<keyToHide.size();i++){
        cout<<" row: "<<*next(numbers.begin(),i*2)<< " cols : "<<*next(numbers.begin(),i*2+1)<<endl;
        Point_<int> po;
        po.y = *next(numbers.begin(),i*2);
        po.x= *next(numbers.begin(),i*2+1);
        img.at<uchar>(po)=keyToHide.at(i);
        finalKey.push_back(po.y);
        finalKey.push_back(po.x);
    }
    cout<<" --------"<<endl;
    cout<<" --------\n"<<finalKey<<"\n---------"<<endl;
    /*for (int i = 0; i < img.rows; i++) {//loop for rows//
        for (int j = 0; j < img.cols*3; j++) {
            Point_<int> po;
            po.y = i;
            po.x= j;
            int pixcel = (int) img.at<uchar>(po);
            img.at<uchar>(po) = pixcel;
        }
    }*/
    namedWindow("Creating Your video ", cv::WINDOW_AUTOSIZE);
    imshow("frames "+frame, img);
    waitKey(25);
    return img;
}
/**
 *
 * @param inputVideo
 * @param outputVideo
 * @param keyToHide
 * @param finalKey
 */
void createVideo(VideoCapture inputVideo,VideoWriter &outputVideo,string keyToHide,string &finalKey){
    int i = 1;
    int frameNumber = inputVideo.get(CAP_PROP_FRAME_COUNT);
    int modOfFrames = keyToHide.size()/frameNumber + 1;
    cout<<"number of frames : "<<frameNumber<<endl;
    cout<<"mod Of Frames : "<<modOfFrames<<endl;
    while (i++) {
        string key=keyToHide.substr(i,i==frameNumber?keyToHide.size():modOfFrames*(i+1));
        cout<<"key : "<<key<<endl;
        Mat frame;
        inputVideo >> frame;
        if (frame.empty())
            break;
        frame = showImage(frame, i - 1,key,finalKey);
        outputVideo << frame;
    }
}
/**
 *
 * @param inputVideo
 * @param videoName
 * @param outputName
 * @param outputVideo
 * @return
 */
VideoWriter createVideoHeading(VideoCapture inputVideo, char *videoName,String  outputName,VideoWriter &outputVideo){
    Size S = Size((int) inputVideo.get(CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) inputVideo.get(CAP_PROP_FRAME_HEIGHT));
    int ex = static_cast<int>(inputVideo.get(CAP_PROP_FOURCC));     // Get Codec Type- Int form
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    outputVideo.open(outputName, VideoWriter::fourcc('M', 'J', 'P', 'G') , inputVideo.get(CAP_PROP_FPS), S, true);
    if (!outputVideo.isOpened()){
        cout  << "Could not open the output video for write: " << videoName << endl;
    }
    cout << "Input frame resolution: Width=" << S.width << "  Height=" << S.height
         << " of nr#: " << inputVideo.get(CAP_PROP_FRAME_COUNT) << endl;
    cout << "Input codec type: " << EXT << endl;
    return outputVideo;
}
/**
 *
 * @return
 */
char getRandom(){
    const size_t max_index = (sizeof(charset) - 1);
    return charset[ rand() % max_index ];
}
/**
 *
 * @param key
 * @param first
 * @param size
 */
void populate(string &key,bool first,int size){
    if(key.size()<200){
        char cch =  getRandom();
        if (first) {
            key.push_back(cch);
            cch =  getRandom();
            key.push_back(cch);
        } else {
            key.push_back(cch);
            cch =  getRandom();
            key.push_back(cch);
        }
        populate(key,first,size);
    }
    else {
        cout << "missing key generating" << endl;
        char cch =  getRandom();
        if (first) {
            key.push_back(cch);
            key.push_back(size);
        } else {
            key.push_back(size);
            key.push_back(cch);
        }
        cout << "missing key size "<<size << endl;
    }

}
/**
 *
 * @param msg
 * @param dKey
 * @param dKeyTemp
 * @param type
 */
void  generateKey(string  msg,string &dKey,string &dKeyTemp,char type){
    char cch = getRandom();
    int index =  rand()%10;
    bool first = rand()%2;
    int size  = msg.size();
    if(size>100){
        cout<<"msg is too long "<<endl;
        msg = msg.substr(0,99);
        cout<<"msg will converted to :  "<<msg<<endl;
    }
    if (first ) {
        dKeyTemp.push_back(type);
        dKeyTemp.push_back(cch);
    } else {
        dKeyTemp.push_back(cch);
        dKeyTemp.push_back(type);

    }
    for (int i=0;i<size;i++){
        cch =getRandom();
        if (first) {
            dKeyTemp.push_back(msg[i]+index);
            dKeyTemp.push_back(cch+index);
        } else {
            dKeyTemp.push_back(cch+index);
            dKeyTemp.push_back(msg[i]+index);
        }
    }
    populate(dKeyTemp,first,size);
    cout<<"Key Generated befor index : "<<dKeyTemp<<endl;
    dKeyTemp.push_back(first);
    dKeyTemp.push_back(index);
    cout<<"Key Generated index: "<<index<<endl;
    cout<<"Key Generated first: "<<first<<endl;
    cout<<"Key Generated : "<<dKeyTemp<<endl;
}
/**
 *
 * @param eKey
 * @param msg
 */
void generateMsg(string eKey,string &msg,char &type){
    int size  = eKey.size();
    int amend  = 0 ;
    cout<<eKey.at(size-1);
    cout<<eKey.at(size-2);
    int index = int (eKey.at(size-1));
    bool first = bool (eKey.at(size-2));
    if(!first) {
        amend = 1;
    }
    int msgSize = int(eKey.at(size-3-(!first?-1:0)));
    for(int i=0;i<msgSize*2+2;){
        if(i==0){
            type = int (eKey.at(amend + i)-index)==1?'I':'V';
        }
        else {
            msg.push_back(eKey.at(amend + i)-index);
        }
        i+=2;
    }
    cout<<"eKey is :"<<eKey<<endl;
    cout<<"msg is :"<<msg<<endl;
    cout<<"it's is :"<<type<<endl;
}
/**
 *
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char** argv) {
    int option;
    int type;
    char typeDecoded;
    const size_t BUFFER_SIZE = 1024;
    char *fileName= new char[BUFFER_SIZE];
    string key = "";
    string msg= "";
    string dKey= "";
    string finalKey="";
    cout<<"Option 1- Encode 2- decode"<<endl;
    cin>>option;
    cout << "Option 1 -Image any number - video" << endl;
    cin >> type;
    if(option == 1) {
        cout << "Enter file Name (* for dft)" << endl;
        cin >> fileName;
        cout << "Enter encryption key" << endl;
        cin >> key;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter message to encrypt" << endl;
        getline(cin, msg);
        generateKey(msg,key,dKey,type);
        msg = "";
        //generateMsg(dKey,msg,typeDecoded);
        if (type == 1) {
            if (strcmp(fileName, "*") == 0) {
                fileName = argv[1];
            }
            Mat img = imread(fileName);
            if (img.empty()) {
                cout << "Not a valid image file" << endl;
            } else {
                Mat outImg =  showImage(img, 1,dKey,finalKey);
                imwrite("./out.jpg", outImg);
            }
        } else {
            if (strcmp(fileName, "*") == 0) {
                fileName = argv[2];
            }
            VideoCapture inputVideo(fileName);
            if (!inputVideo.isOpened()) {
                cout << "Error opening video stream or file" << endl;
            } else {
                VideoWriter outputVideo;
                createVideoHeading(inputVideo, fileName, "./out.avi",outputVideo);
                createVideo(inputVideo,outputVideo,dKey,finalKey);
                outputVideo.release();
                inputVideo.release();
            }
        }
        std::ofstream out("./output.txt");
        out << finalKey;
        out.close();
    }
    else{
        cout << "Enter file Name and location " << endl;
        cin >> fileName;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter decryption key" << endl;
        getline(cin, dKey);
        generateMsg(dKey,msg,typeDecoded);
    }
    destroyAllWindows();
    fileName = NULL;
    return 0;
}
