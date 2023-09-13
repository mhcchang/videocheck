package jnatest;

import com.sun.jna.Memory;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
 
public class CFJnaTest {
 
	public static void main(String[] args) {
 
		//配置参数
		Pointer resultv = Pointer.NULL;
		IntByReference resultLength = new IntByReference(); 

		CFJna instance = CFJna.library;
        
		Pointer resBody = Pointer.NULL;
        
		//!!! 1.先申请内存
		resBody = new Memory(1024);           
        
        IntByReference resultLength1 = new IntByReference();
        
        int nret = instance.VideoSdk_Init(2, 25, 0.75);
        if (nret != 0) {
            System.out.println("errr");
            return;
        }

        String jsonres="{\n" +
                "  \"DeviceList\": [\n" +
                "    {\n" +
                "      \"DeviceId\": \"34020000001320000002\",\n" +
                "      \"rtspUrl\": \"rtsp://localhost/stream11\"\n" +
                "    },\n" +
                "    {\n" +
                "      \"DeviceId\": \"222000002\",\n" +
                "      \"rtspUrl\": \"rtsp://172.16.50.165:554/rtp/42E075F9\"\n" +
                "    }\n" +
                "  ],\n" +
                "  \"Duration\": 10,\n" +
                "  \"Immediately\": true,\n" +
                "  \"ScheduleTime\": \"07:00\",\n" +
                "  \"MaxThread\": 64\n" +
                "}";

        nret = instance.VideoSdk_SetStreamStatusQuest(jsonres);
        System.out.println(String.format(" VideoSdk_SetStreamStatusQuest return=%d \n", nret));
        try {
            Thread.sleep(8000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

		//!!! 2. resultLength1这个值动态库里面返回的不对  后面修改一下  可以先用 其他方式获取
    
        nret = instance.VideoSdk_GetStreamStatusRespond(jsonres, resBody, resultLength1);
        if(nret!=0){
            System.out.println("errr:"+nret);
            return;
        }

        //resultLength1 值不对
        System.out.println("resultLength:" + resultLength1.getValue());

        //3 获取方式 1
        byte[] byteArray = resBody.getByteArray(0, resBody.getString(0).length());

        try {
        	//3 获取方式2
        	System.out.println("-------- resBody:" + resBody.getString(0));

        	System.out.println("-----byteArray- resBody:"); 
        	System.out.println(new String(byteArray,"GB2312"));
        } catch (Exception e) {
        	System.out.println("GB2312 error");
            e.printStackTrace();
        }		
		
        System.out.println("return !");
	}
 
}
