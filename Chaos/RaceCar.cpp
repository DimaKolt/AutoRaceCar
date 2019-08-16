#include "RaceCar.h"
#include <vector>
#include "Arduino.h"
#include <thread>
#include "zlib.h"


#include "iostream"

//#define DEBUG_MODE
#define SERVER_PORT 5556
#define MAX_NUM_USERS 5
RaceCar::RaceCar()
{
    _motor_control = std::make_shared<Arduino>();
//    _bitcraze = std::make_shared<Bitcraze>();
    _tcp_client = ITcpClient::create();
    _tcp_server = ITcpServer::create();

    _camera_thread = nullptr;
    _carcontrol_thread = nullptr;
    _bitcraze_thread = nullptr;
    _is_running = true;

}

RaceCar::~RaceCar()
{

    std::cout << "enter diestruction" <<std::endl;
    if (_camera_thread){
        std::cout << "camera destructor" <<std::endl;
         _camera_thread->join();
         std::cout << "camera destruction finished" <<std::endl;
     }
    if (_carcontrol_thread){
        std::cout << "serial destructor" <<std::endl;
        _motor_control->stop();

        _carcontrol_thread->join();
        std::cout << "seial destruction finished" <<std::endl;
     }
    if (_bitcraze_thread){
        std::cout << "bitcraze destructor" <<std::endl;
//        _bitcraze->stop();

        _bitcraze_thread->join();
        std::cout << "bitcraze destruction finished" <<std::endl;

    }
}

RaceCar &RaceCar::connect(const string& ip, const unsigned short& port,const string& server_ip)
{
    std::cout << "enter conect()" <<std::endl;



    if(_camera.connectCamera()){
        _is_cammera_connected = true;
        std::cout << "Camera ON" <<std::endl;
    } else {
        std::cout << "Camera NOT CONNECTED" <<std::endl;
    }

    if ( _camera.isConnect() ){
         std::cout << "try _tcp_client" <<std::endl;
         _tcp_client->connect(ip, port);
         std::cout << "Camera connected" <<std::endl;
    } else {
        std::cout << "Camera NOT CONNECTED" <<std::endl;
    }
    if(_tcp_client->isConnected()){
        _is_tcp_client_connected = true;
        std::cout << "connected to sever" <<std::endl;
    } else {
        std::cout << "server NOT CONNECTED" <<std::endl;
    }

    if(_motor_control->connect()){
        _is_motor_control_connected = true;
        std::cout << "connected to motor Conmtrol" <<std::endl;
    } else {
        std::cout << "motor Conmtrol NOT CONNECTED" <<std::endl;
    }

    if(_bitcraze.connect()){
        _is_bitcraze_connected = true;
        std::cout << "connected to Bitcraze" <<std::endl;
    } else {
        std::cout << "bitcraze NOT CONNECTED" <<std::endl;
    }



    _tcp_server->bind(server_ip,SERVER_PORT,MAX_NUM_USERS); //to allow athers to connect
    if(_tcp_server->isBind()){
        std::cout << "bind success" << std::endl;
        _tcp_server->setUnblocking(true);
        _is_tcp_server_connected = true;
    } else {
        std::cout << "bind FAILED" <<std::endl;
    }

    std::cout << "end connections" <<std::endl;
    return *this;

}

RaceCar &RaceCar::run()
{
    std::cout << "enter run" <<std::endl;
    if(_is_tcp_client_connected && _is_cammera_connected){
            _camera.setupColorImage(RealSense::ColorFrameFormat::RGB8,RealSense::ColorRessolution::R_640x480, RealSense::ColorCamFps::F_30hz);
            _camera.setupDepthImage(RealSense::DepthRessolution::R_480x270, RealSense::DepthCamFps::F_30hz);
            _camera.setupGyro();
            _camera.setupAccel();
            std::cout << "Camera setuped" <<std::endl;
            _camera.startCamera();
            std::cout << "Camera started" <<std::endl;
            _camera_thread = std::make_shared<std::thread>(&RaceCar::getCameraOutput,this);
    }
    if(_is_motor_control_connected && _tcp_server->isBind()){
            std::cout << "connected to Motor Control" <<std::endl;
            _carcontrol_thread = std::make_shared<std::thread>(&RaceCar::arduinoCommunications,this);
    }
    if(_is_bitcraze_connected){
        std::cout << "connected to BitCraze" <<std::endl;
        _bitcraze_thread = std::make_shared<std::thread>(&RaceCar::getBitCrazeOutput,this);

    }
}


RaceCar &RaceCar::parseCmdString(const char cmd)
{

    switch (cmd) {

    case 's': {
        _motor_control->stop();
        break;
    }
    case 'u': {
        _motor_control->changeSpeedBy(1);
        break;
    }
    case 'd': {
        _motor_control->changeSpeedBy(-1);
        break;
    }
    case 'l': {
        _motor_control->changeAngleBy(-2);
        break;
    }
    case 'r': {
        _motor_control->changeAngleBy(2);
        break;
    }
    case 'q': {
        _is_running=false;
        break;
    }
//    default: _motor_control->stop();
    }

}

