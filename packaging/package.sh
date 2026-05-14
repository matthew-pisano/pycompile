#!/bin/bash
set -e

mkdir -p dist

for DISTRO in fedora ubuntu; do
  echo "Building package for ${DISTRO}..."
  podman build -f packaging/Containerfile.${DISTRO} -t pycompile-build-${DISTRO} .
  CONTAINER=$(podman create pycompile-build-${DISTRO})
  RPM_PATH=$(podman run --rm pycompile-build-${DISTRO} find /build/build -maxdepth 1 -name "pycompile-*")
  podman cp "${CONTAINER}:${RPM_PATH}" dist
  podman rm "${CONTAINER}"
done
