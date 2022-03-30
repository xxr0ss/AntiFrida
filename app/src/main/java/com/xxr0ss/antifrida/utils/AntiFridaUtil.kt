package com.xxr0ss.antifrida.utils

import java.io.File
import java.lang.Exception
import android.util.Log


object AntiFridaUtil {
    private var TAG: String = "AntiFridaUtil"

    fun checkFridaByProcMaps(targets: List<String>, via: ReadVia): Boolean {
        val maps = when(via) {
            ReadVia.JVM -> getProcMaps()
            ReadVia.ORIG_SYSCALL -> nativeGetProcMaps()
            ReadVia.CUSTOMIZED_SYSCALL -> nativeGetProcMaps(true)
        }

        if (maps == null) {
            Log.d(TAG, "maps got null")
            return false
        }

        Log.d(TAG, maps)
        for (target in targets) {
            if (target in maps)
                return true
        }
        return false
    }


    private fun getProcMaps(): String? {
        try{
            val mapsFile = File("/proc/self/maps")
            return mapsFile.readText()
        }catch (e: Exception) {
            Log.e(TAG, e.stackTraceToString())
        }
        return null
    }

    private external fun nativeGetProcMaps(useCustomizedSyscall: Boolean = false): String?

    external fun checkFridaByPort(port: Int): Boolean

    init {
        System.loadLibrary("antifrida")
    }
}

enum class ReadVia(val value: Int) {
    JVM(0),
    ORIG_SYSCALL(1),
    CUSTOMIZED_SYSCALL(2);

    companion object{
        fun fromInt(value: Int) = values().first {it.value == value}
    }
}