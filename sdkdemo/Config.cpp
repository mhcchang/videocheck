#include "Config.h"

#include <cvcam/base/portable.h>
#include <cvcam/base/logger.h>
#include <cvcam/base/string_utils.h>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/utils.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <fstream>
#include <experimental/filesystem>

namespace std {
    namespace fs = std::experimental::filesystem;
}

int Config::Check(std::string device)
{
	int nres = -1;
	std::string strPath = "./etc/scene.json";

	std::ifstream ifs1(strPath);
	rapidjson::IStreamWrapper isw1(ifs1);

	rapidjson::Document doc;
	doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(isw1);
	if (doc.HasParseError() || !doc.IsObject())
	{
		CVCAM_WARN("[Config] load scene info failed");
		return nres;
	}

	std::string sceneId = "";
	std::string sceneName = "";
	std::string deviceId = "";

	if (!doc.HasMember("sceneId"))
	{
		CVCAM_WARN("[Config] load scene info failed");
		return nres;
	}

	sceneId = doc["sceneId"].GetString();
	sceneName = doc["sceneName"].GetString();
	deviceId = doc["deviceId"].GetString();

	if (deviceId == "")
	{
		CVCAM_WARN("[Config] load device info failed");
		return nres;
	}
	doc.Clear();

	//device inf
	strPath = "./etc/device.json";
	std::fstream ifs(strPath);
	rapidjson::IStreamWrapper isw(ifs);
	doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(isw);
	if (doc.HasParseError() || !doc.IsObject())
	{
		CVCAM_WARN("[Config] load device info failed");
		return nres;
	}

	if (!doc.HasMember("deviceId"))
	{
		CVCAM_WARN("[Config] load device info failed");
		return nres;
	}

	if (doc["deviceId"].GetString() != deviceId)
	{
		CVCAM_WARN("[Config] load device info failed id not match");
		return nres;
	}
	int forwardRtspPort = 0;
	if (!doc.HasMember("forwardRtspPort") || !doc["forwardRtspPort"].IsInt())
	{
		CVCAM_WARN("[Config] load device info failed, reference info error");
		return nres;
	}
	forwardRtspPort = doc["forwardRtspPort"].GetInt();
	if (!doc.HasMember("reference") || !doc["reference"].IsArray())
	{
		CVCAM_WARN("[Config] load device info failed, reference info error");
		return nres;
	}
	//!!!!
	nres = 0;

	std::vector<Stream> streamsNew;
	rapidjson::Value arrReference = doc["reference"].GetArray();
	int ll = arrReference.Size();
	for (int k1 = 0; k1 < (int)arrReference.Size(); k1++)
	{
		if (arrReference[k1].IsObject())
		{
			Stream streamtmp;
			int id = -1;
			if (arrReference[k1].HasMember("cameraId") && arrReference[k1]["cameraId"].IsInt())
			{
				id = arrReference[k1]["cameraId"].GetInt();
			}
			else
				continue;

			//找到原来的流信息  返回值+1
			bool blFound = getStreamById(id, streamtmp);
			if (blFound)
				nres = 1;

			streamtmp.id = arrReference[k1]["cameraId"].GetInt();
			if (arrReference[k1].HasMember("cameraIndex") && arrReference[k1]["cameraIndex"].IsInt())
			{
				streamtmp.name = arrReference[k1]["cameraIndex"].GetString();
				
			}
			else
				continue;

			std::string ip = "";
			if (arrReference[k1].HasMember("ip") && arrReference[k1]["ip"].IsString())
			{
				ip = arrReference[k1]["ip"].GetString();
			}
			std::string transMainUrl = "";
			if (arrReference[k1].HasMember("transMainUrl") && arrReference[k1]["transMainUrl"].IsString())
			{
				transMainUrl = arrReference[k1]["transMainUrl"].GetString();
			}
			std::string transSubUrl = "";
			if (arrReference[k1].HasMember("transSubUrl") && arrReference[k1]["transSubUrl"].IsString())
			{
				transSubUrl = arrReference[k1]["transSubUrl"].GetString();
			}
			streamtmp.isReferenceCamera = true;

			streamtmp.mainRtspUrl = "rtsp://" + ip + std::to_string(forwardRtspPort) + transMainUrl;
			streamtmp.subRtspUrl = "rtsp://" + ip + std::to_string(forwardRtspPort) + transSubUrl;
			streamsNew.push_back(streamtmp);
		}
	}

	rapidjson::Value arrDetail = doc["detail"].GetArray();
	ll = arrDetail.Size();
	for (int k2 = 0; k2 < (int)arrDetail.Size(); k2++)
	{
		if (arrDetail[k2].IsObject())
		{
			Stream streamtmpdetail;
			int id = -1;
			if (arrDetail[k2].HasMember("cameraId") && arrDetail[k2]["cameraId"].IsInt())
			{
				id = arrDetail[k2]["cameraId"].GetInt();
			}

			else
				continue;

			//找到原来的流信息  返回值+2
			bool blFound = getStreamById(id, streamtmpdetail);
			if (blFound)
				nres |= 2;

			streamtmpdetail.id = arrReference[k2]["cameraId"].GetInt();
			if (arrDetail[k2].HasMember("cameraIndex") && arrDetail[k2]["cameraIndex"].IsInt())
			{
				streamtmpdetail.name = arrDetail[k2]["cameraIndex"].GetString();
			}
			else
				continue;

			std::string ip = "";
			if (arrDetail[k2].HasMember("ip") && arrDetail[k2]["ip"].IsString())
			{
				ip = arrDetail[k2]["ip"].GetString();
			}
			std::string transMainUrl = "";
			if (arrDetail[k2].HasMember("transMainUrl") && arrDetail[k2]["transMainUrl"].IsString())
			{
				transMainUrl = arrDetail[k2]["transMainUrl"].GetString();
			}
			std::string transSubUrl = "";
			if (arrDetail[k2].HasMember("transSubUrl") && arrDetail[k2]["transSubUrl"].IsString())
			{
				transSubUrl = arrDetail[k2]["transSubUrl"].GetString();
			}
			streamtmpdetail.isReferenceCamera = false;

			streamtmpdetail.mainRtspUrl = "rtsp://" + ip + std::to_string(forwardRtspPort) + transMainUrl;
			streamtmpdetail.subRtspUrl = "rtsp://" + ip + std::to_string(forwardRtspPort) + transSubUrl;
			streamsNew.push_back(streamtmpdetail);
		}
	}

	//save new
	streams.clear();
	streams.assign(streamsNew.begin(), streamsNew.end());
	global.sceneId = sceneId;
	save();

	return nres;
}

