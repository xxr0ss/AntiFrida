package com.xxr0ss.antifrida.utils

import java.io.File
import java.lang.Exception
import android.util.Log


object AntiFridaUtil {
    private var TAG: String = "AntiFridaUtil"

    var maps_file_content: String? = null

    fun checkFridaByProcMaps(targets: List<String>, via: ReadVia): Boolean {
        maps_file_content = when (via) {
            ReadVia.JVM -> readProcMaps()
            ReadVia.ORIG_SYSCALL -> nativeReadProcMaps()
            ReadVia.CUSTOMIZED_SYSCALL -> nativeReadProcMaps(true)
        }

        if (maps_file_content == null) {
            Log.d(TAG, "maps got null")
            return false
        }

        Log.d(TAG, maps_file_content!!)
        for (target in targets) {
            if (target in maps_file_content!!)
                return true
        }
        return false
    }


    private fun readProcMaps(): String? {
        try {
            val mapsFile = File("/proc/self/maps")
            return mapsFile.readText()
        } catch (e: Exception) {
            Log.e(TAG, e.stackTraceToString())
        }
        return null
    }

    private external fun nativeReadProcMaps(useCustomizedSyscall: Boolean = false): String?

    external fun checkFridaByPort(port: Int): Boolean

    external fun scanModulesForSignature(
        signature: String,
        useCustomizedSyscalls: Boolean = false
    ): Boolean

    external fun checkBeingDebugged(useCustomizedSyscall: Boolean=false): Boolean

    init {
        System.loadLibrary("antifrida")
    }
}

enum class ReadVia(val value: Int) {
    JVM(0),
    ORIG_SYSCALL(1),
    CUSTOMIZED_SYSCALL(2);

    companion object {
        fun fromInt(value: Int) = values().first { it.value == value }
    }
}