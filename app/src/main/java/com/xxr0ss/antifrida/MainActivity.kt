package com.xxr0ss.antifrida

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.provider.Settings
import com.xxr0ss.antifrida.databinding.ActivityMainBinding
import com.xxr0ss.antifrida.utils.AntiFridaUtil
import android.util.Log
import android.widget.Toast
import com.xxr0ss.antifrida.utils.SuperUser
import com.xxr0ss.antifrida.utils.UStats


class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding

    private val TAG = "MainActivity"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        if (UStats.getUsageStatsList(this).isEmpty()) {
            Log.d(TAG, "UsageStats not available")
            val intent = Intent(Settings.ACTION_USAGE_ACCESS_SETTINGS)
            startActivity(intent)
        } else {
            Log.d(TAG, "UsageStats ready")
        }

        binding.btnTestRoot.setOnClickListener {
            if (!SuperUser.rooted)
                SuperUser.tryRoot(packageCodePath)
            Log.d(TAG, if (SuperUser.rooted) "rooted" else "no root")
            Toast.makeText(
                this, if (SuperUser.rooted)
                    "Rooted" else "No root",
                Toast.LENGTH_SHORT
            ).show()
        }

        binding.btnCheckMaps.setOnClickListener {
            val blocklist = listOf("frida-agent", "frida-gadget")
            Toast.makeText(
                this, if (AntiFridaUtil.checkFridaByModuleEnum(blocklist))
                    "frida module detected" else "No frida module detected", Toast.LENGTH_SHORT
            ).show()
        }

        binding.btnCheckPort.setOnClickListener {
            Toast.makeText(
                this, if (AntiFridaUtil.checkFridaByPort(27042))
                    "frida default port 27042 detected" else "no frida default port detected",
                Toast.LENGTH_SHORT
            ).show()
        }

        binding.btnCheckProcesses.setOnClickListener {
            if (!SuperUser.rooted){
                Toast.makeText(this, "Get rooted first", Toast.LENGTH_SHORT).show()
                return@setOnClickListener
            }
            UStats.logCurrentUsageStats(this)
            val result = SuperUser.execRootCmd("ps -ef")
            Log.i(TAG, "Root cmd result (size ${result.length}): $result ")
            binding.tvStatus.text = result

            Toast.makeText(
                this, if(result.contains("frida-server"))
                    "frida-server process detected" else "no frida-server process found",
                Toast.LENGTH_SHORT
            ).show()
        }
    }
}