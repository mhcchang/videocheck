package com.qy.web.until.diagnose.vidcheck.cpp;

import com.qy.web.until.diagnose.cpp.JInParam;
import com.qy.web.until.diagnose.cpp.JResultInfo;
import com.qy.web.until.diagnose.cpp.ZVideoDiagnoseLibrarysTwo;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import org.springframework.stereotype.Service;

import java.io.File;

//import com.sun.jna.Structure;
//import lombok.Data;

@Service
public  interface chcekVideoDiagnoseLibrarys extends Library {

//	ZVideoDiagnoseLibrarysTwo INSTANCE = (ZVideoDiagnoseLibrarysTwo)Native.load("JVDEX", ZVideoDiagnoseLibrarysTwo.class);
    String property = System.getProperty("user.dir");
	String fileSeperator = File.separator;
	String osName=System.getProperties().getProperty("os.name").toLowerCase();
	String windwosPahts=property+ fileSeperator+"configVidCheck"+fileSeperator+"windows"+fileSeperator+"VideoSdk.dll";
	String linuxPahts=property+ fileSeperator+"config"+fileSeperator+"libJVDUtilsDLL.so";
 	String JVDEXPath =osName.contains("windows")?windwosPahts:linuxPahts;
	//dll的绝对文件路径
	chcekVideoDiagnoseLibrarys INSTANCE = Native.load(JVDEXPath, chcekVideoDiagnoseLibrarys.class);

//	public Pointer JEngineCreate(int usrID, String onnxPath, String datPath, int[] errorRet);


	/**
	 * @brief 获取最后一次调用函数时的错误值
	 * @return 如果调用没有错误 返回0 否则返回非零值  请参考C_ERROR_XXXX定义得具体内容
	 */
	public int  VideoSdk_GetLastError();

	/**
	 * @brief 初始化
	 * @param delayVideo 检测时长缺省值 一般设为2秒
	 * @param frameRate 视频流基础帧率 缺省为25
	 * @param qualityThreshold 检测流的质量阈值 确实设置为 0.9为高质量
	 * @return 返回值 0 成功; 其他 失败
	 */
	public int  VideoSdk_Init(int delayVideo, int frameRate, double qualityThreshold);

	/**
	 * @brief 反初始化
	 * @return 返回值 0 成功; 其他 失败
	 */
	public int  VideoSdk_Uninit();

	/**
	 * @brief 设置日志级别
	 * @param level
	TRACE=0; DEBUG=1; INFO=2; WARN=3; ERROR=4 FATAL=5 缺省为error
	 * @return 返回值 0 成功; 其他 失败
	 */
	public void  VideoSdk_SetLogLevel(int level);

	/**
	 * @brief 视频流状态查询
	 * @param quest 请求内容
	json格式
	 * @return 返回值 0 成功; 其他 失败
	 */
	public int  VideoSdk_SetStreamStatusQuest( String quest);

	/**
	 * @brief 获取视频流状态结果
	 * @param quest 请求内容
	json格式
	 * @param resBody 返回结果
	json格式
	 * @param reslen
	json格式
	 * @return 返回值 0 成功; 其他 失败
	 */
	public int  VideoSdk_GetStreamStatusRespond(Pointer quest, Pointer resBody, IntByReference reslen);

	/**
	 * @brief 强制关键帧状态查询
	 * @param quest 请求内容
	json格式
	 * @return 返回值 0 成功; 其他 失败
	 */
	public int  VideoSdk_SetForceKeyFrameQuest(String quest);

	/**
	 * @brief 强制关键帧状态查询
	 * @param quest 请求内容
	json格式
	 * @param resBody 返回结果
	json格式
	 * @param reslen
	json格式
	 * @return 返回值 0 成功; 其他 失败
	 */
	public int  VideoSdk_GetForceKeyFrameRespond(String quest, String resBody, Pointer reslen);

	/**
	 * @brief 关键帧状态查询
	 * @param quest 请求内容
	json格式
	 * @return 返回值 0 成功; 其他 失败
	 */
	public int  VideoSdk_SetKeyFrameQuest(String quest);

	/**
	 * @brief 关键帧状态获取
	 * @param quest 请求内容
	json格式
	 * @param resBody 返回结果
	json格式
	 * @param reslen
	json格式
	 * @return 返回值 0 成功; 其他 失败
	 */
	public int  VideoSdk_GetKeyFrameRespond(String quest, String resBody, String reslen);

	/**
	 * @brief 巡检
	 * @param quest 请求内容
	json格式
	 * @return 返回值 0 成功; 其他 失败
	 */
	public int  VideoSdk_SetPollingQuest(String quest);

	/**
	 * @brief 巡检结果
	 * @param quest 请求内容
	json格式
	 * @param resBody 返回结果
	json格式
	 * @param reslen
	json格式
	 * @return 返回值 0 成功; 其他 失败
	 */
	public int  VideoSdk_GetPollingRespond(String quest, String resBody, Pointer reslen);
}
