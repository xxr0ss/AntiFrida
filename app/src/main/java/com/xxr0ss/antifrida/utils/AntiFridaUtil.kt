package com.xxr0ss.antifrida.utils

import java.io.File
import java.lang.Exception
import android.util.Log

object AntiFridaUtil {
    private var TAG: String = "AntiFridaUtil"


    external fun checkFridaByModuleEnum(target: String): Boolean

    fun getProcMaps(): String {
        try{
            val mapsFile = File("/proc/self/maps")
            return mapsFile.readText()
        }catch (e: Exception) {
            Log.e(TAG, e.toString())
        }
        return ""
    }

    init {
        System.loadLibrary("antifrida")
    }
}