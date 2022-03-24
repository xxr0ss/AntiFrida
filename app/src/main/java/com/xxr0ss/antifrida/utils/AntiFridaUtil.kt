package com.xxr0ss.antifrida.utils

import android.app.ActivityManager
import java.io.File
import java.lang.Exception
import android.util.Log
import java.net.Socket

object AntiFridaUtil {
    private var TAG: String = "AntiFridaUtil"


    fun checkFridaByModuleEnum(target: String): Boolean {
        val maps = getProcMaps()
        Log.d(TAG, maps)
        return maps.contains(target)
    }

    external fun checkFridaByPort(port: Int): Boolean

//    fun checkFridaByPort(port: Int): Boolean {
//        try{
//            val socket = Socket("127.0.0.1", port)
//            socket.close()
//            return true;
//        }catch(e: Exception) {
//            Log.e(TAG, e.stackTraceToString())
//        }
//        return false;
//    }


    private fun getProcMaps(): String {
        try{
            val mapsFile = File("/proc/self/maps")
            return mapsFile.readText()
        }catch (e: Exception) {
            Log.e(TAG, e.stackTraceToString())
        }
        return ""
    }

    fun checkFridaByEnumRunningProcesses(): Boolean {
        return false
    }

    init {
        System.loadLibrary("antifrida")
    }
}