RaceCar &RaceCar::arduinoCommunications()
{
    //TODO in case of crash, arduino should stop itself
    //TODO avoid terminate from write
    //TODO ardoino functions for "#"
    std::cout << "waiting for connection" << std::endl;


    while (_is_running)
    {
        if(_tcp_server->hasConnectionWithSocket(_socket))
        {
            char cmd = ' ';
            _tcp_server->receive(_socket,&cmd, 1);
            parseCmdString(cmd);
//            std::cout << cmd << std::endl;

        }

        else
        {
            _socket = _tcp_server->waitForConnections(1);
            std::cout << "wait to asafff" << std::endl;
            if(_socket > 0)
            {
                _tcp_server->setClientUnblocking(_socket,true);
                std::cout << "someone connected" << std::endl;
            }
        }

    }
    std::cout <<"CarControl thread finished" << std::endl;
    _motor_control->stop();
    return *this;
}

PacketToRemote::ColorDataAndPeriphelSensors RaceCar::buildColorPacket(const Camera::ColorImage &image){

    PacketToRemote::ColorDataAndPeriphelSensors packet = {};

    packet.accel_data = _camera.getAccelData();
    packet.euler_angl = _camera.getEulerAngels();

    _flow_mtx.lock();
    packet.flow_data = _flow_data;
    _flow_mtx.unlock();

    packet.image.frame_num = image.frame_num;
    packet.image.height = image.height;
    packet.image.width = image.width;
    packet.image.size = image.size;
    packet.image.timestamp_ms = image.timestamp_ms;

    _jpeg_comp.compress(image.data);
    packet.image.compressed_size = _jpeg_comp.getCompressedSize();
    packet.image.compresed_data = _jpeg_comp.getOutput();

    return packet;
}

PacketToRemote::header RaceCar::buildColorHeader(){
    PacketToRemote::header header = {};
    header.type_code = PacketToRemote::COLOR_HEADER;
    header.total_size = sizeof(PacketToRemote::ColorDataAndPeriphelSensors)-sizeof(PacketToRemote::ColorImage::compresed_data);
    return header;
}

RaceCar &RaceCar::getCameraOutput()
{
    std::cout << "enter Camera thread" <<std::endl;
    while (_is_running && _tcp_client->isConnected())
    {
//        std::cout << _is_running <<std::endl;
        _camera.captureFrame();
        Camera::ColorImage image=_camera.getColorImage();

        PacketToRemote::ColorDataAndPeriphelSensors packet = buildColorPacket(image);
        PacketToRemote::header header = buildColorHeader();

        _tcp_client->send(reinterpret_cast<char*>(&header), sizeof(header));
        _tcp_client->send(reinterpret_cast<char*>(&packet), header.total_size);
        _tcp_client->send(reinterpret_cast<char*>(packet.image.compresed_data), packet.image.compressed_size);

    }
    std::cout << "Camera thread finished" <<std::endl;
    return *this;
}

RaceCar &RaceCar::getBitCrazeOutput()
{
    std::cout << "enter BitCraze thread" << std::endl;
    _bitcraze.requestFlowData();
    while (_is_running) {
        Flow flow_data = _bitcraze.getFlowOutput(); //TODO send flow to asaf
//        std::cout << flow_data.deltaX <<"..." << flow_data.deltaY << "..."<<flow_data.range << "..." <<flow_data.mili_sec << std::endl;

        std::lock_guard<std::mutex> lock(_flow_mtx);
        _flow_data = flow_data;

    }
    std::cout << "BitCraze thread finished" << std::endl;
}

//RaceCar  &RaceCar::sendFlowOutput(Flow data)
//{
//    char* ptr = (char*) &data;
//    _tcp_server->send(_socket,ptr,sizeof(Flow));
//}
//static void splitString(const string &str, std::vector<string> &output)
//{
//    string::size_type start = 0; // Where to start
//    string::size_type last = str.find_first_of(" "); // Finds the first space

//    // npos means that the find_first_of wasn't able to find what it was looking for
//    // in this case it means it couldn't find another space so we are at the end of the
//    // words in the string.
//    while (last != string::npos)
//    {
//        // If last is greater then start we have a word ready
//        if (last > start)
//        {
//            output.push_back(str.substr(start, last - start)); // Puts the word into a vector look into how the method substr() works
//        }

//        start = ++last; // Reset start to the first character of the next word
//        last = str.find_first_of(" ", last); // This means find the first space and we start searching at the first character of the next word
//    }

//    // This will pickup the last word in the file since it won't be added to the vector inside our loop
//    output.push_back(str.substr(start));
//}


