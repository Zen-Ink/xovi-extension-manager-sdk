#!/usr/bin/env bash
set -euo pipefail
sdk=$(cd "$(dirname "$0")/.." && pwd)
build=$(mktemp -d)
trap 'rm -rf "$build"' EXIT
bash "$sdk/tests/i18n_resources.sh" "$build"
c++ -std=c++17 -fPIC $(pkg-config --cflags Qt6Quick Qt6Qml) "$sdk/tests/i18n_tests.cpp" "$build/translations.cpp" $(pkg-config --libs Qt6Quick Qt6Qml) -o "$build/test"
QT_QPA_PLATFORM=offscreen "$build/test"
QT_QPA_PLATFORM=offscreen "$build/test" --application-engine
