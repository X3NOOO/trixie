set -e

BUILDDIR="./build"
OUTDIR="./out"

BINNAME="$(basename $(pwd))"

BUILDIGNORE_REGEX=("*/examples/*")

CC="clang"
CFLAGS+=" -Wall -Wextra -Wpedantic -Wno-language-extension-token"
LDFLAGS+=" -lcjson -lpcre2-8 -lcurl"

windows() {
    echo Build target set to Windows.
    CFLAGS+=" --target=x86_64-pc-windows-gnu"
    LDFLAGS="-L/lib/x86_64-linux-gnu/ -L./lib/"
    BINNAME+=".exe"
}

CFILES=$(find . -name "*.c")

clean() {
    echo Cleaning...
    rm -rf $BUILDDIR
    rm -rf $OUTDIR
}

debug() {
    CFLAGS+=" -DDEBUG -ggdb"
}

build() {
    echo Building...
    mkdir -p $BUILDDIR

    for FILE in $CFILES; do
        for SKIP in ${BUILDIGNORE_REGEX[@]}; do
            if [[ $FILE =~ $SKIP ]]; then
                continue 2
            fi
        done

        $CC $CFLAGS -c $FILE -o $BUILDDIR/$(basename $FILE .c).o
    done

    mkdir -p $OUTDIR
    $CC $CFLAGS $LDFLAGS $BUILDDIR/*.o -o $OUTDIR/$BINNAME
}

if [ $# -eq 0 ]; then
    echo "Usage: $0 [function1] [function2] ..."
    exit 1
fi

for func in "$@"; do
    if [ "$(type -t $func)" == "function" ]; then
        $func
    else
        echo "Function '$func' not found."
    fi
done
