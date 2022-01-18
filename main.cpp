
#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

const size_t BUFFER_SIZE = 1024;
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

void decodeImage(Mat img,int x,int y,int frame,string &decodedKey){
    Point_<int> po;
    Point_<int> po1;
    po.y = y;
    po.x = x;
    po1.y=po.y+1;
    po1.x=po.x+1;
    unsigned int key =( unsigned int )img.at<uchar>(po);
    unsigned int  key1 =( unsigned int )img.at<uchar>(po1);
    cout <<"Frame : "<<frame<< " po.y "<<po.y<<" "<<" po.x "<<po.x<<" key1 "<< key1 <<" key "<<key<<endl;
    key = key + key1*255;
    decodedKey.push_back((unsigned char )key);
}


void generate(VideoCapture inputVideo,string key,string &decodedKey){
    int i = -1;
    int frameNumber = inputVideo.get(CAP_PROP_FRAME_COUNT);
    int numberOfHiddenFrames = key.size()/6;
    cout<<"number of frames : "<<frameNumber<<endl;
    cout<<"number of Hidden Frames : "<<numberOfHiddenFrames<<endl;
    while (true) {
        i++;
        Mat frame;
        inputVideo >> frame;
        if (frame.empty() || i>=numberOfHiddenFrames)
            break;
        if(i<=numberOfHiddenFrames) {
             int y = (unsigned char)key.at(i*6);
            y = y+ (unsigned char) key.at(i*6+1)*255;
             int x =  (unsigned char)key.at(i*6+2);
            x = x+ (unsigned char)key.at(i*6+3)*255;
             int frameNo =  (unsigned char) key.at(i*6+4);
            frameNo = frameNo+ (unsigned char)key.at(i*6+5)*255;
            decodeImage(frame,x,y,frameNo,decodedKey);
        }
    }
}

void generate(Mat image,string key,string &decodedKey){
    int i = -1;
    int numberOfHiddenFrames = key.size()/3;
    cout<<"number of Hidden Frames : "<<numberOfHiddenFrames<<endl;
    while (i++) {
        if (i>numberOfHiddenFrames)
            break;
        if(i<=numberOfHiddenFrames) {
            int y = (unsigned char)key.at(i*6);
            y = y+ (unsigned char) key.at(i*6+1)*255;
            int x =  (unsigned char)key.at(i*6+2);
            x = x+ (unsigned char)key.at(i*6+3)*255;
            int frameNo =  (unsigned char) key.at(i*6+4);
            frameNo = frameNo+ (unsigned char)key.at(i*6+5)*255;
            decodeImage(image,x,y,frameNo,decodedKey);
        }
    }
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
    if(keyToHide!=""  && keyToHide.size()>0) {
        set<int> numbers = genNumbers(keyToHide.size(), img.rows, img.cols);
        for (int i = 0; i < keyToHide.size(); i++) {
            Point_<int> po;
            Point_<int> po1;
            po.y = *next(numbers.begin(), i * 2);
            po.x = *next(numbers.begin(), i * 2 + 1);
            po1.y=po.y+1;
            po1.x=po.x+1;

            unsigned int kyInt = keyToHide.at(i);
            unsigned int kryC = kyInt%255;
            img.at<uchar>(po) = (kyInt/255);
            img.at<uchar>(po1) = kryC;

            unsigned char y = po.y%255;
            unsigned char x = po.x%255;
            unsigned char f = frame%255;
            finalKey.push_back(y);
            finalKey.push_back(po.y/255);
            finalKey.push_back(x);
            finalKey.push_back(po.x/255);
            finalKey.push_back(frame);
            finalKey.push_back(f/255);
            cout <<"Frame : "<<frame<< " Rows :" << img.rows << " Cols : " << img.cols <<" key to hide "<<keyToHide.at(i)<<" kyInt "<<kyInt/255 <<" kryC " <<kryC<<" po.y "<<po.y<<" "<<po.y/255 <<" po.x "<<po.x<<" "<<po.x/255<<endl;

        }
        namedWindow("Creating Your video", cv::WINDOW_AUTOSIZE);
        imshow("frames", img);
        waitKey(20);
    }
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
        Mat frame;
        inputVideo >> frame;
        if (frame.empty())
            break;
        if(modOfFrames*(i)<=keyToHide.size()) {
            string key = keyToHide.substr(i*modOfFrames, 1);
            frame = showImage(frame, i - 1, key, finalKey);
        }
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
    if(key.size()<50){
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
        char cch =  getRandom();
        if (first) {
            key.push_back(cch);
            key.push_back(size);
        } else {
            key.push_back(size);
            key.push_back(cch);
        }
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
    if(size>25){
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
void generateMsg(string eKey,string &msg){
    int size  = eKey.size();
    int amend  = 0 ;
    char type;
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
    cout<<"Key is :"<<eKey<<endl;
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
    int option,type;
    char *fileName= new char[BUFFER_SIZE];

    string key = "";
    string msg= "";
    string dKeyLocation= "";
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
        cout<<"key to hide "<<dKey<<endl;
        msg = "";
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
        cout<<finalKey<<endl;
        ofstream out("./output.txt");
        out << finalKey;
        out.close();





        cout << "Enter file Name and location * for dft" << endl;
        cin >> fileName;
        cout << "Enter decryption key location * for dft" << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, dKeyLocation);
        ifstream inFile;
        if (dKeyLocation == "*") {
            dKeyLocation = argv[3];
        }
        inFile.open(dKeyLocation);
        if (!inFile) {
            cerr << "Unable to open file datafile.txt";

        }
        else {
            dKey = "";

            cout<<finalKey<<endl;
            if(type ==1){
                if (strcmp(fileName, "*")== 0) {
                    fileName = argv[5];
                }
                Mat img = imread(fileName);
                generate( img,finalKey,dKey);
            }
            else{
                if (strcmp(fileName, "*")== 0) {
                    fileName = argv[4];
                }
                VideoCapture inputVideo(fileName);
                generate( inputVideo, finalKey,dKey);
            }
            cout<<"Final key : "<<dKey<<endl;
            generateMsg(dKey,msg);
        }
    }
    else{
        cout << "Enter file Name and location * for dft" << endl;
        cin >> fileName;
        cout << "Enter decryption key location * for dft" << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, dKeyLocation);
        ifstream inFile;
        if (dKeyLocation == "*") {
            dKeyLocation = argv[3];
        }
        inFile.open(dKeyLocation);
        if (!inFile) {
            cerr << "Unable to open file datafile.txt";

        }
        else {
            dKey = "";
            if (inFile.is_open()) {
                ifstream t("file.txt");
                stringstream buffer;
                buffer << t.rdbuf();
                while (!inFile.eof()) {
                    string output;
                    inFile >> output;
                    dKey.append(output);
                }
            }
            cout<<dKey<<endl;
            if(type ==1){
                if (strcmp(fileName, "*")== 0) {
                    fileName = argv[5];
                }
                Mat img = imread(fileName);
                generate( img,dKey,finalKey);
            }
            else{
                if (strcmp(fileName, "*")== 0) {
                    fileName = argv[4];
                }
                VideoCapture inputVideo(fileName);
                generate( inputVideo, dKey,finalKey);
            }
            cout<<"Final key : "<<finalKey<<endl;
            generateMsg(finalKey,msg);
        }
    }
    destroyAllWindows();
    fileName = NULL;
    key = "";
    msg= "";
    dKeyLocation= "";
    dKey= "";
    finalKey="";
    return 0;
}
