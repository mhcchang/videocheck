#ifndef __GM_RECORD_CONFIG_H__
#define __GM_RECORD_CONFIG_H__

#include <cvcam/base/portable.h>
#include <cvcam/detectionsdk.h>
#include <cvcam/ws_object_server.h>
#include <string>
#include <vector>
#include <map>

#define CONFIG_FILE_PATH "./etc/config.json"
#define CONFIG_PATH "./etc/"

#ifdef CVCAM_WINDOWS
#define CVCAM_INFO_DIR  "C:/ProgramData/CvCam/info"
#else
#define CVCAM_INFO_DIR  "/etc/cvcam/info"
#endif

class Config
{
public:
	struct Global
	{
        std::string serviceId;
        std::string sceneId;
        std::string modelName;
        std::string apiType;
        std::string appId;
        std::string appSecret;
        std::string serverAddr;
        std::string serverPort;
        std::string httpPath;
        bool disableDetection;
        std::string detectionLevel;
        bool onlyKeyFrame;
        int frameInterval;
        int detectInterval;
        int saveInterval;
        double threshould;
        std::string outputFormat;
        int imageMinSize;
        double imageEnlargementRatio;
        int objectCountLimit;
        int bodySizeLimit;
        int statsInterval;
        int outputImage;
        int saveImage;
        std::string imagePath;
        int retentionPeriod;
        int freeDiskSpaceLimit;
        std::string duplicateAlgo;
        int decoderFlags;
	};

    struct CameraParams
    {
        int detectInterval;
        int saveInterval;
    };

    CameraParams referenceCameraParams;
    CameraParams detailCameraParams;

    cvcam::WsObjectServer::Params wsServerParams;

    struct MQParams
    {
        bool enabled;
        std::string host;
        int port;
        std::string username;
        std::string password;
        int queueMaxLength;
        int msgExpiredTime;
    };
    MQParams mqParams;

    struct MngParams
    {
        std::string host;
        std::string port;
        std::string url;

        int connectTimeout = 10;
        int idleTimeout = 30;
        int reconnectInterval = 10;
    };
    MngParams mngParams;

	struct Stream
	{
		int id;
		std::string name;
		std::string subRtspUrl;
		std::string mainRtspUrl;
        bool isReferenceCamera;
	};

    struct Detector
    {
        int gpuId;
        std::vector<int> streams;
        std::map<std::string, std::string> detectorParams;
        bool isImageExtractedInStandaloneThread; // extra info, used by MQClient
    };

private:
    std::map<std::string, std::string> detectorParams;
    bool isImageExtractedInStandaloneThread; // extra info, used by MQClient

public:
	Global global;
	std::vector<Stream> streams;
    std::vector<Detector> detectors;
    std::map<int, std::vector<cvcam::ObjectRect>> cameraRois;

	std::string m_strConfig;
public:
	bool load();
    bool save();
    bool getStreamById(int id, Stream& stream);

	//8.4
	bool ChangeCfg(std::string str);
	//8.5
	//bool ChangeReloadCfg(std::string str);
protected:
	//0 no changed, 1 immediate, 2 delay
	int Check(std::string device);
	bool parseMngInfo(const std::string& path, MngParams& mngParams);
};

#endif//__GM_RECORD_CONFIG_H__