bool Config::load()
{
    if (!parseMngInfo(CVCAM_INFO_DIR "/mng/mng.json", mngParams))
    {
        CVCAM_WARN("[Config] load mng info failed");
    }
	//reset configure file
	Check("");

    std::ifstream ifs(CONFIG_FILE_PATH);
    rapidjson::IStreamWrapper isw(ifs);

    rapidjson::Document doc;
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(isw);


	rapidjson::StringBuffer buf;
	rapidjson::Writer<rapidjson::StringBuffer> jWriter(buf);
	doc.Accept(jWriter);

	m_strConfig = buf.GetString();

    if (doc.HasParseError())
        return false;

    if (!doc.IsObject())
        return false;

    if (!doc.HasMember("global"))
        return false;

	if (!doc["global"].IsObject())
		return false;

    rapidjson::GetOptionalStringMember(doc["global"], "serviceId", global.serviceId);
    rapidjson::GetOptionalStringMember(doc["global"], "sceneId", global.sceneId);

    if (doc["global"].HasMember("workMode"))
        global.modelName = doc["global"]["workMode"].GetString();
    else
        global.modelName = "object";

    if (doc["global"].HasMember("apiType"))
        global.apiType = doc["global"]["apiType"].GetString();

    if (doc["global"].HasMember("appId"))
        global.appId = doc["global"]["appId"].GetString();

    if (doc["global"].HasMember("appSecret"))
        global.appSecret = doc["global"]["appSecret"].GetString();

    if (doc["global"].HasMember("serverAddr"))
        global.serverAddr = doc["global"]["serverAddr"].GetString();

    if (doc["global"].HasMember("serverPort"))
    {
        if (doc["global"]["serverPort"].IsString())
            global.serverPort = doc["global"]["serverPort"].GetString();
        else
            global.serverPort = std::to_string(doc["global"]["serverPort"].GetInt());
    }
    else
        global.serverPort = "80";

    if (doc["global"].HasMember("httpPath"))
        global.httpPath = doc["global"]["httpPath"].GetString();
    else
        global.httpPath = "/";    

    if (doc["global"].HasMember("disableDetection"))
        global.disableDetection = doc["global"]["disableDetection"].GetInt() != 0;
    else
        global.disableDetection = false;

    if (doc["global"].HasMember("detectionLevel"))
        global.detectionLevel = doc["global"]["detectionLevel"].GetString();

    if (doc["global"].HasMember("onlyKeyFrame"))
        global.onlyKeyFrame = doc["global"]["onlyKeyFrame"].GetInt() != 0;
    else
        global.onlyKeyFrame = false;

    if (doc["global"].HasMember("frameInterval"))
        global.frameInterval = doc["global"]["frameInterval"].GetInt();
    else
        global.frameInterval = 0;

    if (doc["global"].HasMember("detectInterval"))
        global.detectInterval = doc["global"]["detectInterval"].GetInt();
    else
        global.detectInterval = 0;

    if (doc["global"].HasMember("saveInterval"))
        global.saveInterval = doc["global"]["saveInterval"].GetInt();
    else
        global.saveInterval = global.detectInterval;

    if (doc["global"].HasMember("threshould"))
        global.threshould = doc["global"]["threshould"].GetDouble();
    else
        global.threshould = 0.0;

    if (doc["global"].HasMember("outputFormat"))
        global.outputFormat = doc["global"]["outputFormat"].GetString();
    else
        global.outputFormat = "bmp";

    if (doc["global"].HasMember("imageMinSize"))
        global.imageMinSize = doc["global"]["imageMinSize"].GetInt();
    else
        global.imageMinSize = 0;

    if (doc["global"].HasMember("imageEnlargementRatio"))
        global.imageEnlargementRatio = doc["global"]["imageEnlargementRatio"].GetDouble();
    else
        global.imageEnlargementRatio = 0.0;

    if (doc["global"].HasMember("objectCountLimit"))
        global.objectCountLimit = doc["global"]["objectCountLimit"].GetInt();
    else
        global.objectCountLimit = 0;

    if (doc["global"].HasMember("bodySizeLimit"))
        global.bodySizeLimit = doc["global"]["bodySizeLimit"].GetInt();
    else
        global.bodySizeLimit = 0;

    if (doc["global"].HasMember("saveImage"))
        global.saveImage = doc["global"]["saveImage"].GetInt();
    else
        global.saveImage = 0;

    rapidjson::GetOptionalStringMember(doc["global"], "imagePath", global.imagePath);
    if (global.imagePath.empty())
        global.imagePath = "images";
    rapidjson::GetOptionalIntMember(doc["global"], "retentionPeriod", global.retentionPeriod, 1);
    rapidjson::GetOptionalIntMember(doc["global"], "freeDiskSpaceLimit", global.freeDiskSpaceLimit, 10);

    if (doc["global"].HasMember("statsInterval"))
        global.statsInterval = doc["global"]["statsInterval"].GetInt();
    else
        global.statsInterval = 600;

    global.decoderFlags = 0;

    if (doc["global"].HasMember("squareRoi"))
    {
        int squareRoi = doc["global"]["squareRoi"].GetInt();
        if (squareRoi)
            global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_SQUARE_ROI;
    }

    if (doc["global"].HasMember("boundedByBorder"))
    {
        int boundedByBorder = doc["global"]["boundedByBorder"].GetInt();
        if (boundedByBorder)
            global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_BOUNDED_BY_BORDER;
    }

    int removeOverlapped = 1;
    if (doc["global"].HasMember("removeOverlapped"))
        removeOverlapped = doc["global"]["removeOverlapped"].GetInt();
    if (removeOverlapped)
        global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_REMOVE_OVERLAPPED_OBJECT;

    int sortByProb = 1;
    if (doc["global"].HasMember("sortByProb"))
        sortByProb = doc["global"]["sortByProb"].GetInt();
    if (sortByProb)
        global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_SORT_BY_PROB;

    if (doc["global"].HasMember("outputImage"))
        global.outputImage = doc["global"]["outputImage"].GetInt();
    if (!global.outputImage && !global.saveImage)
        global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_NO_IMAGE;

    if (doc["global"].HasMember("removeDuplicate"))
    {
        int removeDuplicate = doc["global"]["removeDuplicate"].GetInt();
        if (removeDuplicate)
            global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_REMOVE_DUPLICATE_OBJECT;
    }

    if (doc["global"].HasMember("duplicateAlgo"))
        global.duplicateAlgo = doc["global"]["duplicateAlgo"].GetString();

    detectorParams["object_idFormat"] = "short_uuid"; // default
    detectorParams["outputImage_standaloneThread"] = "true";
    isImageExtractedInStandaloneThread = true;
    if (doc.HasMember("detectorParams") && doc["detectorParams"].IsObject())
    {
        rapidjson::Value jParams = doc["detectorParams"].GetObject();

        for (auto it = jParams.MemberBegin(); it != jParams.MemberEnd(); ++it)
        {
            std::string name = it->name.GetString();
            std::string value = ToString(it->value);
            detectorParams[name] = value;

            if (name == "outputImage_standaloneThread")
            {
                isImageExtractedInStandaloneThread = cvcam::base::strToBool(value);
            }
        }
    }

    if (doc.HasMember("wsserver"))
    {
        rapidjson::Value jServer = doc["wsserver"].GetObject();

        if (jServer.HasMember("host"))
            wsServerParams.host = jServer["host"].GetString();

        if (jServer.HasMember("port"))
            wsServerParams.port = (unsigned short)jServer["port"].GetInt();
    }

    referenceCameraParams.detectInterval = global.detectInterval;
    referenceCameraParams.saveInterval = global.saveInterval;
    if (doc.HasMember("referenceCameraParams"))
    {
        rapidjson::Value jCameraParams = doc["referenceCameraParams"].GetObject();

        rapidjson::GetIntMember(jCameraParams, "detectInterval", referenceCameraParams.detectInterval);
        rapidjson::GetIntMember(jCameraParams, "saveInterval", referenceCameraParams.saveInterval);
    }

    detailCameraParams.detectInterval = global.detectInterval;
    detailCameraParams.saveInterval = global.saveInterval;
    if (doc.HasMember("detailCameraParams"))
    {
        rapidjson::Value jCameraParams = doc["detailCameraParams"].GetObject();

        rapidjson::GetIntMember(jCameraParams, "detectInterval", detailCameraParams.detectInterval);
        rapidjson::GetIntMember(jCameraParams, "saveInterval", detailCameraParams.saveInterval);
    }

	if (!doc["detailCameras"].IsArray())
		return false;

	for (rapidjson::SizeType i = 0; i < doc["detailCameras"].Size(); ++i)
	{
		rapidjson::Value jStream = doc["detailCameras"][i].GetObject();

		if (!jStream.HasMember("subRtspURL"))
			return false;
		if (!jStream.HasMember("mainRtspURL"))
			return false;

		Stream stream;

		if (jStream.HasMember("id"))
			stream.id = jStream["id"].GetInt();
		else
			stream.id = -1;

		stream.name = jStream["name"].GetString();
		stream.subRtspUrl = jStream["subRtspURL"].GetString();
		stream.mainRtspUrl = jStream["mainRtspURL"].GetString();
        stream.isReferenceCamera = false;

		streams.push_back(stream);
	}

	if (doc.HasMember("referenceCamera"))
	{
		rapidjson::Value jStream = doc["referenceCamera"].GetObject();

		Stream stream;

		if (jStream.HasMember("id"))
			stream.id = jStream["id"].GetInt();
		else
			stream.id = -1;

		stream.name = jStream["name"].GetString();
		stream.subRtspUrl = jStream["subRtspURL"].GetString();
		stream.mainRtspUrl = jStream["mainRtspURL"].GetString();
        stream.isReferenceCamera = true;

		streams.push_back(stream);
	}

    if (doc.HasMember("detectors"))
    {
        for (rapidjson::SizeType i = 0; i < doc["detectors"].Size(); ++i)
        {
            rapidjson::Value jDetector = doc["detectors"][i].GetObject();            

            Detector detector;

            if (jDetector.HasMember("gpuId"))
                detector.gpuId = jDetector["gpuId"].GetInt();
            else
                detector.gpuId = 0;

            if (jDetector.HasMember("streams"))
            {
                for (rapidjson::SizeType j = 0; j < jDetector["streams"].Size(); ++j)
                {
                    int streamId = jDetector["streams"][j].GetInt();
                    detector.streams.push_back(streamId);
                }
            }

            detector.detectorParams = detectorParams;
            detector.isImageExtractedInStandaloneThread = isImageExtractedInStandaloneThread;
            if (jDetector.HasMember("detectorParams") && jDetector["detectorParams"].IsObject())
            {
                rapidjson::Value jParams = jDetector["detectorParams"].GetObject();

                for (auto it = jParams.MemberBegin(); it != jParams.MemberEnd(); ++it)
                {
                    std::string name = it->name.GetString();
                    std::string value = ToString(it->value);
                    detector.detectorParams[name] = value;

                    if (name == "output_image.standalone_thread")
                    {
                        detector.isImageExtractedInStandaloneThread = cvcam::base::strToBool(value);
                    }
                }
            }

            detectors.push_back(detector);
        }
    }

    if (doc.HasMember("cameraRois"))
    {
        for (rapidjson::SizeType i = 0; i < doc["cameraRois"].Size(); ++i)
        {
            rapidjson::Value jCameraRois = doc["cameraRois"][i].GetObject();

            if (!jCameraRois.HasMember("cameraId"))
                continue;

            int cameraId = jCameraRois["cameraId"].GetInt();

            if (jCameraRois.HasMember("enabled"))
            {
                if (!rapidjson::ToBool(jCameraRois["enabled"]))
                    continue;
            }

            if (!jCameraRois.HasMember("rois"))
                continue;

            std::vector<cvcam::ObjectRect> rois;
            for (rapidjson::SizeType j = 0; j < jCameraRois["rois"].Size(); ++j)
            {
                rapidjson::Value jRoi = jCameraRois["rois"][j].GetObject();

                cvcam::ObjectRect roi;
                roi.x = rapidjson::ToDouble(jRoi["x"]);
                roi.y = rapidjson::ToDouble(jRoi["y"]);
                roi.width = rapidjson::ToDouble(jRoi["width"]);
                roi.height = rapidjson::ToDouble(jRoi["height"]);
                rois.push_back(roi);
            }

            if (!rois.empty())
            {
                cameraRois[cameraId] = rois;
            }
        }
    }

    if (doc.HasMember("mqParams"))
    {
        rapidjson::Value jMQParams = doc["mqParams"].GetObject();

        rapidjson::GetOptionalBoolMember(jMQParams, "enabled", mqParams.enabled, false);
        rapidjson::GetOptionalStringMember(jMQParams, "host", mqParams.host, "127.0.0.1");
        rapidjson::GetOptionalIntMember(jMQParams, "port", mqParams.port, 5672);
        rapidjson::GetOptionalStringMember(jMQParams, "username", mqParams.username, "guest");
        rapidjson::GetOptionalStringMember(jMQParams, "password", mqParams.password, "guest");
        rapidjson::GetOptionalIntMember(jMQParams, "queueMaxLength", mqParams.queueMaxLength, 10000);
        rapidjson::GetOptionalIntMember(jMQParams, "msgExpiredTime", mqParams.msgExpiredTime, 2000);
    }
    else
    {
        mqParams.enabled = false;
    }

	return true;
}

