package com.xxr0ss.antifrida

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.xxr0ss.antifrida.databinding.ActivityMainBinding
import com.xxr0ss.antifrida.utils.AntiFridaUtil
import android.util.Log
import android.widget.Toast


class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding

    private val TAG = "MainActivity"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.btnCheckMaps.setOnClickListener {
            val maps = AntiFridaUtil.getProcMaps()
            binding.tvStatus.text = maps
            Log.d(TAG, "btnCheckMaps clicked")
            Toast.makeText(
                this, when (maps.contains("frida-agent")) {
                    true -> "frida agent detected"
                    false -> "No frida agent detected"
                }, Toast.LENGTH_SHORT
            ).show()
        }

    }
}