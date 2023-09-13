package jnatest;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
 
public interface CFJna extends Library {
 
	CFJna library = (CFJna) Native.loadLibrary("D:\\mhchang\\workspace\\jnatest\\bin\\VideoSdk.dll", CFJna.class);
	

	public int  VideoSdk_GetLastError();

	/**
	 * @brief 鍒濆鍖�
	 * @param delayVideo 妫�娴嬫椂闀跨己鐪佸�� 涓�鑸涓�2绉�
	 * @param frameRate 瑙嗛娴佸熀纭�甯х巼 缂虹渷涓�25
	 * @param qualityThreshold 妫�娴嬫祦鐨勮川閲忛槇鍊� 纭疄璁剧疆涓� 0.9涓洪珮璐ㄩ噺
	 * @return 杩斿洖鍊� 0 鎴愬姛; 鍏朵粬 澶辫触
	 */
	public int  VideoSdk_Init(int delayVideo, int frameRate, double qualityThreshold);

	/**
	 * @brief 鍙嶅垵濮嬪寲
	 * @return 杩斿洖鍊� 0 鎴愬姛; 鍏朵粬 澶辫触
	 */
	public int  VideoSdk_Uninit();

	/**
	 * @brief 璁剧疆鏃ュ織绾у埆
	 * @param level
	TRACE=0; DEBUG=1; INFO=2; WARN=3; ERROR=4 FATAL=5 缂虹渷涓篹rror
	 * @return 杩斿洖鍊� 0 鎴愬姛; 鍏朵粬 澶辫触
	 */
	public void  VideoSdk_SetLogLevel(int level);

	/**
	 * @brief 瑙嗛娴佺姸鎬佹煡璇�
	 * @param quest 璇锋眰鍐呭
	json鏍煎紡
	 * @return 杩斿洖鍊� 0 鎴愬姛; 鍏朵粬 澶辫触
	 */
	public int  VideoSdk_SetStreamStatusQuest( String quest);

	/**
	 * @brief 鑾峰彇瑙嗛娴佺姸鎬佺粨鏋�
	 * @param quest 璇锋眰鍐呭
	json鏍煎紡
	 * @param resBody 杩斿洖缁撴灉
	json鏍煎紡
	 * @param reslen
	json鏍煎紡
	 * @return 杩斿洖鍊� 0 鎴愬姛; 鍏朵粬 澶辫触
	 */
	public int  VideoSdk_GetStreamStatusRespond(String quest, Pointer resBody, IntByReference reslen);

	/**
	 * @brief 寮哄埗鍏抽敭甯х姸鎬佹煡璇�
	 * @param quest 璇锋眰鍐呭
	json鏍煎紡
	 * @return 杩斿洖鍊� 0 鎴愬姛; 鍏朵粬 澶辫触
	 */
	public int  VideoSdk_SetForceKeyFrameQuest(String quest);

	/**
	 * @brief 寮哄埗鍏抽敭甯х姸鎬佹煡璇�
	 * @param quest 璇锋眰鍐呭
	json鏍煎紡
	 * @param resBody 杩斿洖缁撴灉
	json鏍煎紡
	 * @param reslen
	json鏍煎紡
	 * @return 杩斿洖鍊� 0 鎴愬姛; 鍏朵粬 澶辫触
	 */
	public int  VideoSdk_GetForceKeyFrameRespond(String quest, Pointer resBody, IntByReference reslen);

	/**
	 * @brief 鍏抽敭甯х姸鎬佹煡璇�
	 * @param quest 璇锋眰鍐呭
	json鏍煎紡
	 * @return 杩斿洖鍊� 0 鎴愬姛; 鍏朵粬 澶辫触
	 */
	public int  VideoSdk_SetKeyFrameQuest(String quest);

	/**
	 * @brief 鍏抽敭甯х姸鎬佽幏鍙�
	 * @param quest 璇锋眰鍐呭
	json鏍煎紡
	 * @param resBody 杩斿洖缁撴灉
	json鏍煎紡
	 * @param reslen
	json鏍煎紡
	 * @return 杩斿洖鍊� 0 鎴愬姛; 鍏朵粬 澶辫触
	 */
	public int  VideoSdk_GetKeyFrameRespond(String quest, Pointer resBody, IntByReference reslen);

	/**
	 * @brief 宸℃
	 * @param quest 璇锋眰鍐呭
	json鏍煎紡
	 * @return 杩斿洖鍊� 0 鎴愬姛; 鍏朵粬 澶辫触
	 */
	public int  VideoSdk_SetPollingQuest(String quest);

	/**
	 * @brief 宸℃缁撴灉
	 * @param quest 璇锋眰鍐呭
	json鏍煎紡
	 * @param resBody 杩斿洖缁撴灉
	json鏍煎紡
	 * @param reslen
	json鏍煎紡
	 * @return 杩斿洖鍊� 0 鎴愬姛; 鍏朵粬 澶辫触
	 */
	public int  VideoSdk_GetPollingRespond(String quest, Pointer resBody, IntByReference reslen);
	
}
