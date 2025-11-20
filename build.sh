#!/bin/sh

# CONSTANT DEFINTIONS #########################################################

BUILD_DIR="build"

# Define all .c files to compile in one place here
SOURCES=$(cat <<EOF
src/core.c
src/cube.c
src/input.c
src/main.c
EOF
)

LINUX="linux"
MACOS="macos"
WINDOWS="windows"
WEB="web"

# FUNCTIONS ###################################################################

help() {
    echo "Please specify a valid target platform:"
    echo "  $0 $LINUX"
    echo "  $0 $MACOS"
    echo "  $0 $WINDOWS"
    echo "  $0 $WEB"
    exit 1
}

help_web() {
    echo "Please source emsdk before trying to build for $WEB"
    echo ". \$HOME/emsdk/emsdk_env.sh"
    exit 1
}

# DIRECTORY SETUP #############################################################

if [ -z $1 ]; then
    help
fi

PLATFORM=$1

# Shift so that we can pass extra flags easier using $@
shift

# Determine if supplied platform is valid and ensure build directory exists
case $PLATFORM in
    $LINUX | $MACOS | $WINDOWS | $WEB)
        mkdir -p $BUILD_DIR

        TARGET_DIR="$BUILD_DIR/$PLATFORM"
        mkdir -p "$TARGET_DIR"
        ;;

    *)
        echo "[ FAILED ] Invalid platform: $PLATFORM"
        usage
        ;;
esac

# COMPILATION #################################################################

case $PLATFORM in
    $LINUX)
        # https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
        cc $SOURCES -DPLATFORM_LINUX \
            -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 \
            -Wall \
            "$@" \
            -o $TARGET_DIR/game
        ;;
    $MACOS)
        # https://github.com/raysan5/raylib/wiki/Working-on-macOS
        eval cc $SOURCES -DPLATFORM_MACOS \
            -framework IOKit -framework Cocoa -framework OpenGL \
            $(pkg-config --libs --cflags raylib) \
            -Wall \
            "$@" \
            -o $TARGET_DIR/game
        ;;
    $WINDOWS)
        # https://github.com/raysan5/raylib/wiki/Working-on-Windows
        gcc $SOURCES -DPLATFORM_WINDOWS \
            -lraylib -lgdi32 -lwinmm \
            -Wall \
            "$@" \
            -o $TARGET_DIR/game.exe
        ;;
    $WEB)
        if [ -z "$EMSDK" ]; then
            help_web
        fi

        # https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)
        # Also define WEB so code can run specific web logic
        emcc -o $TARGET_DIR/index.html \
            $SOURCES \
            -Os -Wall \
            $HOME/raylib/src/web/libraylib.web.a \
            -I. -I$HOME/raylib/src -L. -L$HOME/raylib/src/web \
            -s USE_GLFW=3 \
            -s ASYNCIFY \
            --shell-file $HOME/raylib/src/minshell.html \
            --preload-file src/data \
            -s TOTAL_STACK=64MB \
            -s INITIAL_MEMORY=128MB \
            -DPLATFORM_WEB \
            "$@"
        ;;
esac

# Exit the script if the last command, compilation, was unsuccessful
if [ $? -ne 0 ]; then
    echo "[ FAILED ] Game compilation was unsuccessful"
    exit 1
fi

# RUN EXECUTABLE ##############################################################

case $PLATFORM in
    $LINUX)
        $TARGET_DIR/game
        ;;
    $MACOS)
        $TARGET_DIR/game
        ;;
    $WINDOWS)
        $TARGET_DIR/game.exe
        ;;
    $WEB)
        emrun $TARGET_DIR/index.html
        ;;
esac
