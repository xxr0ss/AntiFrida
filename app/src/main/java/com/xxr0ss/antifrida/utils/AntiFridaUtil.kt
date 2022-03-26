package com.xxr0ss.antifrida.utils

import java.io.File
import java.lang.Exception
import android.util.Log


object AntiFridaUtil {
    private var TAG: String = "AntiFridaUtil"


    fun checkFridaByModuleEnum(target: String): Boolean {
        val maps = getProcMaps()
        Log.d(TAG, maps)
        return target in maps
    }

    external fun checkFridaByPort(port: Int): Boolean


    private fun getProcMaps(): String {
        try{
            val mapsFile = File("/proc/self/maps")
            return mapsFile.readText()
        }catch (e: Exception) {
            Log.e(TAG, e.stackTraceToString())
        }
        return ""
    }


    init {
        System.loadLibrary("antifrida")
    }
}