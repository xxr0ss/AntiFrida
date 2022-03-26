package com.xxr0ss.antifrida.utils

import java.io.File
import java.lang.Exception
import android.util.Log


object AntiFridaUtil {
    private var TAG: String = "AntiFridaUtil"


    fun checkFridaByModuleEnum(targets: List<String>): Boolean {
        val maps = getProcMaps()
        Log.d(TAG, maps)
        for (target in targets) {
            if (target in maps)
                return true
        }
        return false
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