bool Config::getStreamById(int id, Stream& streamOut)
{
    for (const Stream& stream : streams)
    {
        if (stream.id == id)
        {
            streamOut = stream;
            return true;
        }
    }
    return false;
}

bool Config::parseMngInfo(const std::string& path, MngParams& mngParams)
{
    std::ifstream ifs(path);
    rapidjson::IStreamWrapper isw(ifs);

    rapidjson::Document doc;
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(isw);

    if (doc.HasParseError())
        return false;

    if (!doc.IsObject())
        return false;

    if (!rapidjson::GetStringMember(doc, "ip", mngParams.host))
        return false;

    if (!rapidjson::GetStringMember(doc, "port", mngParams.port))
        return false;

    if (!rapidjson::GetStringMember(doc, "detectServiceUrl", mngParams.url))
        return false;

    return true;
}

bool Config::ChangeCfg(std::string str)
{
	using namespace rapidjson;

	Document doc;
	doc.Parse(str.data(), str.size());
	if (doc.HasParseError())
		return false;

	if (!doc.HasMember("global"))
		return false;

	if (!doc["global"].IsObject())
		return false;

	if (doc["global"].HasMember("serviceId"))
		global.serviceId = doc["global"]["serviceId"].GetString();

	if (doc["global"].HasMember("apiType"))
		global.apiType = doc["global"]["apiType"].GetString();

	if (doc["global"].HasMember("appId"))
		global.appId = doc["global"]["appId"].GetString();

	if (doc["global"].HasMember("httpPath"))
		global.httpPath = doc["global"]["httpPath"].GetString();
	else
		global.httpPath = "/";

	if (doc["global"].HasMember("detectionLevel"))
		global.detectionLevel = doc["global"]["detectionLevel"].GetString();

	if (doc["global"].HasMember("onlyKeyFrame"))
		global.onlyKeyFrame = doc["global"]["onlyKeyFrame"].GetInt() != 0;


	if (doc["global"].HasMember("frameInterval"))
		global.frameInterval = doc["global"]["frameInterval"].GetInt();

	if (doc["global"].HasMember("detectInterval"))
		global.detectInterval = doc["global"]["detectInterval"].GetInt();

	if (doc["global"].HasMember("saveInterval"))
		global.saveInterval = doc["global"]["saveInterval"].GetInt();


	if (doc["global"].HasMember("threshould"))
		global.threshould = doc["global"]["threshould"].GetDouble();

	if (doc["global"].HasMember("outputFormat"))
		global.outputFormat = doc["global"]["outputFormat"].GetString();

	if (doc["global"].HasMember("imageMinSize"))
		global.imageMinSize = doc["global"]["imageMinSize"].GetInt();

	if (doc["global"].HasMember("imageEnlargementRatio"))
		global.imageEnlargementRatio = doc["global"]["imageEnlargementRatio"].GetDouble();

	if (doc["global"].HasMember("objectCountLimit"))
		global.objectCountLimit = doc["global"]["objectCountLimit"].GetInt();


	if (doc["global"].HasMember("bodySizeLimit"))
		global.bodySizeLimit = doc["global"]["bodySizeLimit"].GetInt();

	if (doc["global"].HasMember("saveImage"))
		global.saveImage = doc["global"]["saveImage"].GetInt();

	if (doc["global"].HasMember("imagePath"))
		global.imagePath = doc["global"]["imagePath"].GetString();

	if (global.imagePath.empty())
		global.imagePath = "images";

	if (doc["global"].HasMember("retentionPeriod"))
		global.retentionPeriod = doc["global"]["retentionPeriod"].GetInt();
	if (doc["global"].HasMember("freeDiskSpaceLimit"))
		global.freeDiskSpaceLimit = doc["global"]["freeDiskSpaceLimit"].GetInt();

	if (doc["global"].HasMember("statsInterval"))
		global.statsInterval = doc["global"]["statsInterval"].GetInt();

	global.decoderFlags = 0;

	if (doc["global"].HasMember("squareRoi"))
	{
		int squareRoi = doc["global"]["squareRoi"].GetInt();
		if (squareRoi)
			global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_SQUARE_ROI;
	}

	if (doc["global"].HasMember("boundedByBorder"))
	{
		int boundedByBorder = doc["global"]["boundedByBorder"].GetInt();
		if (boundedByBorder)
			global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_BOUNDED_BY_BORDER;
	}

	int removeOverlapped = 1;
	if (doc["global"].HasMember("removeOverlapped"))
		removeOverlapped = doc["global"]["removeOverlapped"].GetInt();
	if (removeOverlapped)
		global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_REMOVE_OVERLAPPED_OBJECT;

	int sortByProb = 1;
	if (doc["global"].HasMember("sortByProb"))
		sortByProb = doc["global"]["sortByProb"].GetInt();
	if (sortByProb)
		global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_SORT_BY_PROB;

	if (doc["global"].HasMember("outputImage"))
		global.outputImage = doc["global"]["outputImage"].GetInt();
	if (!global.outputImage && !global.saveImage)
		global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_NO_IMAGE;

	if (doc["global"].HasMember("removeDuplicate"))
	{
		int removeDuplicate = doc["global"]["removeDuplicate"].GetInt();
		if (removeDuplicate)
			global.decoderFlags |= cvcam::ObjectDetector::OUTPUT_FLAG_REMOVE_DUPLICATE_OBJECT;
	}

	if (doc["global"].HasMember("duplicateAlgo"))
		global.duplicateAlgo = doc["global"]["duplicateAlgo"].GetString();

	detectorParams["object_idFormat"] = "short_uuid"; // default
	detectorParams["outputImage_standaloneThread"] = "true";
	isImageExtractedInStandaloneThread = true;
	if (doc.HasMember("detectorParams") && doc["detectorParams"].IsObject())
	{
		rapidjson::Value jParams = doc["detectorParams"].GetObject();

		for (auto it = jParams.MemberBegin(); it != jParams.MemberEnd(); ++it)
		{
			std::string name = it->name.GetString();
			std::string value = ToString(it->value);
			detectorParams[name] = value;

			if (name == "outputImage_standaloneThread")
			{
				isImageExtractedInStandaloneThread = cvcam::base::strToBool(value);
			}
		}
	}

	if (doc.HasMember("referenceCameraParams"))
	{
		if (doc["referenceCameraParams"].HasMember("detectInterval"))
			referenceCameraParams.detectInterval = doc["referenceCameraParams"]["detectInterval"].GetInt();
		if (doc["referenceCameraParams"].HasMember("saveInterval"))
			referenceCameraParams.saveInterval = doc["referenceCameraParams"]["saveInterval"].GetInt();
	}

	if (doc.HasMember("detailCameraParams"))
	{
		if (doc["detailCameraParams"].HasMember("detectInterval"))
			detailCameraParams.detectInterval = doc["detailCameraParams"]["detectInterval"].GetInt();
		if (doc["detailCameraParams"].HasMember("saveInterval"))
			detailCameraParams.saveInterval = doc["detailCameraParams"]["saveInterval"].GetInt();
		//rapidjson::Value jCameraParams = doc["detailCameraParams"].GetObject();

		//if (jCameraParams.HasMember("detectInterval"))
		//	detailCameraParams.detectInterval = doc["detailCameraParams"]["detectInterval"].GetInt();
		//if (jCameraParams.HasMember("saveInterval"))
		//	detailCameraParams.saveInterval = doc["detailCameraParams"]["saveInterval"].GetInt();

	}
	if (doc["global"].HasMember("workMode"))
		global.modelName = doc["global"]["workMode"].GetString();

	if (doc["global"].HasMember("appSecret"))
		global.appSecret = doc["global"]["appSecret"].GetString();
	if (doc["global"].HasMember("serverAddr"))
		global.serverAddr = doc["global"]["serverAddr"].GetString();
	if (doc["global"].HasMember("serverPort"))
		global.serverPort = doc["global"]["serverPort"].GetInt();
	if (doc["global"].HasMember("disableDetection"))
		global.disableDetection = doc["global"]["disableDetection"].GetInt() != 0;

	if (doc.HasMember("wsserver"))
	{
		rapidjson::Value jServer = doc["wsserver"].GetObject();

		if (jServer.HasMember("host"))
			wsServerParams.host = jServer["host"].GetString();

		if (jServer.HasMember("port"))
			wsServerParams.port = (unsigned short)jServer["port"].GetInt();
	}

	if (doc.HasMember("detectors"))
	{
		detectors.clear();

		for (rapidjson::SizeType i = 0; i < doc["detectors"].Size(); ++i)
		{
			rapidjson::Value jDetector = doc["detectors"][i].GetObject();

			Detector detector;

			if (jDetector.HasMember("gpuId"))
				detector.gpuId = jDetector["gpuId"].GetInt();
			else
				detector.gpuId = 0;

			if (jDetector.HasMember("streams"))
			{
				for (rapidjson::SizeType j = 0; j < jDetector["streams"].Size(); ++j)
				{
					int streamId = jDetector["streams"][j].GetInt();
					detector.streams.push_back(streamId);
				}
			}

			detector.detectorParams = detectorParams;
			detector.isImageExtractedInStandaloneThread = isImageExtractedInStandaloneThread;
			if (jDetector.HasMember("detectorParams") && jDetector["detectorParams"].IsObject())
			{
				rapidjson::Value jParams = jDetector["detectorParams"].GetObject();

				for (auto it = jParams.MemberBegin(); it != jParams.MemberEnd(); ++it)
				{
					std::string name = it->name.GetString();
					std::string value = ToString(it->value);
					detector.detectorParams[name] = value;

					if (name == "output_image.standalone_thread")
					{
						detector.isImageExtractedInStandaloneThread = cvcam::base::strToBool(value);
					}
				}
			}

			detectors.push_back(detector);
		}
	}

	if (doc.HasMember("cameraRois"))
	{
		cameraRois.clear();
		for (rapidjson::SizeType i = 0; i < doc["cameraRois"].Size(); ++i)
		{
			rapidjson::Value jCameraRois = doc["cameraRois"][i].GetObject();

			if (!jCameraRois.HasMember("cameraId"))
				continue;

			int cameraId = jCameraRois["cameraId"].GetInt();

			if (jCameraRois.HasMember("enabled"))
			{
				if (!rapidjson::ToBool(jCameraRois["enabled"]))
					continue;
			}

			if (!jCameraRois.HasMember("rois"))
				continue;

			std::vector<cvcam::ObjectRect> rois;
			for (rapidjson::SizeType j = 0; j < jCameraRois["rois"].Size(); ++j)
			{
				rapidjson::Value jRoi = jCameraRois["rois"][j].GetObject();

				cvcam::ObjectRect roi;
				roi.x = rapidjson::ToDouble(jRoi["x"]);
				roi.y = rapidjson::ToDouble(jRoi["y"]);
				roi.width = rapidjson::ToDouble(jRoi["width"]);
				roi.height = rapidjson::ToDouble(jRoi["height"]);
				rois.push_back(roi);
			}

			if (!rois.empty())
			{
				cameraRois[cameraId] = rois;
			}
		}
	}

	if (doc.HasMember("mqParams"))
	{
		rapidjson::Value jMQParams = doc["mqParams"].GetObject();

		if (doc["mqParams"].HasMember("enabled"))
			mqParams.enabled = doc["mqParams"]["enabled"].GetBool();
		if (doc["mqParams"].HasMember("host"))
			mqParams.host = doc["mqParams"]["host"].GetString();
		if (doc["mqParams"].HasMember("port"))
			mqParams.port = doc["mqParams"]["port"].GetInt();
		if (doc["mqParams"].HasMember("username"))
			mqParams.username = doc["mqParams"]["username"].GetString();
		if (doc["mqParams"].HasMember("password"))
			mqParams.password = doc["mqParams"]["password"].GetString();
		if (doc["mqParams"].HasMember("queueMaxLength"))
			mqParams.queueMaxLength = doc["mqParams"]["queueMaxLength"].GetInt();
		if (doc["mqParams"].HasMember("msgExpiredTime"))
			mqParams.msgExpiredTime = doc["mqParams"]["msgExpiredTime"].GetInt();
	}

	save();
	return true;
}
//
//bool Config::ChangeReloadCfg(std::string str)
//{
//	using namespace rapidjson;
//
//	Document doc;
//	doc.Parse(str.data(), str.size());
//	if (doc.HasParseError())
//		return false;
//
//	if (!doc.HasMember("global"))
//		return false;
//
//	if (!doc["global"].IsObject())
//		return false;
//	rapidjson::GetOptionalStringMember(doc["global"], "serviceId", global.serviceId);
//
//
//
//	save();
//	return true;
//}

