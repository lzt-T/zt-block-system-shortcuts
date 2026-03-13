{
  "targets": [
    {
      "target_name": "zt-block-system-shortcuts",
      "sources": [
        "src/common/addon.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "conditions": [
        [
          "OS=='win'",
          {
            "sources": [
              "src/windows/win_impl.cpp"
            ],
            "libraries": [
              "-luser32"
            ],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "ExceptionHandling": 1,
                "AdditionalOptions": [ "/utf-8" ]
              }
            }
          }
        ],
        [
          "OS=='mac'",
          {
            "sources": [
              "src/macos/mac_impl.mm"
            ],
            "link_settings": {
              "libraries": [
                "-framework ApplicationServices",
                "-framework CoreFoundation",
                "-framework Carbon"
              ]
            },
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "CLANG_CXX_LIBRARY": "libc++",
              "MACOSX_DEPLOYMENT_TARGET": "10.15",
              "CLANG_ENABLE_OBJC_ARC": "YES"
            }
          }
        ]
      ]
    }
  ]
}
