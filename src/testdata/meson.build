deps = [json_dep]

exe = executable('testcases', [vcs_dep ,
'src/main.cpp', 
src_wh, 
],
link_with: libsecp256k1, 
  include_directories: [ include_glaze, include_ftxui, include_secp256k1,include_wh, include_httplib,  './src', include_tl],
  cpp_args: ['-DDISABLE_LIBUV', '-DUNICODE', '-D_UNICODE'], # add unicode
  dependencies: deps,
  install : true)