bool Config::save()
{
    // backup config
    std::error_code ec;
    std::fs::copy_file(CONFIG_FILE_PATH, CONFIG_FILE_PATH ".bak", ec);

    std::ofstream ofs(CONFIG_FILE_PATH);
    if (!ofs.is_open())
        return false;

    rapidjson::OStreamWrapper osw(ofs);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

	rapidjson::Document doc;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	doc.SetObject();

	rapidjson::Value globalObj(rapidjson::kObjectType);
	globalObj.AddMember("serviceId", global.serviceId, allocator);
	globalObj.AddMember("sceneId", global.sceneId, allocator);
	globalObj.AddMember("modelName", global.modelName, allocator);
	globalObj.AddMember("apiType", global.apiType, allocator);
	globalObj.AddMember("appId", global.appId, allocator);
	globalObj.AddMember("appSecret", global.appSecret, allocator);
	globalObj.AddMember("serverAddr", global.serverAddr, allocator);
	globalObj.AddMember("serverPort", global.serverPort, allocator);
	globalObj.AddMember("httpPath", global.httpPath, allocator);
	globalObj.AddMember("disableDetection", global.disableDetection, allocator);
	globalObj.AddMember("detectionLevel", global.detectionLevel, allocator);
	globalObj.AddMember("onlyKeyFrame", global.onlyKeyFrame, allocator);
	globalObj.AddMember("frameInterval", global.frameInterval, allocator);
	globalObj.AddMember("detectInterval", global.detectInterval, allocator);
	globalObj.AddMember("saveInterval", global.saveInterval, allocator);
	globalObj.AddMember("threshould", global.threshould, allocator);
	globalObj.AddMember("outputFormat", global.outputFormat, allocator);
	globalObj.AddMember("imageMinSize", global.imageMinSize, allocator);
	globalObj.AddMember("imageEnlargementRatio", global.imageEnlargementRatio, allocator);
	globalObj.AddMember("objectCountLimit", global.objectCountLimit, allocator);
	globalObj.AddMember("bodySizeLimit", global.bodySizeLimit, allocator);
	globalObj.AddMember("statsInterval", global.statsInterval, allocator);
	globalObj.AddMember("outputImage", global.outputImage, allocator);
	globalObj.AddMember("saveImage", global.saveImage, allocator);
	globalObj.AddMember("imagePath", global.imagePath, allocator);
	globalObj.AddMember("retentionPeriod", global.retentionPeriod, allocator);
	globalObj.AddMember("freeDiskSpaceLimit", global.freeDiskSpaceLimit, allocator);
	globalObj.AddMember("duplicateAlgo", global.duplicateAlgo, allocator);
	globalObj.AddMember("decoderFlags", global.decoderFlags, allocator);

	doc.AddMember("global", globalObj, allocator);
	rapidjson::Value detectorParamsObj(rapidjson::kObjectType);
	for (auto &it : detectorParams)
	{
		detectorParamsObj.AddMember(rapidjson::Value(it.first, allocator), it.second, allocator);
	}
	doc.AddMember("detectorParams", detectorParamsObj, allocator);

	rapidjson::Value wsServerParamsObj(rapidjson::kObjectType);
	wsServerParamsObj.AddMember("host", wsServerParams.host, allocator);
	wsServerParamsObj.AddMember("port", wsServerParams.port, allocator);
	doc.AddMember("wsserver", wsServerParamsObj, allocator);

	rapidjson::Value mqParamsObj(rapidjson::kObjectType);
	mqParamsObj.AddMember("enabled", mqParams.enabled, allocator);
	mqParamsObj.AddMember("host", mqParams.host, allocator);
	mqParamsObj.AddMember("username", mqParams.username, allocator);
	mqParamsObj.AddMember("password", mqParams.password, allocator);
	mqParamsObj.AddMember("msgExpiredTime", mqParams.msgExpiredTime, allocator);
	doc.AddMember("mqParams", mqParamsObj, allocator);

	rapidjson::Value detectorsObj(rapidjson::kArrayType);
	for (auto& dt : detectors)
	{
		rapidjson::Value detectorObj(rapidjson::kObjectType);
		detectorObj.AddMember("gpuId", dt.gpuId, allocator);

		rapidjson::Value dtsObj(rapidjson::kArrayType);
		for (auto &dts : dt.streams)
		{
			dtsObj.PushBack(dts, allocator);
		}
		detectorObj.AddMember("streams", dtsObj, allocator);
	}
	doc.AddMember("detectors", detectorsObj, allocator);

	rapidjson::Value cameraRoisObj(rapidjson::kArrayType);
	for (auto & itroi : cameraRois)
	{
		//camera 
		rapidjson::Value roisCameraObj(rapidjson::kObjectType);
		roisCameraObj.AddMember("cameraId", itroi.first, allocator);
		roisCameraObj.AddMember("enabled", true, allocator);

		rapidjson::Value roisObj(rapidjson::kArrayType);
		for (auto & rect : itroi.second)
		{
			rapidjson::Value objRect(rapidjson::kObjectType);
			objRect.AddMember("x", rect.x, allocator);
			objRect.AddMember("y", rect.y, allocator);
			objRect.AddMember("width", rect.width, allocator);
			objRect.AddMember("height", rect.height, allocator);
			roisObj.PushBack(objRect, allocator);
		}
		roisCameraObj.AddMember("rois", roisObj, allocator);
	}
	doc.AddMember("cameraRois", cameraRoisObj, allocator);
	
	rapidjson::Value referenceCameraParamsObj(rapidjson::kObjectType);
	referenceCameraParamsObj.AddMember("detectInterval", referenceCameraParams.detectInterval, allocator);
	referenceCameraParamsObj.AddMember("saveInterval", referenceCameraParams.saveInterval, allocator);
	doc.AddMember("referenceCameraParams", referenceCameraParamsObj, allocator);

	rapidjson::Value detailCameraParamsObj(rapidjson::kObjectType);
	detailCameraParamsObj.AddMember("detectInterval", detailCameraParams.detectInterval, allocator);
	detailCameraParamsObj.AddMember("saveInterval", detailCameraParams.saveInterval, allocator);
	doc.AddMember("detailCameraParams", detailCameraParamsObj, allocator);

	rapidjson::Value referenceCameraObj(rapidjson::kObjectType);
	for (auto & stream : streams)
	{
		if (stream.isReferenceCamera)
		{
			referenceCameraObj.AddMember("id", stream.id, allocator);
			referenceCameraObj.AddMember("name", stream.name, allocator);
			referenceCameraObj.AddMember("subRtspURL", stream.subRtspUrl, allocator);
			referenceCameraObj.AddMember("mainRtspURL", stream.mainRtspUrl, allocator);

			break;
		}
	}
	doc.AddMember("referenceCamera", referenceCameraObj, allocator);

	rapidjson::Value detailCamerasObj(rapidjson::kArrayType);
	for (auto & stream : streams)
	{
		if (stream.isReferenceCamera)
		{
			continue;
		}
		rapidjson::Value detailObj(rapidjson::kObjectType);
		detailObj.AddMember("id", stream.id, allocator);
		detailObj.AddMember("name", stream.name, allocator);
		detailObj.AddMember("subRtspURL", stream.subRtspUrl, allocator);
		detailObj.AddMember("mainRtspURL", stream.mainRtspUrl, allocator);

		detailCamerasObj.PushBack(detailObj, allocator);
	}
	doc.AddMember("detailCameras", detailCamerasObj, allocator);

	//save all
	doc.Accept(writer);

	return true;
}

