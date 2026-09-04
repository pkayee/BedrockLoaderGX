Place icon_data.h here.

Generate it from your image with:

    xxd -i icon.jpg > src/res/icon_data.h

then edit the top of the generated file to rename the arrays. xxd names them
after the input path, so you will get something like _path_to_icon_jpg[] --
rename to:

    static const unsigned char icon_png[] = { ... };
    static const unsigned int  icon_png_len = 7034;

Until this file exists, comment out the #include "res/icon_data.h" line and the
writeIfAbsent(PATH_ICON, ...) call in src/libraries/filesystem.cpp.
