#!/usr/bin/env bash
set -euo pipefail
build=$(mktemp -d)
trap 'rm -rf "$build"' EXIT
for id in first second; do
    printf '%s' "$id" > "$build/$id.txt"
    printf '<RCC><qresource prefix="/%s"><file alias="value">%s.txt</file></qresource></RCC>' "$id" "$id" > "$build/$id.qrc"
    "$(pkg-config --variable=libexecdir Qt6Core)/rcc" -name qmake_qmake_qm_files "$build/$id.qrc" -o "$build/$id.cpp"
    c++ -fPIC -shared "$build/$id.cpp" $(pkg-config --cflags --libs Qt6Core) -o "$build/$id.so"
    c++ -fPIC -shared -Wl,-Bsymbolic-functions "$build/$id.cpp" $(pkg-config --cflags --libs Qt6Core) -o "$build/$id-fixed.so"
done
cat > "$build/main.cpp" <<'CPP'
#include <QCoreApplication>
#include <QFile>
#include <dlfcn.h>
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    if (!dlopen(argv[1],RTLD_NOW|RTLD_GLOBAL) || !dlopen(argv[2],RTLD_NOW|RTLD_GLOBAL)) return 2;
    QFile first(":/first/value"), second(":/second/value");
    return first.open(QIODevice::ReadOnly) && second.open(QIODevice::ReadOnly)
        && first.readAll()=="first" && second.readAll()=="second" ? 0 : 1;
}
CPP
c++ "$build/main.cpp" $(pkg-config --cflags --libs Qt6Core) -ldl -o "$build/check"
if "$build/check" "$build/first.so" "$build/second.so"; then
    echo 'FAIL collision fixture unexpectedly loaded both resources'; exit 1
fi
echo 'PASS reproduces missing catalog resources with global same-name RCC initializers'
"$build/check" "$build/first-fixed.so" "$build/second-fixed.so"
"$build/check" "$build/second-fixed.so" "$build/first-fixed.so"
echo 'PASS locally bound initializers register both resource roots in either load order'
