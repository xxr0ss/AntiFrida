package com.xxr0ss.antifrida.utils

import android.app.usage.UsageStats
import android.app.usage.UsageStatsManager
import android.content.Context
import android.util.Log
import java.util.*

object UStats {
    private val TAG: String = "UStats"

    fun getUsageStatsList(context: Context) : List<UsageStats>{
        val usm: UsageStatsManager = context.getSystemService(Context.USAGE_STATS_SERVICE)
                as UsageStatsManager

        val endTime = Calendar.getInstance().timeInMillis
        val startTime = endTime - 1000 * 1000

        return usm.queryUsageStats(UsageStatsManager.INTERVAL_DAILY, startTime, endTime)
    }

    fun logCurrentUsageStats(context: Context) {
        val usageStats = getUsageStatsList(context)
        usageStats.forEach {
            Log.d(TAG, it.packageName)
        }
    }
}