package com.xxr0ss.antifrida.utils

import android.util.Log
import java.io.DataOutputStream

object SuperUser {
    private val TAG: String = "SuperUser"
    var rooted = false

    fun tryRoot(pkgCodePath: String) {
        var process: Process? = null
        var os: DataOutputStream? = null
        try {
            process = Runtime.getRuntime().exec("su")
            os = DataOutputStream(process.outputStream)
            os.writeBytes("chmod 777 ${pkgCodePath}\n")
            os.writeBytes("exit\n")
            os.flush()
            rooted = process.waitFor() == 0
        } catch (e: Exception) {
            rooted = false
        } finally {
            os?.close()
            process?.destroy()
        }
    }
}