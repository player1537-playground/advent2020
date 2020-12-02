#!/usr/bin/env bash

root=$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)
build=${root:?}/build
stage=${root:?}/stage

go-cmake() {
	cmake -H"${root:?}" -B"${build:?}" \
		-DCMAKE_INSTALL_PREFIX:PATH="${stage:?}" \
		"$@"
}

go-make() {
	make -C"${build:?}" \
		VERBOSE=1 \
		"$@"
}

go-"$@"
