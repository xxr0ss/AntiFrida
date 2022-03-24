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
            Toast.makeText(
                this, when (AntiFridaUtil.checkFridaByModuleEnum("frida-agent")) {
                    true -> "frida agent detected"
                    false -> "No frida agent detected"
                }, Toast.LENGTH_SHORT
            ).show()
        }

        binding.btnCheckPort.setOnClickListener {
            Toast.makeText(
                this, when(AntiFridaUtil.checkFridaByPort(27042)) {
                    true -> "frida default port 27042 detected"
                    false -> "no frida default port detected"
                }, Toast.LENGTH_SHORT
            ).show()
        }

    }
}