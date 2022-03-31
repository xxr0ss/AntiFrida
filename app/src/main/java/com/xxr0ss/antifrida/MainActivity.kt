package com.xxr0ss.antifrida

import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.AdapterView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.xxr0ss.antifrida.databinding.ActivityMainBinding
import com.xxr0ss.antifrida.utils.AntiFridaUtil
import com.xxr0ss.antifrida.utils.ReadVia
import com.xxr0ss.antifrida.utils.SuperUser


class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding

    private val TAG = "MainActivity"

    private var posReadVia: Int = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        SuperUser.tryRoot(packageCodePath)
        binding.rootStatus.text = "rooted: ${SuperUser.rooted.toString()}"

        binding.spinnerVia.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
            override fun onItemSelected(parent: AdapterView<*>?, view: View?, pos: Int, id: Long) {
                posReadVia = pos
                Log.d(TAG, "onItemSelected: $pos $id")
            }

            override fun onNothingSelected(parent: AdapterView<*>?) {
                Log.d(TAG, "onNothingSelected: null")
            }
        }

        binding.btnCheckMaps.setOnClickListener {
            // put possible frida module names here
            val blocklist = listOf("frida-agent", "frida-gadget")
            Toast.makeText(
                this,
                (if (AntiFridaUtil.checkFridaByProcMaps(blocklist, ReadVia.fromInt(posReadVia)))
                    "frida module detected" else "No frida module detected")
                        + " via ${ReadVia.fromInt(posReadVia).name}",
                Toast.LENGTH_SHORT
            ).show()
            binding.textStatus.editableText.clear()
            binding.textStatus.editableText.append(
                when (AntiFridaUtil.maps_file_content) {
                    null -> "no maps file data"
                    else -> "maps file:\n ${AntiFridaUtil.maps_file_content}"
                }
            )
        }

        binding.btnCheckPort.setOnClickListener {
            Toast.makeText(
                this, if (AntiFridaUtil.checkFridaByPort(27042))
                    "frida default port 27042 detected" else "no frida default port detected",
                Toast.LENGTH_SHORT
            ).show()
        }

        binding.btnCheckProcesses.setOnClickListener {
            if (!SuperUser.rooted) {
                SuperUser.tryRoot(packageCodePath)
                if (!SuperUser.rooted)
                    return@setOnClickListener
            }
            val result = SuperUser.execRootCmd("ps -ef")
            Log.i(TAG, "Root cmd result (size ${result.length}): $result ")
            binding.textStatus.text.clear()
            binding.textStatus.text.append(result)

            Toast.makeText(
                this, if (result.contains("frida-server"))
                    "frida-server process detected" else "no frida-server process found",
                Toast.LENGTH_SHORT
            ).show()
        }

        binding.btnScanModules.setOnClickListener {
            val useMySyscalls = binding.switchUseMySyscalls.isChecked
            Toast.makeText(
                this, if (AntiFridaUtil.scanModulesForSignature("frida:rpc", useMySyscalls))
                    "frida signature found" else "no frida signature found", Toast.LENGTH_SHORT
            ).show()
        }
    }
}